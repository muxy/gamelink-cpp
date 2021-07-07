#include "gamelink.h"
#include <cstdio>
#include <iostream>

namespace gamelink
{
	static const size_t CONNECTION_URL_BUFFER_LENGTH = 256;
	static const char CONNECTION_URL_SANDBOX[] = "sandbox.gamelink.muxy.io";
	static const char CONNECTION_URL_PRODUCTION[] = "gamelink.muxy.io";

	namespace detail
	{
		string ProjectionWebsocketConnectionURL(
			const string& clientId, 
			ConnectionStage stage, 
			const string& projection, 
			int projectionMajor, int projectionMinor, int projectionPatch)
		{
			char buffer[CONNECTION_URL_BUFFER_LENGTH];
			// Ignore obviously too-large client IDs
			if (clientId.size() > 100)
			{
				return string("");
			}

			const char * url = nullptr;
			if (stage == CONNECTION_STAGE_PRODUCTION)
			{
				url = CONNECTION_URL_PRODUCTION;
			}
			else if (stage == CONNECTION_STAGE_SANDBOX)
			{
				url = CONNECTION_URL_SANDBOX;
			}

			if (!url)
			{
				return string("");
			}

			if (projectionMajor < 0 || projectionMinor < 0 || projectionPatch < 0)
			{
				return string("");
			}

			int output = snprintf(buffer, CONNECTION_URL_BUFFER_LENGTH, "%s/%d.%d.%d/%s/%d.%d.%d/%s",
				url,
				MUXY_GAMELINK_VERSION_MAJOR, MUXY_GAMELINK_VERSION_MINOR, MUXY_GAMELINK_VERSION_PATCH,
				projection.c_str(),
				projectionMajor, projectionMinor, projectionPatch,
				clientId.c_str());

			if (output > 0 && output < CONNECTION_URL_BUFFER_LENGTH)
			{
				return string(buffer);
			}

			return string("");
		}
	}

	string WebsocketConnectionURL(const string& clientID, ConnectionStage stage)
	{
		char buffer[CONNECTION_URL_BUFFER_LENGTH];
		// Ignore obviously too-large client IDs
		if (clientID.size() > 100)
		{
			return string("");
		}

		const char * url = nullptr;
		if (stage == CONNECTION_STAGE_PRODUCTION)
		{
			url = CONNECTION_URL_PRODUCTION;
		}
		else if (stage == CONNECTION_STAGE_SANDBOX)
		{
			url = CONNECTION_URL_SANDBOX;
		}

		if (!url)
		{
			return string("");
		}

		int output = snprintf(buffer, CONNECTION_URL_BUFFER_LENGTH, "%s/%d.%d.%d/%s",
			url,
			MUXY_GAMELINK_VERSION_MAJOR, MUXY_GAMELINK_VERSION_MINOR, MUXY_GAMELINK_VERSION_PATCH,
			clientID.c_str());

		if (output > 0 && output < CONNECTION_URL_BUFFER_LENGTH)
		{
			return string(buffer);
		}

		return string("");
	}

	Payload::Payload(string data)
		: waitingForResponse(ANY_REQUEST_ID)
		, data(data)
	{
	}

	SDK::SDK()
		: _user(NULL)
		, _currentRequestId(1)
		, _onDebugMessage(0, 0, detail::CALLBACK_PERSISTENT){};

	SDK::~SDK()
	{
		// Clean up unsent messages
		for (uint32_t i = 0; i < _queuedPayloads.size(); ++i)
		{
			delete _queuedPayloads[i];
		}
	}

	RequestId SDK::nextRequestId()
	{
		// Wrap around at 32k
		RequestId id = (_currentRequestId++ & 0x7F);
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
		_lock.lock();
		bool result = HasPayloadsNoLock();
		_lock.unlock();

		return result;
	}

	bool SDK::HasPayloadsNoLock() const
	{
		if (_queuedPayloads.size() > 0)
		{
			if (_queuedPayloads.front()->waitingForResponse != ANY_REQUEST_ID)
			{
				return false;
			}

			return true;
		}

		return false;
	}

	void SDK::ForeachPayload(SDK::NetworkCallback networkCallback, void* user)
	{
		while (true)
		{
			Payload* payload = NULL;
			_lock.lock();
			if (HasPayloadsNoLock())
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
				if (payload->data.size() > 0)
				{
					networkCallback(user, payload);
				}

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

		_lock.lock();
		// Set any waits for the id just received to any_request_id
		for (uint32_t i = 0; i < _queuedPayloads.size(); ++i)
		{
			if (_queuedPayloads[i]->waitingForResponse == env.meta.request_id && env.meta.request_id != ANY_REQUEST_ID)
			{
				_queuedPayloads[i]->waitingForResponse = ANY_REQUEST_ID;
			}
		}

		// Clear any waits at the front of the queue.
		while (_queuedPayloads.size() > 0)
		{
			Payload* p = _queuedPayloads.front();
			if (p->waitingForResponse == ANY_REQUEST_ID && p->data.size() == 0)
			{
				_queuedPayloads.pop_front();
				delete p;
			}
			else
			{
				break;
			}
		}
		_lock.unlock();

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
				const schema::Error * err = FirstError(authResp);
				if (!err)
				{
					_user = new schema::User(authResp.data.jwt, authResp.data.refresh);
					_storedJWT = authResp.data.jwt;
					_storedRefresh = authResp.data.refresh;
				}

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
			else if (env.meta.target == "poll")
			{
				schema::GetPollResponse pollResp;
				success = schema::ParseResponse(bytes, length, pollResp);

				if (success)
				{
					_onGetPoll.invoke(pollResp);
				}
			}
			else if (env.meta.target == "config")
			{
				schema::GetConfigResponse configResp;
				success = schema::ParseResponse(bytes, length, configResp);
				if (success)
				{
					if (configResp.data.configId == "combined")
					{
						schema::GetCombinedConfigResponse combinedResp;
						success = schema::ParseResponse(bytes, length, combinedResp);
						if (success)
						{
							_onGetCombinedConfig.invoke(combinedResp);
						}
					}
					else
					{
						_onGetConfig.invoke(configResp);
					}
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
			else if (env.meta.target == "twitchPurchaseBits")
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
			else if (env.meta.target == "config")
			{
				schema::ConfigUpdateResponse resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onConfigUpdate.invoke(resp);
				}
			}
		}
		else
		{
			success = true;
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
		if (!(_storedRefresh == gamelink::string("")))
		{
			schema::AuthenticateWithRefreshTokenRequest p(_storedClientId, _storedRefresh);
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

	void SDK::WaitForResponse(RequestId req)
	{
		Payload* wait = new Payload("");
		wait->waitingForResponse = req;

		_lock.lock();
		_queuedPayloads.push_back(wait);
		_lock.unlock();
	}

	RequestId SDK::SendBroadcast(const string& topic, const nlohmann::json& msg)
	{
		schema::BroadcastRequest<nlohmann::json> payload(topic, msg);
		return queuePayload(payload);
	}
}
