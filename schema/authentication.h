#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#define MUXY_GAMELINK_SCHEMA_AUTHENTICATION_H
#include "schema/envelope.h"

namespace gamelink
{
	namespace schema
	{

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct MUXY_GAMELINK_API AuthenticateWithPINRequestBody
		{
			/// PIN string, as obtained from the REST API
			string pin;

			/// Client ID, as obtained from Twitch.
			string client_id;
		};

		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithPINRequestBody, "pin", pin, "client_id", client_id);

		struct MUXY_GAMELINK_API AuthenticateWithPINRequest : SendEnvelope<AuthenticateWithPINRequestBody>
		{
			/// Creates an authorization request.
			/// @param[in] clientId Client ID.
			/// @param[in] pin PIN obtained from user input.
			AuthenticateWithPINRequest(const string& clientId, const string& pin);
		};

		struct MUXY_GAMELINK_API AuthenticateWithGameAndPINRequestBody
		{
			/// PIN string, as obtained from the REST API
			string pin;

			/// Client ID, as obtained from Twitch.
			string client_id;

			/// Game ID, assigned by Muxy.
			string game_id;
		};
		
		MUXY_GAMELINK_SERIALIZE_3(AuthenticateWithGameAndPINRequestBody, "pin", pin, "client_id", client_id, "game_id", game_id);

		struct MUXY_GAMELINK_API AuthenticateWithGameAndPINRequest : SendEnvelope<AuthenticateWithGameAndPINRequestBody>
		{
			/// Creates an authorization request.
			/// @param[in] clientId Client ID.
			/// @param[in] gameId Game ID from Twitch.
			/// @param[in] pin PIN obtained from user input.
			AuthenticateWithGameAndPINRequest(const string& clientId, const string& gameId, const string& pin);
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct MUXY_GAMELINK_API AuthenticateWithRefreshTokenRequestBody
		{
			string refresh;

			/// Client ID, as obtained from Twitch.
			// TODO: Needs to be 'clientId' everywhere to match network as much as possible (when that change takes place, remove this when it does). NETWORK == camelCase
			string client_id;
		};

		MUXY_GAMELINK_SERIALIZE_2(AuthenticateWithRefreshTokenRequestBody, "refresh", refresh, "client_id", client_id);

		struct MUXY_GAMELINK_API AuthenticateWithRefreshTokenRequest : SendEnvelope<AuthenticateWithRefreshTokenRequestBody>
		{
			/// Creates an authorization request
			/// @param[in] clientId Client Id.
			/// @param[in] RefreshToken Refresh token obtained from authorization.
			AuthenticateWithRefreshTokenRequest(const string& clientId, const string& refreshToken);
		};
		
		struct MUXY_GAMELINK_API AuthenticateWithRefreshTokenAndGameRequestBody
		{
			string refresh;

			/// Client ID, as obtained from Twitch.
			// TODO: Needs to be 'clientId' everywhere to match network as much as possible (when that change takes place, remove this when it does). NETWORK == camelCase
			string client_id;

			/// Game ID, assigned by Muxy.
			string game_id;
		};

		MUXY_GAMELINK_SERIALIZE_3(AuthenticateWithRefreshTokenAndGameRequestBody, "refresh", refresh, "client_id", client_id, "game_id", game_id);

		struct MUXY_GAMELINK_API AuthenticateWithRefreshTokenAndGameRequest : SendEnvelope<AuthenticateWithRefreshTokenAndGameRequestBody>
		{
			/// Creates an authorization request
			/// @param[in] clientId Client Id.
			/// @param[in] gameId Game ID from Twitch.
			/// @param[in] RefreshToken Refresh token obtained from authorization.
			AuthenticateWithRefreshTokenAndGameRequest(const string& clientId, const string& gameId, const string& refreshToken);
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct MUXY_GAMELINK_API SubscribeAuthenticationRequest : SendEnvelope<EmptyBody>
		{
			SubscribeAuthenticationRequest();
		};

		struct MUXY_GAMELINK_API SubscribeAuthenticationResponse : ReceiveEnvelope<OKResponseBody>
		{
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		struct MUXY_GAMELINK_API AuthenticateResponseBody
		{
			/// Signed JWT. Will expire.
			string jwt;

			/// Refresh token, used to reauth after the JWT expires.
			string refresh;

			/// Information about the channel the auth was done with
			string twitch_name;
		};
		MUXY_GAMELINK_SERIALIZE_3(AuthenticateResponseBody, "jwt", jwt, "refresh", refresh, "twitch_name", twitch_name);

		struct MUXY_GAMELINK_API AuthenticateResponse : ReceiveEnvelope<AuthenticateResponseBody>
		{
		};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		class MUXY_GAMELINK_API User
		{
		public:
			User(string jwt, string refreshToken, string twitchName);

			const string& GetJWT() const;
			const string& GetRefreshToken() const;
			const string& GetTwitchName() const;
			// string GetOpaqueID();
		private:
			string jwt;
			string refreshToken;
			string twitchName;
		};
	}
}
#endif
