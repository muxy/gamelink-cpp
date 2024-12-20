#include "gamelink.h"
#include <cstdio>
#include <iostream>

namespace gamelink
{
	static const size_t CONNECTION_URL_BUFFER_LENGTH = 256;
	static const char CONNECTION_URL_SANDBOX[] = "gamelink.sandbox.muxy.io";
	static const char CONNECTION_URL_PRODUCTION[] = "gamelink.muxy.io";

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
		if (stage == ConnectionStage::Production)
		{
			url = CONNECTION_URL_PRODUCTION;
		}
		else if (stage == ConnectionStage::Sandbox)
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

	string WebsocketConnectionURL(const string& clientID, ConnectionStage stage)
	{
		char buffer[CONNECTION_URL_BUFFER_LENGTH];
		// Ignore obviously too-large client IDs
		if (clientID.size() > 100)
		{
			return string("");
		}

		const char * url = nullptr;
		if (stage == ConnectionStage::Production)
		{
			url = CONNECTION_URL_PRODUCTION;
		}
		else if (stage == ConnectionStage::Sandbox)
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

	const schema::Error* FirstError(const schema::ReceiveEnvelopeCommon& recv)
	{
		if (recv.errors.empty())
		{
			return NULL;
		}

		return &recv.errors[0];
	}

	bool HasPrefix(const string& s, const string& prefix)
	{
		if (s.size() < prefix.size())
		{
			return false;
		}

		const char * sstr = s.c_str();
		const char * prefixstr = prefix.c_str();

		for (uint32_t i = 0; i < prefix.size(); ++i)
		{
			if (prefixstr[i] != sstr[i])
			{
				return false;
			}
		}

		return true;
	}

	uint32_t GetPollWinnerIndex(const std::vector<int>& results)
	{
		int winner = 0;
		int index = 0;
		for (int i = 0; i < results.size(); i++)
		{
			if (results[i] > winner)
			{
				winner = results[i];
				index = i;
			}
		}

		return index;
	}

	Payload::Payload(string data)
		: waitingForResponse(ANY_REQUEST_ID)
		, _data(data)
	{
	}

	const char* Payload::Data() const
	{
		return _data.c_str();
	}

	uint32_t Payload::Length() const
	{
		return static_cast<uint32_t>(_data.size());
	}

	SDK::SDK()
		: _user(NULL)
		, _currentRequestId(1)
		, _onDebugMessage(0, 0, detail::CALLBACK_PERSISTENT, string("??#debug"))
		, _onPollUpdate(this, "OnPollUpdate", 1)
		, _onAuthenticate(this, "OnAuthenticate", 2)
		, _onStateUpdate(this, "OnStateUpdate", 3)
		, _onGetState(this, "OnGetState", 4)
		, _onTransaction(this, "OnTransaction", 5)
		, _onGetPoll(this, "OnGetPoll", 6)
		, _onDatastreamUpdate(this, "OnDatastreamUpdate", 7)
		, _onGetConfig(this, "OnGetConfig", 8)
		, _onGetCombinedConfig(this, "OnGetCombinedConfig", 9)
		, _onConfigUpdate(this, "OnConfigUpdate", 10)
		, _onGetOutstandingTransactions(this, "OnGetOutstandingTransactions", 11)
		, _onGetDrops(this, "OnGetDrops", 12)
		, _onMatchmakingUpdate(this, "OnMatchmakingUpdate", 13)
		, _onMatchPollUpdate(this, "OnMatchPollUpdate", 14)
	{}

	SDK::~SDK()
	{
		// Clean up unsent messages
		for (uint32_t i = 0; i < _queuedPayloads.size(); ++i)
		{
			delete _queuedPayloads[i];
		}

		if (_user)
		{
			delete _user;
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
			uint32_t bufferLength = s->Length() + 128;
			char* buffer = new char[bufferLength];

			int offset = snprintf(buffer, bufferLength, "send len=%d msg=", static_cast<int>(s->Length()));
			memcpy(buffer + offset, s->Data(), s->Length());
			buffer[s->Length() + offset] = '\0';

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
				debugLogPayload(payload);
				if (payload->Length() > 0)
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
		bool parsedFromBuffer = false;
		schema::ReceiveEnvelope<schema::EmptyBody> env = schema::ParseEnvelope(bytes, length, &parseEnvelopeSuccess);

		// Take lock
		_lock.lock();
		if (!parseEnvelopeSuccess)
		{
			// Attempt to append to the buffer and reparse.
			size_t oldSize = _receiveBuffer.size();

			// If the resultant size is greater than 4mb, reject.
			if (_receiveBuffer.size() + length > 1024 * 1024 * 4)
			{
				// Clear out the receive buffer, it has been rejected.
				std::vector<char> empty;
				_receiveBuffer.swap(empty);

				_lock.unlock();
				return false;
			}

			_receiveBuffer.resize(_receiveBuffer.size() + length);
			if (_receiveBuffer.size() > 0)
			{
				memcpy(&_receiveBuffer[oldSize], bytes, length);
				env = schema::ParseEnvelope(_receiveBuffer.data(), _receiveBuffer.size(), &parseEnvelopeSuccess);
				if (parseEnvelopeSuccess)
				{
					parsedFromBuffer = true;
				}
			}
		}

		if (!parseEnvelopeSuccess)
		{
			_lock.unlock();
			return false;
		}

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
			if (p->waitingForResponse == ANY_REQUEST_ID && p->_data.size() == 0)
			{
				_queuedPayloads.pop_front();
				delete p;
			}
			else
			{
				break;
			}
		}

		// Successful parse of envelope, swap out the bytes and length values.
		std::vector<char> receivedBytes;
		if (!_receiveBuffer.empty())
		{
			_receiveBuffer.swap(receivedBytes);
		}

		if (parsedFromBuffer && !receivedBytes.empty())
		{
			bytes = receivedBytes.data();
			length = receivedBytes.size();
		}
		_lock.unlock();
		// Unlock

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
				const schema::Error* err = FirstError(authResp);
				if (!err)
				{
					_lock.lock();
					if (_user)
					{
						delete _user;
					}
					_user = new schema::User(authResp.data.jwt, authResp.data.refresh, authResp.data.twitch_name, authResp.data.twitch_id);
					_lock.unlock();

					_storedJWT = authResp.data.jwt;
					_storedRefresh = authResp.data.refresh;
				}

				_onAuthenticate.Invoke(authResp);
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
					_onGetState.Invoke(stateResp);
				}
			}
			else if (env.meta.target == "poll")
			{
				schema::GetPollResponse pollResp;
				success = schema::ParseResponse(bytes, length, pollResp);
				if (success)
				{
					_onGetPoll.Invoke(pollResp);
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
							_onGetCombinedConfig.Invoke(combinedResp);
						}
					}
					else
					{
						_onGetConfig.Invoke(configResp);
					}
				}
			}
			else if (env.meta.target == "transaction")
			{
				schema::GetOutstandingTransactionsResponse resp;
				success = schema::ParseResponse(bytes, length, resp);

				if (success)
				{
					_onGetOutstandingTransactions.Invoke(resp);
				}
			}
			else if (env.meta.target == "drops") {
				schema::GetDropsResponse resp;
				success = schema::ParseResponse(bytes, length, resp);

				if (success)
				{
					_onGetDrops.Invoke(resp);
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
					_onPollUpdate.Invoke(pollResp);
				}
			}
			else if (env.meta.target == "channel")
			{
				schema::SubscribeStateUpdateResponse<nlohmann::json> resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onStateUpdate.Invoke(resp);
				}
			}
			else if (env.meta.target == "twitchPurchaseBits" || env.meta.target == "transaction")
			{
				schema::TransactionResponse resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onTransaction.Invoke(resp);
				}
			}
			else if (env.meta.target == "datastream")
			{
				schema::DatastreamUpdate resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onDatastreamUpdate.Invoke(resp);
				}
			}
			else if (env.meta.target == "config")
			{
				schema::ConfigUpdateResponse resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onConfigUpdate.Invoke(resp);
				}
			}
			else if (env.meta.target == "matchmaking")
			{
				schema::MatchmakingUpdate resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onMatchmakingUpdate.Invoke(resp);
				}
			}
			else if (env.meta.target == "match_poll")
			{
				schema::MatchPollUpdate resp;
				success = schema::ParseResponse(bytes, length, resp);
				if (success)
				{
					_onMatchPollUpdate.Invoke(resp);
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
			Payload* authenticationPayload = nullptr;
			if (_storedGameId == gamelink::string(""))
			{
				schema::AuthenticateWithRefreshTokenRequest p(_storedClientId, _storedRefresh);
				authenticationPayload = new Payload(gamelink::string(to_string(p).c_str()));
			}
			else
			{
				schema::AuthenticateWithRefreshTokenAndGameRequest p(_storedClientId, _storedGameId, _storedRefresh);
				authenticationPayload = new Payload(gamelink::string(to_string(p).c_str()));
			}

			// Push the authentication request to the front of the queue.
			_lock.lock();
			_queuedPayloads.push_front(authenticationPayload);
			_lock.unlock();

			// Replay subscriptions.
			for (size_t i = 0; i < _subscriptionSets._skus.size(); ++i)
			{
				schema::SubscribeTransactionsRequest payload(_subscriptionSets._skus[i].target);
				queuePayload(payload);
			}

			// Replay polls.
			for (size_t i = 0; i < _subscriptionSets._polls.size(); ++i)
			{
				schema::SubscribePollRequest payload(_subscriptionSets._polls[i].target);
				queuePayload(payload);
			}

			// Replay config
			for (int i = 0; i < static_cast<int>(ConfigTarget::ConfigTargetCount); ++i)
			{
				if (_subscriptionSets._configurationChanges[i].state == detail::SubscriptionState::Active)
				{
					schema::SubscribeToConfigRequest payload(static_cast<ConfigTarget>(i));
					queuePayload(payload);
				}
			}

			// Replay state
			for (int i = 0; i < static_cast<int>(StateTarget::StateCount); ++i)
			{
				if (_subscriptionSets._stateSubscriptions[i].state == detail::SubscriptionState::Active)
				{
					schema::SubscribeStateRequest payload(static_cast<StateTarget>(i));
					queuePayload(payload);
				}
			}

			if (_subscriptionSets._datastream.state == detail::SubscriptionState::Active)
			{
				schema::SubscribeDatastreamRequest payload;
				queuePayload(payload);
			}

			if (_subscriptionSets._matchmakingInvite.state  == detail::SubscriptionState::Active)
			{
				schema::SubscribeMatchmakingRequest payload;
				queuePayload(payload);
			}
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

	void SDK::InvokeOnDebugMessage(const string& message)
	{
		_onDebugMessage.invoke(message);
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

	RequestId SDK::SendBroadcast(const string& topic)
	{
		schema::BroadcastRequest<nlohmann::json> payload(topic, nlohmann::json::object());
		return queuePayload(payload);
	}

	RequestId SDK::SetGameMetadata(const gamelink::GameMetadata& meta)
	{
		if (!VerifyGameMetadataLimits(meta))
		{
			return gamelink::REJECTED_REQUEST_ID;
		}

		schema::SetGameMetadataRequest payload(meta);
		return queuePayload(payload);
	}

	bool SDK::VerifyPollLimits(const string& prompt, const std::vector<string>& options)
	{
		const uint32_t BUF_SIZE = 256;
		char buffer[BUF_SIZE];

		if (options.size() > gamelink::limits::POLL_MAX_OPTIONS)
		{
			snprintf(buffer, BUF_SIZE, "Poll options size %u is larger than the max allowed %u", static_cast<uint32_t>(options.size()), gamelink::limits::POLL_MAX_OPTIONS);
			InvokeOnDebugMessage(buffer);
			return false;
		}

		if (prompt.size() > gamelink::limits::POLL_MAX_PROMPT_SIZE)
		{
			snprintf(buffer, BUF_SIZE, "Poll prompt size %u is larger than the max allowed %u", static_cast<uint32_t>(prompt.size()), gamelink::limits::POLL_MAX_PROMPT_SIZE);
			InvokeOnDebugMessage(buffer);
			return false;
		}

		for (auto opt : options)
		{
			if (opt.size() > gamelink::limits::POLL_MAX_OPTION_NAME_SIZE)
			{
				snprintf(buffer, BUF_SIZE, "Poll option name size %u is larger than the max allowed %u", static_cast<uint32_t>(opt.size()), gamelink::limits::POLL_MAX_OPTION_NAME_SIZE);
				InvokeOnDebugMessage(buffer);
				return false;
			}
		}

		return true;
	}

	bool SDK::VerifyGameMetadataLimits(const gamelink::GameMetadata& meta)
	{
		const uint32_t BUF_SIZE = 256;
		char buffer[BUF_SIZE];

		if (meta.game_name.size() > gamelink::limits::METADATA_MAX_GAME_NAME_SIZE)
		{
			snprintf(buffer, BUF_SIZE, "Game Metadata game name size %u is larger than the max allowed %u", static_cast<uint32_t>(meta.game_name.size()), gamelink::limits::METADATA_MAX_GAME_NAME_SIZE);
			InvokeOnDebugMessage(buffer);
			return false;
		}
		if (meta.game_logo.size() > gamelink::limits::METADATA_MAX_GAME_LOGO_SIZE)
		{
			snprintf(buffer, BUF_SIZE, "Game Metadata game logo size %u is larger than the max allowed %u", static_cast<uint32_t>(meta.game_logo.size()), gamelink::limits::METADATA_MAX_GAME_LOGO_SIZE);
			InvokeOnDebugMessage(buffer);
			return false;
		}

		return true;
	}
}
