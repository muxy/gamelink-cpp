#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_PURCHASE_H
#define MUXY_GAMELINK_SCHEMA_PURCHASE_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		template<typename T>
		struct TwitchPurchaseBitsResponseBody
		{
			/// The ID of the purchase, unique for each unique purchase
            string id;

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
			T additional;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_8(TwitchPurchaseBitsResponseBody,
												"id", 
												id,
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

		template<typename T>
		struct TwitchPurchaseBitsResponse : ReceiveEnvelope<TwitchPurchaseBitsResponseBody<T>>
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
	}
}

#endif
