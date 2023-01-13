#include "catch2/catch.hpp"
#include "gamelink.h"
#include "util.h"

TEST_CASE("Reconnect will push an authentication request to the start of the queue", "[sdk][reconnect]")
{
	gamelink::SDK sdk;
	sdk.AuthenticateWithPIN("client_id", "1234");
	sdk.ForeachPayload([](const gamelink::Payload*){});

	REQUIRE(!sdk.IsAuthenticated());
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt",
			"refresh": "refresh"
		}
	})";

	bool ok = sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(ok);
	REQUIRE(sdk.IsAuthenticated());

	sdk.HandleReconnect();
	validateSinglePayload(sdk, R"({
		"action":"authenticate",
		"data":{
			"client_id":"client_id",
			"refresh":"refresh"
		},
		"params":{
			"request_id":65535
		}
	})");
}

TEST_CASE("Reconnect will push an authentication request to the start of the queue with game id", "[sdk][reconnect]")
{
	gamelink::SDK sdk;
	sdk.AuthenticateWithGameIDAndPIN("client_id", "rock paper scissors", "1234");
	sdk.ForeachPayload([](const gamelink::Payload*){});

	REQUIRE(!sdk.IsAuthenticated());
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt",
			"refresh": "refresh"
		}
	})";

	bool ok = sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(ok);
	REQUIRE(sdk.IsAuthenticated());

	sdk.HandleReconnect();
	validateSinglePayload(sdk, R"({
		"action":"authenticate",
		"data":{
			"client_id":"client_id",
			"refresh":"refresh",
			"game_id": "rock paper scissors"
		},
		"params":{
			"request_id":65535
		}
	})");
}

TEST_CASE("Reconnect will redo subscriptions", "[sdk][reconnect]")
{
	gamelink::SDK sdk;
	sdk.AuthenticateWithPIN("client_id", "1234");
	sdk.ForeachPayload([](const gamelink::Payload*){});

	REQUIRE(!sdk.IsAuthenticated());
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt",
			"refresh": "refresh"
		}
	})";

	bool ok = sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(ok);
	REQUIRE(sdk.IsAuthenticated());

	sdk.SubscribeToAllPurchases();
	sdk.SubscribeToDatastream();
	sdk.SubscribeToDatastream();
	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Channel);
	sdk.SubscribeToMatchmakingQueueInvite();
	sdk.SubscribeToPoll("what-toppings");
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Extension);
	sdk.ForeachPayload([](const gamelink::Payload*){});

	sdk.HandleReconnect();

	std::vector<std::string> payloads;
	// Expect 7 payloads:
	// The authentication payload
	// The SKU Sub target=*
	// The datastream sub
	// The state sub (channel)
	// The config sub (extension)
	// The poll sub (what-toppings)
	// The matchmaking sub
	sdk.ForeachPayload([&payloads](const gamelink::Payload* payload)
	{
		payloads.push_back(payload->Data());
	});

	std::vector<std::string> expectedPayloads = {
		// Authentication
		R"({
			"action":"authenticate",
			"data":{
				"client_id":"client_id",
				"refresh":"refresh"
			},
			"params":{
				"request_id":65535
			}
		})",

		R"({"action":"subscribe","data":{"sku":"*"},
			"params":{"request_id":65535,"target":"twitchPurchaseBits"}})",

		R"({"action":"subscribe","data":{"topic_id":"what-toppings"},
			"params":{"request_id":65535,"target":"poll"}})",

		R"({"action":"subscribe","data":{"config_id":"extension"},
			"params":{"request_id":65535,"target":"config"}})",

		R"({"action":"subscribe","data":{"topic_id":"channel"},
			"params":{"request_id":65535,"target":"state"}})",

		R"({"action":"subscribe","data":{"topic_id":""},
			"params":{"request_id":65535,"target":"datastream"}})",

		R"({"action":"subscribe","data":{"operation":"invite"},
			"params":{"request_id":65535,"target":"matchmaking"}})",
	};

	// Ensure we have the full set of expected responses.
	for (size_t i = 0; i < payloads.size(); ++i)
	{
		bool hasExpected = false;
		for (size_t j = 0; j < expectedPayloads.size(); ++j)
		{
			if (JSONEquals(
				ConstrainedString(payloads[i].c_str()),
				ConstrainedString(expectedPayloads[i].c_str())))
			{
				hasExpected = true;
				break;
			}
		}

		REQUIRE(hasExpected);
	}
}

