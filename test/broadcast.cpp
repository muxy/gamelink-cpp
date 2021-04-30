#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

namespace gs = gamelink::schema;

struct BroadcastMessage
{
	uint32_t who;
	std::string item;

	MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(BroadcastMessage, "who", who, "item", item);
};

TEST_CASE("Broadcasting", "[broadcast]")
{
	BroadcastMessage msg;
	msg.who = 12345;
	msg.item = "Thunderfury, Blessed Blade of the Windseeker";

	gs::BroadcastRequest<BroadcastMessage> req("rare-drop", msg);

	SerializeEqual(req, R"({
        "action": "broadcast", 
        "params": {
            "request_id": 65535
        }, 
        "data": {
            "topic": "rare-drop", 
            "data": { "item": "Thunderfury, Blessed Blade of the Windseeker","who":12345}
        }
    })");
}

struct StickerBroadcast
{
	std::string sticker;

	MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(StickerBroadcast, "sticker", sticker);
};

TEST_CASE("Broadcast", "[sdk][broadcast]")
{
	StickerBroadcast msg;
	msg.sticker = "Good Work!";

	gamelink::SDK sdk;
	sdk.SendBroadcast("sticker", msg);
	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({
            "action":"broadcast",
            "data":{
                "topic": "sticker", 
                "data": { "sticker": "Good Work!" }
            },
            "params":{
                "request_id":65535
            }
        })"));
	});

	REQUIRE(!sdk.HasPayloads());
}