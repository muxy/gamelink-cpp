#include "gamelink.h"

namespace gamelink
{
    uint32_t SDK::OnPollUpdate(std::function<void(const schema::PollUpdateResponse& pollResponse)> callback)
	{
		return _onPollUpdate.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnPollUpdate(void (*callback)(void*, const schema::PollUpdateResponse&), void* ptr)
	{
		return _onPollUpdate.set(callback, ptr, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnPollUpdate(uint32_t id)
	{
		if (_onPollUpdate.validateId(id))
		{
			_onPollUpdate.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnPollUpdate");
		}
	}

	RequestId SDK::GetPoll(const string& pollId)
	{
		schema::GetPollRequest packet(pollId);
		return queuePayload(packet);
	}

	RequestId SDK::GetPoll(const string& pollId, std::function<void(const schema::GetPollResponse&)> callback)
	{
		schema::GetPollRequest payload(pollId);
		
		RequestId id = queuePayload(payload);
		_onGetPoll.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::GetPoll(const string& pollId, void (*callback)(void*, const schema::GetPollResponse&), void* user)
	{
		schema::GetPollRequest payload(pollId);

		RequestId id = queuePayload(payload);
		_onGetPoll.set(callback, user, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::CreatePoll(const string& pollId, const string& prompt, const std::vector<string>& options)
	{
		schema::CreatePollRequest packet(pollId, prompt, options);
		return queuePayload(packet);
	}

	RequestId SDK::UnsubscribeFromPoll(const string& pollId)
	{
		schema::UnsubscribePollRequest packet(pollId);
		return queuePayload(packet);
	}

	RequestId SDK::SubscribeToPoll(const string& pollId)
	{
		schema::SubscribePollRequest packet(pollId);
		return queuePayload(packet);
	}

	RequestId SDK::DeletePoll(const string& pollId)
	{
		schema::DeletePollRequest payload(pollId);
		return queuePayload(payload);
	}
}