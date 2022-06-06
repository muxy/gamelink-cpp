#include "schema/matchmaking.h"

namespace gamelink
{
	namespace schema
	{
		MatchmakingUpdate::MatchmakingUpdate()
		{}

		SubscribeMatchmakingRequest::SubscribeMatchmakingRequest()
		{
			action = string("subscribe");
			params.target = string("matchmaking");
			data.operation = "pop";
		}

		UnsubscribeMatchmakingRequest::UnsubscribeMatchmakingRequest()
		{
			action = string("unsubscribe");
			params.target = string("matchmaking");
			data.operation = "pop";
		}
	}
}