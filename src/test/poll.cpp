#include "catch2/catch.hpp"
#include "util.h"

#include "../gamelink.hpp"
#include <iostream>

TEST_CASE("SDK Poll Creation", "[sdk][poll][creation]")
{
	gamelink::SDK sdk;

	sdk.CreatePoll("test-poll", "Me or Them?", {"Me", "Them"});

	REQUIRE(sdk.HasSends());

	sdk.ForeachSend([](gamelink::Send* send) {
		REQUIRE(
			send->data ==
			R"({"action":"create","data":{"options":["Me","Them"],"poll_id":"test-poll","prompt":"Me or Them?"},"params":{"request_id":65535,"target":"poll"}})");
	});

	REQUIRE(!sdk.HasSends());
}

TEST_CASE("SDK Poll Get Results", "[sdk][poll][results]")
{
	gamelink::SDK sdk;

	sdk.GetPoll("test-poll");

	REQUIRE(sdk.HasSends());

	sdk.ForeachSend([](gamelink::Send* send) {
		REQUIRE(send->data == R"({"action":"get","data":{"poll_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})");
	});

	REQUIRE(!sdk.HasSends());
}

TEST_CASE("SDK Poll Subscription", "[sdk][poll][subscription]")
{
	gamelink::SDK sdk;

	sdk.SubscribeToPoll("test-poll");

	REQUIRE(sdk.HasSends());

	sdk.ForeachSend([](gamelink::Send* send) {
		REQUIRE(send->data == R"({"action":"subscribe","data":{"topic_id":"test-poll"},"params":{"request_id":65535,"target":"poll"}})");
	});

	REQUIRE(!sdk.HasSends());
}

TEST_CASE("SDK Poll Update Response", "[sdk][poll][update]")
{
	gamelink::SDK sdk;

	bool received = false;
	auto json = R"({
		"meta": {
			"action": "update",
			"target": "poll"
		},
		"data": {
			"topic_id": "test-poll",
			"poll": {
				"prompt": "Choose one",
				"options": ["Red", "Blue"]
			},
			"results": [3, 2]
		}
	})";

	sdk.OnPollUpdate([&](gamelink::schema::PollUpdateResponse pollResp) {
		received = true;

		std::cout << "received";

		SerializeEqual(pollResp, json);
	});

	sdk.ReceiveMessage(json);

	REQUIRE(received);
}
