#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("Purchase deserialization", "[purchase][deserialization]")
{
	gamelink::schema::TwitchPurchaseBitsResponse<nlohmann::json> resp;

	Deserialize(R"({
		"meta": {
			"action": "update",
			"request_id": 3,
			"target": "twitchBitsPurchase"
		},
		"data": {
			"id": "123-512-abwe",
			"sku": "test-sku",
			"displayName": "Test User",
			"userId": "12345",
			"cost": 42, 
			"timestamp": 100, 
			"username": "test-user",
			"additional": "extra-data"
		}
	})",
				resp);

	REQUIRE(resp.meta.action == "update");
	REQUIRE(resp.meta.request_id == 3);
	REQUIRE(resp.meta.target == "twitchBitsPurchase");

	REQUIRE(resp.data.sku == "test-sku");
	REQUIRE(resp.data.displayName == "Test User");
	REQUIRE(resp.data.userId == "12345");
	REQUIRE(resp.data.username == "test-user");
	REQUIRE(resp.data.additional == "extra-data");
}

TEST_CASE("SDK Twitch Bits Purchase Response", "[sdk][purchase][twitch]")
{
	gamelink::SDK sdk;

	bool received = false;
	const char* json = R"({
		"meta": {
			"action": "update",
			"request_id": 3,
			"target": "twitchBitsPurchase"
		},
		"data": {
			"id": "123-512-abwe",
			"sku": "test-sku",
			"displayName": "Test User",
			"userId": "12345",
			"cost": 42, 
			"timestamp": 100, 
			"username": "test-user",
			"additional": "extra-data"
		}
	})";

	sdk.OnTwitchPurchaseBits([&](gamelink::schema::TwitchPurchaseBitsResponse<nlohmann::json> resp) {
		received = true;
		SerializeEqual(resp, json);
	});

	sdk.ReceiveMessage(json, strlen(json));

	REQUIRE(received);
}


TEST_CASE("Purchase subsciptions", "[purchase]")
{
	gamelink::SDK sdk;
	sdk.SubscribeToSKU("spicy-ketchup");

	REQUIRE(sdk.HasPayloads());
	sdk.ForeachPayload([](const gamelink::Payload* payload) {
		REQUIRE(JSONEquals(payload->data, R"({
            "action": "subscribe", 
            "data": {
                "topic_id": "spicy-ketchup"
            }, 
            "params": {
                "request_id": 65535, 
                "target": "twitchPurchaseBits"
            }
        })"));
	});
	REQUIRE(!sdk.HasPayloads());
}
