#include "schema/poll.h"

namespace gamelink
{
	PollConfiguration::PollConfiguration()
		:userIdVoting(false)
		,distinctOptionsPerUser(1)
		,totalVotesPerUser(1)
		,votesPerOption(1)
		,disabled(false)
		,startsAt(0)
		,endsAt(0)
		,startsIn(0)
		,endsIn(0)
	{}

	namespace schema
	{
		GetPollRequest::GetPollRequest(const string& pollId)
		{
			action = string("get");
			params.target = string("poll");

			data.pollId = pollId;
		}

		DeletePollRequest::DeletePollRequest(const string& pollId)
		{
			action = string("delete");
			params.target = string("poll");

			data.pollId = pollId;
		}

		CreatePollRequest::CreatePollRequest(const string& pollId, const string& prompt, const std::vector<string>& options)
		{
			action = string("create");
			params.target = string("poll");

			data.pollId = pollId;
			data.prompt = prompt;
			data.options = options;
		}

		CreatePollWithConfigurationRequest::CreatePollWithConfigurationRequest(const string& pollId, const string& prompt, const PollConfiguration& config, const std::vector<string>& options)
		{
			action = string("create");
			params.target = string("poll");

			data.pollId = pollId;
			data.prompt = prompt;
			data.options = options;
			data.configuration = config;
		}

		SubscribePollRequest::SubscribePollRequest(const string& pollId)
		{
			action = string("subscribe");
			params.target = string("poll");
			data.topic_id = string(pollId);
		}

		UnsubscribePollRequest::UnsubscribePollRequest(const string& pollId)
		{
			action = string("unsubscribe");
			params.target = string("poll");
			data.topic_id = string(pollId);
		}

		ExpirePollRequest::ExpirePollRequest(const string& pollId)
		{
			action = string("reconfigure");
			params.target = string("poll");
			data.pollId = pollId;
			data.config.endsAt = -1;
		}

		SetPollDisabledStatusRequest::SetPollDisabledStatusRequest(const string& pollId, bool status)
		{
			action = string("reconfigure");
			params.target = string("poll");
			data.pollId = pollId;
			data.config.disabled = status;
		}
	}
}
