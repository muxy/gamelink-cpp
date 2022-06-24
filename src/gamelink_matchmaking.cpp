#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::SubscribeToMatchmakingQueueInvite()
	{
		if (_subscriptionSets.canRegisterMatchmakingQueueInvite())
		{
			schema::SubscribeMatchmakingRequest payload;
			RequestId req = queuePayload(payload);

			_subscriptionSets.registerMatchmakingQueueInvite();
			return req;
		}

		InvokeOnDebugMessage("SubscribeToMatchmakingQueueInvite: duplicated subscription call");
		return ANY_REQUEST_ID;
	}

	RequestId SDK::UnsubscribeFromMatchmakingQueueInvite()
	{
		schema::UnsubscribeMatchmakingRequest payload;
		_subscriptionSets.unregisterMatchmakingQueueInvite();
		return queuePayload(payload);
	}

	RequestId SDK::ClearMatchmakingQueue()
	{
		schema::ClearMatchmakingRequest payload;
		return queuePayload(payload);
	}

	RequestId SDK::RemoveMatchmakingEntry(const string& id)
	{
		schema::RemoveMatchmakingEntryRequest payload(id);
		return queuePayload(payload);
	}

	Event<schema::MatchmakingUpdate>& SDK::OnMatchmakingQueueInvite()
	{
		return _onMatchmakingUpdate;
	}
}