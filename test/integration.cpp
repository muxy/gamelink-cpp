#include "integration_utils.h"

#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include "websocket.h"

#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <ctime>

// The [.] means we don't run this by default.
TEST_CASE_METHOD(IntegrationTestFixture, "Run a poll", "[.][integration]")
{
	Connect();
	gamelink::PollConfiguration cfg;

	std::atomic<bool> gotUpdate;
	std::atomic<bool> gotFinish;

	// Start the poll
	sdk.RunPoll("what-number", "What number is best?", cfg, {
		"One", "Two", "Three", "Four", "Five",
	},
	// Update
	[&](const gamelink::schema::PollUpdateResponse& resp)
	{
		// Got an update
		gotUpdate.store(true);
	},
	// Finish
	[&](const gamelink::schema::PollUpdateResponse& resp)
	{
		gotFinish.store(true);
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Cast a vote
	nlohmann::json vote;
	vote["value"] = 2;

	nlohmann::json resp;
	Request("POST", "vote?id=what-number", &vote, &resp);

	while (!gotUpdate)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	REQUIRE(gotUpdate);

	// Now stop the poll, should get a finish() call
	sdk.StopPoll("what-number");
	while (!gotFinish)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	REQUIRE(gotFinish);
}

TEST_CASE_METHOD(IntegrationTestFixture, "State operations", "[.][integration]")
{
	Connect();

	nlohmann::json obj;
	obj["hello"] = "world";

	int calls = 0;
	nlohmann::json updateData;

	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Channel);
	sdk.OnStateUpdate().Add([&calls, &updateData](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& update) mutable
	{
		calls++;

		// This should be "channel"
		REQUIRE(update.meta.target == gamelink::string("channel"));

		updateData = update.data.state;
	});

	sdk.SetState(gamelink::StateTarget::Channel, obj);
	Sleep();
	REQUIRE(calls == 1);

	sdk.UpdateStateWithDouble(gamelink::StateTarget::Channel, gamelink::Operation::Add,
		gamelink::string("/health"), 1000.0);
	sdk.UpdateStateWithInteger(gamelink::StateTarget::Channel, gamelink::Operation::Add,
		gamelink::string("/exp"), 999);
	sdk.UpdateStateWithBoolean(gamelink::StateTarget::Channel, gamelink::Operation::Add,
		gamelink::string("/in_combat"), true);

	Sleep();

	// Show that these request are reflected in the state the extension frontend can see.
	nlohmann::json resp;
	Request("GET", "channel_state", nullptr, &resp);

	// This handles the patch states done.
	REQUIRE(updateData["health"] == 1000.0);
	REQUIRE(updateData["exp"] == 999);
	REQUIRE(updateData["in_combat"] == true);


	REQUIRE(resp["health"] == 1000.0);
	REQUIRE(resp["exp"] == 999);
	REQUIRE(resp["in_combat"] == true);

	sdk.ClearState(gamelink::StateTarget::Channel);
	Sleep();
	REQUIRE(updateData == nlohmann::json::object());
}

TEST_CASE_METHOD(IntegrationTestFixture, "Config operations", "[.][integration]")
{
	Connect();

	nlohmann::json obj;
	obj["hello"] = "world";

	nlohmann::json result;
	uint32_t calls = 0;

	// This fails, cannot subscribe to Combined. Should be documented.
	// sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Combined);
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Channel);
	sdk.OnConfigUpdate().Add([&](const gamelink::schema::ConfigUpdateResponse& resp)
	{
		calls++;
		result = resp.data.config;
		REQUIRE(resp.data.topicId == gamelink::string("channel"));
	});
	sdk.SetChannelConfig(obj);
	Sleep();
	REQUIRE(calls == 1);


	obj["foo"] = "bar";
	nlohmann::json resp;
	Request("POST", "config/channel", &obj, &resp);
	Sleep();

	REQUIRE(calls == 2);
	REQUIRE(result["foo"] == "bar");
	REQUIRE(result["hello"] == "world");
}

