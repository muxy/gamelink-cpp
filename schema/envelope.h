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
            ReceiveMeta();

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

        template<typename T>
        void to_json(nlohmann::json& out, const ReceiveEnvelope<T>& p)
        {
            out["meta"] = p.meta;
            out["data"] = p.data;

            if (p.errors.size())
            {
                out["errors"] = p.errors;
            }
        }

        template<typename T>
        void from_json(const nlohmann::json& in, ReceiveEnvelope<T>& p)
        {
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "meta", p, meta);
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "data", p, data);
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "errors", p, errors);
        }

        struct SendParameters
        {
            SendParameters();

            uint16_t request_id;
            string target;
        };

        inline void to_json(nlohmann::json& out, const SendParameters& p)
        {
            out["request_id"] = p.request_id;
            if (p.target.size()) 
            {
                out["target"] = p.target;
            }
        }

        inline void from_json(const nlohmann::json& in, SendParameters& p)
        {
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "request_id", p, request_id);
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "target", p, target);
        }

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
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "action", p, action);
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "params", p, params);
            MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "data", p, data);
        }

        // A few common bodies
        namespace bodies
        {
            struct EmptyBody
            {};

            struct OKResponseBody
            {
                bool ok;
            };

            MUXY_GAMELINK_SERIALIZE_1(OKResponseBody, 
                "ok", ok
            )
        }

        // Specialization for empty body serialization
        MUXY_GAMELINK_SERIALIZE_2(SendEnvelope<bodies::EmptyBody>,
            "action", action, 
            "params", params
        );

        MUXY_GAMELINK_SERIALIZE_2(ReceiveEnvelope<bodies::EmptyBody>,
            "meta", meta,
            "errors", errors
        );
    }
}

#endif