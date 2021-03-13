#include "schema/poll.h"

namespace gamelink
{
	namespace schema
	{
		CreatePollRequest::CreatePollRequest(const string& pollId, const string& prompt, const std::vector<string>& options) {
			action = string("create");
			params.target = string("poll");

			data.pollId = pollId;
			data.prompt = prompt;
			data.options = options;
		}
	}
}
