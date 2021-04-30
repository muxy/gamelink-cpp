#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

class BasicState
{
public:
	std::string name;
	uint32_t value;
	std::vector<BasicState> children;

	MUXY_GAMELINK_SERIALIZE_INTRUSIVE_4(BasicState, "name", name, "value", value, "children", children, "priv", priv);

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
    })",
				resp);

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
	op.path = "/children/0/name", op.value = gamelink::schema::atomFromString("percentage mana");

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
    })",
				resp);

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
	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({
            "action": "set", 
            "data": {
                "state_id": "channel",
                "state": {
                    "children": [], 
                    "name": "health", 
                    "value": 42.123
                }
            }, 
            "params":{
                "request_id":65535,
                "target":"state"
            }
        })"));
	});
}

TEST_CASE("SDK State Retreival", "[sdk][state]")
{
	gamelink::SDK sdk;

	uint32_t calls = 0;
	sdk.GetState(gamelink::schema::STATE_TARGET_CHANNEL, [&](const gamelink::schema::GetStateResponse<nlohmann::json>& cb) {
		REQUIRE(cb.data.state["name"] == "health");
		REQUIRE(cb.data.state["value"] == 200.99);
		calls++;
	});

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({
            "action": "get", 
            "data": {
                "state_id": "channel"
            }, 
            "params":{
                "request_id":1,
                "target":"state"
            }
        })"));
	});

	const char* msg = R"({
            "data": {
                "state": {
                    "name": "health", 
                    "value": 100.99
                }
            },
            "meta":{
                "request_id":42,
                "action": "get",
                "target":"state"
            }
        })";
	sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(calls == 0);

	msg = R"({
        "data": {
            "state": {
                "name": "health", 
                "value": 200.99
            }
        },
        "meta":{
            "request_id":1,
            "action": "get",
            "target":"state"
        }
    })";

	// Call this twice. Note that it doesn't fire twice.
	sdk.ReceiveMessage(msg, strlen(msg));
	sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(calls == 1);
}

TEST_CASE("SDK Update State", "[sdk][state]")
{
	gamelink::SDK sdk;

	sdk.UpdateState(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/name", gamelink::schema::atomFromString("whatever"));
	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({
            "action": "patch", 
            "data": {
                "state_id": "channel",
                "state": [{
                    "op": "replace", 
                    "path": "/name", 
                    "value": "whatever"
                }]
            }, 
            "params":{
                "request_id":65535,
                "target":"state"
            }
        })"));
	});
}

TEST_CASE("SDK Subscription", "[sdk][state][subscription]")
{
	gamelink::SDK sdk;

	uint32_t calls = 0;
	uint32_t secondCalls = 0;
	uint32_t errors = 0;
	sdk.OnStateUpdate([&](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& resp) {
		calls++;
		if (!resp.errors.empty())
		{
			errors++;
		}
	});

	// Two onStateUpdate functions.
	uint32_t second = sdk.OnStateUpdate([&](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& resp) { secondCalls++; });

	// Basic usage
	sdk.SubscribeToStateUpdates(gamelink::schema::STATE_TARGET_CHANNEL);
	REQUIRE(calls == 0);
	REQUIRE(errors == 0);

	const char* message = R"({
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
	REQUIRE(secondCalls == calls);

	sdk.DetachOnStateUpdate(second);

	// Reiterate the error message
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

	REQUIRE(calls == 3);
	REQUIRE(errors == 2);
	REQUIRE(secondCalls == 2);
}
