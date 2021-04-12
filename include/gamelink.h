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
		static const uint32_t CALLBACK_PERSISTENT = 0;
		static const uint32_t CALLBACK_ONESHOT = 1;
		static const uint32_t CALLBACK_ONESHOT_CONSUMED = 2;

		template<typename T>
		class Callback
		{
		public:
			typedef void (*RawFunctionPointer)(void*, const T&);

			Callback(uint32_t id, uint16_t targetRequestId, uint32_t oneShotStatus)
				: id(id)
				, targetRequestId(targetRequestId)
				, oneShotStatus(oneShotStatus)
				, _rawCallback(nullptr)
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

				if (oneShotStatus == CALLBACK_ONESHOT)
				{
					oneShotStatus = CALLBACK_ONESHOT_CONSUMED;
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

			void clear()
			{
				_rawCallback = nullptr;
				_user = nullptr;

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

			uint32_t id;
			uint16_t targetRequestId;
			uint32_t oneShotStatus;

		private:
			RawFunctionPointer _rawCallback;
			void* _user;

			std::function<void(const T&)> _callback;
		};

		static const uint16_t ANY_REQUEST_ID = 0xFFFF;

		template<typename T, uint8_t IDMask>
		class CallbackCollection
		{
		public:
			CallbackCollection()
				: currentHandle(0)
			{
			}

			typedef void (*RawFunctionPointer)(void*, const T&);

			bool validateId(uint32_t id)
			{
				// Check the ID byte for consistency.
				return ((id >> 24) & 0xFF) == IDMask;
			}

			uint32_t set(std::function<void(const T&)> fn, uint16_t requestId, uint32_t flags)
			{
				uint32_t id = nextID();
				Callback<T> cb(id, requestId, flags);
				cb.set(fn);

				callbacks.emplace_back(std::move(cb));

				return id;
			}

			uint32_t set(RawFunctionPointer fn, void* user, uint16_t requestId, uint32_t flags)
			{
				uint32_t id = nextID();
				Callback<T> cb(id, requestId, flags);
				cb.set(fn, user);

				callbacks.emplace_back(std::move(cb));

				return id;
			}

			void remove(uint32_t id)
			{
				callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), [id](const Callback<T>& cb) { return cb.id == id; }),
								callbacks.end());
			}

			void invoke(const T& v)
			{
				uint16_t requestId = v.meta.request_id;
				for (uint32_t i = 0; i < callbacks.size(); ++i)
				{
					if (callbacks[i].targetRequestId == ANY_REQUEST_ID || callbacks[i].targetRequestId == requestId)
					{
						callbacks[i].invoke(v);
					}
				}

				callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
											   [](const Callback<T>& cb) { return cb.oneShotStatus == CALLBACK_ONESHOT_CONSUMED; }),
								callbacks.end());
			}

		private:
			uint32_t nextID()
			{
				// Store a byte to determine if the id returned from a set operation belongs to this
				// collection of callbacks.
				static const uint32_t MASK = 0x0FFFFFFFu;
				uint32_t id = (currentHandle & (MASK)) | (static_cast<uint32_t>(IDMask) << 24);
				currentHandle = (currentHandle + 1) & 0x0FFFFFFFu;
				return id;
			}

			uint32_t currentHandle;
			std::vector<Callback<T>> callbacks;
		};
	}

	/// Not thread safe.
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

		typedef void (*NetworkCallback)(void*, const Payload*);
		void ForeachPayload(NetworkCallback cb, void* user);

		bool IsAuthenticated() const;

		const schema::User* GetUser() const;

		/// Sets the OnDebugMessage callback. These messages are emitted
		/// for debugging purposes only. There can only be one OnDebugMessage callback registered.
		void OnDebugMessage(std::function<void(const string&)> callback);
		void OnDebugMessage(void (*callback)(void*, const string&), void* ptr);
		void DetachOnDebugMessage();

		// Callbacks.
		uint32_t OnPollUpdate(std::function<void(const schema::PollUpdateResponse&)> callback);
		uint32_t OnPollUpdate(void (*callback)(void*, const schema::PollUpdateResponse&), void* ptr);
		void DetachOnPollUpdate(uint32_t id);

		uint32_t OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> callback);
		uint32_t OnAuthenticate(void (*callback)(void*, const schema::AuthenticateResponse&), void* ptr);
		void DetachOnAuthenticate(uint32_t id);

		uint32_t OnStateUpdate(std::function<void(const schema::SubscribeStateUpdateResponse<nlohmann::json>&)> callback);
		uint32_t OnStateUpdate(void (*callback)(void*, const schema::SubscribeStateUpdateResponse<nlohmann::json>&), void* ptr);
		void DetachOnStateUpdate(uint32_t id);

		uint32_t OnTwitchPurchaseBits(std::function<void(const schema::TwitchPurchaseBitsResponse<nlohmann::json>&)> callback);
		uint32_t OnTwitchPurchaseBits(void (*callback)(void*, const schema::TwitchPurchaseBitsResponse<nlohmann::json>&), void* ptr);
		void DetachOnTwitchPurchaseBits(uint32_t id);

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
		/// This will generate a StateUpdate subscription event.
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
		/// This will generate a StateUpdate subscription event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] value JSON. Will overwrite any existing state for the given target.
		///                  Must be an object, not an array or primitive type.
		void SetState(const char* target, const nlohmann::json& value);

		/// Queues a request to get state.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		void GetState(const char* target);

		/// Queues a request to get state, using the passed in callback to receive the results.
		/// This callback is called only once.
		void GetState(const char* target, std::function<void(const schema::GetStateResponse<nlohmann::json>&)> callback);
		void GetState(const char* target, void (*callback)(void*, const schema::GetStateResponse<nlohmann::json>&), void* user);

		/// Queues a request to do a single JSON Patch operation on the state object.
		/// This will generate a StateUpdate subscription event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A JSON Patch operation
		/// @param[in] path A JSON Patch path.
		/// @param[in] atom The value to use in the patch operation
		void UpdateState(const char* target, const string& operation, const string& path, const schema::JsonAtom& atom);

		/// Queues a request to do many JSON Patch operations on the state object.
		/// This will generate a StateUpdate subscription event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] begin Pointer to the first element in an array of UpdateOperations
		/// @param[in] end Pointer one past the last element in an array of UpdateOperations
		void UpdateState(const char* target, const schema::PatchOperation* begin, const schema::PatchOperation* end);

		/// Starts subscribing to state updates for the given target.
		/// Updates come through the OnStateUpdate callback
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		void SubscribeToStateUpdates(const char* target);

		template<typename T>
		void SendBroadcast(const string& topic, const T& value)
		{
			nlohmann::json js = nlohmann::json(value);
			SendBroadcast(topic, js);
		}

		// Sends a json mesasge to all users
		void SendBroadcast(const string& topic, const nlohmann::json& message);
	private:
		void debugLogPayload(const Payload*);

		template<typename T>
		void queuePayload(const T& p)
		{
			Payload* payload = new Payload(to_string(p));
			debugLogPayload(payload);
			_queuedPayloads.push(payload);
		}

		std::queue<Payload*> _queuedPayloads;
		schema::User* _user;

		uint16_t currentRequestId;
		uint16_t nextRequestId();

		detail::Callback<string> _onDebugMessage;
		detail::CallbackCollection<schema::PollUpdateResponse, 1> _onPollUpdate;
		detail::CallbackCollection<schema::AuthenticateResponse, 2> _onAuthenticate;
		detail::CallbackCollection<schema::SubscribeStateUpdateResponse<nlohmann::json>, 3> _onStateUpdate;
		detail::CallbackCollection<schema::GetStateResponse<nlohmann::json>, 4> _onGetState;
		detail::CallbackCollection<schema::TwitchPurchaseBitsResponse<nlohmann::json>, 5> _onTwitchPurchaseBits;
	};
}

#endif
