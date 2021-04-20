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
            string id;
			string sku;
			string displayName;
			string userId;
			string username;

			int cost;
            int64_t timestamp;

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
												username,
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
		
		struct SubscribeTransactionsRequest : SendEnvelope<SubscribeTopicRequestBody>
		{
			/// Creates a SubscribeTransactionsRequest
			explicit SubscribeTransactionsRequest(const string& SKU);
		};
	}
}

#endif