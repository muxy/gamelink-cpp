#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("Send subscribe and unsubscribe from matchmaking", "[matchmaking]")
{
	gamelink::SDK sdk;

	sdk.SubscribeToMatchmakingQueueInvite();
	validateSinglePayload(sdk, R"({
		"action": "subscribe",
		"data": {
			"operation": "invite"
		},
		"params": {
			"request_id": 65535,
			"target": "matchmaking"
		}
	})");

	sdk.UnsubscribeFromMatchmakingQueueInvite();
	validateSinglePayload(sdk, R"({
		"action": "unsubscribe",
		"data": {
			"operation": "invite"
		},
		"params": {
			"request_id": 65535,
			"target": "matchmaking"
		}
	})");

	sdk.ClearMatchmakingQueue();
	validateSinglePayload(sdk, R"({
		"action": "clear",
		"params": {
			"request_id": 65535,
			"target": "matchmaking"
		}
	})");

	sdk.RemoveMatchmakingEntry("foobar");
	validateSinglePayload(sdk, R"({
		"action": "remove",
		"data": {
			"id": "foobar"
		},
		"params": {
			"request_id": 65535,
			"target": "matchmaking"
		}
	})");
}

TEST_CASE("Matchmaking callback is invoked with queryable data", "[matchmaking]")
{
	gamelink::SDK sdk;

	bool called = false;
	sdk.OnMatchmakingQueueInvite().Add([&](const gamelink::schema::MatchmakingUpdate& update)
	{
		REQUIRE(update.data.data["mmid"] == "some-matchmaking-id");
		REQUIRE(update.data.bitsSpent == 12);
		REQUIRE(update.data.timestamp == 1654550123);
		REQUIRE(update.data.isFollower == true);
		REQUIRE(update.data.subscriptionTier == 1);

		called = true;
	});

	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "update",
			"target": "matchmaking",
			"timestamp": 1639247223594798
		},
		"data": {
			"data": {
				"mmid": "some-matchmaking-id"
			},
			"bits_spent": 12,
			"timestamp": 1654550123,
			"is_follower": true,
			"subscription_tier": 1
		}
	})";

	sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(called == true);
}