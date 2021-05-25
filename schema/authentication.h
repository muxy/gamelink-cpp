#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#define MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#include "schema/envelope.h"

namespace gamelink
{
	namespace schema
	{

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct AuthenticateWithPINRequestBody
		{
			/// PIN string, as obtained from the REST API
			string pin;

			/// Client ID, as obtained from Twitch.
			string client_id;
		};
		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithPINRequestBody, "pin", pin, "client_id", client_id);
		
		struct AuthenticateWithPINRequest : SendEnvelope<AuthenticateWithPINRequestBody>
		{
			/// Creates an authorization request.
			/// @param[in] clientId Client ID.
			/// @param[in] pin PIN obtained from user input.
			AuthenticateWithPINRequest(const string& clientId, const string& pin);
		};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct AuthenticateWithJWTRequestBody
		{
			/// JWT string, as obtained from previous authorizations
			string jwt;

			/// Client ID, as obtained from Twitch.
			string client_id;
		};
		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithJWTRequestBody, "jwt", jwt, "client_id", client_id);

		struct AuthenticateWithJWTRequest : SendEnvelope<AuthenticateWithJWTRequestBody>
		{
			/// Creates an authorization request
			/// @param[in] clientId Client ID.
			/// @param[in] jwt JWT obtained from previous authorizations.
			AuthenticateWithJWTRequest(const string& clientId, const string& jwt);
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct AuthenticateWithRefreshTokenRequestBody
		{
			string refresh;

			/// Client ID, as obtained from Twitch. 
			// TODO: Needs to be 'clientId' everywhere to match network as much as possible (when that change takes place, remove this when it does). NETWORK == camelCase
			string client_id;
		};
		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithRefreshTokenRequestBody, "refresh", refresh, "client_id", client_id);

		struct AuthenticateWithRefreshTokenRequest : SendEnvelope<AuthenticateWithRefreshTokenRequestBody>
		{
			/// Creates an authorization request
			/// @param[in] clientId Client Id.
			/// @param[in] RefreshToken Refresh token obtained from authorization.
			AuthenticateWithRefreshTokenRequest(const string& clientId, const string& refreshToken);
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct SubscribeAuthenticationRequest : SendEnvelope<EmptyBody>
		{
			SubscribeAuthenticationRequest();
		};

		struct SubscribeAuthenticationResponse : ReceiveEnvelope<OKResponseBody>
		{
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct AuthenticateResponseBody
		{
			/// Signed JWT. Will expire.
			string jwt;

			string refresh;
		};
		MUXY_GAMELINK_SERIALIZE_2(AuthenticateResponseBody, "jwt", jwt, "refresh", refresh);

		struct AuthenticateResponse : ReceiveEnvelope<AuthenticateResponseBody>
		{
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		class User
		{
		public:
			User(string jwt, string refreshToken);

			const string& GetJWT() const;
			const string& GetRefreshToken() const;
			// string GetOpaqueID();
		private:
			string jwt;
			string refreshToken;
		};
	}
}
#endif
