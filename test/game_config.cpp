#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

TEST_CASE("Get config", "[config]")
{
	gamelink::SDK sdk;
	sdk.GetConfig(gamelink::ConfigTarget::Channel, [](const gamelink::schema::GetConfigResponse& resp) {});
	validateSinglePayload(sdk, R"({
		"action": "get",
		"data": {
			"config_id": "channel"
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.GetConfig(gamelink::ConfigTarget::Extension, [](const gamelink::schema::GetConfigResponse& resp) {});
	validateSinglePayload(sdk, R"({
		"action": "get",
		"data": {
			"config_id": "extension"
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.GetCombinedConfig([](const gamelink::schema::GetCombinedConfigResponse& resp) {});
	validateSinglePayload(sdk, R"({
		"action": "get",
		"data": {
			"config_id": "combined"
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");
}

TEST_CASE("Set Config", "[config]")
{
	gamelink::SDK sdk;
	sdk.SetChannelConfig(nlohmann::json::parse(R"({
        "hello": "world"
    })"));

	sdk.ForeachPayload([](const gamelink::Payload* payload) {
		REQUIRE(JSONEquals(payload->data,
						   R"({
            "action": "set",
            "data": {
                "config": {
                    "hello": "world"
                }
            },
            "params": {
                "request_id": 65535,
                "target": "config"
            }
        })"));
	});
}

TEST_CASE("Get Config", "[config]")
{
	gamelink::SDK sdk;
	uint32_t calls = 0;

	sdk.GetConfig(gamelink::ConfigTarget::Channel, [&](const gamelink::schema::GetConfigResponse& resp) {
		REQUIRE(resp.data.config["foo"] == "bar");
		calls++;
	});

	sdk.ForeachPayload([](const gamelink::Payload* payload) {
		REQUIRE(JSONEquals(payload->data,
						   R"({
            "action": "get",
            "data": {
                "config_id": "channel"
            },
            "params": {
                "request_id": 65535,
                "target": "config"
            }
        })"));
	});

	const char* msg = R"({
        "data": {
            "config": {
                "foo": "bar"
            },
            "config_id": "channel"
        },
        "meta":{
            "request_id":1,
            "action": "get",
            "target":"config"
        }
    })";
	sdk.ReceiveMessage(msg, strlen(msg));

	const char* msg2 = R"({
        "data": {
            "channel": {
                "who": "down"
            },
            "extension": {
                "what": "up"
            },
            "config_id": "combined"
        },
        "meta":{
            "request_id":2,
            "action": "get",
            "target":"config"
        }
    })";
	sdk.ReceiveMessage(msg2, strlen(msg2));
	REQUIRE(calls == 1);
}

TEST_CASE("Get Combined Config", "[config]")
{
	gamelink::SDK sdk;
	uint32_t calls = 0;

	sdk.GetCombinedConfig([&](const gamelink::schema::GetCombinedConfigResponse& resp) {
		REQUIRE(resp.data.config.channel["who"] == "down");
		REQUIRE(resp.data.config.extension["what"] == "up");
		calls++;
	});

	sdk.ForeachPayload([](const gamelink::Payload* payload) {
		REQUIRE(JSONEquals(payload->data,
						   R"({
            "action": "get",
            "data": {
                "config_id": "combined"
            },
            "params": {
                "request_id": 65535,
                "target": "config"
            }
        })"));
	});

	const char* msg = R"({
        "data": {
            "config": {
                "foo": "bar"
            },
            "config_id": "channel"
        },
        "meta":{
            "request_id":1,
            "action": "get",
            "target":"config"
        }
    })";
	sdk.ReceiveMessage(msg, strlen(msg));

	const char* msg2 = R"({
        "data": {
            "config": {
                "channel": {
                    "who": "down"
                },
                "extension": {
                    "what": "up"
                }
            },
            "config_id": "combined"
        },
        "meta":{
            "request_id":1,
            "action": "get",
            "target":"config"
        }
    })";
	sdk.ReceiveMessage(msg2, strlen(msg2));
	REQUIRE(calls == 1);
}

TEST_CASE("Subscribe to config", "[config]")
{
	gamelink::SDK sdk;
	uint32_t calls = 0;

	uint32_t updateHandle = sdk.OnConfigUpdate([&](const gamelink::schema::ConfigUpdateResponse& resp) { calls++; });

	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Channel);
	sdk.ForeachPayload([](const gamelink::Payload* payload) {
		REQUIRE(JSONEquals(payload->data,
						   R"({
            "action": "subscribe",
            "data": {
                "config_id": "channel"
            },
            "params": {
                "request_id": 65535,
                "target": "config"
            }
        })"));
	});

	const char* msg = R"({
        "data": {
            "config": {
                "foo": "bar"
            },
            "config_id": "channel"
        },
        "meta":{
            "request_id":1,
            "action": "get",
            "target":"config"
        }
    })";
	sdk.ReceiveMessage(msg, strlen(msg));

	const char* msg2 = R"({
        "data": {
            "channel": {
                "who": "down"
            },
            "extension": {
                "what": "up"
            },
            "config_id": "combined"
        },
        "meta":{
            "request_id":1,
            "action": "get",
            "target":"config"
        }
    })";
	sdk.ReceiveMessage(msg2, strlen(msg2));

	const char* msg3 = R"({
        "data": {
            "config": {
                "show_health": true
            },
            "config_id": "channel"
        },
        "meta": {
            "request_id": 123,
            "action": "update",
            "target": "config"
        }
    })";

	sdk.ReceiveMessage(msg3, strlen(msg3));
	sdk.ReceiveMessage(msg3, strlen(msg3));
	sdk.ReceiveMessage(msg3, strlen(msg3));
	REQUIRE(calls == 3);

	sdk.DetachOnConfigUpdate(updateHandle);
	sdk.ReceiveMessage(msg3, strlen(msg3));
	sdk.ReceiveMessage(msg3, strlen(msg3));
	sdk.ReceiveMessage(msg3, strlen(msg3));
	REQUIRE(calls == 3);
}

TEST_CASE("Update configuration", "[config]")
{
	gamelink::SDK sdk;

	sdk.UpdateChannelConfigWithObject(gamelink::Operation::Add, "/character", nlohmann::json::parse(R"({
		"class": "wizard"
	})"));
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"config": [
				{ "op": "add", "path": "/character", "value": { "class": "wizard" }}
			]
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.UpdateChannelConfigWithBoolean(gamelink::Operation::Add, "/b", false);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"config": [
				{ "op": "add", "path": "/b", "value": false }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.UpdateChannelConfigWithDouble(gamelink::Operation::Add, "/b", 44.15);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"config": [
				{ "op": "add", "path": "/b", "value": 44.15 }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.UpdateChannelConfigWithInteger(gamelink::Operation::Add, "/b", -100);
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"config": [
				{ "op": "add", "path": "/b", "value": -100 }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.UpdateChannelConfigWithLiteral(gamelink::Operation::Add, "/b", R"([{ "literal": "json" }])");
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"config": [
				{ "op": "add", "path": "/b", "value": [
					{ "literal": "json" }
				]}
			]
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.UpdateChannelConfigWithNull(gamelink::Operation::Add, "/b");
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"config": [
				{ "op": "add", "path": "/b", "value": null }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");

	sdk.UpdateChannelConfigWithString(gamelink::Operation::Add, "/b", "Gandalf");
	validateSinglePayload(sdk, R"({
		"action": "patch",
		"data": {
			"config": [
				{ "op": "add", "path": "/b", "value": "Gandalf" }
			]
		},
		"params": {
			"request_id": 65535,
			"target": "config"
		}
	})");
}