#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include "websocket.h"
#include "integration_utils.h"

#include <thread>
#include <atomic>
#include <string>
#include <sstream>

// The [.] means we don't run this by default.
TEST_CASE_METHOD(IntegrationTestFixture, "Run a poll", "[.][integration]")
{
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

	// This handles the patch states done.
	REQUIRE(updateData["health"] == 1000.0);
	REQUIRE(updateData["exp"] == 999);
	REQUIRE(updateData["in_combat"] == true);

	// Show that these request are reflected in the state the extension frontend can see.
	nlohmann::json resp;
	Request("GET", "channel_state", nullptr, &resp);

	REQUIRE(resp["health"] == 1000.0);
	REQUIRE(resp["exp"] == 999);
	REQUIRE(resp["in_combat"] == true);

	sdk.ClearState(gamelink::StateTarget::Channel);
	Sleep();
	REQUIRE(updateData == nlohmann::json::object());
}

TEST_CASE_METHOD(IntegrationTestFixture, "Config operations", "[.][integration]")
{
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

TEST_CASE_METHOD(IntegrationTestFixture, "Disconnect into a reconnection works", "[.][integration][t]")
{
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
#endif

