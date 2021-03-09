#include "schema/state.h"

namespace gamelink
{
    namespace schema
    {
        GetStateRequest::GetStateRequest(const char * target)
        {
            action = string("get");
            params.target = string(target);
        }
        
        UpdateStateRequest::UpdateStateRequest(const char * target)
        {
            action = string("update");
            params.target = string(target);
        }

        SubscribeStateRequest::SubscribeStateRequest(const char * target)
        {
            action = string("subscribe");
            params.target = string("state");
            data.topic_id = string(target);    
        }
    }
}