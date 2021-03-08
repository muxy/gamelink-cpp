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

        AuthenticateWithPINRequest::AuthenticateWithPINRequest(const string& client, const string& pin)
        {
            action = string("authenticate");
            params.target = string("");
            data.pin = pin;
            data.client_id = client;
        }
    }
}