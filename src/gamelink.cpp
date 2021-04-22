#ifndef INCLUDE_MUXY_GAMELINK_CPP
#define INCLUDE_MUXY_GAMELINK_CPP

#include "gamelink.h"
#include <cstdio>
#include <iostream>

namespace gamelink
{
	Payload::Payload(string data)
	{
		this->data = data;
	}

	SDK::SDK()
		: _user(NULL)
		, _currentRequestId(1)
		, _onDebugMessage(0, 0, detail::CALLBACK_PERSISTENT){};

	SDK::~SDK()
	{
		// Clean up unsent messages
		while (HasPayloads())
		{
			Payload* send = _queuedPayloads.front();
			_queuedPayloads.pop_front();
			delete send;
		}
	}

	uint16_t SDK::nextRequestId()
	{
		// Wrap around at 32k
		uint16_t id = (_currentRequestId++ & 0x7F);
		return id;
	}

	void SDK::debugLogPayload(const Payload* s)
	{
		if (_onDebugMessage.valid())
		{
			uint32_t bufferLength = s->data.size() + 128;
			char* buffer = new char[bufferLength];

			int offset = snprintf(buffer, bufferLength, "send len=%d msg=", static_cast<int>(s->data.size()));
			memcpy(buffer + offset, s->data.c_str(), s->data.size());
			buffer[s->data.size() + offset] = '\0';

			_onDebugMessage.invoke(string(buffer));

			delete[] buffer;
		}
	}

	bool SDK::HasPayloads() const
	{
		return _queuedPayloads.size() > 0;
	}

	void SDK::ForeachPayload(SDK::NetworkCallback networkCallback, void* user)
	{
		while (true)
		{
			Payload* payload = NULL;
			_lock.lock();
			if (HasPayloads())
			{
				payload = _queuedPayloads.front();
				_queuedPayloads.pop_front();
				_lock.unlock();
			}
			else
			{
				_lock.unlock();
				break;
			}

			if (payload)
			{
				networkCallback(user, payload);
				delete payload;
			}
		}
	}

