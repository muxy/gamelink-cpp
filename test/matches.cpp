#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("Matches operations", "[matches]")
{
	gamelink::SDK sdk;

	sdk.CreateMatch("my-cool-match");
	validateSinglePayload(sdk,  R"({
		"action": "create",
		"data": {
			"id": "my-cool-match"
		},
		"params": {
			"request_id": 65535,
			"target": "match"
		}
	})");

	sdk.AddChannelsToMatch("my-cool-match", {
		"1001", "1002", "1003",
	});
	validateSinglePayload(sdk,  R"({
		"action": "add_channels",
		"data": {
			"id": "my-cool-match",
			"channel_ids": ["1001", "1002", "1003"]
		},
		"params": {
			"request_id": 65535,
			"target": "match"
		}
	})");

	sdk.KeepMatchAlive("my-cool-match");
	validateSinglePayload(sdk,  R"({
		"action": "keepalive",
		"data": {
			"id": "my-cool-match"
		},
		"params": {
			"request_id": 65535,
			"target": "match"
		}
	})");

	sdk.RemoveChannelsFromMatch("my-cool-match", {
		"1001"
	});

	validateSinglePayload(sdk,  R"({
		"action": "remove_channels",
		"data": {
			"id": "my-cool-match",
			"channel_ids": ["1001"]
		},
		"params": {
			"request_id": 65535,
			"target": "match"
		}
	})");
}

namespace gs = gamelink::schema;
TEST_CASE("Match polls operations", "[matches]")
{
	gamelink::SDK sdk;

	sdk.CreateMatch("my-cool-match");
	validateSinglePayload(sdk,  R"({
		"action": "create",
		"data": {
			"id": "my-cool-match"
		},
		"params": {
			"request_id": 65535,
			"target": "match"
		}
	})");

	sdk.AddChannelsToMatch("my-cool-match", {
		"1001", "1002", "1003",
	});
	validateSinglePayload(sdk,  R"({
		"action": "add_channels",
		"data": {
			"id": "my-cool-match",
			"channel_ids": ["1001", "1002", "1003"]
		},
		"params": {
			"request_id": 65535,
			"target": "match"
		}
	})");

	gamelink::PollConfiguration config;
	config.disabled = false;
	config.distinctOptionsPerUser = 1;
	config.totalVotesPerUser = 1024;
	config.userIdVoting = true;
	config.startsAt = 10;
	config.endsAt = 20;
	config.votesPerOption = 1024;

	int updateCalls = 0;
	std::function<void (const gs::MatchPollUpdate&)> update = [&updateCalls](const gs::MatchPollUpdate& resp)
	{
		REQUIRE(resp.data.pollId == "test-poll");
		REQUIRE(resp.data.matchId == "my-cool-match");

		REQUIRE(resp.data.results.count("1001") == 1);
		REQUIRE(resp.data.results.find("1001")->second.results[0] == 1);
		REQUIRE(resp.data.results.find("1001")->second.results[1] == 3);
		REQUIRE(resp.data.results.count("1002") == 0);
		REQUIRE(resp.data.results.count("1003") == 0);
		updateCalls++;
	};

	int finishCalls = 0;
	std::function<void (const gs::MatchPollUpdate&)> finish = [&finishCalls](const gs::MatchPollUpdate& resp)
	{
		REQUIRE(resp.data.pollId == "test-poll");
		finishCalls++;
	};

	const char* updateMessage = R"({
		"meta": {
			"action": "update",
			"target": "match_poll"
		},
		"data": {
			"poll_id": "test-poll",
			"match_id": "my-cool-match",
			"status": "active",

			"results": {
				"1001": {
					"count": 4,
					"mean": 0.0,
					"sum": 0.0,

					"results": [1, 3],
					"poll": {
						"poll_id": "test-poll",
						"prompt": "Is a hot dog a sandwich?",
						"options": ["Yes", "No"],
						"status": "active"
					}
				}
			}
		}
	})";

	const char * finishMessage = R"({
		"meta": {
			"action": "update",
			"target": "match_poll"
		},
		"data": {
			"poll_id": "test-poll",
			"match_id": "my-cool-match",
			"status": "expired",

			"results": {
				"1001": {
					"count": 4,
					"mean": 0.0,
					"sum": 0.0,

					"results": [1, 3],
					"poll": {
						"poll_id": "test-poll",
						"prompt": "Is a hot dog a sandwich?",
						"options": ["Yes", "No"],
						"status": "expired"
					}
				}
			}
		}
	})";

	sdk.RunMatchPoll("my-cool-match", "test-poll",
		"Is a hot dog a sandwich?",
		config,
		{ "Yes", "No" },
		update, finish);


	sdk.ReceiveMessage(updateMessage, strlen(updateMessage));
	sdk.ReceiveMessage(updateMessage, strlen(updateMessage));
	sdk.ReceiveMessage(updateMessage, strlen(updateMessage));
	sdk.ReceiveMessage(finishMessage, strlen(finishMessage));
	sdk.ReceiveMessage(finishMessage, strlen(finishMessage));
	sdk.ReceiveMessage(finishMessage, strlen(finishMessage));

	REQUIRE(updateCalls == 3);
	REQUIRE(finishCalls == 1);
}