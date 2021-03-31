#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#define MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#include "schema/envelope.h"

namespace gamelink
{
	namespace schema
	{
		struct AuthenticateWithPINRequestBody
		{
			/// PIN string, as obtained from the REST API
			string pin;

			/// Client ID, as obtained from Twitch.
			string client_id;
		};
		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithPINRequestBody, "pin", pin, "client_id", client_id);

		struct AuthenticateWithJWTRequestBody
		{
			/// JWT string, as obtained from previous authorizations
			string jwt;

			/// Client ID, as obtained from Twitch.
			string client_id;
		};
		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithJWTRequestBody, "jwt", jwt, "client_id", client_id);

		struct AuthenticateJWTResponseBody
		{
			/// Signed JWT. Will expire.
			string jwt;
		};
		MUXY_GAMELINK_SERIALIZE_1(AuthenticateJWTResponseBody, "jwt", jwt);

		struct SubscribeAuthenticationRequest : SendEnvelope<EmptyBody>
		{
			SubscribeAuthenticationRequest();
		};

		struct SubscribeAuthenticationResponse : ReceiveEnvelope<OKResponseBody>
		{};

		struct AuthenticateWithPINRequest : SendEnvelope<AuthenticateWithPINRequestBody>
		{
			/// Creates an authorization request.
			/// @param[in] clientID Client ID.
			/// @param[in] pin PIN obtained from user input.
			AuthenticateWithPINRequest(const string& clientID, const string& pin);
		};

		struct AuthenticateWithJWTRequest : SendEnvelope<AuthenticateWithJWTRequestBody>
		{
			/// Creates an authorization request
			/// @param[in] clientID Client ID.
			/// @param[in] jwt JWT obtained from previous authorizations.
			AuthenticateWithJWTRequest(const string& clientID, const string& jwt);
		};

		struct AuthenticateResponse : ReceiveEnvelope<AuthenticateJWTResponseBody>
		{};

		class User
		{
		public:
			explicit User(string jwt);

			const string& GetJWT() const;
			// string GetOpaqueID();
		private:
			string jwt;
		};
	}
}
#endif
