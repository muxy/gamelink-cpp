#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include "websocket.h"
#include "integration_http.h"

#include <thread>
#include <atomic>
#include <string>
#include <sstream>

struct Empty
{
	MUXY_GAMELINK_SERIALIZE_INTRUSIVE_0(Empty);
};

struct GamelinkTokenResponse
{
	std::string token;

	MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GamelinkTokenResponse, "token", token);
};

struct IntegrationTestFixture
{
	IntegrationTestFixture()
		: done(false)
	{
		const char * jwtenv = std::getenv("MUXY_INTEGRATION_JWT");
		std::string jwt;
		if (jwtenv)
		{
			jwt = jwtenv;
		}
		REQUIRE(jwt.size());

		const char * clientEnv = std::getenv("MUXY_INTEGRATION_ID");
		std::string client;
		if (clientEnv)
		{
			client = clientEnv;
		}


		curl_global_init(CURL_GLOBAL_ALL);
		gamelink::string url = gamelink::WebsocketConnectionURL(client.c_str(), gamelink::ConnectionStage::Sandbox);
		connection = std::unique_ptr<WebsocketConnection>(new WebsocketConnection(url.c_str(), 80));

		connection->onMessage([&](const char* bytes, uint32_t len) {
			sdk.ReceiveMessage(bytes, len);
		});

		sdk.OnDebugMessage([](const gamelink::string& str)
		{
			std::cerr <<"!<   " << str.c_str() << "\n";
		});

		runner = std::unique_ptr<std::thread>(new std::thread([&]()
		{
			while (!done)
			{
				sdk.ForeachPayload([&](const gamelink::Payload* send) {
					std::cerr << "!>   send len=" << send->Length() << " msg=" << std::string(send->Data(), send->Data() + send->Length()) << "\n";
					connection->send(send->Data(), send->Length());
				});

				connection->run();
			}
		}));

		REQUIRE(client.size());

		std::stringstream ss;
		ss << "Authorization: " << client << " " << jwt;
		authHeader = ss.str();

		static std::string refreshToken;
		if (refreshToken.empty())
		{
			Empty empty;
			GamelinkTokenResponse resp;
			request("POST", "gamelink/token", authHeader.c_str(), &empty, &resp);

			gamelink::RequestId req = sdk.AuthenticateWithPIN(gamelink::string(client.c_str()), gamelink::string(resp.token.c_str()));
			sdk.WaitForResponse(req);
		}
		else
		{
			gamelink::RequestId req = sdk.AuthenticateWithRefreshToken(gamelink::string(client.c_str()), gamelink::string(refreshToken.c_str()));
			sdk.WaitForResponse(req);
		}

		sdk.OnAuthenticate().Add([&](const gamelink::schema::AuthenticateResponse& response) {
			const gamelink::schema::Error* err = gamelink::FirstError(response);
			if (err)
			{
				std::cout << "Couldn't authenticate. " << response.errors[0].title.c_str() << ": " << response.errors[0].detail.c_str() << "\n";
				return;
			}

			refreshToken = std::string(response.data.refresh.c_str());
		});

		while (!sdk.IsAuthenticated())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	~IntegrationTestFixture()
	{
		done.store(true);
		runner->join();
		curl_global_cleanup();
	}

	void FlushSDKMessages()
	{
		while (sdk.HasPayloads())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	gamelink::SDK sdk;
	std::string authHeader;
	std::unique_ptr<WebsocketConnection> connection;
	std::unique_ptr<std::thread> runner;

	std::atomic<bool> done;
};

// The [.] means we don't run this by default.
TEST_CASE_METHOD(IntegrationTestFixture, "Run a poll", "[.][integration]")
{
	gamelink::PollConfiguration cfg;

	std::atomic<bool> gotUpdate;
	std::atomic<bool> gotFinish;

	// Star the poll
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

	FlushSDKMessages();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Cast a vote
	nlohmann::json vote;
	vote["value"] = 2;

	nlohmann::json resp;
	request("POST", "vote?id=what-number", authHeader.c_str(), &vote, &resp);

	while (!gotUpdate)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	REQUIRE(gotUpdate);

	// Now stop the poll, should get a finish() call
	sdk.StopPoll("what-number");
	FlushSDKMessages();

	while (!gotFinish)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	REQUIRE(gotFinish);
}

TEST_CASE_METHOD(IntegrationTestFixture, "State operations", "[.][integration]")
{
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
	FlushSDKMessages();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	REQUIRE(calls == 1);

	sdk.UpdateStateWithDouble(gamelink::StateTarget::Channel, gamelink::Operation::Add,
		gamelink::string("/health"), 1000.0);
	FlushSDKMessages();

	sdk.UpdateStateWithInteger(gamelink::StateTarget::Channel, gamelink::Operation::Add,
		gamelink::string("/exp"), 999);
	FlushSDKMessages();

	sdk.UpdateStateWithBoolean(gamelink::StateTarget::Channel, gamelink::Operation::Add,
		gamelink::string("/in_combat"), true);
	FlushSDKMessages();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// This handles the patch states done.
	REQUIRE(updateData["health"] == 1000.0);
	REQUIRE(updateData["exp"] == 999);
	REQUIRE(updateData["in_combat"] == true);

	sdk.ClearState(gamelink::StateTarget::Channel);
	FlushSDKMessages();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	REQUIRE(updateData == nlohmann::json::object());
}

TEST_CASE_METHOD(IntegrationTestFixture, "Config operations", "[.][integration][t]")
{
	nlohmann::json obj;
	obj["hello"] = "world";

	// This fails, cannot subscribe to Combined. Should be documented.
	// sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Combined);
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Channel);
	sdk.OnConfigUpdate().Add([&](const gamelink::schema::ConfigUpdateResponse& resp)
	{
		REQUIRE(resp.data.topicId == gamelink::string("channel"));
	});

	sdk.SetChannelConfig(obj);

	FlushSDKMessages();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

#endif

