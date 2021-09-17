#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_PURCHASE_H
#define MUXY_GAMELINK_SCHEMA_PURCHASE_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		struct Transaction
		{
			/// The External ID of the purchase, unique for each unique purchase, and service dependent.
			string id;

			/// The Muxy ID of this transaction. Used to make additional calls regarding this purchase.
			string muxyId;

			/// SKU of the item
			string sku;

			/// Human readable display name of the product
			string displayName;

			/// UserID of the user who purchased the product
			string userId;

			/// Human readable username of the user who purchased the product
			string userName;

			/// Cost in coins of the product
			int cost;

			/// Millisecond unix timestamp of the purchase.
			int64_t timestamp;

			/// Arbitrary additional data, added by the extension to this purchase receipt.
			nlohmann::json additional;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_9(Transaction,
												"id",
												id,
												"muxy_id",
												muxyId,
												"sku",
												sku,
												"displayName",
												displayName,
												"userId",
												userId,
												"username",
												userName,
												"cost",
												cost,
												"timestamp",
												timestamp,
												"additional",
												additional);
		};

		struct TransactionResponse : ReceiveEnvelope<Transaction>
		{
		};

		struct GetOutsandingTransactionsRequestBody
		{
			string sku;
			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetOutsandingTransactionsRequestBody, "sku", sku);
		};

		struct GetOutstandingTransactionsRequest : SendEnvelope<GetOutsandingTransactionsRequestBody>
		{
			explicit GetOutstandingTransactionsRequest(const string& sku);
		};

		struct GetOutstandingTransactionsResponseBody
		{
			std::vector<Transaction> transactions;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetOutstandingTransactionsResponseBody, "transactions", transactions);
		};

		struct GetOutstandingTransactionsResponse : ReceiveEnvelope<GetOutstandingTransactionsResponseBody>
		{
		};

		struct SubscribePurchaseRequestBody
		{
			string sku;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SubscribePurchaseRequestBody, "sku", sku);
		};

		struct UnsubscribePurchaseRequestBody
		{
			string sku;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(UnsubscribePurchaseRequestBody, "sku", sku);
		};

		struct SubscribeTransactionsRequest : SendEnvelope<SubscribePurchaseRequestBody>
		{
			/// Creates a SubscribeTransactionsRequest
			explicit SubscribeTransactionsRequest(const string& SKU);
		};

		struct UnsubscribeTransactionsRequest : SendEnvelope<UnsubscribePurchaseRequestBody>
		{
			/// Creates an UnsubscribeTransactionsRequest
			explicit UnsubscribeTransactionsRequest(const string& SKU);
		};

		struct RefundTransactionBody
		{
			string transactionId;
			string userId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(RefundTransactionBody, "transaction_id", transactionId, "user_id", userId);
		};

		struct RefundTransactionRequest : SendEnvelope<RefundTransactionBody>
		{
			RefundTransactionRequest(const string& transactionId, const string& userId);
		};

		struct RefundTransactionBySKUBody
		{
			string SKU;
			string userId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(RefundTransactionBySKUBody, "sku", SKU, "user_id", userId);
		};

		struct RefundTransactionBySKURequest : SendEnvelope<RefundTransactionBySKUBody>
		{
			RefundTransactionBySKURequest(const string& sku, const string& userId);
		};

		struct ValidateTransactionBody
		{
			string transactionId;
			string details;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ValidateTransactionBody, "transaction_id", transactionId, "details", details);
		};

		struct ValidateTransactionRequest : SendEnvelope<ValidateTransactionBody>
		{
			ValidateTransactionRequest(const string& transactionId, const string& details);
		};
	}
}

#endif
