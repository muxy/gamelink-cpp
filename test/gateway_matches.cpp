#include "catch2/catch.hpp"
#include "util.h"

#include "gateway.h"


TEST_CASE("Create a gamechanger", "[gateway][matches]")
{
	gateway::SDK sdk("This is a game");

	gateway::GamechangerMatchPollConfiguration cfg;
	cfg.Duration = 120;
	cfg.Prompt = "Empower my snacks";
	cfg.Options = {
		"Empower",
	};

	cfg.Mode = gateway::PollMode::Chaos;
	cfg.UserData.Name = "Delivery Budget";
	cfg.UserData.Tiers = {
		gateway::GamechangerTier{
			.IncrementalText = "Spend $1 more",
			.IncrementalValue = 1,

			.EffectText = "Spend $10 more",
			.EffectValue = 10,

			.TierDuration = 120,
			.TierThreshold = 0
		},

		gateway::GamechangerTier{
			.EffectText = "Spend $100 more",
			.EffectValue = 100,

			.IncrementalText = "Spend $10 more",
			.IncrementalValue = 10,

			.TierDuration = 60,
			.TierThreshold = 200
		},

		gateway::GamechangerTier{
			.EffectText = "Spend $200 more",
			.EffectValue = 200,

			.IncrementalText = "Spend $20 more",
			.IncrementalValue = 20,

			.TierDuration = 30,
			.TierThreshold = 400
		},

		gateway::GamechangerTier{
			.EffectText = "Go for broke!!",
			.EffectValue = 1000,

			.IncrementalText = "Please no more :(",
			.IncrementalValue = 5,

			.TierDuration = 15,
			.TierThreshold = 1000
		},
	};

	sdk.RunMatchPoll("my-cool-match", cfg);
	REQUIRE(sdk.HasPayloads());

	validateSinglePayload(sdk, R"({
		"action": "delete",
		"params":{
			"request_id": 65535,
			"target": "match_poll"
		},
		"data": {
			"id": "my-cool-match",
			"poll_id": "default"
		}
	})");

	const char* msg = R"({
		"meta": {
			"action": "nothing",
			"request_id": 1
		}
	})";
	sdk.ReceiveMessage(msg, strlen(msg));

	validateSinglePayload(sdk, R"({
		"action": "subscribe",
		"data": {
			"topic_id": "my-cool-match"
		},
		"params": {
			"request_id": 65535,
			"target": "match_poll"
		}
	})");

	const char* msg2 = R"({
		"meta": {
			"action": "nothing",
			"request_id": 2
		}
	})";
	sdk.ReceiveMessage(msg2, strlen(msg2));

	const char* expected =  R"({
		"action": "create",
		"data": {
			"id": "my-cool-match",
			"poll": {
			"config": {
				"disabled": false,
				"distinctOptionsPerUser": 258,
				"endsAt": 0,
				"endsIn": 120,
				"startsAt": 0,
				"startsIn": 0,
				"totalVotesPerUser": 1024,
				"userIDVoting": true,
				"votesPerOption": 1024
			},
			"options": [
				"Empower"
			],
			"poll_id": "default",
			"prompt": "Empower my snacks",
			"user_data": {
				"name": "Delivery Budget",
				"type": "gamechanger",
				"tiers": [
				{
					"effect_text": "Spend $10 more",
					"effect_value": 10.0,
					"incremental_effect_text": "Spend $1 more",
					"incremental_effect_value": 1.0,
					"tier_duration": 120,
					"tier_threshold": 0
				},
				{
					"effect_text": "Spend $100 more",
					"effect_value": 100.0,
					"incremental_effect_text": "Spend $10 more",
					"incremental_effect_value": 10.0,
					"tier_duration": 60,
					"tier_threshold": 200
				},
				{
					"effect_text": "Spend $200 more",
					"effect_value": 200.0,
					"incremental_effect_text": "Spend $20 more",
					"incremental_effect_value": 20.0,
					"tier_duration": 30,
					"tier_threshold": 400
				},
				{
					"effect_text": "Go for broke!!",
					"effect_value": 1000.0,
					"incremental_effect_text": "Please no more :(",
					"incremental_effect_value": 5.0,
					"tier_duration": 15,
					"tier_threshold": 1000
				}
				]
			}
			}
		},
		"params": {
			"request_id": 65535,
			"target": "match_poll"
		}
	})";

	validateSinglePayload(sdk, expected);
}
