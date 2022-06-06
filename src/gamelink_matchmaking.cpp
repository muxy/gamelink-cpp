#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::SubscribeToMatchmakingQueueInvite()
	{
		schema::SubscribeMatchmakingRequest payload;
		return queuePayload(payload);
	}

	RequestId SDK::UnsubscribeFromMatchmakingQueueInvite()
	{
		schema::UnsubscribeMatchmakingRequest payload;
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

	uint32_t SDK::OnMatchmakingQueueInvite(std::function<void(const schema::MatchmakingUpdate&)> callback)
	{
		return _onMatchmakingUpdate.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnMatchmakingQueueInvite(void (callback)(void*, const schema::MatchmakingUpdate&), void* user)
	{
		return _onMatchmakingUpdate.set(callback, user, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnMatchmakingQueueInvite(uint32_t handle)
	{
		_onMatchmakingUpdate.remove(handle);
	}
}