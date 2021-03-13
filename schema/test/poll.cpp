#include "catch2/catch.hpp"
#include "util.h"

#include "schema/config.h"
#include "schema/poll.h"

TEST_CASE("Poll Creation", "[poll][creation]")
{
	// Create poll without user data
	gamelink::schema::CreatePollRequest req("poll-id", "Yes or No?", { "Yes", "No" });
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

	gamelink::schema::CreateUserDataPollRequest<std::map<gamelink::schema::string, gamelink::schema::string>> req2("poll-id", "Yes or No?", { "Yes", "No" }, userData);
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
