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
            T state;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SetStateRequestBody, "state", state);
        };

        template<typename T>
        struct StateResponseBody
        {
            bool ok;
            T state;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(StateResponseBody, 
                "ok", ok, 
                "state", state
            );
        };
     
        static const char * STATE_TARGET_CHANNEL = "channel";
        static const char * STATE_TARGET_EXTENSION = "extension";

        template<typename T>
        struct SetStateRequest : SendEnvelope< SetStateRequestBody<T> >
        {
            SetStateRequest(const char * target, const T& value)
            {
                this->action = string("set");
                this->params.target = string(target);
                this->data.state = value;
            }
        };

        template<typename T>
        struct SetStateResponse : ReceiveEnvelope< StateResponseBody<T> >
        {};

        struct GetStateRequest : SendEnvelope< EmptyBody >
        {
            explicit GetStateRequest(const char * target);
        };

        template<typename T>
        struct GetStateResponse : ReceiveEnvelope< StateResponseBody<T> >
        {};

        // Update state
        struct UpdateOperation
        {
            string operation;
            string path;
            JsonAtom value;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(UpdateOperation, 
                "op", operation, 
                "path", path, 
                "value", value
            ); 
        };

        struct UpdateStateRequestBody
        {
            std::vector<UpdateOperation> state;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(UpdateStateRequestBody, "state", state);
        };

        struct UpdateStateRequest : SendEnvelope< UpdateStateRequestBody >
        {
            explicit UpdateStateRequest(const char * target);
        };

        // Subscription
        template<typename T>
        struct StateUpdateBody
        {
            string topic_id;
            T state;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(StateUpdateBody, 
                "topic_id", topic_id, 
                "state", state
            );
        };

        struct SubscribeStateRequest : SendEnvelope< SubscribeTopicRequestBody >
        {
            explicit SubscribeStateRequest(const char * target);
        };

        template<typename T>
        struct SubscribeStateUpdateResponse : ReceiveEnvelope< StateUpdateBody<T> >
        {};
    }
}
#endif