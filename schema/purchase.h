#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_PURCHASE_H
#define MUXY_GAMELINK_SCHEMA_PURCHASE_H
#include "schema/envelope.h"

namespace gamelink
{
	namespace schema
	{
		template<typename T>
		struct TwitchPurchaseBitsResponseBody
		{
			string sku;
			string displayName;
			string userId;
			string username;

			T additional;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_5(TwitchPurchaseBitsResponseBody,
												"sku",
												sku,
												"displayName",
												displayName,
												"userId",
												userId,
												"username",
												username,
												"additional",
												additional);
		};

		template<typename T>
		struct TwitchPurchaseBitsResponse : ReceiveEnvelope<TwitchPurchaseBitsResponseBody<T>>
		{
		};
	}
}

#endif
