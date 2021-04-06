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

		AuthenticateWithPINRequest::AuthenticateWithPINRequest(const string& client, const string& pin)
		{
			action = string("authenticate");
			params.target = string("");
			data.pin = pin;
			data.client_id = client;
		}

		AuthenticateWithJWTRequest::AuthenticateWithJWTRequest(const string& client, const string& jwt)
		{
			action = string("authenticate");
			params.target = string("");
			data.jwt = jwt;
			data.client_id = client;
		}

		User::User(string jwt)
			: jwt(std::move(jwt))
		{
		}

		const string& User::GetJWT() const
		{
			return this->jwt;
		}
	}
}