TEST_CASE_METHOD(IntegrationTestFixture, "Poll lifetime management", "[.][integration]")
{
	Connect();

	gamelink::PollConfiguration cfg;
	cfg.endsAt = UnixNow() + 5;
	cfg.startsAt = UnixNow() + 3;

	std::vector<gamelink::schema::PollUpdateResponse> responses;

	gamelink::RequestId id = sdk.SubscribeToPoll("timed-poll");
	sdk.OnPollUpdate().Add([&](const gamelink::schema::PollUpdateResponse& resp)
	{
		responses.push_back(resp);
	});

	sdk.WaitForResponse(id);
	sdk.CreatePollWithConfiguration("timed-poll", "How fast can you react", cfg, {
		"yes"
	});

	Sleep(6);

	// No votes cast, expect an 'active' and 'expired' call
	REQUIRE(responses.size() == 2);
	REQUIRE(responses[0].data.poll.status == "active");
	REQUIRE(responses[0].data.count == 0);
	REQUIRE(responses[1].data.poll.status == "expired");
	REQUIRE(responses[1].data.count == 0);
}

TEST_CASE_METHOD(IntegrationTestFixture, "Poll lifetime management, with multiple stop calls", "[.][integration]")
{
	Connect();

	gamelink::PollConfiguration cfg;
	cfg.endsAt = UnixNow() + 100;
	cfg.startsAt = UnixNow() + 3;

	std::vector<gamelink::schema::PollUpdateResponse> responses;

	gamelink::RequestId id = sdk.SubscribeToPoll("timed-poll");
	sdk.OnPollUpdate().Add([&](const gamelink::schema::PollUpdateResponse& resp)
	{
		responses.push_back(resp);
	});

	sdk.WaitForResponse(id);
	sdk.CreatePollWithConfiguration("timed-poll", "How fast can you react", cfg, {
		"yes"
	});

	Sleep(6);

	// No votes cast, expect an 'active' and 'expired' call
	REQUIRE(responses.size() == 1);
	REQUIRE(responses[0].data.poll.status == "active");
	REQUIRE(responses[0].data.count == 0);

	sdk.StopPoll("timed-poll");
	Sleep();
	sdk.StopPoll("timed-poll");
	Sleep();
	sdk.StopPoll("timed-poll");
	Sleep();
	sdk.StopPoll("timed-poll");
	Sleep();
	sdk.StopPoll("timed-poll");
	Sleep();

	REQUIRE(responses.size() == 2);
	REQUIRE(responses[1].data.poll.status == "expired");
	REQUIRE(responses[1].data.count == 0);
}

TEST_CASE_METHOD(IntegrationTestFixture, "Disconnect into a reconnection works", "[.][integration]")
{
	Connect();

	ForceDisconnect();
	Sleep();

	nlohmann::json value;
	value["reconnected"] = true;
	sdk.SetState(gamelink::StateTarget::Channel, value);
	sdk.GetState(gamelink::StateTarget::Channel, [](const gamelink::schema::GetStateResponse<nlohmann::json>& resp)
	{
		REQUIRE(resp.data.state["reconnected"] == true);
	});

	// After a handle reconnect, stuff works, even if there were API calls between the reconnect and disconnect.
	sdk.HandleReconnect();
	Reconnect();

	Sleep();
}

TEST_CASE_METHOD(IntegrationTestFixture, "GetDrops gets a response", "[.][integration]")
{
	Connect();

	int calls = 0;
	sdk.GetDrops("*", [&](const gamelink::schema::GetDropsResponse& drops)
	{
		calls++;
	});

	Sleep();
	REQUIRE(calls == 1);
}

