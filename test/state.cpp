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

	gamelink::schema::SetStateRequest<BasicState> req(gamelink::StateTarget::Channel, parent);

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
	gamelink::schema::PatchStateRequest req(gamelink::StateTarget::Channel);

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
            "state_id": "channel",
            "state": [
                { "op" : "replace", "path": "/children/0/name", "value": "percentage mana" }
            ]
        }
    })");
}

TEST_CASE("State get serialization", "[state][serialization]")
{
	gamelink::schema::GetStateRequest req(gamelink::StateTarget::Channel);
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
	gamelink::schema::SubscribeStateRequest req(gamelink::StateTarget::Extension);

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
            "target": "extension"
        },
        "data": {
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
	REQUIRE(resp.meta.target == "extension");

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

	sdk.SetState(gamelink::StateTarget::Channel, st);
	validateSinglePayload(sdk, R"({
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
	})");

	sdk.ClearState(gamelink::StateTarget::Channel);
	validateSinglePayload(sdk, R"({
		"action": "set",
		"data": {
			"state_id": "channel",
			"state": {}
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");
}

TEST_CASE("SDK State Retreival", "[sdk][state]")
{
	gamelink::SDK sdk;

	uint32_t calls = 0;
	sdk.GetState(gamelink::StateTarget::Channel, [&](const gamelink::schema::GetStateResponse<nlohmann::json>& cb) {
		REQUIRE(cb.data.state["name"] == "health");
		REQUIRE(cb.data.state["value"] == 200.99);
		calls++;
	});

	validateSinglePayload(sdk, R"({
		"action": "get",
		"data": {
			"state_id": "channel"
		},
		"params":{
			"request_id":1,
			"target":"state"
		}
	})");

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

	sdk.UpdateStateWithString(gamelink::StateTarget::Channel, gamelink::Operation::Replace, "/name", "whatever");
	validateSinglePayload(sdk, R"({
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
	})");
}

TEST_CASE("SDK Update array", "[sdk][state][target]")
{
	gamelink::SDK sdk;

	int integerRoll = 1;
	std::string stringRoll = "world";

	sdk.UpdateStateWithArray(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/rolls", &integerRoll, &integerRoll + 1);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [{
				"op": "add",
				"path": "/rolls",
				"value": [1]
			}]
		},
		"params":{
			"request_id":65535,
			"target":"state"
		}
	})");

	sdk.UpdateStateWithArray(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/rolls", &stringRoll, &stringRoll + 1);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [{
				"op": "add",
				"path": "/rolls",
				"value": ["world"]
			}]
		},
		"params":{
			"request_id":65535,
			"target":"state"
		}
	})");
}

TEST_CASE("SDK Subscription", "[sdk][state][subscription]")
{
	gamelink::SDK sdk;

	uint32_t calls = 0;
	uint32_t secondCalls = 0;
	uint32_t errors = 0;
	sdk.OnStateUpdate().Add([&](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& resp) {
		calls++;
		if (!resp.errors.empty())
		{
			errors++;
		}
	});

	// Two onStateUpdate functions.
	uint32_t second = sdk.OnStateUpdate().Add([&](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& resp) { secondCalls++; });

	// Basic usage
	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Channel);
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

	sdk.OnStateUpdate().Remove(second);

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

TEST_CASE("PatchList equality", "[state]")
{
	gamelink::SDK sdk;

	sdk.UpdateStateWithObject(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/character", nlohmann::json::parse(R"({
		"class": "wizard"
	})"));

	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/character", "value": { "class": "wizard" }}
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");

	gamelink::PatchList list;
	list.UpdateStateWithObject(gamelink::Operation::Add, "/character", nlohmann::json::parse(R"({
		"class": "wizard"
	})"));
	sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/character", "value": { "class": "wizard" }}
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");
}

TEST_CASE("PatchList no elements", "[state]")
{
	gamelink::SDK sdk;
	sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, gamelink::PatchList());

	uint32_t count = 0;
	sdk.ForeachPayload([&](const gamelink::Payload* send) { count++; });

	REQUIRE(count == 0);
}

TEST_CASE("PatchList many patches", "[state]")
{
	gamelink::PatchList list;
	for (uint32_t i = 0; i < 100; ++i)
	{
		list.UpdateStateWithInteger(gamelink::Operation::Add, "/somearr/-1", i);
	}

	gamelink::SDK sdk;
	sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);
	uint32_t count = 0;
	sdk.ForeachPayload([&](const gamelink::Payload* send) {
		count++;

		gamelink::schema::PatchStateRequest request(gamelink::StateTarget::Channel);
		Deserialize(send->Data(), request);

		REQUIRE(request.data.state_id == gamelink::TARGET_STRINGS[static_cast<int>(gamelink::StateTarget::Channel)]);
		REQUIRE(request.data.state.size() == 100);
	});

	REQUIRE(count == 1);
}

