#include "schema/game_config.h"

namespace gamelink
{
    namespace schema
    {
        GetConfigRequest::GetConfigRequest(const char* target)
        {
            action = string("get");
			params.target = string("config");
            data.configId = target;
        }

        SetConfigRequest::SetConfigRequest(const nlohmann::json& js)
        {
            action = string("set");
			params.target = string("config");
            data.config = js;
        }

        SubscribeToConfigRequest::SubscribeToConfigRequest(const char* target)
        {
            action = string("subscribe");
            params.target = string("config");
            data.configId = target;
        }

        UnsubscribeToConfigRequest::UnsubscribeToConfigRequest(const char* target)
        {
            action = string("unsubscribe");
            params.target = string("config");
            data.configId = target;
        }
    }
}