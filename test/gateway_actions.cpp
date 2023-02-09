#include "catch2/catch.hpp"
#include "util.h"

#include "gateway.h"
#include "gateway_c.h"

TEST_CASE("Set action list", "[gateway][actions]")
{
	gateway::SDK sdk("This is a game");

	gateway::Action drawThreeCards;
	drawThreeCards.Category = gateway::ActionCategory::Help;
	drawThreeCards.Count = gateway::ACTION_INFINITE_USES;
	drawThreeCards.Description = "Draw three cards";
	drawThreeCards.Icon = "mdi:draw";
	drawThreeCards.ID = "draw3";
	drawThreeCards.Impact = 5;
	drawThreeCards.Name = "Ancestral Recall";
	drawThreeCards.State = gateway::ActionState::Available;

	sdk.SetActions(&drawThreeCards, &drawThreeCards + 1);

	REQUIRE(sdk.HasPayloads());

	validateSinglePayload(sdk, R"({
		"action": "set", 
		"params":{
			"request_id": 65535,
			"target": "gatewayActions"
		},
		"data": {
			"actions": [{
				"category": "help", 
				"count": -1, 
				"description": "Draw three cards",
				"icon": "mdi:draw", 
				"sku": "draw3", 
				"impact": 5, 
				"name": "Ancestral Recall", 
				"state": "available"
			}]
		}
	})");
}

TEST_CASE("Set action status", "[gateway][actions]")
{
	gateway::SDK sdk("This is a game");
	sdk.SetActionCount("draw3", 10);
	validateSinglePayload(sdk, R"({
		"action": "set", 
		"params":{
			"request_id": 65535,
			"target": "gatewayActionState"
		},
		"data": {
			"sku": "draw3", 
			"count": 10
		}
	})");

	sdk.DisableAction("draw3");
	validateSinglePayload(sdk, R"({
		"action": "set", 
		"params":{
			"request_id": 65535,
			"target": "gatewayActionState"
		},
		"data": {
			"sku": "draw3", 
			"state": "unavailable"
		}
	})");

	sdk.EnableAction("draw3");
	validateSinglePayload(sdk, R"({
		"action": "set", 
		"params":{
			"request_id": 65535,
			"target": "gatewayActionState"
		},
		"data": {
			"sku": "draw3", 
			"state": "available"
		}
	})");
}