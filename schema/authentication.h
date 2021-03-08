#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#define MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#include "schema/envelope.h"


namespace gamelink
{
    namespace schema
    {
        struct SubscribeAuthenticationRequest : SendEnvelope<EmptyBody>
        {
            SubscribeAuthenticationRequest();
        };

        struct SubscribeAuthenticationResponse : ReceiveEnvelope<OKResponse>
        {};
    }
}
#endif