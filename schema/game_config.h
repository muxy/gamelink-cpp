#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_GAMECONFIG_H
#define MUXY_GAMELINK_SCHEMA_GAMECONFIG_H
#include "schema/envelope.h"
#include "schema/subscription.h"
#include "schema/state.h"

namespace gamelink
{
    namespace schema
    {
        struct MUXY_GAMELINK_API SetConfigRequestBody
        {
            nlohmann::json config;
            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SetConfigRequestBody, "config", config);
        };

        struct MUXY_GAMELINK_API GetConfigRequestBody
        {
            // Either 'channel', 'extension' or 'combined'
            string configId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetConfigRequestBody, "config_id", configId);
        };

        struct MUXY_GAMELINK_API ConfigResponseBody
        {
            nlohmann::json config;
            string configId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ConfigResponseBody, "config", config, "config_id", configId);
        };

        struct MUXY_GAMELINK_API ConfigUpdateBody
        {
            nlohmann::json config;
            string topicId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ConfigUpdateBody, "config", config, "config_id", topicId);
        };

        struct MUXY_GAMELINK_API CombinedState
        {
            nlohmann::json channel;
            nlohmann::json extension;
            string configId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(CombinedState, "channel", channel, "extension", extension, "config_id", configId);
        };

        struct MUXY_GAMELINK_API CombinedStateResponseBody
        {
            CombinedState config;
            string configId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(CombinedStateResponseBody, "config", config, "config_id", configId);
        };

        struct MUXY_GAMELINK_API SubscribeConfigRequestBody
        {
            string configId;
			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SubscribeConfigRequestBody, "config_id", configId);
        };

        struct MUXY_GAMELINK_API UnsubscribeConfigRequestBody
        {
            string configId;
			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(UnsubscribeConfigRequestBody, "config_id", configId);
        };

		struct MUXY_GAMELINK_API PatchConfigRequestBody
		{
			std::vector<PatchOperation> config;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(PatchConfigRequestBody, "config", config);
		};

		struct MUXY_GAMELINK_API PatchConfigRequest : SendEnvelope<PatchConfigRequestBody>
		{
			PatchConfigRequest();
		};

        struct MUXY_GAMELINK_API GetConfigRequest : SendEnvelope<GetConfigRequestBody>
        {
            /// Creates a GetConfig request.
            /// @param[in] target one of the CONFIG_TARGET constants
            explicit GetConfigRequest(ConfigTarget target);
        };

        struct MUXY_GAMELINK_API SetConfigRequest : SendEnvelope<SetConfigRequestBody>
        {
            explicit SetConfigRequest(const nlohmann::json& js);
        };

        struct MUXY_GAMELINK_API GetConfigResponse : ReceiveEnvelope<ConfigResponseBody>
        {};

        struct MUXY_GAMELINK_API GetCombinedConfigResponse : ReceiveEnvelope<CombinedStateResponseBody>
        {};

        struct MUXY_GAMELINK_API SubscribeToConfigRequest : SendEnvelope<SubscribeConfigRequestBody>
        {
			explicit SubscribeToConfigRequest(ConfigTarget target);
        };

        struct MUXY_GAMELINK_API UnsubscribeFromConfigRequest : SendEnvelope<SubscribeConfigRequestBody>
        {
			explicit UnsubscribeFromConfigRequest(ConfigTarget target);
        };

        struct MUXY_GAMELINK_API ConfigUpdateResponse : ReceiveEnvelope<ConfigUpdateBody>
		{
		};
    }
}
#endif