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
	}
}
