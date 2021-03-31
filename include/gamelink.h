#pragma once
#ifndef INCLUDE_MUXY_GAMELINK_H
#define INCLUDE_MUXY_GAMELINK_H

#include <fmt/format.h>
#include <queue>

#include "schema/schema.h"

namespace gamelink
{
	class Send
	{
	public:
		Send(string data);

		string data;
	};

	class SDK
	{
	public:
		SDK();
		~SDK();

		bool ReceiveMessage(const char * bytes, uint32_t length);

		bool HasSends()
		{
			return _sendQueue.size() > 0;
		}

		void ForeachSend(const std::function<void(const Send* send)>& networkCallback);

		bool IsAuthenticated();

		schema::User* GetUser();

		void OnPollUpdate(std::function<void(const schema::PollUpdateResponse& pollResponse)> callback);

		/// Queues an authentication request using a PIN code, as received by the user from an extension's config view.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] pin 		The PIN input from the broadcaster
		void AuthenticateWithPIN(const string& clientId, const string& pin);

		/// Queues an authentication request using a JWT, as received after a successful PIN authentication request.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] jwt 		The stored JWT from a previous authentication
		void AuthenticateWithJWT(const string& clientId, const string& jwt);

		void GetPoll(const string& pollId);

		void CreatePoll(const string& pollId, const string& prompt, const std::vector<string>& options);

		void SubscribeToPoll(const string& pollId);

		/// Deletes the poll with the given ID.
		///
		/// @param[in] pollId 	The ID of the poll to delete.
		void DeletePoll(const string& pollId);

	private:
		std::queue<Send*> _sendQueue;
		schema::User* _user;

		std::function<void(const schema::PollUpdateResponse& pollResponse)> _onPollUpdate;
	};
}

#endif
