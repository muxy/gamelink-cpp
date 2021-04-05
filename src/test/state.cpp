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
    sdk.ForeachSend([](const gamelink::Send * send){
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
     sdk.ForeachSend([](const gamelink::Send * send){
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