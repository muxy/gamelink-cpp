#include "catch2/catch.hpp"
#include "util.h"

#include "schema/state.h"

class BasicState
{
public:
    std::string name;
    uint32_t value;
    std::vector<BasicState> children;

    MUXY_GAMELINK_SERIALIZE_INTRUSIVE_4(BasicState, 
        "name", name, 
        "value", value, 
        "children", children, 
        "priv", priv
    );

    void setPriv(uint32_t p)
    {
        priv = p;
    }

    uint32_t getPriv() const
    {
        return priv;
    }
private:
    uint32_t priv;
};

TEST_CASE("Basic state serialization", "[state][serialization]")
{
    BasicState parent;
    parent.name = "hp"; 
    parent.value = 100;
    parent.setPriv(32);

    BasicState child;
    child.name = "percentage hp";
    child.value = 50;
    child.setPriv(1);

    parent.children.push_back(child);

    gamelink::schema::SetStateRequest<BasicState> req(gamelink::schema::STATE_TARGET_CHANNEL, parent);

    SerializeEqual(req, R"({
        "action": "set", 
        "params": {
            "request_id": 65535,
            "target": "state"
        }, 
        "data": {
            "state_id": "channel",
            "state": {
                "name": "hp", 
                "value": 100, 
                "priv": 32, 
                "children": [{
                        "name": "percentage hp", 
                        "value": 50, 
                        "priv": 1, 
                        "children": []
                    }
                ]
            }
        }
    })");
}

TEST_CASE("State response deserialization", "[state][deserialization]")
{
    gamelink::schema::SetStateResponse<BasicState> resp;

    Deserialize(R"({
        "meta": {
            "action": "set",
            "request_id": 4,
            "target": "state"
        }, 
        "data": {
            "ok": true,
            "state": {
                "name": "hp", 
                "value": 100, 
                "priv": 32, 
                "children": [{
                        "name": "percentage hp", 
                        "value": 50, 
                        "priv": 1, 
                        "children": []
                    }
                ]
            }
        }
    })", resp);
    
    REQUIRE(resp.meta.action == "set");
    REQUIRE(resp.meta.request_id == 4);
    REQUIRE(resp.meta.target == "state");

    REQUIRE(resp.data.ok == true);
    REQUIRE(resp.data.state.name == "hp");
    REQUIRE(resp.data.state.value == 100);
    REQUIRE(resp.data.state.getPriv() == 32);
    REQUIRE(resp.data.state.children.size() == 1);
    REQUIRE(resp.data.state.children[0].name == "percentage hp");
    REQUIRE(resp.data.state.children[0].value == 50);
    REQUIRE(resp.data.state.children[0].getPriv() == 1);
    REQUIRE(resp.data.state.children[0].children.size() == 0);
}

TEST_CASE("State update serialization", "[state][serialization]")
{
    gamelink::schema::PatchStateRequest req(gamelink::schema::STATE_TARGET_EXTENSION);
    
    gamelink::schema::PatchOperation op;
    op.operation = "replace";
    op.path = "/children/0/name", 
    op.value = gamelink::schema::atomFromString("percentage mana");

    req.data.state.push_back(op);

    SerializeEqual(req, R"({
        "action": "patch", 
        "params": {
            "request_id": 65535, 
            "target": "state"
        }, 
        "data": {
            "state_id": "extension",
            "state": [
                { "op" : "replace", "path": "/children/0/name", "value": "percentage mana" }
            ]
        }
    })");
}

TEST_CASE("State get serialization", "[state][serialization]")
{
    gamelink::schema::GetStateRequest req(gamelink::schema::STATE_TARGET_CHANNEL);
    SerializeEqual(req, R"({
        "action": "get", 
        "params": {
            "request_id": 65535, 
            "target": "state"
        }, 
        "data": {
            "state_id": "channel"
        }
    })");
}

TEST_CASE("State subscribe serialization", "[state][serialization]")
{
    gamelink::schema::SubscribeStateRequest req(gamelink::schema::STATE_TARGET_EXTENSION);

    SerializeEqual(req, R"({
        "action": "subscribe", 
        "params": {
            "request_id": 65535, 
            "target": "state"
        }, 
        "data": {
            "topic_id": "extension"
        }
    })");
}

TEST_CASE("State update deserialize", "[state][deserialization]")
{
     gamelink::schema::SubscribeStateUpdateResponse<BasicState> resp;

    Deserialize(R"({
        "meta": {
            "action": "update",
            "request_id": 4,
            "target": "state"
        }, 
        "data": {
            "topic_id": "extension",
            "state": {
                "name": "hp", 
                "value": 100, 
                "priv": 32, 
                "children": [{
                        "name": "percentage hp", 
                        "value": 50, 
                        "priv": 1, 
                        "children": []
                    }
                ]
            }
        }
    })", resp);
    
    REQUIRE(resp.meta.action == "update");
    REQUIRE(resp.meta.request_id == 4);
    REQUIRE(resp.meta.target == "state");

    REQUIRE(resp.data.topic_id == "extension");
    REQUIRE(resp.data.state.name == "hp");
    REQUIRE(resp.data.state.value == 100);
    REQUIRE(resp.data.state.getPriv() == 32);
    REQUIRE(resp.data.state.children.size() == 1);
    REQUIRE(resp.data.state.children[0].name == "percentage hp");
    REQUIRE(resp.data.state.children[0].value == 50);
    REQUIRE(resp.data.state.children[0].getPriv() == 1);
    REQUIRE(resp.data.state.children[0].children.size() == 0);
}