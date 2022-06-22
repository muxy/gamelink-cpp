#pragma once
#ifndef INCLUDE_MUXY_GAMELINK_H
#define INCLUDE_MUXY_GAMELINK_H

#include "schema/schema.h"
#include <deque>

namespace gamelink
{
	/// FirstError gets a pointer to the first error in the errors array in a
	/// receive envelope if it exists.
	///
	/// @param[in] recv A receive envelope
	/// @returns Pointer to first error in the errors array of the envelope. If no
	///          such error exists, returns the null pointer.
	MUXY_GAMELINK_API const schema::Error* FirstError(const schema::ReceiveEnvelopeCommon& recv);

	/// HasPrefix tests whether the string 's' begins with 'prefix'.
	///
	/// @param[in] s The string to test.
	/// @param[in] prefix The prefix to test for
	/// @returns True if 's' begins with 'prefix'
	MUXY_GAMELINK_API bool HasPrefix(const string& source, const string& prefix);

	/// GetPollWinnerIndex grabs the index of the winning result.
	///
	/// @param[in] results The results from the poll
	/// @returns Index of winning result

	MUXY_GAMELINK_API uint32_t GetPollWinnerIndex(const std::vector<int>& results);

	/// RequestId is an 16bit unsigned integer that represents a request.
	/// Obtained through SDK methods.
	typedef uint16_t RequestId;

	/// Constant RequestId that represents the "don't care" request id.
	static const RequestId ANY_REQUEST_ID = 0xFFFF;

	/// Payload represents a block of data to be sent through the websocket.
	class MUXY_GAMELINK_API Payload
	{
		friend class SDK;
		explicit Payload(string data);
		RequestId waitingForResponse;

	public:
		/// Data to be sent.
		const string data;
	};

	enum ConnectionStage
	{
		CONNECTION_STAGE_PRODUCTION = 0,
		CONNECTION_STAGE_SANDBOX,
	};

	class SDK;

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

			Callback()
				: _id(UINT32_MAX)
				, _targetRequestId(ANY_REQUEST_ID)
				, _status(UINT32_MAX)
				, _name("")
				, _rawCallback(nullptr)
				, _user(nullptr)
			{
			}
			Callback(uint32_t id, RequestId targetRequestId, uint32_t status, string name)
				: _id(id)
				, _targetRequestId(targetRequestId)
				, _status(status)
				, _name(std::move(name))
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
			RequestId _targetRequestId;
			uint32_t _status;
			string _name;
		private:
			RawFunctionPointer _rawCallback;
			void* _user;

