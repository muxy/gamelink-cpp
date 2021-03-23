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
				/// PIN string, as obtained from the REST API
				string pin;

				/// Client ID, as obtained from Twitch.
				string client_id;
			};

			MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithPINBody, "pin", pin, "client_id", client_id);

			struct AuthenticateWithJWTBody
			{
				/// JWT string, as obtained from previous authorizations
				string jwt;

				/// Client ID, as obtained from Twitch.
				string client_id;
			};

			MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithJWTBody, "jwt", jwt, "client_id", client_id);

			struct JWTResponseBody
			{
				/// Signed JWT. Will expire.
				string jwt;
			};

			MUXY_GAMELINK_SERIALIZE_1(JWTResponseBody, "jwt", jwt);
		}

		struct SubscribeAuthenticationRequest : SendEnvelope<bodies::EmptyBody>
		{
			SubscribeAuthenticationRequest();
		};

		struct SubscribeAuthenticationResponse : ReceiveEnvelope<bodies::OKResponseBody>
		{};

		struct AuthenticateWithPINRequest : SendEnvelope<bodies::AuthenticateWithPINBody>
		{
			/// Creates an authorization request.
			/// @param[in] clientID Client ID.
			/// @param[in] pin PIN obtained from user input.
			AuthenticateWithPINRequest(const string& clientID, const string& pin);
		};

		struct AuthenticateWithJWTRequest : SendEnvelope<bodies::AuthenticateWithJWTBody>
		{
			/// Creates an authorization request
			/// @param[in] clientID Client ID.
			/// @param[in] jwt JWT obtained from previous authorizations.
			AuthenticateWithJWTRequest(const string& clientID, const string& jwt);
		};

		struct AuthenticateResponse : ReceiveEnvelope<bodies::JWTResponseBody>
		{};

		class User
		{
		public:
			explicit User(string jwt);

			const string& GetJWT();
			// string GetOpaqueID();

		private:
			string jwt;
		};
	}
}
#endif
