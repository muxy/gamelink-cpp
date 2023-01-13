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

TEST_CASE("Get outstanding transactions", "[purchase]")
{
	gamelink::SDK sdk;
	sdk.GetOutstandingTransactions("*", [](const gamelink::schema::GetOutstandingTransactionsResponse& resp)
	{
		REQUIRE(resp.data.transactions.size() == 3);
		REQUIRE(resp.data.transactions[0].displayName == "first user");
		REQUIRE(resp.data.transactions[1].displayName == "second user");
		REQUIRE(resp.data.transactions[2].displayName == "third user");
	});

	const char* json = R"({
		"meta": {
			"action": "get",
			"request_id": 3,
			"target": "transaction"
		},
		"data": {
			"transactions": [
				{
					"id": "123-512-abwe-1",
					"sku": "test-sku",
					"muxy_id": "abc",
					"currency": "coins",
					"displayName": "first user",
					"userId": "12345",
					"cost": 42,
					"timestamp": 100,
					"username": "test-user",
					"additional": "extra-data"
				},
				{
					"id": "123-512-abwe-2",
					"sku": "test-sku",
					"muxy_id": "abc",
					"currency": "coins",
					"displayName": "second user",
					"userId": "12345",
					"cost": 42,
					"timestamp": 100,
					"username": "test-user",
					"additional": "extra-data"
				},
				{
					"id": "123-512-abwe-3",
					"sku": "test-sku",
					"muxy_id": "abc",
					"currency": "coins",
					"displayName": "third user",
					"userId": "12345",
					"cost": 42,
					"timestamp": 100,
					"username": "test-user",
					"additional": "extra-data"
				}
			]
		}
	})";


	sdk.ReceiveMessage(json, strlen(json));
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

TEST_CASE("Refund request by txid", "[purchase]")
{
	gamelink::SDK sdk;
	sdk.RefundTransactionByID("some-tx-id", "some-user-id");
	
	validateSinglePayload(sdk, R"({
		"action": "refund",
		"data": {
			"transaction_id": "some-tx-id",
			"user_id": "some-user-id"
		},
		"params": {
			"request_id": 65535,
			"target": "transaction"
		}
	})");

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("Refund request by sku", "[purchase]")
{
	gamelink::SDK sdk;
	sdk.RefundTransactionBySKU("some-sku", "some-user-id");
	
	validateSinglePayload(sdk, R"({
		"action": "refund",
		"data": {
			"sku": "some-sku",
			"user_id": "some-user-id"
		},
		"params": {
			"request_id": 65535,
			"target": "transaction"
		}
	})");

	REQUIRE(!sdk.HasPayloads());
}

TEST_CASE("Validate request", "[purchase]")
{
	gamelink::SDK sdk;
	sdk.ValidateTransaction("some-tx-id", "Human readable description");
	
	validateSinglePayload(sdk, R"({
		"action": "validate",
		"data": {
			"transaction_id": "some-tx-id",
			"details": "Human readable description"
		},
		"params": {
			"request_id": 65535,
			"target": "transaction"
		}
	})");

	REQUIRE(!sdk.HasPayloads());
}