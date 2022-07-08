#include "gamelink.h"

namespace gamelink
{
	Event<schema::PollUpdateResponse>& SDK::OnPollUpdate()
	{
		return _onPollUpdate;
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

	RequestId SDK::CreatePoll(const string& pollId, const string& prompt, const string* start, const string* end)
	{
		std::vector<string> opts(start, end);

		schema::CreatePollRequest packet(pollId, prompt, opts);
		return queuePayload(packet);
	}

	RequestId SDK::CreatePollWithConfiguration(const string& pollId, const string& prompt, const PollConfiguration& config, const std::vector<string>& options)
	{
		schema::CreatePollWithConfigurationRequest packet(pollId, prompt, config, options);
		return queuePayload(packet);
	}

	RequestId SDK::CreatePollWithConfiguration(const string& pollId, const string& prompt, const PollConfiguration& config, const string* start, const string* end)
	{
		std::vector<string> opts(start, end);

		schema::CreatePollWithConfigurationRequest packet(pollId, prompt, config, opts);
		return queuePayload(packet);
	}

	RequestId SDK::SubscribeToPoll(const string& pollId)
	{
		if (_subscriptionSets.canRegisterPoll(pollId))
		{
			schema::SubscribePollRequest packet(pollId);
			RequestId req = queuePayload(packet);

			_subscriptionSets.registerPoll(pollId);
			return req;
		}

		char buffer[128];
		snprintf(buffer, 128, "SubscribeToPoll: duplicated subscription call with target=%s", pollId.c_str());
		InvokeOnDebugMessage(buffer);

		return ANY_REQUEST_ID;
	}

	RequestId SDK::UnsubscribeFromPoll(const string& pollId)
	{
		schema::UnsubscribePollRequest packet(pollId);
		_subscriptionSets.unregisterPoll(pollId);
		return queuePayload(packet);
	}

	RequestId SDK::DeletePoll(const string& pollId)
	{
		schema::DeletePollRequest payload(pollId);
		return queuePayload(payload);
	}

	RequestId SDK::CreateTimedPoll(const string& pollId,
								   const string& prompt,
								   const std::vector<string>& options,
								   float duration,
								   std::function<void(const schema::GetPollResponse&)> onFinishCallback)
	{
		detail::TimedPoll tp(pollId, duration);
		tp.onFinishCallback.set(onFinishCallback);

		_lock.lock();
		_timedPolls.push_back(tp);
		_lock.unlock();
		return SDK::CreatePoll(pollId, prompt, options);
	}

	RequestId SDK::CreateTimedPoll(const string& pollId,
								   const string& prompt,
								   const std::vector<string>& options,
								   float duration,
								   void (*onFinishCallback)(void*, const schema::GetPollResponse&),
								   void* user)
	{
		detail::TimedPoll tp(pollId, duration);
		tp.onFinishCallback.set(onFinishCallback, user);

		_lock.lock();
		_timedPolls.push_back(tp);
		_lock.unlock();
		return SDK::CreatePoll(pollId, prompt, options);
	}

	void SDK::TickTimedPolls(float dt)
	{
		_lock.lock();
		for (auto &tp: _timedPolls)
		{
			tp.duration -= dt;
			if (tp.duration <= 0 && !tp.finished)
			{
				SDK::GetPoll(tp.pollId, [&tp](const schema::GetPollResponse& Resp)
				{
					tp.finished = true;
					tp.onFinishCallback.invoke(Resp);
				});
			}
		}

		auto it = _timedPolls.begin();
		while (it != _timedPolls.end())
		{
			if (it->finished)
			{
				SDK::DeletePoll(it->pollId);
				it = _timedPolls.erase(it);
			}
			else
			{
				++it;
			}
		}
		_lock.unlock();
	}
}
