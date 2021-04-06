#pragma once
#ifndef INCLUDE_MUXY_GAMELINK_H
#define INCLUDE_MUXY_GAMELINK_H

#include "schema/schema.h"
#include <queue>

namespace gamelink
{
	class Payload
	{
	public:
		Payload(string data);

		string data;
	};

	namespace detail
	{
		template<typename T>
		class Callback
		{
		public:
			typedef void (*RawFunctionPointer)(void*, const T&);

			Callback()
				: _rawCallback(nullptr)
				, _user(nullptr)
			{
			}

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

			void set(std::function<void(const T&)> fn)
			{
				_rawCallback = nullptr;
				_user = nullptr;

				_callback = fn;
			}

			void set(RawFunctionPointer cb, void* user)
			{
				_rawCallback = cb;
				_user = user;

				_callback = std::function<void(const T&)>();
			}

			bool valid() const
			{
				if (_rawCallback) 
				{
					return true;
				}

				if (_callback)
				{
					return true;
				}

				return false;
			}
		private:
			RawFunctionPointer _rawCallback;
			void* _user;

			std::function<void(const T&)> _callback;
		};
	}

	
	class SDK
	{
	public:
		SDK();
		~SDK();

		bool ReceiveMessage(const char* bytes, uint32_t length);

		bool HasPayloads()
		{
			return _queuedPayloads.size() > 0;
		}

		template<typename T>
		void ForeachPayload(const T& networkCallback)
		{
			while (HasPayloads())
			{
				Payload* payload = _queuedPayloads.front();
				_queuedPayloads.pop();

				networkCallback(payload);

				// Clean up send
				delete payload;
			}
		}

		typedef void (*NetworkCallback)(const Payload*);
		void ForeachSend(NetworkCallback cb, void* user);

		bool IsAuthenticated() const;

		const schema::User* GetUser() const;

		/// Sets the OnDebugMessage callback. These messages are emitted
		/// for debugging purposes only.
		void OnDebugMessage(std::function<void(const string&)> callback);
		void OnDebugMessage(void (*callback)(void*, const string&), void *ptr);
		
		// Callbacks
		void OnPollUpdate(std::function<void(const schema::PollUpdateResponse&)> callback);
		void OnPollUpdate(void (*callback)(void*, const schema::PollUpdateResponse&), void* ptr);

		void OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> callback);
		void OnAuthenticate(void (*callback)(void*, const schema::AuthenticateResponse&), void* ptr);

		void OnStateUpdate(std::function<void(const schema::SubscribeStateUpdateResponse<nlohmann::json>&)> callback);
		void OnStateUpdate(void (*callback)(void*, const schema::SubscribeStateUpdateResponse<nlohmann::json>&), void* ptr);

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

		// Poll stuff, all async.

		/// Queues a request to get poll information, including results, for the poll with the given ID.
		/// Roughly equivilent to a single poll subscription update.
		/// Results are obtained through the OnPollUpdate callback.
		///
		/// @param[in] pollId The Poll ID to get information for
		void GetPoll(const string& pollId);

		/// Queues a request to create a poll.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] options An array of options to store in the poll.
		void CreatePoll(const string& pollId, const string& prompt, const std::vector<string>& options);

		/// Queues a request to create a poll.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] optionsBegin Pointer to the first element in an array of options to store in the poll.
		/// @param[in] optionsEnd Pointer one past the final entry in an array of options to store in the poll.
		void CreatePoll(const string& pollId, const string& prompt, const string* optionsBegin, const string* optionsEnd);

		/// Subscribes to updates for a given poll.
		/// Updates come through the OnPollUpdate callback.
		/// Once a poll stops receiving new votes, the subscription will stop receiving new updates.
		///
		/// @param[in] pollId The Poll ID to create
		void SubscribeToPoll(const string& pollId);

		/// Deletes the poll with the given ID.
		///
		/// @param[in] pollId 	The ID of the poll to delete.
		void DeletePoll(const string& pollId);

		// State operations, all async.

		/// Queues a request to replace the entirety of state with new information.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] value A serializable type. Will overwrite any existing state for the given target.
		///                  Cannot be an array or primitive type.
		template<typename T>
		void SetState(const char* target, const T& value)
		{
			nlohmann::json js = nlohmann::json(value);
			SetState(target, js);
		};

		/// Queues a request to replace the entirety of state with new information.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] value JSON. Will overwrite any existing state for the given target.
		///                  Must be an object, not an array or primitive type.
		void SetState(const char* target, const nlohmann::json& value);

		/// Queues a request to get state.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		void GetState(const char* target);

		/// Queues a request to do a single JSON Patch operation on the state object.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A JSON Patch operation
		/// @param[in] path A JSON Patch path.
		/// @param[in] atom The value to use in the patch operation
		void UpdateState(const char* target, const string& operation, const string& path, const schema::JsonAtom& atom);

		/// Queues a request to do many JSON Patch operations on the state object.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] begin Pointer to the first element in an array of UpdateOperations
		/// @param[in] end Pointer one past the last element in an array of UpdateOperations
		void UpdateState(const char* target, const schema::UpdateOperation* begin, const schema::UpdateOperation* end);

		/// Starts subscribing to state updates for the given target.
		/// Updates come through the OnStateUpdate callback
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		void SubscribeToStateUpdates(const char* target);
	private:
		void debugLogPayload(const Payload *);

		template<typename T>
		void queuePayload(const T& p)
		{
			Payload* payload = new Payload(to_string(p));
			debugLogPayload(payload);
			_queuedPayloads.push(payload);
		}

		std::queue<Payload*> _queuedPayloads;
		schema::User* _user;

		detail::Callback<string> _onDebugMessage;

		detail::Callback<schema::PollUpdateResponse> _onPollUpdate;
		detail::Callback<schema::AuthenticateResponse> _onAuthenticate;
		detail::Callback<schema::SubscribeStateUpdateResponse<nlohmann::json>> _onStateUpdate;
	};
}

#endif
