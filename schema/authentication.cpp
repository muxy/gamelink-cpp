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

		AuthenticateWithPINRequest::AuthenticateWithPINRequest(const string& ClientId, const string& PIN)
		{
			action = string("authenticate");
			params.target = string("");
			data.pin = PIN;
			data.client_id = ClientId;
		}

		AuthenticateWithJWTRequest::AuthenticateWithJWTRequest(const string& ClientId, const string& JWT)
		{
			action = string("authenticate");
			params.target = string("");
			data.jwt = JWT;
			data.client_id = ClientId;
		}

		AuthenticateWithRefreshTokenRequest::AuthenticateWithRefreshTokenRequest(const string& ClientId, const string& RefreshToken)
		{
			action = string("authenticate");
			params.target = string("");
			data.refresh = RefreshToken;
			data.client_id = ClientId;
		}

		User::User(string JWT, string RefreshToken)
			: JWT(std::move(JWT))
			, RefreshToken(std::move(RefreshToken))
		{
		}
		const string& User::GetJWT() const { return this->JWT; }
		const string& User::GetRefreshToken() const { return this->RefreshToken; }
	}
}
