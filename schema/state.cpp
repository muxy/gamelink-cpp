#include "schema/state.h"

namespace gamelink
{
    namespace schema
    {
        GetStateRequest::GetStateRequest(const char * target)
        {
            action = string("get");
            params.target = string("state");
            data.state_id = string(target);
        }
        
        PatchStateRequest::PatchStateRequest(const char * target)
        {
            action = string("patch");
            params.target = string("state");
            data.state_id = string(target);
        }

        SubscribeStateRequest::SubscribeStateRequest(const char * target)
        {
            action = string("subscribe");
            params.target = string("state");
            data.topic_id = string(target);
        }
    }
}