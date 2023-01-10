#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("Purchase deserialization", "[purchase][deserialization]")
{
	gamelink::schema::TransactionResponse resp;

	Deserialize(R"({
		"meta": {
			"action": "update",
			"request_id": 3,
			"target": "twitchBitsPurchase"
		},
		"data": {
			"id": "123-512-abwe",
			"muxy_id": "abc",
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
	REQUIRE(resp.data.muxyId == "abc");
	REQUIRE(resp.data.displayName == "Test User");
	REQUIRE(resp.data.userId == "12345");
	REQUIRE(resp.data.userName == "test-user");
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
			"target": "twitchPurchaseBits"
		},
		"data": {
			"id": "123-512-abwe",
			"sku": "test-sku",
			"muxy_id": "abc",
			"currency": "coins",
			"displayName": "Test User",
			"userId": "12345",
			"cost": 42,
			"timestamp": 100,
			"username": "test-user",
			"additional": "extra-data"
		}
	})";

	sdk.OnTransaction().Add([&](gamelink::schema::TransactionResponse resp) {
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

	validateSinglePayload(sdk, R"({
		"action": "subscribe",
		"data": {
			"sku": "spicy-ketchup"
		},
		"params": {
			"request_id": 65535,
			"target": "twitchPurchaseBits"
		}
	})");

	REQUIRE(!sdk.HasPayloads());
}
