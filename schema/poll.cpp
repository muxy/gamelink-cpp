#include "schema/poll.h"

namespace gamelink
{
	namespace schema
	{
		GetPollRequest::GetPollRequest(const string& pollId)
		{
			action = string("get");
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

		SubscribePollRequest::SubscribePollRequest(const string& pollId)
		{
			action = string("subscribe");
			params.target = string("poll");
			data.topic_id = string(pollId);
		}
	}
}
