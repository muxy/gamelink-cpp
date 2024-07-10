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
		if (!VerifyPollLimits(prompt, options))
		{
			return gamelink::REJECTED_REQUEST_ID;
		}

		schema::CreatePollRequest packet(pollId, prompt, options);
		return queuePayload(packet);
	}

	RequestId SDK::CreatePoll(const string& pollId, const string& prompt, const string* start, const string* end)
	{
		std::vector<string> opts(start, end);

		if (!VerifyPollLimits(prompt, opts))
		{
			return gamelink::REJECTED_REQUEST_ID;
		}

		schema::CreatePollRequest packet(pollId, prompt, opts);
		return queuePayload(packet);
	}

	RequestId SDK::CreatePollWithConfiguration(const string& pollId, const string& prompt, const PollConfiguration& config, const std::vector<string>& options)
	{
		if (!VerifyPollLimits(prompt, options))
		{
			return gamelink::REJECTED_REQUEST_ID;
		}

		schema::CreatePollWithConfigurationRequest packet(pollId, prompt, config, options);
		return queuePayload(packet);
	}

	RequestId SDK::CreatePollWithConfiguration(const string& pollId, const string& prompt, const PollConfiguration& config, const string* start, const string* end)
	{
		std::vector<string> opts(start, end);

		if (!VerifyPollLimits(prompt, opts))
		{
			return gamelink::REJECTED_REQUEST_ID;
		}

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

	RequestId SDK::RunPoll(
		const string& pollId,
		const string& prompt,
		const PollConfiguration& config,
		const string* optionsBegin,
		const string* optionsEnd,
		std::function<void(const schema::PollUpdateResponse&)> onUpdateCallback,
		std::function<void(const schema::PollUpdateResponse&)> onFinishCallback)
	{
		RequestId del = DeletePoll(pollId);
		RequestId unsub = UnsubscribeFromPoll(pollId);

		WaitForResponse(del);
		WaitForResponse(unsub);

		SubscribeToPoll(pollId);
		RequestId result = CreatePollWithConfiguration(pollId, prompt, config, optionsBegin, optionsEnd);

		char buffer[128];
		snprintf(buffer, 128, "<runpoll>_%s", pollId.c_str());

		gamelink::string callbackName = gamelink::string(buffer);

		bool hasCalledOnFinish = false;
		OnPollUpdate().AddUnique(callbackName, [this, callbackName, pollId, onUpdateCallback, onFinishCallback, hasCalledOnFinish](const schema::PollUpdateResponse& resp) mutable
		{
			// The requirements on string type overloading don't require operator !=
			if (!(resp.data.poll.pollId == pollId))
			{
				return;
			}

			if (resp.data.poll.status == gamelink::string("expired"))
			{
				if (!hasCalledOnFinish)
				{
					onFinishCallback(resp);
					this->UnsubscribeFromPoll(pollId);
					this->OnPollUpdate().RemoveByName(callbackName);
					hasCalledOnFinish = true;
				}
			}
			else
			{
				onUpdateCallback(resp);
			}
		});

		return result;
	}

	RequestId SDK::RunPoll(
		const string& pollId,
		const string& prompt,
		const PollConfiguration& config,
		const std::vector<string>& options,
		std::function<void(const schema::PollUpdateResponse&)> onUpdateCallback,
		std::function<void(const schema::PollUpdateResponse&)> onFinishCallback)
	{
		if (options.size() == 0)
		{
			return gamelink::REJECTED_REQUEST_ID;
		}

		const string* begin = nullptr;
		if (!options.empty())
		{
			begin = options.data();
		}

		const string* end = nullptr;
		if (!options.empty())
		{
			end = options.data() + options.size();
		}

		return RunPoll(
			pollId,
			prompt,
			config,
			begin, end,
			std::move(onUpdateCallback),
			std::move(onFinishCallback)
		);
	}

	RequestId SDK::RunPoll(const string& pollId,
		const string& prompt,
		const PollConfiguration& config,
		const string* optionsBegin,
		const string* optionsEnd,
		void (*onUpdateCallback)(void*, const schema::PollUpdateResponse&),
		void (*onFinishCallback)(void*, const schema::PollUpdateResponse&),
		void* user)
	{
		return RunPoll(
			pollId,
			prompt,
			config,
			optionsBegin,
			optionsEnd,
			[=](const schema::PollUpdateResponse& resp)
			{
				onUpdateCallback(user, resp);
			},
			[=](const schema::PollUpdateResponse& resp)
			{
				onFinishCallback(user, resp);
			}
		);
	}

	RequestId SDK::StopPoll(const string& pollId)
	{
		schema::ExpirePollRequest req(pollId);
		return queuePayload(req);
	}

	RequestId SDK::SetPollDisabled(const string& pollId, bool disabled)
	{
		schema::SetPollDisabledStatusRequest req(pollId, disabled);
		return queuePayload(req);
	}
}
