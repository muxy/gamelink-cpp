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


		AuthenticateWithPINRequest::AuthenticateWithPINRequest(const string& clientId, const string& pin)
		{
			action = string("authenticate");
			params.target = string("");
			data.pin = pin;
			data.client_id = clientId;
		}

		AuthenticateWithGameAndPINRequest::AuthenticateWithGameAndPINRequest(const string& clientId, const string& gameId, const string& pin)
		{
			action = string("authenticate");
			params.target = string("");
			data.pin = pin;
			data.client_id = clientId;
			data.game_id = gameId;
		}

		AuthenticateWithRefreshTokenRequest::AuthenticateWithRefreshTokenRequest(const string& clientId, const string& refreshToken)
		{
			action = string("authenticate");
			params.target = string("");
			data.refresh = refreshToken;
			data.client_id = clientId;
		}

		AuthenticateWithRefreshTokenAndGameRequest::AuthenticateWithRefreshTokenAndGameRequest(const string& clientId, const string& gameId, const string& refreshToken)
		{
			action = string("authenticate");
			params.target = string("");
			data.refresh = refreshToken;
			data.client_id = clientId;
			data.game_id = gameId;
		}

		User::User(string jwt, string refreshToken, string twitchName, string twitchID)
			: jwt(std::move(jwt))
			, refreshToken(std::move(refreshToken))
			, twitchName(std::move(twitchName))
			, twitchID(std::move(twitchID))
		{
		}

		const string& User::GetJWT() const
		{
			return jwt;
		}

		const string& User::GetRefreshToken() const
		{
			return refreshToken;
		}

		const string& User::GetTwitchName() const
		{
			return twitchName;
		}

		const string& User::GetTwitchID() const
		{
			return twitchID;
		}
	}
}