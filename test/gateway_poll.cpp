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