#include "schema/state.h"

namespace gamelink
{
	namespace schema
	{
		GetStateRequest::GetStateRequest(StateTarget target)
		{
			action = string("get");
			params.target = string("state");
			data.state_id = string(TARGET_STRINGS[static_cast<int>(target)]);

		}

		PatchStateRequest::PatchStateRequest(StateTarget target)
		{
			action = string("patch");
			params.target = string("state");
			data.state_id = string(TARGET_STRINGS[static_cast<int>(target)]);
		}

		SubscribeStateRequest::SubscribeStateRequest(StateTarget target)
		{
			action = string("subscribe");
			params.target = string("state");
			data.topic_id = string(TARGET_STRINGS[static_cast<int>(target)]);
		}

		UnsubscribeStateRequest::UnsubscribeStateRequest(StateTarget target)
		{
			action = string("unsubscribe");
			params.target = string("state");
			data.topic_id = string(TARGET_STRINGS[static_cast<int>(target)]);
		}
	}
}