TEST_CASE("Update state", "[state]")
{
	gamelink::SDK sdk;
	sdk.UpdateStateWithObject(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/character", nlohmann::json::parse(R"({
		"class": "wizard"
	})"));
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/character", "value": { "class": "wizard" }}
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");

	sdk.UpdateStateWithBoolean(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/b", false);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/b", "value": false }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");

	sdk.UpdateStateWithDouble(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/b", 44.15);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/b", "value": 44.15 }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");

	sdk.UpdateStateWithInteger(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/b", -100);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/b", "value": -100 }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");

	sdk.UpdateStateWithLiteral(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/b", R"([{ "literal": "json" }])");
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/b", "value": [
					{ "literal": "json" }
				]}
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");

	sdk.UpdateStateWithNull(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/b");
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/b", "value": null }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");

	sdk.UpdateStateWithString(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/b", "Gandalf");
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"state_id": "channel",
			"state": [
				{ "op": "add", "path": "/b", "value": "Gandalf" }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "state"
		}
	})");
}

TEST_CASE("Update state with patch list", "[state]")
{
	gamelink::SDK sdk;
	{
		gamelink::PatchList list;
		list.UpdateStateWithObject(gamelink::Operation::Add, "/character", nlohmann::json::parse(R"({
				"class": "wizard"
			})"));
		sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

		validateSinglePayload(sdk, R"({
			"action": "patch",
			"data": {
				"state_id": "channel",
				"state": [
					{ "op": "add", "path": "/character", "value": { "class": "wizard" }}
				]
			},
			"params": {
				"request_id": 65535,
				"target": "state"
			}
		})");
	}

	{
		gamelink::PatchList list;
		list.UpdateStateWithBoolean(gamelink::Operation::Add, "/b", false);
		sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

		validateSinglePayload(sdk, R"({
			"action": "patch",
			"data": {
				"state_id": "channel",
				"state": [
					{ "op": "add", "path": "/b", "value": false }
				]
			},
			"params": {
				"request_id": 65535,
				"target": "state"
			}
		})");
	}
	{
		gamelink::PatchList list;
		list.UpdateStateWithDouble(gamelink::Operation::Add, "/b", 44.15);
		sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

		validateSinglePayload(sdk, R"({
			"action": "patch",
			"data": {
				"state_id": "channel",
				"state": [
					{ "op": "add", "path": "/b", "value": 44.15 }
				]
			},
			"params": {
				"request_id": 65535,
				"target": "state"
			}
		})");
	}
	{
		gamelink::PatchList list;
		list.UpdateStateWithInteger(gamelink::Operation::Add, "/b", -100);
		sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

		validateSinglePayload(sdk, R"({
			"action": "patch",
			"data": {
				"state_id": "channel",
				"state": [
					{ "op": "add", "path": "/b", "value": -100 }
				]
			},
			"params": {
				"request_id": 65535,
				"target": "state"
			}
		})");
	}
	{
		gamelink::PatchList list;
		list.UpdateStateWithLiteral(gamelink::Operation::Add, "/b", R"([{ "literal": "json" }])");
		sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

		validateSinglePayload(sdk, R"({
			"action": "patch",
			"data": {
				"state_id": "channel",
				"state": [
					{ "op": "add", "path": "/b", "value": [
						{ "literal": "json" }
					]}
				]
			},
			"params": {
				"request_id": 65535,
				"target": "state"
			}
		})");
	}
	{
		gamelink::PatchList list;
		list.UpdateStateWithNull(gamelink::Operation::Add, "/b");
		sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

		validateSinglePayload(sdk, R"({
			"action": "patch",
			"data": {
				"state_id": "channel",
				"state": [
					{ "op": "add", "path": "/b", "value": null }
				]
			},
			"params": {
				"request_id": 65535,
				"target": "state"
			}
		})");
	}
	{
		gamelink::PatchList list;
		list.UpdateStateWithString(gamelink::Operation::Add, "/b", "Gandalf");
		sdk.UpdateStateWithPatchList(gamelink::StateTarget::Channel, list);

		validateSinglePayload(sdk, R"({
			"action": "patch",
			"data": {
				"state_id": "channel",
				"state": [
					{ "op": "add", "path": "/b", "value": "Gandalf" }
				]
			},
			"params": {
				"request_id": 65535,
				"target": "state"
			}
		})");
	}
}