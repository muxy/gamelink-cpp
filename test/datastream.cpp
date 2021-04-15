#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

TEST_CASE("Subscribe to Datastream", "[datastream]")
{
	gamelink::SDK sdk;
	sdk.SubscribeToDatastream();

	REQUIRE(sdk.HasPayloads());
	sdk.ForeachPayload([](const gamelink::Payload* payload) {
		REQUIRE(JSONEquals(payload->data,
						   R"({
            "action": "subscribe", 
            "data": {
                "topic_id": ""
            }, 
            "params": {
                "request_id": 65535, 
                "target": "datastream"
            }
        })"));
	});
	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("Receive from Datastream", "[datastream]")
{
	gamelink::SDK sdk;

	uint32_t calls = 0;
	sdk.OnDatastream([&](const gamelink::schema::DatastreamUpdate& update) {
		calls++;
		REQUIRE(update.data.events.size() == 2);
		REQUIRE(update.data.events[0].event["name"] == "user-clicked");
		REQUIRE(update.data.events[1].event["name"] == "user-didnt-click");
	});

	const char* msg = R"({
        "meta": {
            "request_id": 65535, 
            "action": "update", 
            "target": "datastream", 
            "timestamp": 1000
        }, 
        "data": {
            "events": [{
                "event": {
                    "name": "user-clicked"
                }, 
                "timestamp": 1
            }, {
                "event": {
                    "name": "user-didnt-click"
                },
                "timestamp": 2
            }]
        }
    })";

	sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(calls == 1);
}