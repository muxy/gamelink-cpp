#include "catch2/catch.hpp"
#include "util.h"
#include "gamelink.h"

TEST_CASE("Broadcast returns true from ReceiveMessage", "[sdk]")
{
	gamelink::SDK sdk;
	std::string msg = R"({"meta":{"request_id":16,"action":"broadcast","target":"","timestamp":1624574713916905},"data":{"ok":true}})";
	bool ok = sdk.ReceiveMessage(msg.data(), msg.size());

	REQUIRE(ok);
}

TEST_CASE("Fragmented messages will work", "[sdk]")
{
	gamelink::SDK sdk;
	std::string msg = R"({"meta":{"request_id":16,"action":"broad)";
	bool ok = sdk.ReceiveMessage(msg.data(), msg.size());
	REQUIRE(!ok);

	msg = R"(cast","target":"","timestamp":1624574713916905},"data":{"ok":true}})";
	ok = sdk.ReceiveMessage(msg.data(), msg.size());

	REQUIRE(ok);
}

TEST_CASE("Fragmented drops message", "[sdk]")
{
	gamelink::SDK sdk;

	bool called = false;
	sdk.GetDrops("FULFILLED", [&](const gamelink::schema::GetDropsResponse&){
		called = true;
	});

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

	const char * msg = R"({"meta":{"request_id":1,"action":"get","target":"drops","timestamp":1639247223594798},"data)";
	sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(called == false);

	msg = R"(":{"drops":[{"id":"123","benefit_id":"bag","user_id":"1234","game_id":"1234","fulfillment_status":"CLAIMED","service":"twitch","last_updated":"2021-12-09T18:52:53Z"}]}})";
	sdk.ReceiveMessage(msg, strlen(msg));

	REQUIRE(called == true);
}


TEST_CASE("Fragments in error will recover after successful parse", "[sdk][fragment]")
{
	gamelink::SDK sdk;

	bool called = false;
	sdk.GetDrops("FULFILLED", [&](const gamelink::schema::GetDropsResponse&){
		called = true;
	});

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

	const char * msg = R"({"meta":{"request_id":1,"action":"get","target":"drops","timestamp":1639247223594798},"data)";
	sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(called == false);

	// This clears the broadcast buffer
	msg = R"({"meta":{"request_id":16,"action":"broadcast","target":"","timestamp":1624574713916905},"data":{"ok":true}})";
	sdk.ReceiveMessage(msg, strlen(msg));

	msg = R"(":{"drops":[{"id":"123","benefit_id":"bag","user_id":"1234","game_id":"1234","fulfillment_status":"CLAIMED","service":"twitch","last_updated":"2021-12-09T18:52:53Z"}]}})";
	sdk.ReceiveMessage(msg, strlen(msg));

	REQUIRE(called == false);
}