TEST_CASE_METHOD(IntegrationTestFixture, "Transactions Support", "[.][integration][t]")
{
	Connect();

	int calls = 0;
	sdk.SubscribeToAllPurchases();
	sdk.OnTransaction().Add([&](const gamelink::schema::TransactionResponse& resp)
	{
		if (calls == 0)
		{
			REQUIRE(resp.data.sku == "muxy-bits-50");
		}
		else
		{
			REQUIRE(resp.data.sku == "costs-ten");
			REQUIRE(resp.data.currency == "coins");
		}

		calls++;
	});

	// Forge a transaction receipt. This does not work on production.
	if (signature.empty())
	{
		return;
	}

	// JWT header
	std::string header = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";

	std::time_t now;
	std::time(&now);

	std::tm* gmt;
	gmt = std::gmtime(&now);

	char dateBuffer[256];
	std::strftime(dateBuffer, 256, "%F %T.000000000 +0000 UTC", gmt);

	// Parse the passed in JWT and pull out the claims object.
	std::istringstream splitter(jwt);
	std::string output;

	// Ignore header.
	getline(splitter, output, '.');

	// Get body.
	getline(splitter, output, '.');
	nlohmann::json inputClaims = nlohmann::json::parse(Decode64(output));
	std::string userId = inputClaims["user_id"].get<std::string>();

	// JWT claims
	char claimsBuffer[1024];
	snprintf(claimsBuffer, 1024, R"===({
		"exp": %lld,
		"topic": "bits_transaction_receipt",
		"data": {
			"transactionId": "fake-transaction-%lld",
			"time": "%s",
			"userId": "%s",
			"product": {
				"domainId": "twitch.ext.%s",
				"sku": "muxy-bits-50",
				"cost": {
					"amount": 50,
					"type": "bits"
				}
			}
		}
	})===", UnixNow() + 1000, UnixNow() + 1000, dateBuffer, userId.c_str(), client.c_str());

	std::string claims = Base64(claimsBuffer);
	std::string receipt = header + "." + claims + "." + signature;

	nlohmann::json body;
	body["transactionReceipt"] = receipt;
	body["displayName"] = "somebody";

	int resp = Request("POST", "bits/transactions", &body, nullptr);
	REQUIRE(resp == 201);

	Sleep();
	REQUIRE(calls == 1);

	// The above operation has credited the user with at least 50 coins, use them to generate
	// coin-based SKUs
	sdk.SetChannelConfig(nlohmann::json::parse(R"===({
		"skus": {
			"costs-ten": {
				"price": 10,
				"displayName": "This costs ten coins"
			}
		}
	})==="));
	Sleep();

	nlohmann::json purchaseBody = nlohmann::json::parse(R"===({
		"sku": "costs-ten",
		"price": 10
	})===");

	for (uint32_t i = 0; i < 5; ++i)
	{
		resp = Request("POST", "coins/purchase", &purchaseBody, nullptr);
		REQUIRE(resp == 200);
	}

	Sleep();
	REQUIRE(calls == 6);
}

TEST_CASE_METHOD(IntegrationTestFixture, "Transactions Support through gateway", "[.][integration][t]")
{
	Connect();

	size_t bitsCalls = 0;
	gateway.OnBitsUsed([&](const gateway::BitsUsed& used)
	{
		// This should get 1 call, to purchase 50 coins.
		REQUIRE(used.SKU == "muxy-bits-50");
		REQUIRE(used.Bits == 50);

		bitsCalls++;
	});

	size_t coinCalls = 0;
	gateway.OnActionUsed([&](const gateway::ActionUsed& used)
	{
		// This should get 5 calls, which are all coins
		REQUIRE(used.ActionID == "costs-ten");
		REQUIRE(used.Cost == 10);

		coinCalls++;
	});

	// Forge a transaction receipt. This does not work on production.
	if (signature.empty())
	{
		return;
	}

	// JWT header
	std::string header = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";

	std::time_t now;
	std::time(&now);

	std::tm* gmt;
	gmt = std::gmtime(&now);

	char dateBuffer[256];
	std::strftime(dateBuffer, 256, "%F %T.000000000 +0000 UTC", gmt);

	// Parse the passed in JWT and pull out the claims object.
	std::istringstream splitter(jwt);
	std::string output;

	// Ignore header.
	getline(splitter, output, '.');

	// Get body.
	getline(splitter, output, '.');
	nlohmann::json inputClaims = nlohmann::json::parse(Decode64(output));
	std::string userId = inputClaims["user_id"].get<std::string>();

	// JWT claims
	char claimsBuffer[1024];
	snprintf(claimsBuffer, 1024, R"===({
		"exp": %lld,
		"topic": "bits_transaction_receipt",
		"data": {
			"transactionId": "fake-transaction-%lld",
			"time": "%s",
			"userId": "%s",
			"product": {
				"domainId": "twitch.ext.%s",
				"sku": "muxy-bits-50",
				"cost": {
					"amount": 50,
					"type": "bits"
				}
			}
		}
	})===", UnixNow() + 1000, UnixNow() + 1000, dateBuffer, userId.c_str(), client.c_str());

	std::string claims = Base64(claimsBuffer);
	std::string receipt = header + "." + claims + "." + signature;

	nlohmann::json body;
	body["transactionReceipt"] = receipt;
	body["displayName"] = "somebody";

	int resp = Request("POST", "bits/transactions", &body, nullptr);
	REQUIRE(resp == 201);

	Sleep();
	REQUIRE(bitsCalls == 1);

	// The above operation has credited the user with at least 50 coins, use them to generate
	// coin-based SKUs
	sdk.SetChannelConfig(nlohmann::json::parse(R"===({
		"skus": {
			"costs-ten": {
				"price": 10,
				"displayName": "This costs ten coins"
			}
		}
	})==="));
	Sleep();

	nlohmann::json purchaseBody = nlohmann::json::parse(R"===({
		"sku": "costs-ten",
		"price": 10
	})===");

	for (uint32_t i = 0; i < 5; ++i)
	{
		resp = Request("POST", "coins/purchase", &purchaseBody, nullptr);
		REQUIRE(resp == 200);
	}

	Sleep();
	REQUIRE(bitsCalls == 1);
	REQUIRE(coinCalls == 5);
}