			std::function<void(const T&)> _callback;
		};

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
				// Set a name that is fairly unlikely to be used in removeByname
				// so that removeByName("") doesn't remove all callbacks by mistake.
				Callback<T>* cb = new Callback<T>(id, requestId, flags, "??#_muxy");
				cb->set(fn);

				_lock.lock();
				_callbacks.push_back(cb);
				_lock.unlock();

				return id;
			}

			uint32_t set(RawFunctionPointer fn, void* user, uint16_t requestId, uint32_t flags)
			{
				uint32_t id = nextID();
				Callback<T>* cb = new Callback<T>(id, requestId, flags, "??#_muxy");
				cb->set(fn, user);

				_lock.lock();
				_callbacks.push_back(cb);
				_lock.unlock();

				return id;
			}

			uint32_t setUnique(string name, std::function<void(const T&)> fn, uint16_t requestId, uint32_t flags)
			{
				uint32_t id = nextID();
				Callback<T>* cb = new Callback<T>(id, requestId, flags, name);
				cb->set(fn);

				removeByName(name);

				_lock.lock();
				_callbacks.push_back(cb);
				_lock.unlock();

				return id;
			}

			uint32_t setUnique(string name, RawFunctionPointer fn, void* user, uint16_t requestId, uint32_t flags)
			{
				uint32_t id = nextID();
				Callback<T>* cb = new Callback<T>(id, requestId, flags, name);
				cb->set(fn, user);

				removeByName(name);

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

			void removeByName(const string& name)
			{
				_lock.lock();
				for (uint32_t i = 0; i < _callbacks.size(); ++i)
				{
					if (_callbacks[i]->_name == name)
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
				auto it = std::remove_if(_callbacks.begin(), _callbacks.end(), [](const Callback<T>* cb) {
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

		/// Returns the URL to connect for the given clientID, stage, projection
		/// and projection version. This function should be called instead of
		/// WebsocketConnectionURL when writing a projection into a different language.
		///
		/// @param[in] clientId The extension's client ID.
		/// @param[in] stage The stage to connect to, either CONNECTION_STAGE_PRODUCTION or
		///                  CONNECTION_STAGE_SANDBOX.
		/// @param[in] projection The projection name. Must be under 8 characters, and must
		/// 					  be a URL-safe string. Examples are 'c' or 'csharp'
		/// @param[in] projectionMajor The major version of this projection.
		/// @param[in] projectionMinor The minor version of this projection.
		/// @param[in] projectionPatch The patch version of this projection.
		/// @return Returns the URL to connect to. Returns an empty string on error.
		MUXY_GAMELINK_API string ProjectionWebsocketConnectionURL(const string& clientId,
																  ConnectionStage stage,
																  const string& projection,
																  int projectionMajor,
																  int projectionMinor,
																  int projectionPatch);

		// TimedPoll is used internally to hold data from CreateTimedPoll
		struct TimedPoll
		{
			string pollId;
			float duration;
			detail::Callback<const schema::GetPollResponse&> onFinishCallback;
			bool finished;

			TimedPoll(string pollId, float duration)
				: pollId(pollId)
				, duration(duration)
				, onFinishCallback(0, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT, string(""))
				, finished(false)
			{
			}
		};

		enum class SubscriptionState
		{
			Inactive,
			Active
		};

		// SubscriptionSets is used to hold the state of subscriptions
		class SubscriptionSets
		{
			friend class gamelink::SDK;
		public:
			// All methods should be called under a lock.
			void replay(SDK* sdk);

			bool canRegisterSKU(const string& sku);
			void registerSKU(const string& sku);
			void unregisterSKU(const string& sku);

			bool canRegisterPoll(const string& pid);
			void registerPoll(const string& pid);
			void unregisterPoll(const string& pid);

			bool canRegisterConfigurationChanges(ConfigTarget target);
			void registerConfigurationChanges(ConfigTarget target);
			void unregisterConfigurationChanges(ConfigTarget target);

			bool canRegisterStateUpdate(StateTarget target);
			void registerStateUpdates(StateTarget target);
			void unregisterStateUpdates(StateTarget target);

			bool canRegisterDatastream();
			void registerDatastream();
			void unregisterDatastream();

			bool canRegisterMatchmakingQueueInvite();
			void registerMatchmakingQueueInvite();
			void unregisterMatchmakingQueueInvite();
		private:
			struct Subscription
			{
				SubscriptionState state;
			};

			struct SubscriptionWithString : Subscription
			{
				string target;
			};

			std::vector<SubscriptionWithString> _polls;
			std::vector<SubscriptionWithString> _skus;

			Subscription _configurationChanges[static_cast<int>(ConfigTarget::ConfigTargetCount)];
			Subscription _stateSubscriptions[static_cast<int>(StateTarget::StateCount)];

			Subscription _datastream;
			Subscription _matchmakingInvite;

			gamelink::lock _lock;
		};
	}

	/// Returns the URL to connect to for the given clientID and stage.
	/// This returned URL doesn't have the protocol ('ws://' or 'wss://') prefix.
	///
	/// @param[in] clientId The extension's client ID.
	/// @param[in] stage The stage to connect to, either CONNECTION_STAGE_PRODUCTION or
	///                  CONNECTION_STAGE_SANDBOX.
	/// @return Returns the URL to connect to. Returns an empty string on error.
	MUXY_GAMELINK_API string WebsocketConnectionURL(const string& clientId, ConnectionStage stage);

	// The PatchList set provides an easy api to generate a list of patches and send them
	// in one request.
	class MUXY_GAMELINK_API PatchList
	{
		friend class SDK;
	public:
		PatchList();
		explicit PatchList(uint32_t preallocate);

		PatchList(const PatchList&) = delete;
		PatchList& operator=(const PatchList&) = delete;
		PatchList(PatchList&&) = delete;
		PatchList& operator=(const PatchList&&) = delete;

		/// Queues a request to do many JSON Patch operations on the state object.
		/// This will generate a StateUpdate event.
		///
		/// @param[in] begin Pointer to the first element in an array of UpdateOperations
		/// @param[in] end Pointer one past the last element in an array of UpdateOperations
		void UpdateState(const schema::PatchOperation* begin, const schema::PatchOperation* end);

		/// Helper function that will call UpdateState with the input object as the value.
		///
		/// @param[in] path A JSON Patch path.
		/// @param[in] obj The value to use in the patch operation
		template<typename T>
		void UpdateStateWithObject(Operation operation, const string& path, const T& obj)
		{
			nlohmann::json js = nlohmann::json(obj);
			return UpdateStateWithJson(operation, path, js);
		}

		/// Helper function that will call UpdateState with the input array as the value.
		///
		/// @param[in] path A JSON Patch path.
		/// @param[in] begin Pointer to the first element in an array of serializable objects.
		/// @param[in] end Pointer to one past the last element in an array of serializable objects.
		template<typename T>
		void UpdateStateWithArray(Operation operation, const string& path, const T* begin, const T* end)
		{
			nlohmann::json js = nlohmann::json::array();

			int index = 0;
			while (begin != end)
			{
				js[index++] = nlohmann::json(*begin);
				++begin;
			}

			return UpdateStateWithJson(operation, path, js);
		}

		/// Helper function that will call UpdateState with the input integer as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] i The value to use in the patch operation
		void UpdateStateWithInteger(Operation operation, const string& path, int64_t i);

		/// Helper function that will call UpdateState with the input double as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] d The value to use in the patch operation
		void UpdateStateWithDouble(Operation operation, const string& path, double d);

		/// Helper function that will call UpdateState with a boolean value
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] b The value to use in the patch operation
		void UpdateStateWithBoolean(Operation operation, const string& path, bool b);

		/// Helper function that will call UpdateState with the input string as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] s The value to use in the patch operation
		void UpdateStateWithString(Operation operation, const string& path, const string& s);

		/// Helper function that will call UpdateState with the input json object literal as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] js The value to use in the patch operation
		void UpdateStateWithLiteral(Operation operation, const string& path, const string& js);

		/// Helper function that will call UpdateState with null as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		void UpdateStateWithNull(Operation operation, const string& path);

		/// Helper function that will call UpdateState the input json object as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] js The value to use in the patch operation
		void UpdateStateWithJson(Operation operation, const string& path, const nlohmann::json& js);

		/// Helper function that will update state with an empty array
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		void UpdateStateWithEmptyArray(Operation operation, const string& path);

		/// Check if the PatchList is empty
		bool Empty() const;

		/// Clear the PatchList
		void Clear();

	private:
		gamelink::lock lock;
		std::vector<schema::PatchOperation> operations;
	};

	/// The SDK class exposes functionality to interact with the Gamelink SDK.
	///
	/// @remark Most functions are thread safe, and can be called from multiple threads
	///         concurrently, unless specifically denoted.
	class MUXY_GAMELINK_API SDK
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
		/// may invoke any callbacks that have been attached.
		/// @warning This function must only be called from one thread, and must not be
		///          invoked recursively. This function will invoke any callbacks in the same
		///          thread it is called in. Any arguments to any callbacks invoked by this
		///          function only live until the callback returns. This means that rescheduling
		///          a callback must copy any parameters.
		///
		/// @param[in] bytes Pointer to contiguous array of bytes that represent a network message.
		///                  This byte array must stay live until ReceiveMessage terminates.
		/// @param[in] length Length of the bytes array.
		/// @return Returns true if the message was parsed correctly.
		bool ReceiveMessage(const char* bytes, uint32_t length);

		/// Call this after a websocket reconnect after disconnect.
		/// This queues in an authorization message before any additional
		/// messages are sent.
		void HandleReconnect();

		/// Returns true if there are a non-zero amount of payloads to send.
		/// @remark This method is thread safe, but it is possible for
		///         `if (HasPayloads()) { ForEachPayload(...) }` to execute
		///         the callback to ForEachPayload zero times.
		///
		/// @return returns if there are payloads to send.
		bool HasPayloads() const;

		/// Invokes a callable type for each avaliable payload.
		/// May invoke the callback zero times if there are no outstanding payloads.
		/// Does not internally sleep.
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
						networkCallback(payload);
					}
					delete payload;
				}
			}
		}

		/// Type used in ForeachPayload below. Takes in the user pointer as the first argument,
		/// and a pointer to a payload as the second parameter.
		typedef void (*NetworkCallback)(void*, const Payload*);

		/// Invokes a function pointer for each avaliable payload.
		/// May invoke the callback zero times if there are no outstanding payloads.
		/// Does not internally sleep.
		///
		/// @param[in] cb Callback to be invoked for each avaliable payload
		/// @param[in] user User pointer that is passed into the callback
		void ForeachPayload(NetworkCallback cb, void* user);

		/// Returns if an authentication message has been received.
		///
		/// @return true if an authentication message has been received.
		bool IsAuthenticated() const;

		/// Gets the currently authenticated user. Will return NULL if !IsAuthenticated()
		///
		/// @return The currently authenticated user, or null if no authentication message
		///         has been recieved.
		const schema::User* GetUser() const;

		/// Gets the ClientID that was last passed into AuthenticateWithPIN or AuthenticateWithJWT
		///
		/// @return c-string representation of the input ClientID. This pointer is valid as long
		///         as the SDK object.
		const char* GetClientId() const;

		/// Sets the OnDebugMessage callback. This is invoked for debugging purposes only.
		/// There can only be one OnDebugMessage callback registered. Registering another
		/// callback will overwrite an existing one.
		///
		/// @param[in] callback Callback to log a debug message
		void OnDebugMessage(std::function<void(const string&)> callback);

		/// Sets the OnDebugMessage callback with a function pointer and user pointer.
		/// This is invoked for debugging purposes only.
		/// There can only be one OnDebugMessage callback registered. Registering another
		/// callback will overwrite an existing one.
		///
		/// @param[in] callback Callback to log a debug message.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		void OnDebugMessage(void (*callback)(void*, const string&), void* ptr);

		/// Detaches the OnDebugMessage callback, so no additional calls will be made.
		void DetachOnDebugMessage();

		/// Invokes the OnDebugMessage callback with the given message. Generally used
		/// by language projections, and not intended for general usage.
		void InvokeOnDebugMessage(const string& msg);

		/// Waits for a request to be responded to before sending further requests.
		/// @remark WaitForResponse is used to ensure a serializing of requests on the server.
		///			As an example, issuing back-to-back calls to DeletePoll, CreatePoll is
		///         a common way to reset a poll. However, if DeletePoll happens after CreatePoll
		///         it can result in the newly created poll being immediately destroyed.
		///         To prevent such reorderings, do id = DeletePoll(); WaitForResponse(id); CreatePoll()
		///
		/// @remark Waiting for the same request multiple times is a no-op.
		/// @warning Waiting for an invalid request id will result in no further messages sent. This
		///          API does not check for request id validity.
		///
		/// @param[in] req A request id, as returned from an API call.
		void WaitForResponse(RequestId req);

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

		/// Starts subscribing to OnTransaction updates for a specific SKU
		///
		/// @param[in] sku SKU of item to subscribe to
		/// @return RequestId of the generated request
		RequestId SubscribeToSKU(const string& sku);

		/// Unsubscribes from a specific SKU listened to by SubscribeToSKU
		///
		/// @param[in] sku SKU of item to unsubscribe to
		/// @return RequestId of the generated request
		RequestId UnsubscribeFromSKU(const string& sku);

		/// Subscribes to all SKUs.
		/// @return RequestId of the generated request
		RequestId SubscribeToAllPurchases();

		/// Unsubscribes from all SKUs.
		/// @return RequestId of the generated request
		RequestId UnsubscribeFromAllPurchases();

		/// Sets the OnTransaction callback. This callback is invoked whenever a transaction message
		/// is received.
		/// @remarks The purchase message has been authenticated and deduplicated by the server.
		///          This callback receives all SKUs purchased, so a callback for a specific SKU should
		///          test the SKU in the callback.
		///
		/// @param[in] callback Callback to invoke when a twitch purchase message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnTransaction.
		uint32_t OnTransaction(std::function<void(const schema::TransactionResponse&)> callback);

		/// Sets the OnTransaction callback. This callback is invoked when twitch purchase
		/// message is received.
		/// See the std::function overload for remarks.
		///
		/// @param[in] callback Callback to invoke when a twitch purchase message is received.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnTransaction.
		uint32_t OnTransaction(void (*callback)(void*, const schema::TransactionResponse&), void* ptr);

		/// Detaches an OnTransaction callback.
		///
		/// @deprecated
		/// @param[in] id A handle obtained from calling OnTransaction. Invalid handles are ignored.
		void DetachOnTransaction(uint32_t id);

		/// Gets a list of unvalidated transactions.
		///
		/// @remarks These unvalidated transactions are ordered by purchase time, from the least recent
		///          to most recent. Returns up to 10 entries at a time.
		/// @param[in] sku SKU of the transactions to get. Can use '*' to get all SKUs.
		/// @param[in] callback Callback to invoke after getting the outstanding transactions from the server.
		/// @return RequestId of the generated request
		RequestId GetOutstandingTransactions(const string& sku,
											 std::function<void(const schema::GetOutstandingTransactionsResponse&)> callback);

		/// Gets a list of unvalidated transactions.
		///
		/// @remarks These unvalidated transactions are ordered by purchase time, from the least recent
		///          to most recent. Returns up to 10 entries at a time.
		/// @param[in] sku SKU of the transactions to get. Can use '*' to get all SKUs.
		/// @param[in] callback Callback to invoke after getting the outstanding transactions from the server.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return RequestId of the generated request
		RequestId GetOutstandingTransactions(const string& sku,
											 void (*callback)(void*, const schema::GetOutstandingTransactionsResponse&),
											 void* ptr);

		/// Refunds a transaction by SKU and UserID
		///
		/// @param[in] sku The SKU of the transaction to refund.
		/// @param[in] userId The UserID of the user the transaction is to be refunded to.
		/// @return RequestId of the generated request
		RequestId RefundTransactionBySKU(const string& sku, const string& userid);

		/// Refunds a transaction by transaction id and UserID
		///
		/// @param[in] txid The Muxy transaction id of the transaction to refund.
		/// @param[in] userId The UserID of the user the transaction is to be refunded to.
		/// @return RequestId of the generated request
		RequestId RefundTransactionByID(const string& txid, const string& userid);

		/// Validates a specific transaction
		///
		/// @param[in] txid The Muxy transaction id of the transaction to validate.
		/// @param[in] details Optional details about this validation.
		/// @return RequestId of the generated request
		RequestId ValidateTransaction(const string& txid, const string& details);

		/// Gets drops of a given status. Valid status is FULFILLED and CLAIMED.
		///
		/// @param[in] status The string status of the set of drops to get. One of FULFILLED, CLAIMED
		///                   or empty or '*' to get drops of all statuses.
		/// @param[in] callback Callback to invoke after getting the drops from the server.
		/// @return RequestId of the generated request
		RequestId GetDrops(const string& status, std::function<void(const schema::GetDropsResponse&)> callback);

		/// Gets drops of a given status. Valid status is FULFILLED and CLAIMED.
		///
		/// @param[in] status The string status of the set of drops to get. One of FULFILLED, CLAIMED
		///                   or empty or '*' to get drops of all statuses.
		/// @param[in] callback Callback to invoke after getting the drops from the server.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return RequestId of the generated request
		RequestId GetDrops(const string& status, void (*callback)(void*, const schema::GetDropsResponse&), void* ptr);

		/// Moves a single drop from 'CLAIMED' status to 'FULFILLED' status.
		///
		/// @param[in] id the ID of the drop to update the status of.
		/// @return RequestId of the generated request.
		RequestId ValidateDrop(const string& id);

		/// Deauths the user from the server. Additional requests will not succeed until another
		/// successful authentication request is sent.
		///
		/// @return RequestId of the generated request
		RequestId Deauthenticate();

		/// Queues an authentication request using a PIN code, as received by the user from an
		/// extension's config view.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] pin 		The PIN input from the broadcaster
		/// @return RequestId of the generated request
		RequestId AuthenticateWithPIN(const string& clientId, const string& pin);

		/// Queues an authentication request using a PIN code, as received by the user from an
		/// extension's config view.
		/// This overload attaches a one-shot callback to be called when the authentication response
		/// message is received.
		///
		/// @param[in] clientId The extension's client ID
		/// @param[in] pin 		The PIN input from the broadcaster
		/// @param[in] callback Callback that is invoked once when this authentication request
		///                     is responded to.
		/// @return RequestId of the generated request
		RequestId
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
		/// @return RequestId of the generated request
		RequestId AuthenticateWithPIN(const string& clientId,
									  const string& pin,
									  void (*callback)(void*, const schema::AuthenticateResponse&),
									  void* user);

		/// Queues an authentication request using a JWT, as received after a successful PIN authentication request.
		///
		/// @param[in] clientId 	The extension's client ID
		/// @param[in] refreshToken The stored refresh token from a previous authentication
		/// @return RequestId of the generated request
		RequestId AuthenticateWithRefreshToken(const string& clientId, const string& refreshToken);

		/// Queues an authentication request using a JWT, as received after a successful PIN authentication request.
		/// This overload attaches a one-shot callback to be called when the authentication response
		/// message is received.
		///
		/// @param[in] clientId 	The extension's client ID
		/// @param[in] refreshToken The stored refresh token from a previous authentication
		/// @param[in] callback 	Callback that is invoked once when this authentication request
		///                     	is responded to.
		/// @return RequestId of the generated request
		RequestId AuthenticateWithRefreshToken(const string& clientId,
											   const string& refreshToken,
											   std::function<void(const schema::AuthenticateResponse&)> callback);

		/// Queues an authentication request using a JWT, as received after a successful PIN authentication request.
		/// This overload attaches a one-shot callback to be called when the authentication response
		/// message is received.
		///
		/// @param[in] clientId 	The extension's client ID
		/// @param[in] refreshToken The stored refresh token from a previous authentication
		/// @param[in] callback 	Callback that is invoked once when this authentication request
		///                     	is responded to.
		/// @param[in] user     	User pointer that is passed into the callback whenever it is invoked.
		/// @return RequestId of the generated request
		RequestId AuthenticateWithRefreshToken(const string& clientId,
											   const string& refreshToken,
											   void (*callback)(void*, const schema::AuthenticateResponse&),
											   void* user);

		// Poll stuff, all async.

		/// Queues a request to get poll information, including results, for the poll with the given ID.
		/// Roughly equivilent to a single poll subscription update.
		/// Results are obtained through the OnPollUpdate callback.
		///
		/// @param[in] pollId The Poll ID to get information for
		/// @return RequestId of the generated request
		RequestId GetPoll(const string& pollId);

		/// Queues a request to get poll information. This overload attaches a one-shot callback to be
		/// called when poll information is received.
		///
		/// @param[in] pollId   The Poll ID to get information for
		/// @param[in] callback Callback invoked when this get poll request is responded to.
		/// @return RequestId of the generated request
		RequestId GetPoll(const string& pollId, std::function<void(const schema::GetPollResponse&)> callback);

		/// Queues a request to get poll information. This overload attaches a one-shot callback to be
		/// called when poll information is received.
		///
		/// @param[in] pollId   The Poll ID to get information for
		/// @param[in] callback Callback invoked when this get poll request is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		/// @return RequestId of the generated request
		RequestId GetPoll(const string& pollId, void (*callback)(void*, const schema::GetPollResponse&), void* user);

		/// Queues a request to create a poll.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] options An array of options to store in the poll.
		/// @return RequestId of the generated request
		RequestId CreatePoll(const string& pollId, const string& prompt, const std::vector<string>& options);

		/// Queues a request to create a poll.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] optionsBegin Pointer to the first element in an array of options to store in the poll.
		/// @param[in] optionsEnd Pointer one past the final entry in an array of options to store in the poll.
		/// @return RequestId of the generated request
		RequestId CreatePoll(const string& pollId, const string& prompt, const string* optionsBegin, const string* optionsEnd);


		/// Queues a request to create a poll with configuration options.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] config The PollConfiguration instance to use to configure the poll with.
		/// @param[in] options An array of options to store in the poll.
		/// @return RequestId of the generated request
		RequestId CreatePollWithConfiguration(const string& pollId, const string& prompt, const PollConfiguration& config, const std::vector<string>& options);

		/// Queues a request to create a poll with configuration options.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] config The PollConfiguration instance to use to configure the poll with.
		/// @param[in] optionsBegin Pointer to the first element in an array of options to store in the poll.
		/// @param[in] optionsEnd Pointer one past the final entry in an array of options to store in the poll.
		/// @return RequestId of the generated request
		RequestId CreatePollWithConfiguration(const string& pollId, const string& prompt, const PollConfiguration& config, const string* optionsBegin, const string* optionsEnd);

		/// Queues a request to create a timed poll.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] options An array of options to store in the poll.
		/// @param[in] duration How long the poll will last for (in your own provided unit of time).
		/// @param[in] onFinishCallback Callback to be called when poll finishes.
		/// @return RequestId of the generated request
		RequestId CreateTimedPoll(const string& pollId,
									   const string& prompt,
									   const std::vector<string>& options,
									   float duration,
									   std::function<void(const schema::GetPollResponse&)> onFinishCallback);

		/// Queues a request to create a timed poll.
		///
		/// @param[in] pollId The Poll ID to create
		/// @param[in] prompt The Prompt to store in the poll.
		/// @param[in] options An array of options to store in the poll.
		/// @param[in] duration How long the poll will last for (in your own provided unit of time).
		/// @param[in] onFinishCallback Callback to be called when poll finishes.
		/// @param[in] user User data to pass into the provided callback
		/// @return RequestId of the generated request
		RequestId CreateTimedPoll(const string& pollId,
									   const string& prompt,
									   const std::vector<string>& options,
									   float duration,
									   void (*onFinishCallback)(void*, const schema::GetPollResponse&),
									   void* user);

		/// Ticks all timed polls and subtracts dt from the polls duration, callbacks are triggered when duration is <= 0
		/// @param[in] dt Time to subtract from duration (in your own provided unit of time)
		void TickTimedPolls(float dt);

		/// Subscribes to updates for a given poll.
		/// Updates come through the OnPollUpdate callback.
		/// Once a poll stops receiving new votes, the subscription will stop receiving new updates.
		///
		/// @param[in] pollId The Poll ID to subscribe to
		/// @return RequestId of the generated request
		RequestId SubscribeToPoll(const string& pollId);

		/// Unsubscribes to updates for a given poll
		///
		/// @param[in] pollId The Poll ID to unsubscribe to
		/// @return RequestId of the generated request
		RequestId UnsubscribeFromPoll(const string& pollId);

		/// Deletes the poll with the given ID.
		///
		/// @param[in] pollId 	The ID of the poll to delete.
		/// @return RequestId of the generated request
		RequestId DeletePoll(const string& pollId);

		// Config operations, all async.

		/// Queues a request to get cofiguration. This overload attaches a one-shot callback to be
		/// called when config is received.
		///
		/// @param[in] target   Either CONFIG_TARGET_CHANNEL or CONFIG_TARGET_EXTENSION
		/// @param[in] callback Callback invoked when this get request is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		/// @return RequestId of the generated request
		RequestId GetConfig(ConfigTarget target, std::function<void(const schema::GetConfigResponse&)> callback);

		/// Queues a request to get cofiguration. This overload attaches a one-shot callback to be
		/// called when config is received.
		///
		/// @param[in] target   Either CONFIG_TARGET_CHANNEL or CONFIG_TARGET_EXTENSION
		/// @param[in] callback Callback invoked when this get request is responded to.
		/// @return RequestId of the generated request
		RequestId GetConfig(ConfigTarget target, void (*callback)(void*, const schema::GetConfigResponse&), void*);

		/// Queues a request to get combined cofiguration. This overload attaches a one-shot callback to be
		/// called when config is received.
		///
		/// @param[in] callback Callback invoked when this get request is responded to.
		/// @return RequestId of the generated request
		RequestId GetCombinedConfig(std::function<void(const schema::GetCombinedConfigResponse&)> callback);

		/// Queues a request to get combined cofiguration. This overload attaches a one-shot callback to be
		/// called when config is received.
		///
		/// @param[in] callback Callback invoked when this get request is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		/// @return RequestId of the generated request
		RequestId GetCombinedConfig(void (*callback)(void*, const schema::GetCombinedConfigResponse&), void*);

		/// Queues a request to do many JSON Patch operations on the channel config object.
		/// This will generate a ConfigUpdate event.
		///
		/// @param[in] begin Pointer to the first element in an array of UpdateOperations
		/// @param[in] end Pointer one past the last element in an array of UpdateOperations
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfig(const schema::PatchOperation* begin, const schema::PatchOperation* end);

		/// Updates channel configuration with an object, placed at a path.
		///
		/// @param[in] path A JSON Patch path.
		/// @param[in] obj The value to use in the patch operation
		/// @return RequestId of the generated request
		template<typename T>
		RequestId UpdateChannelConfigWithObject(Operation operation, const string& path, const T& obj)
		{
			nlohmann::json js = nlohmann::json(obj);
			return UpdateChannelConfigWithJson(operation, path, js);
		}

		/// Updates channel configuration with the input array as the value.
		///
		/// @param[in] path A JSON Patch path.
		/// @param[in] begin Pointer to the first element in an array of serializable objects.
		/// @param[in] end Pointer to one past the last element in an array of serializable objects.
		/// @return RequestId of the generated request
		template<typename T>
		RequestId UpdateChannelConfigWithArray(Operation operation, const string& path, const T* begin, const T* end)
		{
			nlohmann::json js = nlohmann::json::array();

			int index = 0;
			while (begin != end)
			{
				js[index] = nlohmann::json(*begin);
				++begin;
			}

			return UpdateChannelConfigWithJson(operation, path, js);
		}

		/// Helper function that will call UpdateChannelConfig with the input integer as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] i The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfigWithInteger(Operation operation, const string& path, int64_t i);

		/// Helper function that will call UpdateChannelConfig with the input double as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] d The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfigWithDouble(Operation operation, const string& path, double d);

		/// Helper function that will call UpdateChannelConfig with the input boolean as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] b The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfigWithBoolean(Operation operation, const string& path, bool b);

		/// Helper function that will call UpdateChannelConfig with the input string as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] s The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfigWithString(Operation operation, const string& path, const string& s);

		/// Helper function that will call UpdateChannelConfig with the input json object literal as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] js The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfigWithLiteral(Operation operation, const string& path, const string& js);

		/// Helper function that will call UpdateChannelConfig with null as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfigWithNull(Operation operation, const string& path);

		/// Helper function that will call UpdateChannelConfig the input json object as the value.
		///
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] js The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateChannelConfigWithJson(Operation operation, const string& path, const nlohmann::json& js);

		/// Starts subscribing to configuration updates for a given target.
		///
		/// @param[in] target either CONFIG_TARGET_CHANNEL or CONFIG_TARGET_EXTENSION
		/// @return RequestId of the generated request
		RequestId SubscribeToConfigurationChanges(ConfigTarget target);

		/// Stop subscribing to configuration updates for a given target.
		///
		/// @param[in] target either CONFIG_TARGET_CHANNEL or CONFIG_TARGET_EXTENSION
		/// @return RequestId of the generated request
		RequestId UnsubscribeFromConfigurationChanges(ConfigTarget target);

		/// Sets the current channel configuration. Will trigger OnConfigUpdate.
		///
		/// @param[in] value A serializable type. Will overwrite any existing configuration.
		///                  Cannot be an array or primitive type.
		/// @return RequestId of the generated request
		template<typename T>
		RequestId SetChannelConfig(const T& value)
		{
			nlohmann::json js = nlohmann::json(value);
			return SetChannelConfig(js);
		}

		/// Sets the current channel configuration. Will trigger OnConfigUpdate.
		///
		/// @param[in] js The configuration to set
		/// @return RequestId of the generated request
		RequestId SetChannelConfig(const nlohmann::json& js);

		/// Sets an OnConfigUpdate callback. This callback is invoked when a config update
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when a config update message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnConfigUpdate.
		uint32_t OnConfigUpdate(std::function<void(const schema::ConfigUpdateResponse&)> callback);

		/// Sets an OnConfigUpdate callback. This callback is invoked when a config update
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when a config update message is received.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnConfigUpdate.
		uint32_t OnConfigUpdate(void (*callback)(void*, const schema::ConfigUpdateResponse&), void* user);

		/// Detaches an OnConfigUpdate callback.
		///
		/// @param[in] id A handle obtained from calling OnConfigUpdate. Invalid handles are ignored.
		void DetachOnConfigUpdate(uint32_t);

		// State operations, all async.

		/// Queues a request to replace the entirety of state with new information.
		/// This will generate a StateUpdate subscription event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] value A serializable type. Will overwrite any existing state for the given target.
		///                  Cannot be an array or primitive type.
		/// @return RequestId of the generated request
		template<typename T>
		RequestId SetState(StateTarget target, const T& value)
		{
			nlohmann::json js = nlohmann::json(value);
			return SetState(target, js);
		};

		/// Queues a request to replace the entirety of state with new information.
		/// This will generate a StateUpdate subscription event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] value JSON. Will overwrite any existing state for the given target.
		///                  Must be an object, not an array or primitive type.
		/// @return RequestId of the generated request
		RequestId SetState(StateTarget target, const nlohmann::json& value);

		/// Queues a request to replace the entirety of state the empty object {}
		/// This will generate a StateUpdate subscription event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @return RequestId of the generated request
		RequestId ClearState(StateTarget target);

		/// Queues a request to get state.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @return RequestId of the generated request
		RequestId GetState(StateTarget target);

		/// Queues a request to get state. This overload attaches a one-shot callback to be
		/// called when state is received.
		///
		/// @param[in] target   Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] callback Callback invoked when this state request is responded to.
		/// @return RequestId of the generated request
		RequestId GetState(StateTarget target, std::function<void(const schema::GetStateResponse<nlohmann::json>&)> callback);

		/// Queues a request to get state. This overload attaches a one-shot callback to be
		/// called when state is received.
		///
		/// @param[in] target   Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] callback Callback invoked when this state request is responded to.
		/// @param[in] user     User pointer that is passed into the callback whenever it is invoked.
		/// @return RequestId of the generated request
		RequestId GetState(StateTarget target, void (*callback)(void*, const schema::GetStateResponse<nlohmann::json>&), void* user);

		/// Queues a request to do many JSON Patch operations on the state object.
		/// This will generate a StateUpdate event.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] begin Pointer to the first element in an array of UpdateOperations
		/// @param[in] end Pointer one past the last element in an array of UpdateOperations
		/// @return RequestId of the generated request
		RequestId UpdateState(StateTarget target, const schema::PatchOperation* begin, const schema::PatchOperation* end);

		/// Helper function that will call UpdateState with the input object as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] path A JSON Patch path.
		/// @param[in] obj The value to use in the patch operation
		/// @return RequestId of the generated request
		template<typename T>
		RequestId UpdateStateWithObject(StateTarget target, Operation operation, const string& path, const T& obj)
		{
			nlohmann::json js = nlohmann::json(obj);
			return UpdateStateWithJson(target, operation, path, js);
		}

		/// Helper function that will call UpdateState with the input array as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] path A JSON Patch path.
		/// @param[in] begin Pointer to the first element in an array of serializable objects.
		/// @param[in] end Pointer to one past the last element in an array of serializable objects.
		/// @return RequestId of the generated request
		template<typename T>
		RequestId UpdateStateWithArray(StateTarget target, Operation operation, const string& path, const T* begin, const T* end)
		{
			nlohmann::json js = nlohmann::json::array();

			int index = 0;
			while (begin != end)
			{
				js[index++] = nlohmann::json(*begin);
				++begin;
			}

			return UpdateStateWithJson(target, operation, path, js);
		}

		/// Helper function that will call UpdateState with the input integer as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] i The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateStateWithInteger(StateTarget target, Operation operation, const string& path, int64_t i);

		/// Helper function that will call UpdateState with the input double as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] d The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateStateWithDouble(StateTarget target, Operation operation, const string& path, double d);

		/// Helper function that will call UpdateState with a boolean value
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] b The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateStateWithBoolean(StateTarget target, Operation operation, const string& path, bool b);

		/// Helper function that will call UpdateState with the input string as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] s The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateStateWithString(StateTarget target, Operation operation, const string& path, const string& s);

		/// Helper function that will call UpdateState with the input json object literal as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] js The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateStateWithLiteral(StateTarget target, Operation operation, const string& path, const string& js);

		/// Helper function that will call UpdateState with null as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @return RequestId of the generated request
		RequestId UpdateStateWithNull(StateTarget target, Operation operation, const string& path);

		/// Helper function that will call UpdateState the input json object as the value.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] operation A valid JSON Patch operation, or "add_intermediates" or "remove_value"
		/// @param[in] path A JSON Patch path.
		/// @param[in] js The value to use in the patch operation
		/// @return RequestId of the generated request
		RequestId UpdateStateWithJson(StateTarget target, Operation operation, const string& path, const nlohmann::json& js);

		/// Helper function that will call UpdateState with the given patch list
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @param[in] list A patch list, created and filled elsewhere
		/// @return RequestId of the generated request
		RequestId UpdateStateWithPatchList(StateTarget target, const PatchList& list);

		/// Starts subscribing to state updates for the given target.
		/// Updates come through the OnStateUpdate callback
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @return RequestId of the generated request
		RequestId SubscribeToStateUpdates(StateTarget target);

		/// Stops subscribing to state updates for the given target.
		///
		/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
		/// @return RequestId of the generated request
		RequestId UnsubscribeFromStateUpdates(StateTarget target);

		/// Sends a broadcast to all viewers on the channel using the extension.
		/// @remark The serialized size of the value parameter must be under 8 kilobytes.
		///
		/// @param[in] topic The topic of the message to send. The frontend uses this value
		///                  to filter messages.
		/// @param[in] value Serializable, arbitrary object.
		/// @return RequestId of the generated request
		template<typename T>
		RequestId SendBroadcast(const string& topic, const T& value)
		{
			schema::BroadcastRequest<T> payload(topic, value);
			return queuePayload(payload);
		}

		/// Sends a broadcast to all viewers on the channel using the extension.
		/// @remark The serialized size of the message parameter must be under 8 kilobytes.
		///
		/// @param[in] topic The topic of the message to send. The frontend uses this value
		///                  to filter messages.
		/// @param[in] message Arbitrary json object. May not be a primitive or array.
		/// @return RequestId of the generated request
		RequestId SendBroadcast(const string& topic, const nlohmann::json& message);

		/// Sends a broadcast to all viewers on the channel using the extension.
		/// @param[in] topic The topic of the message to send. The frontend uses this value
		///                  to filter messages.
		/// @return RequestId of the generated request
		RequestId SendBroadcast(const string& topic);

		/// Sends a request to subscribe to the datastream.
		/// @return RequestId of the generated request
		RequestId SubscribeToDatastream();

		/// Sends a request to unsubscribe to the datastream.
		/// @return RequestId of the generated request
		RequestId UnsubscribeFromDatastream();

		/// Sets an OnDatastream callback. This callback is invoked when a datastream update
		/// message is received.
		///
		/// @param[in] callback Callback to invoke when a datastream update message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnDatastream.
		uint32_t OnDatastream(std::function<void(const schema::DatastreamUpdate&)> callback);

		/// Sets an OnDatastream callback. This callback is invoked when a datastream update
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

		/// Clears the matchmaking queue
		///
		/// @return RequestId of the generated request
		RequestId ClearMatchmakingQueue();

		/// Removes an entry from the matchmaking queue.
		/// This is usually done after receiving a callback from OnMatchmakingQueueInvite
		/// to show that the game client has acknowledged and invited the user.
		///
		/// @param[in] twitchID The twitchID of of the MatchmakingInformation entry to remove
		///                     from the queue
		/// @return RequestId of the generated request
		RequestId RemoveMatchmakingEntry(const string& twitchID);

		/// Sends a request to subscribe to matchmaking queue invite messages.
		/// @return RequestId of the generated request
		RequestId SubscribeToMatchmakingQueueInvite();

		/// Sends a request to unsubscribe from matchmaking queue invite messages.
		/// @return RequestId of the generated request
		RequestId UnsubscribeFromMatchmakingQueueInvite();

		/// Sets an OnMatchmakingQueueInvite callback. This callback is invoked when a matchmaking queue
		/// invite message is received.
		/// You must call SubscribeToMatchmakingQueueInvite before any callbacks will be invoked.
		///
		/// @param[in] callback Callback to invoke when a queue invite message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnMatchmakingQueueInvite
		uint32_t OnMatchmakingQueueInvite(std::function<void(const schema::MatchmakingUpdate&)> callback);

		/// Sets an OnMatchmakingQueueInvite callback. This callback is invoked when a matchmaking queue
		/// invite message is received.
		/// You must call SubscribeToMatchmakingQueueInvite before any callbacks will be invoked.
		///
		/// @param[in] callback Callback to invoke when a queue invite message is received.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnMatchmakingQueueInvite
		uint32_t OnMatchmakingQueueInvite(void (callback)(void*, const schema::MatchmakingUpdate&), void* user);

		/// Sets an OnMatchmakingQueueInvite callback. This callback is invoked when a matchmaking queue
		/// invite message is received. Any existing callbacks bound with the same name will
		/// unbound before the new callback will be attached.
		/// You must call SubscribeToMatchmakingQueueInvite before any callbacks will be invoked.
		///
		/// @param[in] name An arbitary name to associate with this callback.
		/// @param[in] callback Callback to invoke when a queue invite message is received.
		/// @return Returns an integer handle to the callback, to be used in DetachOnMatchmakingQueueInvite
		uint32_t OnMatchmakingQueueInviteUnique(string name, std::function<void(const schema::MatchmakingUpdate&)> callback);

		/// Sets an OnMatchmakingQueueInvite callback. This callback is invoked when a matchmaking queue
		/// invite message is received. Any existing callbacks bound with the same name will
		/// unbound before the new callback will be attached.
		/// You must call SubscribeToMatchmakingQueueInvite before any callbacks will be invoked.
		///
		/// @param[in] name An arbitary name to associate with this callback.
		/// @param[in] callback Callback to invoke when a queue invite message is received.
		/// @param[in] ptr User pointer that is passed into the callback whenever it is invoked.
		/// @return Returns an integer handle to the callback, to be used in DetachOnMatchmakingQueueInvite
		uint32_t OnMatchmakingQueueInviteUnique(string name, void (callback)(void*, const schema::MatchmakingUpdate&), void* user);

		/// Detaches an OnMatchmakingQueueInvite callback.
		///
		/// @param[in] id A handle obtained from calling OnMatchmakingQueueInvite. Invalid handles are ignored.
		void DetachOnMatchmakingQueueInvite(uint32_t id);
	private:
		void debugLogPayload(const Payload*);

		bool HasPayloadsNoLock() const;

		template<typename T>
		RequestId queuePayload(T& p)
		{
			RequestId id = nextRequestId();
			p.params.request_id = id;

			Payload* payload = new Payload(gamelink::string(to_string(p).c_str()));

			_lock.lock();
			_queuedPayloads.push_back(payload);
			_lock.unlock();

			return id;
		}

		// Fields stored to handle reconnects
		gamelink::string _storedRefresh;
		gamelink::string _storedJWT;
		gamelink::string _storedClientId;

		mutable gamelink::lock _lock;

		std::deque<Payload*> _queuedPayloads;
		std::vector<char> _receiveBuffer;

		schema::User* _user;

		RequestId _currentRequestId;
		RequestId nextRequestId();

		void addToBarrier(uint16_t);
		void removeFromBarrier(uint16_t);

		std::vector<detail::TimedPoll> _timedPolls;
		detail::SubscriptionSets _subscriptionSets;

		detail::Callback<string> _onDebugMessage;

		detail::CallbackCollection<schema::PollUpdateResponse, 1> _onPollUpdate;
		detail::CallbackCollection<schema::AuthenticateResponse, 2> _onAuthenticate;
		detail::CallbackCollection<schema::SubscribeStateUpdateResponse<nlohmann::json>, 3> _onStateUpdate;
		detail::CallbackCollection<schema::GetStateResponse<nlohmann::json>, 4> _onGetState;
		detail::CallbackCollection<schema::TransactionResponse, 5> _onTransaction;
		detail::CallbackCollection<schema::GetPollResponse, 6> _onGetPoll;
		detail::CallbackCollection<schema::DatastreamUpdate, 7> _onDatastreamUpdate;

		detail::CallbackCollection<schema::GetConfigResponse, 8> _onGetConfig;
		detail::CallbackCollection<schema::GetCombinedConfigResponse, 9> _onGetCombinedConfig;
		detail::CallbackCollection<schema::ConfigUpdateResponse, 10> _onConfigUpdate;

		detail::CallbackCollection<schema::GetOutstandingTransactionsResponse, 11> _onGetOutstandingTransactions;
		detail::CallbackCollection<schema::GetDropsResponse, 12> _onGetDrops;

		detail::CallbackCollection<schema::MatchmakingUpdate, 13> _onMatchmakingUpdate;
	};
}

#endif
