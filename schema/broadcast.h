#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_BROADCAST_H
#define MUXY_GAMELINK_SCHEMA_BROADCAST_H
#include "schema/envelope.h"

namespace gamelink
{
    namespace schema
    {
        template<typename T>
        struct BroadcastRequestBody
        {
            T data;
            string topic;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(BroadcastRequestBody, "data", data, "topic", topic);
        };

        template<typename T>
        struct BroadcastRequest : SendEnvelope<BroadcastRequestBody<T>>
        {
            BroadcastRequest(const string& topic, const T& data)
            {
                this->action = string("broadcast");
                this->params.target = string("");

                this->data.topic = topic;
                this->data.data = data;
            }
        };

        struct BroadcastResponse : ReceiveEnvelope<OKResponseBody>
        {};
    }
}

#endif