	bool SDK::ReceiveMessage(const char* bytes, uint32_t length)
	{
		bool success = false;
		bool parseEnvelopeSuccess = false;
		schema::ReceiveEnvelope<schema::EmptyBody> env = schema::ParseEnvelope(bytes, length, &parseEnvelopeSuccess);
		if (!parseEnvelopeSuccess)
		{
			return false;
		}

		if (_onDebugMessage.valid())
		{
			uint32_t bufferLength = length + 128;
			char* buffer = new char[bufferLength];

			int offset = snprintf(buffer, bufferLength, "recv len=%d msg=", static_cast<int>(length));
			memcpy(buffer + offset, bytes, length);
			buffer[length + offset] = '\0';

			_onDebugMessage.invoke(string(buffer));

			delete[] buffer;
		}

		if (env.meta.action == "authenticate")
		{
			// Authentication response
			schema::AuthenticateResponse authResp;
			success = schema::ParseResponse(bytes, length, authResp);
			if (success)
			{
				_user = new schema::User(authResp.data.jwt);
				_storedJWT = authResp.data.jwt;

				_onAuthenticate.invoke(authResp);
			}
		}
		else if (env.meta.action == "get")
		{
			if (env.meta.target == "state")
			{
				schema::GetStateResponse<nlohmann::json> stateResp;
				success = schema::ParseResponse(bytes, length, stateResp);

				if (success)
				{
					_onGetState.invoke(stateResp);
				}
			}

			if (env.meta.target == "poll")
			{
				schema::GetPollResponse pollResp;
				success = schema::ParseResponse(bytes, length, pollResp);

				if (success)
				{
					_onGetPoll.invoke(pollResp);
				}
			}
		}
		else if (env.meta.action == "update")
		{
			if (env.meta.target == "poll")
			{
				// Poll update response
				// TODO Handle a UserDataPollUpdateResponse as well
				schema::PollUpdateResponse pollResp;
				success = schema::ParseResponse<schema::PollUpdateResponse>(bytes, length, pollResp);
				if (success)
				{
					_onPollUpdate.invoke(pollResp);
				}
			}
			else if (env.meta.target == "channel")
			{
				schema::SubscribeStateUpdateResponse<nlohmann::json> resp;

				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onStateUpdate.invoke(resp);
				}
			}
			else if (env.meta.target == "twitchBitsPurchase")
			{
				schema::TwitchPurchaseBitsResponse<nlohmann::json> resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onTwitchPurchaseBits.invoke(resp);
				}
			}
			else if (env.meta.target == "datastream")
			{
				schema::DatastreamUpdate resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onDatastreamUpdate.invoke(resp);
				}
			}
		}

		return success;
	}

	bool SDK::IsAuthenticated() const
	{
		return _user != NULL;
	}

	const schema::User* SDK::GetUser() const
	{
		return _user;
	}

	const char* SDK::GetClientId() const
	{
		return _storedClientId.c_str();
	}

	void SDK::HandleReconnect()
	{
		if (!(_storedJWT == gamelink::string("")))
		{
			schema::AuthenticateWithJWTRequest p(_storedClientId, _storedJWT);
			Payload* payload = new Payload(gamelink::string(to_string(p).c_str()));
			debugLogPayload(payload);

			_lock.lock();
			_queuedPayloads.push_front(payload);
			_lock.unlock();
		}
	}

	// Callbacks
	void SDK::OnDebugMessage(std::function<void(const string&)> callback)
	{
		_onDebugMessage.set(callback);
	}

	void SDK::OnDebugMessage(void (*callback)(void*, const string&), void* ptr)
	{
		_onDebugMessage.set(callback, ptr);
	}

	void SDK::DetachOnDebugMessage()
	{
		_onDebugMessage.clear();
	}

	uint32_t SDK::OnPollUpdate(std::function<void(const schema::PollUpdateResponse& pollResponse)> callback)
	{
		return _onPollUpdate.set(callback, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnPollUpdate(void (*callback)(void*, const schema::PollUpdateResponse&), void* ptr)
	{
		return _onPollUpdate.set(callback, ptr, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnPollUpdate(uint32_t id)
	{
		if (_onPollUpdate.validateId(id))
		{
			_onPollUpdate.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnPollUpdate");
		}
	}

	uint32_t SDK::OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		return _onAuthenticate.set(callback, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnAuthenticate(void (*callback)(void*, const schema::AuthenticateResponse&), void* ptr)
	{
		return _onAuthenticate.set(callback, ptr, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
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

	uint32_t SDK::OnStateUpdate(std::function<void(const schema::SubscribeStateUpdateResponse<nlohmann::json>&)> callback)
	{
		return _onStateUpdate.set(callback, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnStateUpdate(void (*callback)(void*, const schema::SubscribeStateUpdateResponse<nlohmann::json>&), void* ptr)
	{
		return _onStateUpdate.set(callback, ptr, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnStateUpdate(uint32_t id)
	{
		if (_onStateUpdate.validateId(id))
		{
			_onStateUpdate.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into OnStateUpdate");
		}
	}

	void SDK::SubscribeToSKU(const string& sku)
	{
		schema::SubscribeTransactionsRequest payload(sku);
		queuePayload(payload);
	}

	void SDK::SubscribeToAllPurchases()
	{
		SubscribeToSKU("*");
	}

	void SDK::UnsubscribeFromSKU(const string& sku)
	{
		schema::UnsubscribeTransactionsRequest payload(sku);
		queuePayload(payload);
	}

	void SDK::UnsubscribeFromAllPurchases()
	{
		UnsubscribeFromSKU("*");
	}

	uint32_t SDK::OnTwitchPurchaseBits(std::function<void(const schema::TwitchPurchaseBitsResponse<nlohmann::json>&)> callback)
	{
		return _onTwitchPurchaseBits.set(callback, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnTwitchPurchaseBits(void (*callback)(void*, const schema::TwitchPurchaseBitsResponse<nlohmann::json>&), void* ptr)
	{
		return _onTwitchPurchaseBits.set(callback, ptr, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnTwitchPurchaseBits(uint32_t id)
	{
		if (_onTwitchPurchaseBits.validateId(id))
		{
			_onTwitchPurchaseBits.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnTwitchPurchaseBits");
		}
	}

	void SDK::AuthenticateWithPIN(const string& clientId, const string& pin)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);
		_storedClientId = clientId;

		queuePayload(payload);
	}

	void
	SDK::AuthenticateWithPIN(const string& clientId, const string& pin, std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);
		_storedClientId = clientId;

		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onAuthenticate.set(callback, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::AuthenticateWithPIN(const string& clientId,
								  const string& pin,
								  void (*callback)(void*, const schema::AuthenticateResponse&),
								  void* user)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);
		_storedClientId = clientId;

		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onAuthenticate.set(callback, user, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::AuthenticateWithJWT(const string& clientId, const string& jwt)
	{
		schema::AuthenticateWithJWTRequest payload(clientId, jwt);
		_storedClientId = clientId;

		queuePayload(payload);
	}

	void
	SDK::AuthenticateWithJWT(const string& clientId, const string& jwt, std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		schema::AuthenticateWithJWTRequest payload(clientId, jwt);
		_storedClientId = clientId;

		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onAuthenticate.set(callback, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::AuthenticateWithJWT(const string& clientId,
								  const string& jwt,
								  void (*callback)(void*, const schema::AuthenticateResponse&),
								  void* user)
	{
		schema::AuthenticateWithJWTRequest payload(clientId, jwt);
		_storedClientId = clientId;

		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onAuthenticate.set(callback, user, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::GetPoll(const string& pollId)
	{
		schema::GetPollRequest packet(pollId);
		queuePayload(packet);
	}

	void SDK::GetPoll(const string& pollId, std::function<void(const schema::GetPollResponse&)> callback)
	{
		schema::GetPollRequest payload(pollId);
		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onGetPoll.set(callback, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::GetPoll(const string& pollId, void (*callback)(void*, const schema::GetPollResponse&), void* user)
	{
		schema::GetPollRequest payload(pollId);
		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onGetPoll.set(callback, user, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::CreatePoll(const string& pollId, const string& prompt, const std::vector<string>& options)
	{
		schema::CreatePollRequest packet(pollId, prompt, options);
		queuePayload(packet);
	}

	void SDK::UnsubscribeFromPoll(const string& pollId)
	{
		schema::UnsubscribePollRequest packet(pollId);
		queuePayload(packet);
	}

	void SDK::SubscribeToPoll(const string& pollId)
	{
		schema::SubscribePollRequest packet(pollId);
		queuePayload(packet);
	}

	void SDK::DeletePoll(const string& pollId)
	{
		schema::DeletePollRequest payload(pollId);
		queuePayload(payload);
	}

	void SDK::SetState(const char* target, const nlohmann::json& value)
	{
		schema::SetStateRequest<nlohmann::json> payload(target, value);
		queuePayload(payload);
	}

	void SDK::GetState(const char* target)
	{
		schema::GetStateRequest payload(target);
		queuePayload(payload);
	}

	void SDK::GetState(const char* target, std::function<void(const schema::GetStateResponse<nlohmann::json>&)> callback)
	{
		schema::GetStateRequest payload(target);
		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onGetState.set(callback, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::GetState(const char* target, void (*callback)(void*, const schema::GetStateResponse<nlohmann::json>&), void* user)
	{
		schema::GetStateRequest payload(target);
		uint16_t id = nextRequestId();

		payload.params.request_id = id;
		queuePayload(payload);

		_onGetState.set(callback, user, id, detail::CALLBACK_ONESHOT);
	}

	void SDK::SubscribeToStateUpdates(const char* target)
	{
		schema::SubscribeStateRequest payload(target);
		queuePayload(payload);
	}

	void SDK::UpdateState(const char* target, const string& operation, const string& path, const schema::JsonAtom& atom)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = atom;

		UpdateState(target, &op, &op + 1);
	}

	void SDK::UpdateState(const char* target, const schema::PatchOperation* begin, const schema::PatchOperation* end)
	{
		schema::PatchStateRequest payload(target);
		std::vector<schema::PatchOperation> updates;
		updates.resize(end - begin);
		std::copy(begin, end, updates.begin());

		payload.data.state = std::move(updates);
		queuePayload(payload);
	};

	void SDK::SendBroadcast(const string& topic, const nlohmann::json& msg)
	{
		schema::BroadcastRequest<nlohmann::json> payload(topic, msg);
		queuePayload(payload);
	}

	void SDK::SubscribeToDatastream()
	{
		schema::SubscribeDatastreamRequest payload;
		queuePayload(payload);
	}

	void SDK::UnsubscribeFromDatastream()
	{
		schema::UnsubscribeDatastreamRequest payload;
		queuePayload(payload);
	}

	uint32_t SDK::OnDatastream(std::function<void(const schema::DatastreamUpdate&)> callback)
	{
		return _onDatastreamUpdate.set(callback, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnDatastream(void (*callback)(void*, const schema::DatastreamUpdate&), void* user)
	{
		return _onDatastreamUpdate.set(callback, user, detail::ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnDatastream(uint32_t id)
	{
		if (_onDatastreamUpdate.validateId(id))
		{
			_onDatastreamUpdate.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnDatastream");
		}
	}
}

#endif
