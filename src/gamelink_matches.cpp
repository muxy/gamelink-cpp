#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::CreateMatch(const string& id)
	{
		schema::CreateMatchRequest req(id);
		return queuePayload(req);
	}

	RequestId SDK::KeepMatchAlive(const string& id)
	{
		schema::KeepMatchAliveRequest req(id);
		return queuePayload(req);
	}

	RequestId SDK::AddChannelsToMatch(const string& id, const std::vector<string>& channels)
	{
		schema::AddMatchChannelsRequest req(id, channels);
		return queuePayload(req);
	}

	RequestId SDK::RemoveChannelsFromMatch(const string& id, const std::vector<string>& channels)
	{
		schema::RemoveMatchChannelsRequest req(id, channels);
		return queuePayload(req);
	}

	RequestId SDK::StopMatchPoll(const string& id, const string& pollId)
	{
		schema::ExpireMatchPollRequest req(id, pollId);
		return queuePayload(req);
	}

	RequestId SDK::SendMatchBroadcast(const string& matchId, const string& topic, const nlohmann::json& message)
	{
		schema::BroadcastMatchRequest<nlohmann::json> payload(matchId, topic, message);
		return queuePayload(payload);
	}

	RequestId SDK::RunMatchPoll(
		const string& matchId,
		const string& pollId,
		const string& prompt,
		const PollConfiguration& config,
		const std::vector<string>& opts,
		std::function<void(const schema::MatchPollUpdate&)> onUpdateCallback,
		std::function<void(const schema::MatchPollUpdate&)> onFinishCallback)
	{
		schema::DeleteMatchPollRequest delRequest(matchId, pollId);
		RequestId del = queuePayload(delRequest);
		WaitForResponse(del);

		schema::SubscribeMatchPollRequest subRequest(matchId);
		WaitForResponse(queuePayload(subRequest));

		if (!VerifyPollLimits(prompt, opts))
		{
			return gamelink::REJECTED_REQUEST_ID;
		}

		schema::CreatePollWithConfigurationRequest createPollRequest(pollId, prompt, config, opts);
		schema::CreateMatchPollRequest createMatchPollRequest(matchId, createPollRequest.data);

		RequestId create = queuePayload(createMatchPollRequest);

		char buffer[128];
		snprintf(buffer, 128, "<run_match_poll>_%s", pollId.c_str());

		gamelink::string callbackName = gamelink::string(buffer);

		bool hasCalledOnFinish = false;
		_onMatchPollUpdate.AddUnique(callbackName, [=](const schema::MatchPollUpdate& update) mutable
		{

			bool matches = update.data.matchId == matchId && update.data.pollId == pollId;
			if (!matches)
			{
				return;
			}

			if (update.data.overall.status == gamelink::string("expired"))
			{
				if (!hasCalledOnFinish)
				{
					onFinishCallback(update);

					schema::UnsubscribeMatchPollRequest unsubRequest(matchId);
					this->queuePayload(unsubRequest);
					this->_onMatchPollUpdate.RemoveByName(callbackName);
					hasCalledOnFinish = true;
				}
			}
			else
			{
				onUpdateCallback(update);
			}
		});

		return ANY_REQUEST_ID;
	}

	RequestId SDK::RunMatchPoll(
		const string& matchId,
		const string& pollId,
		const string& prompt,
		const PollConfiguration& config,
		const string* optionsBegin,
		const string* optionsEnd,
		std::function<void(const schema::MatchPollUpdate&)> onUpdateCallback,
		std::function<void(const schema::MatchPollUpdate&)> onFinishCallback
	)
	{
		std::vector<string> opts(optionsBegin, optionsEnd);
		return RunMatchPoll(matchId, pollId,  prompt, config, opts, std::move(onUpdateCallback), std::move(onFinishCallback));
	}
}