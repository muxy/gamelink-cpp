#include "catch2/catch.hpp"
#include "util.h"

#include "config.h"
#include "schema/broadcast.h"

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

    gs::BroadcastRequest req("rare-drop", nlohmann::json(msg).dump());

    SerializeEqual(req, R"({
        "action": "broadcast", 
        "params": {
            "request_id": 65535
        }, 
        "data": {
            "topic": "rare-drop", 
            "message": "{\"item\":\"Thunderfury, Blessed Blade of the Windseeker\",\"who\":12345}"
        }
    })");
}