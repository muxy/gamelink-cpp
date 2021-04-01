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

	namespace detail
	{
		template<typename T>
		class Callback
		{
		public:
			typedef void (*RawFunctionPointer)(void *, const T&);

			Callback()
				:_rawCallback(nullptr)
				,_user(nullptr)
			{}

			void invoke(const T& v)
			{
				if (_rawCallback)
				{
					_rawCallback(_user, v);
				}
				else if (_callback)
				{
					_callback(v);
				}
			}

			void set(std::function<void (const T&)> fn)
			{
				_rawCallback = nullptr;
				_user = nullptr;

				_callback = fn;
			}

			void set(RawFunctionPointer cb, void * user)
			{
				_rawCallback = cb;
				_user = user;

				_callback = std::function<void (const T&)>();
			}
		private:
			RawFunctionPointer _rawCallback;
			void* _user;

			std::function<void (const T&)> _callback;
		};
	}

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

		template<typename T>
		void ForeachSend(const T& networkCallback)
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

		typedef void (*SendCallback)(const Send*);
		void ForeachSend(SendCallback cb, void * user);

		bool IsAuthenticated() const;

		const schema::User* GetUser() const;

		// Callbacks
		void OnPollUpdate(std::function<void(const schema::PollUpdateResponse&)> callback);
		void OnPollUpdate(void (*callback)(void *, const schema::PollUpdateResponse&), void* ptr);

		void OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> callback);
		void OnAuthenticate(void (*callback)(void *, const schema::AuthenticateResponse&), void* ptr);

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

		detail::Callback<schema::PollUpdateResponse> _onPollUpdate;
		detail::Callback<schema::AuthenticateResponse> _onAuthenticate;
	};
}

#endif
