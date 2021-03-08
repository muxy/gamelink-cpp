#include "schema/authentication.h"

namespace gamelink
{
    namespace schema
    {
        SubscribeAuthenticationRequest::SubscribeAuthenticationRequest()
        {
            action = string("subscribe");
            params.target = string("authentication");
        }
    }
}