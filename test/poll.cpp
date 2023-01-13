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
	pollResponse.data.count = 3;
	pollResponse.data.mean = 0.0;
	pollResponse.data.sum = 0.0;
	pollResponse.data.poll.pollId = "poll-id";
	pollResponse.data.poll.prompt = "Yes or No?";
	pollResponse.data.poll.options = {"Yes", "No"};
	pollResponse.data.poll.status = "active";
	pollResponse.data.results = {1, 2};

	SerializeEqual(pollResponse, R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"count": 3,
			"mean": 0.0,
			"sum": 0.0,

			"results": [1, 2],
			"poll": {
				"poll_id": "poll-id",
				"prompt": "Yes or No?",
				"options": ["Yes", "No"],
				"status": "active"
			}
		}
	})");

	Deserialize(R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"count": 3,
			"mean": 0.0,
			"sum": 0.0,

			"results": [1, 2],
			"poll": {
				"poll_id": "poll-id",
				"prompt": "Yes or No?",
				"options": ["Yes", "No"],
				"status": "active"
			}
		}
	})",
				pollResponse);

	REQUIRE(pollResponse.meta.action == "update");
	REQUIRE(pollResponse.meta.target == "poll");
	REQUIRE(pollResponse.data.poll.pollId == "poll-id");
	REQUIRE(pollResponse.data.poll.prompt == "Yes or No?");
	REQUIRE(pollResponse.data.poll.status == "active");
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
	validateSinglePayload(sdk, R"({
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
	})");


	std::vector<gamelink::string> options = {"Me", "Them"};
	sdk.CreatePoll("test-poll", "Me or Them?", options.data(), options.data() + 2);

	validateSinglePayload(sdk, R"({
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
	})");

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("SDK Poll Creation With Options", "[sdk][poll][creation]")
{
	gamelink::SDK sdk;
	gamelink::PollConfiguration config;
	config.userIdVoting = true;
	config.startsAt = 10;
	config.endsAt = 20;
	config.distinctOptionsPerUser = 2;
	config.votesPerOption = 1000;
	config.totalVotesPerUser = 1000;
	config.disabled = true;

	sdk.CreatePollWithConfiguration("test-poll", "Me or Them?", config, {"Me", "Them"});
	REQUIRE(sdk.HasPayloads());

	validateSinglePayload(sdk, R"({
		"action":"create",
		"data":{
			"options":["Me","Them"],
			"poll_id":"test-poll",
			"prompt":"Me or Them?",
			"config": {
				"disabled": true,
				"distinctOptionsPerUser": 2,
				"votesPerOption": 1000,
				"totalVotesPerUser": 1000,
				"userIDVoting": true,
				"startsAt": 10,
				"endsAt": 20
			}
		},
		"params":{
			"request_id":65535,
			"target":"poll"
		}
	})");

	std::vector<gamelink::string> options = {"Me", "Them"};
	sdk.CreatePollWithConfiguration("test-poll", "Me or Them?", config, options.data(), options.data() + 2);
	validateSinglePayload(sdk, R"({
		"action":"create",
		"data":{
			"options":["Me","Them"],
			"poll_id":"test-poll",
			"prompt":"Me or Them?",
			"config": {
				"disabled": true,
				"distinctOptionsPerUser": 2,
				"votesPerOption": 1000,
				"totalVotesPerUser": 1000,
				"userIDVoting": true,
				"startsAt": 10,
				"endsAt": 20
			}
		},
		"params":{
			"request_id":65535,
			"target":"poll"
		}
	})");
}

TEST_CASE("SDK Poll Get Results", "[sdk][poll][results]")
{
	gamelink::SDK sdk;

	sdk.GetPoll("test-poll");

	validateSinglePayload(sdk, R"({"action":"get","data":{"poll_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})");

	uint32_t calls = 0;
	sdk.GetPoll("something-else", [&](const gamelink::schema::GetPollResponse& poll) {
		calls++;

		REQUIRE(poll.data.poll.pollId == "something-else");
	});

	const char* msg = R"({
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
            "request_id":2,
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

TEST_CASE("SDK Poll Get Results C-style", "[sdk][poll][results]")
{
	gamelink::SDK sdk;

	sdk.GetPoll("test-poll");
	validateSinglePayload(sdk, R"({
		"action":"get",
		"data":{"poll_id":"test-poll"},
		"params":{"request_id":65535,"target":"poll"}
	})");


	uint32_t calls = 0;
	sdk.GetPoll("something-else", [](void* data, const gamelink::schema::GetPollResponse& poll) {
		uint32_t* calls = static_cast<uint32_t*>(data);
		(*calls)++;

		REQUIRE(poll.data.poll.pollId == "something-else");
	}, &calls);

	const char* msg = R"({
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
            "request_id":2,
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

	validateSinglePayload(sdk, R"({"action":"subscribe","data":{"topic_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})");
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
			"sum": 2.0,
			"count": 5,
			"mean": 1.2,

			"poll": {
				"poll_id": "test-poll",
				"prompt": "Choose one",
				"options": ["Red", "Blue"],
				"status": "expired"
			},
			"results": [3, 2]
		}
	})";

	sdk.OnPollUpdate().Add([&](gamelink::schema::PollUpdateResponse pollResp) {
		received = true;
		SerializeEqual(pollResp, json);
	});

	sdk.ReceiveMessage(json, strlen(json));
	REQUIRE(received);
}

