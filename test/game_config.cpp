#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

TEST_CASE("Get config", "[config]")
{
    gamelink::SDK sdk;
    sdk.GetConfig(gamelink::schema::CONFIG_TARGET_CHANNEL, [](const gamelink::schema::GetConfigResponse& resp)
    {});

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

    sdk.GetConfig(gamelink::schema::CONFIG_TARGET_EXTENSION, [](const gamelink::schema::GetConfigResponse& resp)
    {});

    sdk.ForeachPayload([](const gamelink::Payload* payload) {
		REQUIRE(JSONEquals(payload->data,
		R"({
            "action": "get", 
            "data": {
                "config_id": "extension"
            }, 
            "params": {
                "request_id": 65535, 
                "target": "config"
            }
        })"));
	});

    sdk.GetCombinedConfig([](const gamelink::schema::GetCombinedConfigResponse& resp)
    {});

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

    sdk.GetConfig(gamelink::schema::CONFIG_TARGET_CHANNEL, [&](const gamelink::schema::GetConfigResponse& resp)
    {
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

    const char * msg = R"({
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

    const char * msg2 = R"({
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

    sdk.GetCombinedConfig([&](const gamelink::schema::GetCombinedConfigResponse& resp)
    {
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

    const char * msg = R"({
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

    const char * msg2 = R"({
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

    uint32_t updateHandle = sdk.OnConfigUpdate([&](const gamelink::schema::ConfigUpdateResponse& resp)
    {
        calls++;
    });

    sdk.SubscribeToConfigurationChanges(gamelink::schema::CONFIG_TARGET_CHANNEL);
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

    const char * msg = R"({
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

    const char * msg2 = R"({
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

    const char * msg3 = R"({
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