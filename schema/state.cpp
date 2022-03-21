#include "schema/state.h"

namespace gamelink
{
	namespace schema
	{
		GetStateRequest::GetStateRequest(const string& target)
		{
			action = string("get");
			params.target = string("state");
			data.state_id = target;
		}

		PatchStateRequest::PatchStateRequest(const string& target)
		{
			action = string("patch");
			params.target = string("state");
			data.state_id = target;
		}

		SubscribeStateRequest::SubscribeStateRequest(const string& target)
		{
			action = string("subscribe");
			params.target = string("state");
			data.topic_id = target;
		}

		UnsubscribeStateRequest::UnsubscribeStateRequest(const string& target)
		{
			action = string("unsubscribe");
			params.target = string("state");
			data.topic_id = target;
		}
	}
}