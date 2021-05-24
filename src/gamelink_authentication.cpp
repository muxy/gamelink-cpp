#include "gamelink.h"

namespace gamelink
{
    uint32_t SDK::OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> Callback)
	{
		return _onAuthenticate.set(Callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnAuthenticate(void (*Callback)(void*, const schema::AuthenticateResponse&), void* Ptr)
	{
		return _onAuthenticate.set(Callback, Ptr, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnAuthenticate(uint32_t Id)
	{
		if (_onAuthenticate.validateId(Id))
		{
			_onAuthenticate.remove(Id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnAuthenticate");
		}
	}

    
	RequestId SDK::AuthenticateWithPIN(const string& ClientId, const string& Pin)
	{
		schema::AuthenticateWithPINRequest Payload(ClientId, Pin);
		_storedClientId = ClientId;

		return queuePayload(Payload);
	}

	RequestId SDK::AuthenticateWithPIN(const string& ClientId, const string& Pin, std::function<void(const schema::AuthenticateResponse&)> Callback)
	{
		schema::AuthenticateWithPINRequest Payload(ClientId, Pin);
		_storedClientId = ClientId;

		RequestId Id = queuePayload(Payload);
		_onAuthenticate.set(Callback, Id, detail::CALLBACK_ONESHOT);
		return Id;
	}

	RequestId SDK::AuthenticateWithPIN(const string& ClientId,
								  const string& Pin,
								  void (*callback)(void*, const schema::AuthenticateResponse&),
								  void* User)
	{
		schema::AuthenticateWithPINRequest Payload(ClientId, Pin);
		_storedClientId = ClientId;

		RequestId Id = queuePayload(Payload);
		_onAuthenticate.set(callback, User, Id, detail::CALLBACK_ONESHOT);
		return Id;
	}

	RequestId SDK::AuthenticateWithJWT(const string& ClientId, const string& JWT)
	{
		schema::AuthenticateWithJWTRequest Payload(ClientId, JWT);
		_storedClientId = ClientId;

		return queuePayload(Payload);
	}

	RequestId SDK::AuthenticateWithJWT(const string& ClientId, const string& JWT, std::function<void(const schema::AuthenticateResponse&)> Callback)
	{
		schema::AuthenticateWithJWTRequest Payload(ClientId, JWT);
		_storedClientId = ClientId;

		RequestId Id = queuePayload(Payload);
		_onAuthenticate.set(Callback, Id, detail::CALLBACK_ONESHOT);
		return Id;
	}

	RequestId SDK::AuthenticateWithJWT(const string& ClientId,
								  const string& JWT,
								  void (*callback)(void*, const schema::AuthenticateResponse&),
								  void* User)
	{
		schema::AuthenticateWithJWTRequest Payload(ClientId, JWT);
		_storedClientId = ClientId;

		RequestId Id = queuePayload(Payload);
		_onAuthenticate.set(callback, User, Id, detail::CALLBACK_ONESHOT);
		return Id;
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& ClientId, const string& RefreshToken)
	{
		schema::AuthenticateWithRefreshTokenRequest Payload(ClientId, RefreshToken);
		_storedClientId = ClientId;

		return queuePayload(Payload);
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& ClientId, const string& RefreshToken, std::function<void(const schema::AuthenticateResponse&)> Callback)
	{
		schema::AuthenticateWithRefreshTokenRequest Payload(ClientId, RefreshToken);
		_storedClientId = ClientId;

		RequestId Id = queuePayload(Payload);
		_onAuthenticate.set(Callback, Id, detail::CALLBACK_ONESHOT);
		return Id;
	}

	RequestId SDK::AuthenticateWithRefreshToken(const string& ClientId,
								  const string& RefreshToken,
								  void (*Callback)(void*, const schema::AuthenticateResponse&),
								  void* User)
	{
		schema::AuthenticateWithRefreshTokenRequest Payload(ClientId, RefreshToken);
		_storedClientId = ClientId;

		RequestId Id = queuePayload(Payload);
		_onAuthenticate.set(Callback, User, Id, detail::CALLBACK_ONESHOT);
		return Id;
	}
}