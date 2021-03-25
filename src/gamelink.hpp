#pragma once
#ifndef INCLUDE_MUXY_GAMELINK_HPP
#define INCLUDE_MUXY_GAMELINK_HPP

#include <fmt/format.h>
#include <queue>

#include "schema/schema.h"

namespace gamelink
{
	class Send
	{
	public:
		Send(std::string data)
		{
			this->data = data;
		}

		std::string data;
	};

	class SDK
	{
	public:
		SDK()
			: _user(NULL)
			, _onPollUpdate(NULL){};
		~SDK()
		{
			// Clean up unsent messages
			while (HasSends())
			{
				Send* send = _sendQueue.front();
				_sendQueue.pop();
				delete send;
			}
		}

		void ReceiveMessage(std::string message)
		{
			auto env = schema::ParseEnvelope(message);

			if (env.meta.action == "authenticate")
			{
				// Authentication response
				schema::AuthenticateResponse authResp;
				schema::ParseResponse<schema::AuthenticateResponse>(message, authResp);
				this->_user = new schema::User(authResp.data.jwt);
			}
			else if (env.meta.action == "update")
			{
				if (env.meta.target == "poll")
				{
					// Poll update response
					// TODO Handle a UserDataPollUpdateResponse as well
					schema::PollUpdateResponse pollResp;
					schema::ParseResponse<schema::PollUpdateResponse>(message, pollResp);

					if (this->_onPollUpdate != NULL)
					{
						this->_onPollUpdate(pollResp);
					}
				}
			}
		}

		bool HasSends()
		{
			return _sendQueue.size() > 0;
		}

		void ForeachSend(const std::function<void(Send* send)>& networkCallback)
		{
			while (HasSends())
			{
				Send* send = _sendQueue.front();
				_sendQueue.pop();

				networkCallback(send);

				// Clean up send
				delete send;
			}
		}

		bool IsAuthenticated()
		{
			return _user != NULL;
		}

		schema::User* GetUser()
		{
			return _user;
		}

		void OnPollUpdate(std::function<void(const schema::PollUpdateResponse& pollResponse)> callback)
		{
			this->_onPollUpdate = callback;
		}

		void AuthenticateWithPIN(const std::string client_id, const std::string pin)
		{
			schema::AuthenticateWithPINRequest packet(client_id, pin);

			auto send = new Send(to_string(packet));
			_sendQueue.push(send);
		}

		void GetPoll(const schema::string& pollId)
		{
			schema::GetPollRequest packet(pollId);

			auto send = new Send(to_string(packet));
			_sendQueue.push(send);
		}

		void CreatePoll(const schema::string& pollId, const schema::string& prompt, const std::vector<schema::string>& options)
		{
			schema::CreatePollRequest packet(pollId, prompt, options);

			auto send = new Send(to_string(packet));
			_sendQueue.push(send);
		}

		void SubscribeToPoll(const schema::string& pollId)
		{
			schema::SubscribePollRequest packet(pollId);

			auto send = new Send(to_string(packet));
			_sendQueue.push(send);
		}

	private:
		std::queue<Send*> _sendQueue;
		schema::User* _user;

		std::function<void(const schema::PollUpdateResponse& pollResponse)> _onPollUpdate;
	};
}

#endif
