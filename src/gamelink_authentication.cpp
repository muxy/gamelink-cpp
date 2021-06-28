#include "gamelink.h"

namespace gamelink
{
    uint32_t SDK::OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		return _onAuthenticate.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnAuthenticate(void (*callback)(void*, const schema::AuthenticateResponse&), void* ptr)
	{
		return _onAuthenticate.set(callback, ptr, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnAuthenticate(uint32_t id)
	{
		if (_onAuthenticate.validateId(id))
		{
			_onAuthenticate.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnAuthenticate");
		}
	}

	RequestId SDK::Deauthenticate()
	{
		// TODO: Send a deauth attempt, instead of just nulling out the user object here.
		delete _user;
		_user = nullptr;

		return ANY_REQUEST_ID;
	}
    
	RequestId SDK::AuthenticateWithPIN(const string& clientId, const string& pin)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);
		_storedClientId = clientId;

		return queuePayload(payload);
	}

	RequestId SDK::AuthenticateWithPIN(const string& clientId, const string& pin, std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::AuthenticateWithPIN(const string& clientId,
								  const string& pin,
								  void (*callback)(void*, const schema::AuthenticateResponse&),
								  void* user)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, user, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& clientId, const string& refreshToken)
	{
		schema::AuthenticateWithRefreshTokenRequest payload(clientId, refreshToken);
		_storedClientId = clientId;

		return queuePayload(payload);
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& clientId, const string& refreshToken, std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		schema::AuthenticateWithRefreshTokenRequest payload(clientId, refreshToken);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& clientId,
								  const string& refreshToken,
								  void (*callback)(void*, const schema::AuthenticateResponse&),
								  void* user)
	{
		schema::AuthenticateWithRefreshTokenRequest payload(clientId, refreshToken);
		_storedClientId = clientId;

		RequestId id = queuePayload(payload);
		_onAuthenticate.set(callback, user, id, detail::CALLBACK_ONESHOT);
		return id;
	}
}