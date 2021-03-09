#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_STATE_H
#define MUXY_GAMELINK_SCHEMA_STATE_H
#include "schema/envelope.h"

namespace gamelink
{
    namespace schema
    {
        // Set / Get state
        namespace bodies
        {
            template<typename T>
            struct SetStateBody
            {
                T state;

                MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SetStateBody, "state", state);
            };

            template<typename T>
            struct StateResponse
            {
                bool ok;
                T state;

                MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(StateResponse, 
                    "ok", ok, 
                    "state", state
                );
            };
        }
     
        static const char * STATE_TARGET_CHANNEL = "channel";
        static const char * STATE_TARGET_EXTENSION = "extension";

        template<typename T>
        struct SetStateRequest : SendEnvelope< bodies::SetStateBody<T> >
        {
            SetStateRequest(const char * target, const T& value)
            {
                this->action = string("set");
                this->params.target = string(target);
                this->data.state = value;
            }
        };

        template<typename T>
        struct SetStateResponse : ReceiveEnvelope< bodies::StateResponse<T> >
        {};

        struct GetStateRequest : SendEnvelope< bodies::EmptyBody >
        {
            explicit GetStateRequest(const char * target);
        };

        template<typename T>
        struct GetStateResponse : ReceiveEnvelope< bodies::StateResponse<T> >
        {};

        // Update state
        namespace bodies
        {
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

            struct UpdateStateBody
            {
                std::vector<UpdateOperation> state;

                MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(UpdateStateBody, "state", state);
            };
        }

        struct UpdateStateRequest : SendEnvelope< bodies::UpdateStateBody >
        {
            explicit UpdateStateRequest(const char * target);
        };

        // Subscription
        namespace bodies
        {
            struct SubscribeStateBody
            {
                string topic_id;
                
                MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SubscribeStateBody, "topic_id", topic_id);
            };

            template<typename T>
            struct StateSubscriptionUpdate
            {
                string topic_id;
                T state;

                MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(StateSubscriptionUpdate, 
                    "topic_id", topic_id, 
                    "state", state
                );
            };
        }

        struct SubscribeStateRequest : SendEnvelope< bodies::SubscribeStateBody >
        {
            explicit SubscribeStateRequest(const char * target);
        };

        template<typename T>
        struct SubscribeStateUpdateResponse : ReceiveEnvelope< bodies::StateSubscriptionUpdate<T> >
        {};
    }
}
#endif