#ifndef INCLUDE_MUXY_GAMELINK_CPP
#define INCLUDE_MUXY_GAMELINK_CPP

#include "gamelink.h"

namespace gamelink
{
	Send::Send(string data)
	{
		this->data = data;
	}

	SDK::SDK()
		: _user(NULL)
	{};

	SDK::~SDK()
	{
		// Clean up unsent messages
		while (HasSends())
		{
			Send* send = _sendQueue.front();
			_sendQueue.pop();
			delete send;
		}
	}

	bool SDK::ReceiveMessage(const char * bytes, uint32_t length)
	{
		bool success = false;
		auto env = schema::ParseEnvelope(bytes, length);

		if (env.meta.action == "authenticate")
		{
			// Authentication response
			schema::AuthenticateResponse authResp;
			success = schema::ParseResponse<schema::AuthenticateResponse>(bytes, length, authResp);
			if (success)
			{
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

	void SDK::OnPollUpdate(std::function<void(const schema::PollUpdateResponse& pollResponse)> callback)
	{
		this->_onPollUpdate.set(callback);
	}

	void SDK::OnPollUpdate(void (*callback)(void *, const schema::PollUpdateResponse&), void* ptr)
	{
		this->_onPollUpdate.set(callback, ptr);
	}

	void SDK::AuthenticateWithPIN(const string& clientId, const string& pin)
	{
		schema::AuthenticateWithPINRequest payload(clientId, pin);

		auto send = new Send(to_string(payload));
		_sendQueue.push(send);
	}

	void SDK::AuthenticateWithJWT(const string& clientId, const string& jwt)
	{
		schema::AuthenticateWithJWTRequest payload(clientId, jwt);

		auto send = new Send(to_string(payload));
		_sendQueue.push(send);
	}

	void SDK::GetPoll(const string& pollId)
	{
		schema::GetPollRequest packet(pollId);

		auto send = new Send(to_string(packet));
		_sendQueue.push(send);
	}

	void SDK::CreatePoll(const string& pollId, const string& prompt, const std::vector<string>& options)
	{
		schema::CreatePollRequest packet(pollId, prompt, options);

		auto send = new Send(to_string(packet));
		_sendQueue.push(send);
	}

	void SDK::SubscribeToPoll(const string& pollId)
	{
		schema::SubscribePollRequest packet(pollId);

		auto send = new Send(to_string(packet));
		_sendQueue.push(send);
	}

	void SDK::DeletePoll(const string& pollId)
	{
		schema::DeletePollRequest payload(pollId);

		auto send = new Send(to_string(payload));
		_sendQueue.push(send);
	}
}

#endif
