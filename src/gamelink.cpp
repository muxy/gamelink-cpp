#ifndef INCLUDE_MUXY_GAMELINK_CPP
#define INCLUDE_MUXY_GAMELINK_CPP

#include "gamelink.h"
#include <cstdio>

namespace gamelink
{
	Payload::Payload(string data)
	{
		this->data = data;
	}

	SDK::SDK()
		: _user(NULL){};

	SDK::~SDK()
	{
		// Clean up unsent messages
		while (HasPayloads())
		{
			Payload* send = _queuedPayloads.front();
			_queuedPayloads.pop();
			delete send;
		}
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

	bool SDK::ReceiveMessage(const char* bytes, uint32_t length)
	{
		bool success = false;
		auto env = schema::ParseEnvelope(bytes, length);

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
			success = schema::ParseResponse<schema::AuthenticateResponse>(bytes, length, authResp);
			if (success)
			{
				_onAuthenticate.invoke(authResp);
				this->_user = new schema::User(authResp.data.jwt);
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

			if (env.meta.target == "channel")
			{
				schema::SubscribeStateUpdateResponse<nlohmann::json> resp;

				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onStateUpdate.invoke(resp);
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

	// Callbacks
	void SDK::OnDebugMessage(std::function<void(const string&)> callback)
	{
		_onDebugMessage.set(callback);
	}

	void SDK::OnDebugMessage(void (*callback)(void*, const string&), void* ptr)
	{
		_onDebugMessage.set(callback, ptr);
	}

	void SDK::OnPollUpdate(std::function<void(const schema::PollUpdateResponse& pollResponse)> callback)
	{
		_onPollUpdate.set(callback);
	}

	void SDK::OnPollUpdate(void (*callback)(void*, const schema::PollUpdateResponse&), void* ptr)
	{
		_onPollUpdate.set(callback, ptr);
	}

	void SDK::OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> callback)
	{
		_onAuthenticate.set(callback);
	}

	void SDK::OnAuthenticate(void (*callback)(void*, const schema::AuthenticateResponse&), void* ptr)
	{
		_onAuthenticate.set(callback, ptr);
	}

	void SDK::OnStateUpdate(std::function<void(const schema::SubscribeStateUpdateResponse<nlohmann::json>&)> callback)
	{
		_onStateUpdate.set(callback);
	}

	void SDK::OnStateUpdate(void (*callback)(void*, const schema::SubscribeStateUpdateResponse<nlohmann::json>&), void* ptr)
	{
		_onStateUpdate.set(callback, ptr);
	}

	void SDK::AuthenticateWithPIN(const string& clientId, const string& pin)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);
		queuePayload(payload);
	}

	void SDK::AuthenticateWithJWT(const string& clientId, const string& jwt)
	{
		schema::AuthenticateWithJWTRequest payload(clientId, jwt);
		queuePayload(payload);
	}

	void SDK::GetPoll(const string& pollId)
	{
		schema::GetPollRequest packet(pollId);
		queuePayload(packet);
	}

	void SDK::CreatePoll(const string& pollId, const string& prompt, const std::vector<string>& options)
	{
		schema::CreatePollRequest packet(pollId, prompt, options);
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
}

#endif
