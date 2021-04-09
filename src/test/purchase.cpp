#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

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
			"sku": "test-sku",
			"displayName": "Test User",
			"userId": "12345",
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
