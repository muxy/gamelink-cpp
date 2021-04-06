#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

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
			})"
		));
	});

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("SDK Poll Get Results", "[sdk][poll][results]")
{
	gamelink::SDK sdk;

	sdk.GetPoll("test-poll");

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data, 
			R"({"action":"get","data":{"poll_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})"
		));
	});

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("SDK Poll Subscription", "[sdk][poll][subscription]")
{
	gamelink::SDK sdk;

	sdk.SubscribeToPoll("test-poll");

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data, 
			R"({"action":"subscribe","data":{"topic_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})"
		));
	});

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("SDK Poll Update Response", "[sdk][poll][update]")
{
	gamelink::SDK sdk;

	bool received = false;
	const char * json = R"({
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
