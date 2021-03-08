#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_H
#define MUXY_GAMELINK_SCHEMA_H
#include "schema/config.h"
#include "schema/serialization.h"

namespace gamelink
{
    namespace schema
    {
        struct ReceiveMeta
        {
            uint16_t request_id;
            string action;
            string target;
            uint64_t timestamp;
        };

        MUXY_GAMELINK_SERIALIZE_4(ReceiveMeta, 
            "request_id", request_id,
            "action", action,
            "target", target,
            "timestamp", timestamp
        )

        struct Error
        {
            uint32_t number;
            string title;
            string detail;
        };

        MUXY_GAMELINK_SERIALIZE_3(Error, 
            "number", number,
            "title", title,
            "detail", detail
        )

        template<typename T>
        struct ReceiveEnvelope
        {
            ReceiveMeta meta;

            T data;
            std::vector<Error> errors;
        };

        struct SendParameters
        {
            SendParameters();

            uint16_t request_id;
            string target;
        };

        MUXY_GAMELINK_SERIALIZE_2(SendParameters, 
            "request_id", request_id, 
            "target", target
        );

        template<typename T>
        struct SendEnvelope
        {
            string action;
            SendParameters params;
            T data;
        };

        template<typename T>
        void to_json(nlohmann::json& out, const SendEnvelope<T>& p)
        {
            out["action"] = p.action;
            out["params"] = p.params;
            out["data"] = p.data;
        }

        template<typename T>
        void from_json(const nlohmann::json& in, SendEnvelope<T>& p)
        {
            in.at("action").get_to(p.action);
            in.at("params").get_to(p.params);
            in.at("data").get_to(p.data);
        }

        // A few common bodies
        struct EmptyBody
        {};

        MUXY_GAMELINK_SERIALIZE_2(SendEnvelope<EmptyBody>, 
            "action", action, 
            "params", params
        );

        struct OKResponse
        {
            bool ok;
        };

        MUXY_GAMELINK_SERIALIZE_1(OKResponse, 
            "ok", ok
        )
    }
}

#endif