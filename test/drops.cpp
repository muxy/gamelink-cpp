#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("Get drops", "[drops]")
{
	gamelink::SDK sdk;
	sdk.GetDrops("FULFILLED", [](const gamelink::schema::GetDropsResponse&) {});

	validateSinglePayload(sdk, R"({
		"action": "get",
		"data": {
			"status": "FULFILLED"
		},
		"params": {
			"request_id": 65535,
			"target": "drops"
		}
	})");

	sdk.ValidateDrop("someid");
	validateSinglePayload(sdk, R"({
		"action": "validate",
		"data": {
			"id": "someid"
		},
		"params": {
			"request_id": 65535,
			"target": "drops"
		}
	})");
}

TEST_CASE("Get drops callback", "[drops]")
{
	gamelink::SDK sdk;

	bool called = false;
	sdk.GetDrops("FULFILLED", [&](const gamelink::schema::GetDropsResponse&) { called = true; });

	validateSinglePayload(sdk, R"({
		"action": "get",
		"data": {
			"status": "FULFILLED"
		},
		"params": {
			"request_id": 65535,
			"target": "drops"
		}
	})");

	const char* msg = R"(
		{"meta":{"request_id":1,"action":"get","target":"drops","timestamp":1639247223594798},"data":{"drops":[{"id":"123","benefit_id":"bag","user_id":"1234","game_id":"1234","fulfillment_status":"CLAIMED","service":"twitch","last_updated":"2021-12-09T18:52:53Z"}]}}
	)";

	sdk.ReceiveMessage(msg, strlen(msg));

	REQUIRE(called == true);
}
