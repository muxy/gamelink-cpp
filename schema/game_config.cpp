#include "schema/game_config.h"
#include "schema/state.h"

namespace gamelink
{
    namespace schema
    {
		PatchConfigRequest::PatchConfigRequest()
		{
			action = string("patch");
			params.target = string("config");
		}

        GetConfigRequest::GetConfigRequest(ConfigTarget target)
        {
            action = string("get");
			params.target = string("config");
			data.configId = TARGET_STRINGS[static_cast<int>(target)];
        }

        SetConfigRequest::SetConfigRequest(const nlohmann::json& js)
        {
            action = string("set");
			params.target = string("config");
            data.config = js;
        }

        SubscribeToConfigRequest::SubscribeToConfigRequest(ConfigTarget target)
        {
            action = string("subscribe");
            params.target = string("config");
			data.configId = TARGET_STRINGS[static_cast<int>(target)];
        }

        UnsubscribeFromConfigRequest::UnsubscribeFromConfigRequest(ConfigTarget target)
        {
            action = string("unsubscribe");
            params.target = string("config");
			data.configId = TARGET_STRINGS[static_cast<int>(target)];
        }
    }
}