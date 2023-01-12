#include "gamelink.h"

namespace gamelink
{
	Event<schema::AuthenticateResponse>& SDK::OnAuthenticate()
	{
		return _onAuthenticate;
	}

	RequestId SDK::Deauthenticate()
	{
		// TODO: Send a deauth attempt, instead of just nulling out the user object here.
		delete _user;
		_user = nullptr;

		return ANY_REQUEST_ID;
	}

	RequestId SDK::AuthenticateWithPINAndGameID(const string& clientId, const string& gameId, const string& pin)
	{
		schema::AuthenticateWithPINRequest payload(clientId, gameId, pin);
		_storedClientId = clientId;

		return queuePayload(payload);
	}

	RequestId SDK::AuthenticateWithPIN(const string& clientId, const string& pin)
	{
		return AuthenticateWithPINAndGameID(clientId, "", pin);
	}

	RequestId SDK::AuthenticateWithPINAndGameID(const string& clientId,
									   const string& gameId,
									   const string& pin,
									   std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		schema::AuthenticateWithPINRequest payload(clientId, gameId, pin);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::AuthenticateWithPIN(const string& clientId, const string& pin, std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		return AuthenticateWithPINAndGameID(clientId, "", pin, callback);
	}

	RequestId SDK::AuthenticateWithPINAndGameID(const string& clientId,
									   const string& gameId,
									   const string& pin,
									   void (*callback)(void*, const schema::AuthenticateResponse&),
									   void* user)
	{
		schema::AuthenticateWithPINRequest payload(clientId, gameId, pin);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, user, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::AuthenticateWithPIN(const string& clientId,
									   const string& pin,
									   void (*callback)(void*, const schema::AuthenticateResponse&),
									   void* user)
	{
		return AuthenticateWithPINAndGameID(clientId, "", pin, callback, user);
	}

	RequestId SDK::AuthenticateWithRefreshTokenAndGameID(const string& clientId, const string& gameId, const string& refreshToken)
	{
		schema::AuthenticateWithRefreshTokenRequest payload(clientId, gameId, refreshToken);
		_storedClientId = clientId;

		return queuePayload(payload);
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& clientId, const string& refreshToken)
	{
		return AuthenticateWithRefreshTokenAndGameID(clientId, "", refreshToken);
	}

	RequestId SDK::AuthenticateWithRefreshTokenAndGameID(const string& clientId,
												const string& gameId,
												const string& refreshToken,
												std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		schema::AuthenticateWithRefreshTokenRequest payload(clientId, gameId, refreshToken);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& clientId,
												const string& refreshToken,
												std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		return AuthenticateWithRefreshTokenAndGameID(clientId, "", refreshToken, callback);
	}

	RequestId SDK::AuthenticateWithRefreshTokenAndGameID(const string& clientId,
												const string& gameId,
												const string& refreshToken,
												void (*callback)(void*, const schema::AuthenticateResponse&),
												void* user)
	{
		schema::AuthenticateWithRefreshTokenRequest payload(clientId, gameId, refreshToken);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, user, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& clientId,
												const string& refreshToken,
												void (*callback)(void*, const schema::AuthenticateResponse&),
												void* user)
	{
		return AuthenticateWithRefreshTokenAndGameID(clientId, "", refreshToken, callback, user);
	}
}