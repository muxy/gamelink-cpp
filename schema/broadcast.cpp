#include "config.h"
#include "schema/broadcast.h"

namespace gamelink
{
    namespace schema
    {
        BroadcastRequest::BroadcastRequest(const string& topic, const string& msg)
        {
            action = string("broadcast");
            params.target = string("");

            data.topic = topic;
            data.message = msg;
        }
    }
}