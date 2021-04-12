#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_BROADCAST_H
#define MUXY_GAMELINK_SCHEMA_BROADCAST_H
#include "schema/envelope.h"

namespace gamelink
{
    namespace schema
    {
        struct BroadcastRequestBody
        {
            string message;
            string topic;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(BroadcastRequestBody, "message", message, "topic", topic);
        };

        struct BroadcastRequest : SendEnvelope<BroadcastRequestBody>
        {
            BroadcastRequest(const string& topic, const string& msg);
        };

        struct BroadcastResponse : ReceiveEnvelope<OKResponseBody>
        {};
    }
}

#endif