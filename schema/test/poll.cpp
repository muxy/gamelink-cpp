#include "catch2/catch.hpp"
#include "util.h"

#include "schema/config.h"
#include "schema/poll.h"

TEST_CASE("Poll Creation", "[poll][creation]")
{
	// Create poll without user data
	gamelink::schema::CreatePollRequest req("poll-id", "Yes or No?", {"Yes", "No"});
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
	std::map<gamelink::schema::string, gamelink::schema::string> userData;
	userData["showTitle"] = "true";
	userData["title"] = "Yes or No?";

	gamelink::schema::CreateUserDataPollRequest<std::map<gamelink::schema::string, gamelink::schema::string>> req2("poll-id", "Yes or No?",
																												   {"Yes", "No"}, userData);
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
	gamelink::schema::PollUpdateResponse pollResponse("poll-id", "Yes or No?", {"Yes", "No"}, {1, 2});
	SerializeEqual(pollResponse, R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"topic_id": "poll-id",
			"results": [1, 2],
			"poll": {
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
			"topic_id": "poll-id",
			"results": [1, 2],
			"poll": {
				"prompt": "Yes or No?",
				"options": ["Yes", "No"]
			}
		}
	})",
				pollResponse);

	REQUIRE(pollResponse.meta.action == "update");
	REQUIRE(pollResponse.meta.target == "poll");
	REQUIRE(pollResponse.data.pollId == "poll-id");
	REQUIRE(pollResponse.data.poll.prompt == "Yes or No?");
	REQUIRE(pollResponse.data.poll.options[0] == "Yes");
	REQUIRE(pollResponse.data.poll.options[1] == "No");
	REQUIRE(pollResponse.data.results[0] == 1);
	REQUIRE(pollResponse.data.results[1] == 2);
}
