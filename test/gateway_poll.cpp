#include "catch2/catch.hpp"
#include "util.h"

#include "gateway.h"
#include "gateway_c.h"

TEST_CASE("Run Poll in Gateway", "[gateway][poll]")
{
	gateway::SDK sdk("This is a game");

	gateway::PollConfiguration config;
	config.Prompt = "Pizza toppings";
	config.Options = {
		"Pepperoni",
		"Cheese"
	};

	size_t finishCalls = 0;
	config.OnUpdate = [&](const gateway::PollUpdate& upd)
	{
		REQUIRE(upd.IsFinal);
		finishCalls++;
	};

	sdk.StartPoll(config);

	REQUIRE(sdk.HasPayloads());
	const char* finishMessage = R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"count": 4,
			"mean": 0.0,
			"sum": 0.0,

			"results": [1, 3],
			"poll": {
				"poll_id": "default",
				"prompt": "Pizza toppings",
				"options": ["Pepperoni", "Cheese"],
				"status": "expired"
			}
		}
	})";

	sdk.ReceiveMessage(finishMessage, strlen(finishMessage));
	REQUIRE(finishCalls == 1);
}

TEST_CASE("Run Poll in Gateway with duration", "[gateway][poll]")
{
	gateway::SDK sdk("This is a game");

	gateway::PollConfiguration config;
	config.Prompt = "Pizza toppings";
	config.Options = {
		"Pepperoni",
		"Cheese"
	};

	// Use std::map<std::string, std::string> to demostrate.
	// In real user-code, this should be a type marked up with the MUXY_SERIALIZE macros.
	config.UserData = std::map<std::string, std::string>{
		{"SauceType", "Red"}
	};

	config.Duration = 20;
	sdk.StartPoll(config);

	REQUIRE(sdk.HasPayloads());
	// Actually hitting this is difficult, since the SDK
	// has waits involved.
	sdk.ForeachPayload([](const gateway::Payload*){});

	const char* msg = R"({
		"meta": {
			"action": "nothing",
			"request_id": 1
		}
	})";
	sdk.ReceiveMessage(msg, strlen(msg));

	const char* msg2 = R"({
		"meta": {
			"action": "nothing",
			"request_id": 2
		}
	})";
	sdk.ReceiveMessage(msg2, strlen(msg2));

	const char* expected =  R"({
		"action": "create",
		"params": {
			"request_id": 65535,
			"target": "poll"
		},
		"data": {
			"poll_id": "default",
			"prompt": "Pizza toppings",
			"options": ["Pepperoni", "Cheese"],
			"config": {
				"disabled": false,
				"distinctOptionsPerUser": 1,
				"endsAt": 0,
				"endsIn": 20,
				"startsAt": 0,
				"startsIn": 0,
				"totalVotesPerUser": 1,
				"userIDVoting": true,
				"votesPerOption": 1
			},
			"user_data": {
				"SauceType": "Red"
			}
		}
	})";

	size_t count = 0;
	sdk.ForeachPayload([=, &count](const gateway::Payload* payload) {
		if (count == 1)
		{
			ConstrainedString str(reinterpret_cast<const char *>(payload->GetData()));
			REQUIRE(JSONEquals(str, ConstrainedString(expected)));
		}
		count++;
	});

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("Run Poll in Gateway, C", "[gateway][poll][c]")
{
	MGW_SDK sdk = MGW_MakeSDK("gameid");

	MGW_PollConfiguration cfg;
	cfg.Prompt = "Pizza toppings";

	const char* options[] = {
		"Pepperoni",
		"Cheese"
	};

	cfg.Options = options;
	cfg.OptionsCount = 2;

	size_t finishCalls = 0;

	cfg.User = &finishCalls;
	cfg.OnUpdate = [](void* user, MGW_PollUpdate* upd)
	{
		REQUIRE(upd->IsFinal);
		(*reinterpret_cast<size_t*>(user))++;
	};

	MGW_SDK_StartPoll(sdk, cfg);

	REQUIRE(MGW_SDK_HasPayloads(sdk));
	const char* finishMessage = R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"count": 4,
			"mean": 0.0,
			"sum": 0.0,

			"results": [1, 3],
			"poll": {
				"poll_id": "default",
				"prompt": "Pizza toppings",
				"options": ["Pepperoni", "Cheese"],
				"status": "expired"
			}
		}
	})";

	MGW_SDK_ReceiveMessage(sdk, finishMessage, strlen(finishMessage));
	REQUIRE(finishCalls == 1);
}