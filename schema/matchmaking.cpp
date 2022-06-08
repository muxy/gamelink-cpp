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
			data.operation = "invite";
		}

		UnsubscribeMatchmakingRequest::UnsubscribeMatchmakingRequest()
		{
			action = string("unsubscribe");
			params.target = string("matchmaking");
			data.operation = "invite";
		}

		ClearMatchmakingRequest::ClearMatchmakingRequest()
		{
			action = string("clear");
			params.target = string("matchmaking");
		}

		RemoveMatchmakingEntryRequest::RemoveMatchmakingEntryRequest(const string& id)
		{
			action = string("remove");
			params.target = string("matchmaking");
			data.id = id;
		}
	}
}