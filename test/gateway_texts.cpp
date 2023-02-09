#include "catch2/catch.hpp"
#include "util.h"

#include "gateway.h"
#include "gateway_c.h"

TEST_CASE("Set text", "[gateway][text]")
{
	gateway::SDK sdk("some game");

	gateway::GameTexts texts;
	texts.Texts.push_back({
		"Matchup", 
		"ZvT", 
		"icon:zvt"
	});

	texts.Texts.push_back({
		"Map", 
		"Vermeer", 
		"icon:map"
	});

	sdk.SetGameTexts(texts);
	validateSinglePayload(sdk, R"({
		"action": "patch", 
		"params": {
			"request_id": 65535, 
			"target": "state"
		},
		"data": {
			"state_id": "channel", 
			"state": [{
				"op": "add", 
				"path": "/game/text/game_data", 
				"value": [{
					"label": "Matchup", 
					"value": "ZvT", 
					"icon": "icon:zvt"
				}, {
					"label": "Map", 
					"value": "Vermeer", 
					"icon": "icon:map"	
				}]
			}]
		}
	})");
}


TEST_CASE("Set text in C", "[gateway][text][c]")
{
	MGW_SDK sdk = MGW_MakeSDK("some game");

	MGW_GameTexts texts;

	MGW_GameText text[2];
	text[0] = MGW_GameText{
		"Matchup", 
		"ZvT", 
		"icon:zvt"
	};

	text[1] = MGW_GameText{
		"Map", 
		"Vermeer", 
		"icon:map"
	};

	texts.Texts = text;
	texts.TextsCount = 2;

	MGW_SDK_SetGameTexts(sdk, &texts);
	validateSinglePayload(sdk, R"({
		"action": "patch", 
		"params": {
			"request_id": 65535, 
			"target": "state"
		},
		"data": {
			"state_id": "channel", 
			"state": [{
				"op": "add", 
				"path": "/game/text/game_data", 
				"value": [{
					"label": "Matchup", 
					"value": "ZvT", 
					"icon": "icon:zvt"
				}, {
					"label": "Map", 
					"value": "Vermeer", 
					"icon": "icon:map"	
				}]
			}]
		}
	})");
}