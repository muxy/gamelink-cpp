#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

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
                "message": "{\"sticker\":\"Good Work!\"}"
            },
            "params":{
                "request_id":65535
            }
        })"));
	});
    
	REQUIRE(!sdk.HasPayloads());
}