#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::SubscribeToMatchmakingQueuePop()
	{
		schema::SubscribeMatchmakingRequest payload;
		return queuePayload(payload);
	}

	RequestId SDK::UnsubscribeFromMatchmakingQueuePop()
	{
		schema::UnsubscribeMatchmakingRequest payload;
		return queuePayload(payload);
	}

	uint32_t SDK::OnMatchmakingQueuePop(std::function<void(const schema::MatchmakingUpdate&)> callback)
	{
		return _onMatchmakingUpdate.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnMatchmakingQueuePop(void (callback)(void*, const schema::MatchmakingUpdate&), void* user)
	{
		return _onMatchmakingUpdate.set(callback, user, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnMatchmakingQueuePop(uint32_t handle)
	{
		_onMatchmakingUpdate.remove(handle);
	}
}