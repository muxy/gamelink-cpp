#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

namespace gs = gamelink::schema;
TEST_CASE("Poll Creation", "[poll][creation]")
{
	// Create poll without user data
	gs::CreatePollRequest req("poll-id", "Yes or No?", {"Yes", "No"});
	SerializeEqual(req, R"({
		"action": "create",
		"params": {
			"request_id": 65535,
			"target": "poll"
		},
		"data": {
			"poll_id": "poll-id",
			"prompt": "Yes or No?",
			"options": ["Yes", "No"]
		}
	})");

	// Create poll with user data
	std::map<std::string, gamelink::string> userData;
	userData["showTitle"] = "true";
	userData["title"] = "Yes or No?";

	gs::CreatePollWithUserDataRequest<std::map<std::string, gamelink::string>> req2("poll-id", "Yes or No?", {"Yes", "No"}, userData);
	SerializeEqual(req2, R"({
		"action": "create",
		"params": {
			"request_id": 65535,
			"target": "poll"
		},
		"data": {
			"poll_id": "poll-id",
			"prompt": "Yes or No?",
			"options": ["Yes", "No"],
			"user_data": {
				"showTitle": "true",
				"title": "Yes or No?"
			}
		}
	})");
}

TEST_CASE("Poll Update Response (De)Serialization", "[poll][update]")
{
	gs::PollUpdateResponse pollResponse;
	pollResponse.data.poll.pollId = "poll-id";
	pollResponse.data.poll.prompt = "Yes or No?";
	pollResponse.data.poll.options = {"Yes", "No"};
	pollResponse.data.results = {1, 2};

	SerializeEqual(pollResponse, R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"results": [1, 2],
			"poll": {
				"poll_id": "poll-id",
				"prompt": "Yes or No?",
				"options": ["Yes", "No"]
			}
		}
	})");

	Deserialize(R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"results": [1, 2],
			"poll": {
				"poll_id": "poll-id",
				"prompt": "Yes or No?",
				"options": ["Yes", "No"]
			}
		}
	})",
				pollResponse);

	REQUIRE(pollResponse.meta.action == "update");
	REQUIRE(pollResponse.meta.target == "poll");
	REQUIRE(pollResponse.data.poll.pollId == "poll-id");
	REQUIRE(pollResponse.data.poll.prompt == "Yes or No?");
	REQUIRE(pollResponse.data.poll.options[0] == "Yes");
	REQUIRE(pollResponse.data.poll.options[1] == "No");
	REQUIRE(pollResponse.data.results[0] == 1);
	REQUIRE(pollResponse.data.results[1] == 2);
}

TEST_CASE("Poll Deletion", "[poll][deletion]")
{
	gs::DeletePollRequest req("test-poll");
	SerializeEqual(req, R"({
		"action": "delete",
		"params": {
			"target": "poll",
			"request_id": 65535
		},
		"data": {
			"poll_id": "test-poll"
		}
	})");
}

TEST_CASE("SDK Poll Creation", "[sdk][poll][creation]")
{
	gamelink::SDK sdk;

	sdk.CreatePoll("test-poll", "Me or Them?", {"Me", "Them"});

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({
				"action":"create",
				"data":{
					"options":["Me","Them"],
					"poll_id":"test-poll",
					"prompt":"Me or Them?"
				},
				"params":{
					"request_id":65535,
					"target":"poll"
				}
			})"));
	});

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("SDK Poll Get Results", "[sdk][poll][results]")
{
	gamelink::SDK sdk;

	sdk.GetPoll("test-poll");

	REQUIRE(sdk.HasPayloads());
	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data, R"({"action":"get","data":{"poll_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})"));
	});
	REQUIRE(!sdk.HasPayloads());

	uint32_t calls = 0;
	sdk.GetPoll("something-else", [&](const gamelink::schema::GetPollResponse& poll) {
		calls++;

		REQUIRE(poll.data.poll.pollId == "something-else");
	});

	const char * msg = R"({
		 "data": {
            "poll": {
                "poll_id": "something-else",
				"prompt": "Superman or Batman", 
				"options": ["Superman", "Batman"], 
				"user_data": {}
            }, 
			"results": [
				100, 
				93
			]
        },
        "meta":{
            "request_id":1,
            "action": "get",
            "target":"poll"
        }
	})";

	sdk.ReceiveMessage(msg, strlen(msg));

	msg = R"({
		 "data": {
            "poll": {
                "poll_id": "wrong",
				"prompt": "no", 
				"options": ["yes", "no"], 
				"user_data": {}
            }, 
			"results": [
				0, 
				1
			]
        },
        "meta":{
            "request_id":2,
            "action": "get",
            "target":"poll"
        }
	})";

	sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(calls == 1);
}

TEST_CASE("SDK Poll Subscription", "[sdk][poll][subscription]")
{
	gamelink::SDK sdk;

	sdk.SubscribeToPoll("test-poll");

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({"action":"subscribe","data":{"topic_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})"));
	});

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("SDK Poll Update Response", "[sdk][poll][update]")
{
	gamelink::SDK sdk;

	bool received = false;
	const char* json = R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"poll": {
				"poll_id": "test-poll",
				"prompt": "Choose one",
				"options": ["Red", "Blue"]
			},
			"results": [3, 2]
		}
	})";

	sdk.OnPollUpdate([&](gamelink::schema::PollUpdateResponse pollResp) {
		received = true;
		SerializeEqual(pollResp, json);
	});

	sdk.ReceiveMessage(json, strlen(json));

	REQUIRE(received);
}
