#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_MATCHMAKING_H
#define MUXY_GAMELINK_SCHEMA_MATCHMAKING_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		struct MUXY_GAMELINK_API MatchmakingOperationBody
		{
			string operation;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(MatchmakingOperationBody, "operation", operation);
		};

		template<typename T>
		struct MUXY_GAMELINK_API MatchmakingInformation
		{
			T data;
			string twitchUsername;
			string twitchID;

			int64_t timestamp;
			bool isFollower;
			int subscriptionTier;
			int bitsSpent;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_7(MatchmakingInformation,
				"data", data,
				"twitch_username", twitchUsername,
				"twitch_id", twitchID,
				"timestamp", timestamp,
				"is_follower", isFollower,
				"subscription_tier", subscriptionTier,
				"bits_spent", bitsSpent
			);
		};

		struct MUXY_GAMELINK_API MatchmakingUpdate : ReceiveEnvelope<MatchmakingInformation<nlohmann::json> >
		{
			MatchmakingUpdate();
		};

		struct MUXY_GAMELINK_API SubscribeMatchmakingRequest : SendEnvelope<MatchmakingOperationBody>
		{
			/// Creates a SubscribeMatchmakingRequest.
			SubscribeMatchmakingRequest();
		};

		struct MUXY_GAMELINK_API UnsubscribeMatchmakingRequest : SendEnvelope<MatchmakingOperationBody>
		{
			/// Creates an UnsubscribeMatchmakingRequest.
			UnsubscribeMatchmakingRequest();
		};
	}
}

#endif
