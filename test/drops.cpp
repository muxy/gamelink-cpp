#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include <iostream>

TEST_CASE("Get drops", "[drops]")
{
	gamelink::SDK sdk;
	sdk.GetDrops("FULFILLED", [](const gamelink::schema::GetDropsResponse&){});

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