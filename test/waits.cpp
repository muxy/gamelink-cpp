#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

namespace gs = gamelink::schema;
TEST_CASE("Wait usage", "[wait]")
{
	gamelink::SDK sdk;

	gamelink::RequestId id = sdk.GetPoll("test-poll");
	sdk.WaitForResponse(id);

	validateSinglePayload(sdk, R"({"action":"get","data":{"poll_id":"test-poll"},"params":{"request_id":1,"target":"poll"}})");

	sdk.GetPoll("something-else");
	// Even though a message was queued, no payloads avaliable due to a wait existing.
	REQUIRE(!sdk.HasPayloads());

	const char* msg = R"({
		 "data": {
            "poll": {
                "poll_id": "test-poll",
				"prompt": "Cool or not cool",
				"options": ["Cool", "Not cool"],
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

	// Unblocked existing payload, send now.
	validateSinglePayload(sdk, R"({"action":"get","data":{"poll_id":"something-else"},"params":{"request_id":2,"target":"poll"}})");
}

TEST_CASE("Multiple waits", "[wait]")
{
	gamelink::SDK sdk;

	gamelink::RequestId id = sdk.GetPoll("test-poll");
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id);
	sdk.GetPoll("something-else");

	validateSinglePayload(sdk, R"({"action":"get","data":{"poll_id":"test-poll"},"params":{"request_id":1,"target":"poll"}})");

	const char* msg = R"({
		"data": {
            "poll": {
                "poll_id": "test-poll",
				"prompt": "Cool or not cool",
				"options": ["Cool", "Not cool"],
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

	// Unblocked existing payload, send now.
	validateSinglePayload(sdk, R"({"action":"get","data":{"poll_id":"something-else"},"params":{"request_id":2,"target":"poll"}})");
}

TEST_CASE("Interleaved waits", "[wait]")
{
	gamelink::SDK sdk;

	gamelink::RequestId id = sdk.GetPoll("test-poll");
	gamelink::RequestId id2 = sdk.GetPoll("other-poll");
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id2);
	sdk.WaitForResponse(id);
	sdk.GetPoll("something-else");
	sdk.WaitForResponse(id2);
	sdk.WaitForResponse(id);
	sdk.WaitForResponse(id2);

	REQUIRE(sdk.HasPayloads());
	sdk.ForeachPayload([](const gamelink::Payload* send) {});
	REQUIRE(!sdk.HasPayloads());

	const char* msg = R"({
		"data": {
            "poll": {
                "poll_id": "test-poll",
				"prompt": "Cool or not cool",
				"options": ["Cool", "Not cool"],
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
	REQUIRE(!sdk.HasPayloads());

	msg = R"({
		"data": {
            "poll": {
                "poll_id": "test-poll",
				"prompt": "Cool or not cool",
				"options": ["Cool", "Not cool"],
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

	// Unblocked request
	REQUIRE(sdk.HasPayloads());
}