TEST_CASE("SDK Run Poll", "[sdk][poll]")
{
	gamelink::SDK sdk;

	gamelink::PollConfiguration config;
	config.disabled = false;
	config.distinctOptionsPerUser = 1;
	config.totalVotesPerUser = 1024;
	config.userIdVoting = true;
	config.startsAt = 10;
	config.endsAt = 20;
	config.votesPerOption = 1024;

	int updateCalls = 0;
	std::function<void (const gs::PollUpdateResponse&)> update = [&updateCalls](const gs::PollUpdateResponse& resp)
	{
		REQUIRE(resp.data.poll.pollId == "test-poll");
		updateCalls++;
	};

	int finishCalls = 0;
	std::function<void (const gs::PollUpdateResponse&)> finish = [&finishCalls](const gs::PollUpdateResponse& resp)
	{
		REQUIRE(resp.data.poll.pollId == "test-poll");
		finishCalls++;
	};

	sdk.RunPoll(
		"test-poll",
		"Is a hot dog a sandwich?",
		config,
		{ "Yes", "No "},
		update,
		finish
	);

	const char * updateMessage = R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"count": 3,
			"mean": 0.0,
			"sum": 0.0,

			"results": [1, 2],
			"poll": {
				"poll_id": "test-poll",
				"prompt": "Is a hot dog a sandwich?",
				"options": ["Yes", "No"],
				"status": "active"
			}
		}
	})";

	const char * otherUpdateMessage = R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"count": 3000,
			"mean": 0.0,
			"sum": 0.0,

			"results": [1000, 2000],
			"poll": {
				"poll_id": "some-other-poll",
				"prompt": "Irrelevant",
				"options": ["Yes", "No"],
				"status": "active"
			}
		}
	})";

	const char * finishMessage = R"({
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
				"poll_id": "test-poll",
				"prompt": "Is a hot dog a sandwich?",
				"options": ["Yes", "No"],
				"status": "expired"
			}
		}
	})";

	// Send a few updates and then a finish, show that there is the expected number of calls.
	sdk.ReceiveMessage(updateMessage, strlen(updateMessage));
	sdk.ReceiveMessage(updateMessage, strlen(updateMessage));
	sdk.ReceiveMessage(updateMessage, strlen(updateMessage));
	sdk.ReceiveMessage(updateMessage, strlen(updateMessage));
	sdk.ReceiveMessage(otherUpdateMessage, strlen(otherUpdateMessage));
	sdk.ReceiveMessage(finishMessage, strlen(finishMessage));
	sdk.ReceiveMessage(finishMessage, strlen(finishMessage));
	sdk.ReceiveMessage(finishMessage, strlen(finishMessage));

	REQUIRE(updateCalls == 4);
	REQUIRE(finishCalls == 1);
}

TEST_CASE("SDK Poll other operations", "[sdk][poll]")
{
	gamelink::SDK sdk;

	sdk.StopPoll("pizza-toppings");
	validateSinglePayload(sdk, R"({
		"action": "reconfigure", 
		"params": {
			"request_id": 65535,
			"target": "poll"
		}, 
		"data": {
			"poll_id": "pizza-toppings",
			"config": {
				"endsAt": -1
			}
		}
	})");

	sdk.SetPollDisabled("pizza-toppings", true);
	validateSinglePayload(sdk, R"({
		"action": "reconfigure", 
		"params": {
			"request_id": 65535,
			"target": "poll"
		}, 
		"data": {
			"poll_id": "pizza-toppings", 
			"config": {
				"disabled": true
			}
		}
	})");

	sdk.SetPollDisabled("pizza-toppings", false);
	validateSinglePayload(sdk, R"({
		"action": "reconfigure", 
		"params": {
			"request_id": 65535,
			"target": "poll"
		}, 
		"data": {
			"poll_id": "pizza-toppings", 
			"config": {
				"disabled": false
			}
		}
	})");
}


TEST_CASE("Poll Reconfigure serialization", "[poll][update]")
{
	gs::ExpirePollRequest expireRequest("foobar");
	SerializeEqual(expireRequest, R"({
		"action": "reconfigure",
		"params": {
			"request_id": 65535,
			"target": "poll"
		},
		"data": {
			"poll_id": "foobar",
			"config": {
				"endsAt": -1
			}
		}
	})");

	gs::SetPollDisabledStatusRequest disableRequest("foobar", true);
	SerializeEqual(disableRequest, R"({
		"action": "reconfigure",
		"params": {
			"request_id": 65535,
			"target": "poll"
		},
		"data": {
			"poll_id": "foobar",
			"config": {
				"disabled": true
			}
		}
	})");
}