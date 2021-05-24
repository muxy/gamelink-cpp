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
			/// @param[in] ClientId Client ID.
			/// @param[in] PIN PIN obtained from user input.
			AuthenticateWithPINRequest(const string& ClientId, const string& PIN);
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
			/// @param[in] ClientId Client ID.
			/// @param[in] JWT JWT obtained from previous authorizations.
			AuthenticateWithJWTRequest(const string& ClientId, const string& JWT);
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct AuthenticateWithRefreshTokenRequestBody
		{
			string refresh;

			/// Client ID, as obtained from Twitch. 
			//needs to be 'clientId' everywhere to match network as much as possible. NETWORK == camelCase
			string client_id;
		};
		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithRefreshTokenRequestBody, "refresh", refresh, "client_id", client_id);

		struct AuthenticateWithRefreshTokenRequest : SendEnvelope<AuthenticateWithRefreshTokenRequestBody>
		{
			/// Creates an authorization request
			/// @param[in] ClientId Client Id.
			/// @param[in] RefreshToken Refresh token obtained from authorization.
			AuthenticateWithRefreshTokenRequest(const string& ClientId, const string& RefreshToken);
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

			// call this refreshToken later?
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
			explicit User(string JWT, string RefreshToken);

			const string& GetJWT() const;
			const string& GetRefreshToken() const;
			// string GetOpaqueID();
		private:
			string JWT;
			string RefreshToken;
		};
	}
}
#endif