TEST_CASE("Reconnect will redo subscriptions, but not after unsubscribing to everything", "[sdk][reconnect]")
{
	gamelink::SDK sdk;
	sdk.AuthenticateWithPIN("client_id", "1234");
	sdk.ForeachPayload([](const gamelink::Payload*){});

	REQUIRE(!sdk.IsAuthenticated());
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt",
			"refresh": "refresh"
		}
	})";

	bool ok = sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(ok);
	REQUIRE(sdk.IsAuthenticated());

	sdk.SubscribeToAllPurchases();
	sdk.SubscribeToDatastream();
	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Channel);
	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Extension);
	sdk.SubscribeToMatchmakingQueueInvite();
	sdk.SubscribeToPoll("what-toppings");
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Channel);
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Extension);

	// Duplicate requests, these should all be deduped by the sdk.
	sdk.SubscribeToAllPurchases();
	sdk.SubscribeToDatastream();
	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Channel);
	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Extension);
	sdk.SubscribeToMatchmakingQueueInvite();
	sdk.SubscribeToPoll("what-toppings");
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Channel);
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Extension);

	uint32_t subscribeCount = 0;
	sdk.ForeachPayload([&](const gamelink::Payload*){
		subscribeCount++;
	});

	sdk.UnsubscribeFromAllPurchases();
	sdk.UnsubscribeFromDatastream();
	sdk.UnsubscribeFromStateUpdates(gamelink::StateTarget::Channel);
	sdk.UnsubscribeFromStateUpdates(gamelink::StateTarget::Extension);
	sdk.UnsubscribeFromMatchmakingQueueInvite();
	sdk.UnsubscribeFromPoll("what-toppings");
	sdk.UnsubscribeFromConfigurationChanges(gamelink::ConfigTarget::Channel);
	sdk.UnsubscribeFromConfigurationChanges(gamelink::ConfigTarget::Extension);
	
	uint32_t unsubscribeCount = 0;
	sdk.ForeachPayload([&](const gamelink::Payload*){
		unsubscribeCount++;
	});

	REQUIRE(subscribeCount == unsubscribeCount);
	
	sdk.HandleReconnect();
	validateSinglePayload(sdk, R"({
		"action":"authenticate",
		"data":{
			"client_id":"client_id",
			"refresh":"refresh"
		},
		"params":{
			"request_id":65535
		}
	})");
}

TEST_CASE("Reconnect will redo subscriptions, but not after unsubscribing", "[sdk][reconnect]")
{
	gamelink::SDK sdk;
	sdk.AuthenticateWithPIN("client_id", "1234");
	sdk.ForeachPayload([](const gamelink::Payload*){});

	REQUIRE(!sdk.IsAuthenticated());
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt",
			"refresh": "refresh"
		}
	})";

	bool ok = sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(ok);
	REQUIRE(sdk.IsAuthenticated());

	sdk.SubscribeToAllPurchases();
	sdk.SubscribeToDatastream();
	sdk.SubscribeToDatastream();
	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Channel);
	sdk.SubscribeToMatchmakingQueueInvite();
	sdk.SubscribeToPoll("what-toppings");
	sdk.SubscribeToConfigurationChanges(gamelink::ConfigTarget::Extension);
	sdk.UnsubscribeFromAllPurchases();

	// "send" all the payloads
	sdk.ForeachPayload([](const gamelink::Payload*){});
	sdk.HandleReconnect();

	std::vector<std::string> payloads;
	// Expect 6 payloads:
	// The authentication payload
	// The datastream sub
	// The state sub (channel)
	// The config sub (extension)
	// The poll sub (what-toppings)
	// The matchmaking sub
	sdk.ForeachPayload([&payloads](const gamelink::Payload* payload)
	{
		payloads.push_back(payload->Data());
	});

	std::vector<std::string> expectedPayloads = {
		// Authentication
		R"({
			"action":"authenticate",
			"data":{
				"client_id":"client_id",
				"refresh":"refresh"
			},
			"params":{
				"request_id":65535
			}
		})",

		R"({"action":"subscribe","data":{"topic_id":"what-toppings"},
			"params":{"request_id":65535,"target":"poll"}})",

		R"({"action":"subscribe","data":{"config_id":"extension"},
			"params":{"request_id":65535,"target":"config"}})",

		R"({"action":"subscribe","data":{"topic_id":"channel"},
			"params":{"request_id":65535,"target":"state"}})",

		R"({"action":"subscribe","data":{"topic_id":""},
			"params":{"request_id":65535,"target":"datastream"}})",

		R"({"action":"subscribe","data":{"operation":"invite"},
			"params":{"request_id":65535,"target":"matchmaking"}})",
	};

	// Ensure we have the full set of expected responses.
	for (size_t i = 0; i < payloads.size(); ++i)
	{
		bool hasExpected = false;
		for (size_t j = 0; j < expectedPayloads.size(); ++j)
		{
			if (JSONEquals(
				ConstrainedString(payloads[i].c_str()),
				ConstrainedString(expectedPayloads[i].c_str())))
			{
				hasExpected = true;
				break;
			}
		}

		REQUIRE(hasExpected);
	}
}