TEST_CASE_METHOD(IntegrationTestFixture, "Datastream operations", "[.][integration]")
{
	Connect();

	int events = 0;
	sdk.OnDatastreamUpdate().Add([&](const gamelink::schema::DatastreamUpdate& update)
	{
		for (size_t i = 0; i < update.data.events.size(); ++i)
		{
			const gamelink::schema::DatastreamEvent& e = update.data.events[i];
			REQUIRE(e.event["click"]["x"] == 100);
			REQUIRE(e.event["click"]["y"] == 120 + events);
			events++;
		}
	});
	sdk.SubscribeToDatastream();

	for (int i = 0; i < 6; i++)
	{
		nlohmann::json datastreamValue;
		datastreamValue["click"]["x"] = 100;
		datastreamValue["click"]["y"] = 120 + i;

		Request("POST", "datastream", &datastreamValue, nullptr);
	}

	Sleep();
	REQUIRE(events == 6);
}

TEST_CASE_METHOD(IntegrationTestFixture, "Matches and polls", "[.][integration]")
{
	Connect();

	sdk.CreateMatch("my-cool-match");

	sdk.AddChannelsToMatch("my-cool-match", {
		"26052853", "89319907", "89368629", "89368745", "124708734"
	});

	size_t updateCalls = 0;
	auto update = [&updateCalls](const gamelink::schema::MatchPollUpdate& update)
	{
		updateCalls++;
	};

	size_t finishCalls = 0;
	auto finish = [&finishCalls](const gamelink::schema::MatchPollUpdate& finish)
	{
		finishCalls++;
	};

	gamelink::PollConfiguration config;
	config.startsIn = 0;
	config.endsIn = 5;

	gamelink::RequestId waiter = sdk.RunMatchPoll("my-cool-match", "what-to-eat",
		"How many pizzas should I buy?",
		config,
		{ "one", "two", "twenty" },
		update,
		finish
	);

	Sleep(1);

	nlohmann::json voteValue;
	voteValue["value"] = 1;
	nlohmann::json unused;

	Request("POST", "vote?id=what-to-eat", &voteValue, &unused);
	Sleep(6);

	REQUIRE(updateCalls > 0);
	REQUIRE(finishCalls == 1);
}

TEST_CASE_METHOD(IntegrationTestFixture, "Client IP access", "[.][integration]")
{
	int code;

	nlohmann::json ip_resp;
	code = RequestURL("GET", "https://api.ipify.org?format=json", &ip_resp);
	REQUIRE(code == 200);

	// Server changes last octet to 0 for security
	unsigned short octet1, octet2, octet3;

	std::string full_ip = ip_resp["ip"];
	sscanf(full_ip.c_str(), "%hu.%hu.%hu.%*hu", &octet1, &octet2, &octet3);

	char ip[64];
	snprintf(ip, 64, "%d.%d.%d.0", octet1, octet2, octet3);

	Connect();
	nlohmann::json info_resp;
	code = Request("GET", "user_info", nullptr, &info_resp);
	REQUIRE(code == 200);

	REQUIRE(ip == info_resp["ip_address"]);
}

#endif
