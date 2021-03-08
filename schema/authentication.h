#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#define MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#include "schema/envelope.h"


namespace gamelink
{
    namespace schema
    {
        namespace bodies
        {
            struct AuthenticateWithPINBody
            {
                string pin;
                string client_id;
            };

            MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithPINBody, 
                "pin", pin, 
                "client_id", client_id
            );

            struct JWTResponseBody
            {
                string jwt;
            };

            MUXY_GAMELINK_SERIALIZE_1(JWTResponseBody, 
                "jwt", jwt
            );
        }

        struct SubscribeAuthenticationRequest : SendEnvelope<bodies::EmptyBody>
        {
            SubscribeAuthenticationRequest();
        };

        struct SubscribeAuthenticationResponse : ReceiveEnvelope<bodies::OKResponseBody>
        {};
        
        struct AuthenticateWithPINRequest : SendEnvelope<bodies::AuthenticateWithPINBody>
        {
            AuthenticateWithPINRequest(const string& clientID, const string& pin);
        };

        struct AuthenticateResponse : ReceiveEnvelope<bodies::JWTResponseBody>
        {};
    }
}
#endif