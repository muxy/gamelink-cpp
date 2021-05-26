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

		AuthenticateWithRefreshTokenRequest::AuthenticateWithRefreshTokenRequest(const string& clientId, const string& refreshToken)
		{
			action = string("authenticate");
			params.target = string("");
			data.refresh = refreshToken;
			data.client_id = clientId;
		}

		User::User(string jwt, string refreshToken)
			: jwt(std::move(jwt))
			, refreshToken(std::move(refreshToken))
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
	}
}
