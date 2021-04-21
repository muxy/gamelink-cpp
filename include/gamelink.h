#pragma once
#ifndef INCLUDE_MUXY_GAMELINK_H
#define INCLUDE_MUXY_GAMELINK_H

#include "schema/schema.h"
#include <deque>

namespace gamelink
{
	// Utility function
	template<typename T>
	const schema::Error* FirstError(const schema::ReceiveEnvelope<T>& recv)
	{
		if (recv.errors.empty())
		{
			return NULL;
		}

		return &recv.errors[0];
	}
	
	class Payload
	{
	public:
		explicit Payload(string data);

		string data;
	};

	namespace detail
	{
		static const uint32_t CALLBACK_PERSISTENT = 0;
		static const uint32_t CALLBACK_ONESHOT = 1;
		static const uint32_t CALLBACK_REMOVED = 2;

		template<typename T>
		class Callback
		{
		public:
			typedef void (*RawFunctionPointer)(void*, const T&);

			Callback(uint32_t id, uint16_t targetRequestId, uint32_t status)
				: _id(id)
				, _targetRequestId(targetRequestId)
				, _status(status)
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

			uint32_t _id;
			uint16_t _targetRequestId;
			uint32_t _status;
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
				: _currentHandle(0)
			{
			}

			~CallbackCollection()
			{
				for (uint32_t i = 0; i < _callbacks.size(); ++i)
				{
					delete _callbacks[i];
				}
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
				Callback<T>* cb = new Callback<T>(id, requestId, flags);
				cb->set(fn);

				_lock.lock();
				_callbacks.push_back(cb);
				_lock.unlock();

				return id;
			}

			uint32_t set(RawFunctionPointer fn, void* user, uint16_t requestId, uint32_t flags)
			{
				uint32_t id = nextID();
				Callback<T>* cb = new Callback<T>(id, requestId, flags);
				cb->set(fn, user);

				_lock.lock();
				_callbacks.push_back(cb);
				_lock.unlock();

				return id;
			}

			void remove(uint32_t id)
			{
				_lock.lock();
				for (uint32_t i = 0; i < _callbacks.size(); ++i)
				{
					if (_callbacks[i]->_id == id)
					{
						_callbacks[i]->_status = CALLBACK_REMOVED;
					}
				}
				_lock.unlock();
			}

			// This must not be called recursively.
			void invoke(const T& v)
			{
				std::vector<Callback<T>*> copy;
				uint16_t requestId = v.meta.request_id;

				_lock.lock();
				copy = _callbacks;
				_lock.unlock();

				// Invoke the copied callbacks.
				for (uint32_t i = 0; i < copy.size(); ++i)
				{
					if (copy[i]->_targetRequestId == ANY_REQUEST_ID || copy[i]->_targetRequestId == requestId)
					{
						// Invoke if valid to do so.
						if (copy[i]->_status == CALLBACK_PERSISTENT || copy[i]->_status == CALLBACK_ONESHOT)
						{
							copy[i]->invoke(v);
						}

						if (copy[i]->_status == CALLBACK_ONESHOT)
						{
							copy[i]->_status = CALLBACK_REMOVED;
						}
					}
				}

				_lock.lock();
				auto it = std::remove_if(_callbacks.begin(), _callbacks.end(), [](const Callback<T>* cb)
				{
					if (cb->_status == CALLBACK_REMOVED)
					{
						delete cb;
						return true;
					}
					return false;
				});

				_callbacks.erase(it, _callbacks.end());
				_lock.unlock();
			}

		private:
			uint32_t nextID()
			{
				// Store a byte to determine if the id returned from a set operation belongs to this
				// collection of callbacks.
				static const uint32_t MASK = 0x0FFFFFFFu;
				uint32_t id = (_currentHandle & (MASK)) | (static_cast<uint32_t>(IDMask) << 24);
				_currentHandle = (_currentHandle + 1) & 0x0FFFFFFFu;
				return id;
			}

			uint32_t _currentHandle;
			gamelink::lock _lock;

			std::vector<Callback<T>*> _callbacks;
		};
	}

	/// Not thread safe.
	class SDK
	{
	public:
		SDK();
		~SDK();

		// Not implemented. SDK is not copyable
		SDK(const SDK&);
		SDK& operator=(const SDK&);

		// Not implemented. SDK is not movable
		SDK(SDK&&);
		SDK& operator=(SDK&&);

		/// Receives a character buffer as a message. This function
		/// may invoke callbacks.
		///
		/// @param[in] bytes Pointer to contiguous array of bytes that represent a network message.
		/// @param[in] length Length of the bytes array.
		/// @return Returns true if the message was parsed correctly.
		bool ReceiveMessage(const char* bytes, uint32_t length);

