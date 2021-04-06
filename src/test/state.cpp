#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

struct GenericState
{
    std::string name;
    double value;

    std::vector<GenericState> children;

    MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(GenericState, "name", name, "value", value, "children", children);
};

TEST_CASE("SDK State Creation", "[sdk][state]")
{
    gamelink::SDK sdk;

    GenericState st;
    st.name = "health";
    st.value = 42.123;

    sdk.SetState(gamelink::schema::STATE_TARGET_CHANNEL, st);
    sdk.ForeachPayload([](const gamelink::Payload * send){
        REQUIRE(JSONEquals(send->data, 
        R"({
            "action": "set", 
            "data": {
                "state": {
                    "children": [], 
                    "name": "health", 
                    "value": 42.123
                }
            }, 
            "params":{
                "request_id":65535,
                "target":"channel"
            }
        })"
        ));
    });
}

TEST_CASE("SDK Update State", "[sdk][state]")
{
    gamelink::SDK sdk;

    sdk.UpdateState(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/name", gamelink::schema::atomFromString("whatever"));
     sdk.ForeachPayload([](const gamelink::Payload * send){
        REQUIRE(JSONEquals(send->data, 
        R"({
            "action": "update", 
            "data": {
                "state": [{
                    "op": "replace", 
                    "path": "/name", 
                    "value": "whatever"
                }]
            }, 
            "params":{
                "request_id":65535,
                "target":"channel"
            }
        })"
        ));
    });
}

TEST_CASE("SDK Subscription", "[sdk][state][subscription]")
{
    gamelink::SDK sdk;

    uint32_t calls = 0;
    uint32_t errors = 0;
    sdk.OnStateUpdate([&](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& resp)
    {
        calls++;
        if (!resp.errors.empty())
        {
            errors++;
        }
    });

    // Basic usage
    sdk.SubscribeToStateUpdates(gamelink::schema::STATE_TARGET_CHANNEL);
    REQUIRE(calls == 0);
    REQUIRE(errors == 0);

    const char * message = R"({
        "meta": {
            "action": "update", 
            "target": "channel"
        }, 
        "data": {
            "state": {
                "mana": 100, 
                "dps": 10562121, 
                "position": {
                    "x": 12, 
                    "y": 34
                }
            }
        }
    })";

    sdk.ReceiveMessage(message, strlen(message));
    REQUIRE(calls == 1);
    REQUIRE(errors == 0);

    // Show errors go through correctly.
    message = R"({
        "meta": {
            "action": "update", 
            "target": "channel"
        }, 
        "errors": [{
            "title": "oh no", 
            "code": 404, 
            "detail": "not found"
        }]
    })";
    sdk.ReceiveMessage(message, strlen(message));
    REQUIRE(calls == 2);
    REQUIRE(errors == 1);

    // Show that other updates don't trigger a state update
    message = R"({
        "meta": {
            "action": "update", 
            "target": "poll"
        }, 
        "data": {}
    })";

    sdk.ReceiveMessage(message, strlen(message));
    REQUIRE(calls == 2);
    REQUIRE(errors == 1);
}
