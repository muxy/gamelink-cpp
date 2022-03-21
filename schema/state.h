#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_STATE_H
#define MUXY_GAMELINK_SCHEMA_STATE_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		// Set / Get state
		template<typename T>
		struct SetStateRequestBody
		{
			/// Either 'channel' or 'extension', based on the target.
			string state_id;

			/// The state to set. The type `T` should be serializable
			/// through use of the MUXY_GAMELINK_SERIALIZE macros.
			T state;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(SetStateRequestBody, "state_id", state_id, "state", state);
		};

		template<typename T>
		struct StateResponseBody
		{
			/// Will always be true.
			bool ok;

			/// State response. The type `T` should be serializable
			/// through use of the MUXY_GAMELINK_SERIALIZE macros.
			T state;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(StateResponseBody, "ok", ok, "state", state);
		};

		/// Channel State target
		static const char STATE_TARGET_CHANNEL[] = "channel";

		/// Extension State target
		static const char STATE_TARGET_EXTENSION[] = "extension";

		template<typename T>
		struct SetStateRequest : SendEnvelope<SetStateRequestBody<T>>
		{
			/// Creates a SetState request.
			/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
			/// @param[in] value A serializable type.
			SetStateRequest(const string& target, const T& value)
			{
				this->action = string("set");
				this->params.target = string("state");
				this->data.state_id = target;
				this->data.state = value;
			}
		};

		template<typename T>
		struct SetStateResponse : ReceiveEnvelope<StateResponseBody<T>>
		{
		};

		struct MUXY_GAMELINK_API GetStateRequestBody
		{
			/// Either 'channel' or 'extension', based on the target.
			string state_id;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetStateRequestBody, "state_id", state_id);
		};

		struct MUXY_GAMELINK_API GetStateRequest : SendEnvelope<GetStateRequestBody>
		{
			/// Creates a GetState request
			/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
			explicit GetStateRequest(const string& target);
		};

		template<typename T>
		struct GetStateResponse : ReceiveEnvelope<StateResponseBody<T>>
		{
		};

		struct MUXY_GAMELINK_API PatchStateRequestBody
		{
			/// Either 'channel' or 'extension', based on the target.
			string state_id;

			std::vector<PatchOperation> state;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(PatchStateRequestBody, "state_id", state_id, "state", state);
		};

		struct MUXY_GAMELINK_API PatchStateRequest : SendEnvelope<PatchStateRequestBody>
		{
			/// Creates an UpdateState request
			/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
			explicit PatchStateRequest(const string& target);
		};

		// Subscription
		template<typename T>
		struct StateUpdateBody
		{
			string topic_id;
			T state;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(StateUpdateBody, "topic_id", topic_id, "state", state);
		};

		struct MUXY_GAMELINK_API SubscribeStateRequest : SendEnvelope<SubscribeTopicRequestBody>
		{
			/// Creates a SubscribeState request
			/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
			explicit SubscribeStateRequest(const string& target);
		};

		template<typename T>
		struct SubscribeStateUpdateResponse : ReceiveEnvelope<StateUpdateBody<T>>
		{
		};

		struct MUXY_GAMELINK_API UnsubscribeStateRequest : SendEnvelope<UnsubscribeTopicRequestBody>
		{
			/// Creates a SubscribeState request
			/// @param[in] target Either STATE_TARGET_CHANNEL or STATE_TARGET_EXTENSION
			explicit UnsubscribeStateRequest(const string& target);
		};
	}
}
#endif