		/// Call this after a websocket reconnect after disconnect.
		/// This queues in an authorization message before any additional
		/// messages are sent.
		void HandleReconnect();

		/// Returns true if there are a non-zero amount of payloads to send.
		///
		/// @return returns if there are payloads to send.
		bool HasPayloads() const;

		/// Invokes a callable type for each avaliable payload.
		///
		/// @param[in] networkCallback callback invoked once for each available payload.
		///                            must be in the form networkCallback(const Payload*)
		template<typename T>
		void ForeachPayload(const T& networkCallback)
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
					networkCallback(payload);
					delete payload;
				}
			}
		}

		typedef void (*NetworkCallback)(void*, const Payload*);

		/// Invokes a function pointer for each avaliable payload.
		///
		/// @param[in] cb Callback to be invoked for each avaliable payload
		/// @param[in] user User pointer that is passed into the callback
		void ForeachPayload(NetworkCallback cb, void* user);

		/// Returns if an authentication message has been received.
		///
		/// @return true if an authentication message has been received.
		bool IsAuthenticated() const;

		/// Gets the currently authenticated user.
		///
		/// @return The currently authenticated user, or null if no authentication message
		///         has been recieved.
		const schema::User* GetUser() const;

		/// Gets the ClientID that was last passed into AuthenticateWithPIN or AuthenticateWithJWT
		///
		/// @return c-string representation of the input ClientID
		const char * GetClientId() const;	

		/// Sets the OnDebugMessage callback. This is invoked for debugging purposes only.
		/// There can only be one OnDebugMessage callback registered.
		///
		/// @param[in] callback Callback to log a debug message
		void OnDebugMessage(std::function<void(const string&)> callback);

		/// Sets the OnDebugMessage callback with a function pointer and user pointer.
		/// This is invoked for debugging purposes only.
		/// There can only be one OnDebugMessage callback registered.
		///
		/// @param[in] callback Callback to log a debug message.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		void OnDebugMessage(void (*callback)(void*, const string&), void* ptr);

		/// Detaches the OnDebugMessage callback, so no additional calls will be made.
		void DetachOnDebugMessage();

		/// Sets the OnPollUpdate callback. This callback is invoked after SubscribeToPoll is called.
		/// @remark SubscribeToPoll takes in a poll id, but can be called multiple times with different poll ids.
		///        Callbacks registered through OnPollUpdate receive all update messages, regardless of poll id.
		///        Callbacks that are designed to only get updates for a specific poll id should test the poll id
		///        from within the callback itself.
		///
		/// @param[in] callback Callback to invoke when a poll update message is received
		/// @return Returns an integer handle to the callback, to be used in DetachOnPollUpdate.
		uint32_t OnPollUpdate(std::function<void(const schema::PollUpdateResponse&)> callback);

		/// Sets the OnPollUpdate callback. This callback is invoked after SubscribeToPoll is called.
		/// See the std::function overload for remarks.
		///
		/// @param[in] callback Callback to invoke when a poll update message is received
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnPollUpdate.
		uint32_t OnPollUpdate(void (*callback)(void*, const schema::PollUpdateResponse&), void* ptr);

		/// Detaches an OnPollUpdate callback.
		///
		/// @param[in] id A handle obtained from calling OnPollUpdate. Invalid handles are ignored.
		void DetachOnPollUpdate(uint32_t id);

		/// Sets the OnAuthenticate callback. This callback is invoked when an authentication
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when an authentication message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnAuthenticate.
		uint32_t OnAuthenticate(std::function<void(const schema::AuthenticateResponse&)> callback);

		/// Sets the OnAuthenticate callback. This callback is invoked when an authentication
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when an authentication message is received.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnAuthenticate.
		uint32_t OnAuthenticate(void (*callback)(void*, const schema::AuthenticateResponse&), void* ptr);

		/// Detaches an OnAuthenticate callback.
		///
		/// @param[in] id A handle obtained from calling OnAuthenticate. Invalid handles are ignored.
		void DetachOnAuthenticate(uint32_t id);

		/// Sets the OnStateUpdate callback. This callback is invoked when a state update
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when a state update message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnStateUpdate.
		uint32_t OnStateUpdate(std::function<void(const schema::SubscribeStateUpdateResponse<nlohmann::json>&)> callback);

		/// Sets the OnStateUpdate callback. This callback is invoked when a state update
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when a state update message is received.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnStateUpdate.
		uint32_t OnStateUpdate(void (*callback)(void*, const schema::SubscribeStateUpdateResponse<nlohmann::json>&), void* ptr);

		/// Detaches an OnStateUpdate callback.
		///
		/// @param[in] id A handle obtained from calling OnStateUpdate. Invalid handles are ignored.
		void DetachOnStateUpdate(uint32_t id);

		/// Starts subscribing to TwitchPurchaseBits updates for a specific SKU
		void SubscribeToSKU(const string& sku);

		/// Subscribes to all SKUs.
		void SubscribeToAllPurchases();

		/// Sets the OnTwitchPurchaseBits callback. This callback is invoked when twitch purchase
		/// message is received.
		/// @remarks The twitch purchase message has been authenticated and deduplicated by the server.
		///          This callback receives all SKUs purchased, so a callback for a specific SKU should
		///          test the SKU in the callback.
		///
		/// @param[in] callback Callback to invoke when a twitch purchase message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnTwitchPurchaseBits.
		uint32_t OnTwitchPurchaseBits(std::function<void(const schema::TwitchPurchaseBitsResponse<nlohmann::json>&)> callback);

		/// Sets the OnTwitchPurchaseBits callback. This callback is invoked when twitch purchase
		/// message is received.
		/// See the std::function overload for remarks.
		///
		/// @param[in] callback Callback to invoke when a twitch purchase message is received.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnTwitchPurchaseBits.
		uint32_t OnTwitchPurchaseBits(void (*callback)(void*, const schema::TwitchPurchaseBitsResponse<nlohmann::json>&), void* ptr);

		/// Detaches an OnTwitchPurchaseBits callback.
		///
		/// @param[in] id A handle obtained from calling OnTwitchPurchaseBits. Invalid handles are ignored.
		void DetachOnTwitchPurchaseBits(uint32_t id);

		/// Queues an authentication request using a PIN code, as received by the user from an
		/// extension's config view.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] pin 		The PIN input from the broadcaster
		void AuthenticateWithPIN(const string& clientId, const string& pin);

		/// Queues an authentication request using a PIN code, as received by the user from an
		/// extension's config view.
		/// This overload attaches a one-shot callback to be called when the authentication response
		/// message is received.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] pin 		The PIN input from the broadcaster
		/// @param[in] callback Callback that is invoked once when this authentication request
		///                     is responded to.
		void
		AuthenticateWithPIN(const string& clientId, const string& pin, std::function<void(const schema::AuthenticateResponse&)> callback);

		/// Queues an authentication request using a PIN code, as received by the user from an
		/// extension's config view.
		/// This overload attaches a one-shot callback to be called when the authentication response
		/// message is received.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] pin 		The PIN input from the broadcaster
		/// @param[in] callback Callback that is invoked once when this authentication request
		///                     is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		void AuthenticateWithPIN(const string& clientId,
								 const string& pin,
								 void (*callback)(void*, const schema::AuthenticateResponse&),
								 void* user);

		/// Queues an authentication request using a JWT, as received after a successful PIN authentication request.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] jwt 		The stored JWT from a previous authentication
		void AuthenticateWithJWT(const string& clientId, const string& jwt);

		/// Queues an authentication request using a JWT, as received after a successful PIN authentication request.
		/// This overload attaches a one-shot callback to be called when the authentication response
		/// message is received.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] jwt 		The stored JWT from a previous authentication
		/// @param[in] callback Callback that is invoked once when this authentication request
		///                     is responded to.
		void
		AuthenticateWithJWT(const string& clientId, const string& pin, std::function<void(const schema::AuthenticateResponse&)> callback);

		/// Queues an authentication request using a JWT, as received after a successful PIN authentication request.
		/// This overload attaches a one-shot callback to be called when the authentication response
		/// message is received.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] jwt 		The stored JWT from a previous authentication
		/// @param[in] callback Callback that is invoked once when this authentication request
		///                     is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		void AuthenticateWithJWT(const string& clientId,
								 const string& pin,
								 void (*callback)(void*, const schema::AuthenticateResponse&),
								 void* user);

		// Poll stuff, all async.

		/// Queues a request to get poll information, including results, for the poll with the given ID.
		/// Roughly equivilent to a single poll subscription update.
		/// Results are obtained through the OnPollUpdate callback.
		///
		/// @param[in] pollId The Poll ID to get information for
		void GetPoll(const string& pollId);

		/// Queues a request to get poll information. This overload attaches a one-shot callback to be
		/// called when poll information is received.
		///
		/// @param[in] pollId   The Poll ID to get information for
		/// @param[in] callback Callback invoked when this get poll request is responded to.
		void GetPoll(const string& pollId, std::function<void(const schema::GetPollResponse&)> callback);

		/// Queues a request to get poll information. This overload attaches a one-shot callback to be
		/// called when poll information is received.
		///
		/// @param[in] pollId   The Poll ID to get information for
		/// @param[in] callback Callback invoked when this get poll request is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		void GetPoll(const string& pollId, void (*callback)(void*, const schema::GetPollResponse&), void* user);

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
		/// @param[in] pollId The Poll ID to subscribe to
		void SubscribeToPoll(const string& pollId);

		/// Unsubscribes to updates for a given poll
		///
		/// @param[in] pollId The Poll ID to unsubscribe to
		void UnsubscribeToPoll(const string& pollId);

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

		/// Queues a request to get state. This overload attaches a one-shot callback to be
		/// called when state is received.
		///
		/// @param[in] target   Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] callback Callback invoked when this state request is responded to.
		void GetState(const char* target, std::function<void(const schema::GetStateResponse<nlohmann::json>&)> callback);

		/// Queues a request to get state. This overload attaches a one-shot callback to be
		/// called when state is received.
		///
		/// @param[in] target   Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] callback Callback invoked when this state request is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		void GetState(const char* target, void (*callback)(void*, const schema::GetStateResponse<nlohmann::json>&), void* user);

		/// Queues a request to do a single JSON Patch operation on the state object.
		/// This will generate a StateUpdate event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A JSON Patch operation
		/// @param[in] path A JSON Patch path.
		/// @param[in] atom The value to use in the patch operation
		void UpdateState(const char* target, const string& operation, const string& path, const schema::JsonAtom& atom);

		/// Queues a request to do many JSON Patch operations on the state object.
		/// This will generate a StateUpdate event.
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

		/// Sends a broadcast to all viewers on the channel using the extension.
		/// @remark The serialized size of the value parameter must be under 8 kilobytes.
		///
		/// @param[in] topic The topic of the message to send. The frontend uses this value
		///                  to filter messages.
		/// @param[in] value Serializable, arbitrary object.
		template<typename T>
		void SendBroadcast(const string& topic, const T& value)
		{
			nlohmann::json js = nlohmann::json(value);
			SendBroadcast(topic, js);
		}

		/// Sends a broadcast to all viewers on the channel using the extension.
		/// @remark The serialized size of the message parameter must be under 8 kilobytes.
		///
		/// @param[in] topic The topic of the message to send. The frontend uses this value
		///                  to filter messages.
		/// @param[in] message Arbitrary json object. May not be a primitive or array.
		void SendBroadcast(const string& topic, const nlohmann::json& message);

		/// Sends a request to subscribe to the datastream.
		void SubscribeToDatastream();

		/// Sets a OnDatastream callback. This callback is invoked when a datastream update
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when a datastream update message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnDatastream.
		uint32_t OnDatastream(std::function<void(const schema::DatastreamUpdate&)> callback);

		/// Sets a OnDatastream callback. This callback is invoked when a datastream update
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when a datastream update message is received.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnDatastream.
		uint32_t OnDatastream(void (*callback)(void*, const schema::DatastreamUpdate&), void* user);

		/// Detaches an OnDatastream callback.
		///
		/// @param[in] id A handle obtained from calling OnDatastream. Invalid handles are ignored.
		void DetachOnDatastream(uint32_t);
	private:
		void debugLogPayload(const Payload*);

		template<typename T>
		void queuePayload(const T& p)
		{
			Payload* payload = new Payload(gamelink::string(to_string(p).c_str()));
			debugLogPayload(payload);

			_lock.lock();
			_queuedPayloads.push_back(payload);
			_lock.unlock();
		}

		// Fields stored to handle reconnects
		gamelink::string _storedJWT;
		gamelink::string _storedClientId;

		gamelink::lock _lock;

		std::deque<Payload*> _queuedPayloads;
		schema::User* _user;

		uint16_t _currentRequestId;
		uint16_t nextRequestId();

		detail::Callback<string> _onDebugMessage;

		detail::CallbackCollection<schema::PollUpdateResponse, 1> _onPollUpdate;
		detail::CallbackCollection<schema::AuthenticateResponse, 2> _onAuthenticate;
		detail::CallbackCollection<schema::SubscribeStateUpdateResponse<nlohmann::json>, 3> _onStateUpdate;
		detail::CallbackCollection<schema::GetStateResponse<nlohmann::json>, 4> _onGetState;
		detail::CallbackCollection<schema::TwitchPurchaseBitsResponse<nlohmann::json>, 5> _onTwitchPurchaseBits;
		detail::CallbackCollection<schema::GetPollResponse, 6> _onGetPoll;
		detail::CallbackCollection<schema::DatastreamUpdate, 7> _onDatastreamUpdate;
	};
}

#endif
