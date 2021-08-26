#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_GAMECONFIG_H
#define MUXY_GAMELINK_SCHEMA_GAMECONFIG_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
    namespace schema
    {
        struct SetConfigRequestBody
        {
            nlohmann::json config;
            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SetConfigRequestBody, "config", config); 
        };

        struct GetConfigRequestBody
        {
            // Either 'channel', 'extension' or 'combined'
            string configId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetConfigRequestBody, "config_id", configId);
        };

        struct ConfigResponseBody
        {
            nlohmann::json config;
            string configId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ConfigResponseBody, "config", config, "config_id", configId);
        };
        
        struct ConfigUpdateBody
        {
            nlohmann::json config;
            string topicId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ConfigUpdateBody, "config", config, "topic_id", topicId);
        };

        struct CombinedState
        {
            nlohmann::json channel;
            nlohmann::json extension;
            string configId;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(CombinedState, "channel", channel, "extension", extension, "config_id", configId);
        };

        struct CombinedStateResponseBody
        {
            CombinedState config;
            string configId;
            
            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(CombinedStateResponseBody, "config", config, "config_id", configId);
        };

        struct SubscribeConfigRequestBody
        {
            string configId;
			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SubscribeConfigRequestBody, "config_id", configId);
        };

        struct UnsubscribeConfigRequestBody
        {
            string configId;
			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(UnsubscribeConfigRequestBody, "config_id", configId);
        };
        
		struct PatchConfigRequestBody
		{
			std::vector<PatchOperation> config;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(PatchConfigRequestBody, "config", config);
		};

		struct PatchConfigRequest : SendEnvelope<PatchConfigRequestBody>
		{
			PatchConfigRequest();
		};

        static const char CONFIG_TARGET_CHANNEL[] = "channel";
        static const char CONFIG_TARGET_EXTENSION[] = "extension";

        struct GetConfigRequest : SendEnvelope<GetConfigRequestBody>
        {
            /// Creates a GetConfig request.
            /// @param[in] target one of the CONFIG_TARGET constants 
            explicit GetConfigRequest(const char* target);
        };

        struct SetConfigRequest : SendEnvelope<SetConfigRequestBody>
        {
            explicit SetConfigRequest(const nlohmann::json& js);
        };

        struct GetConfigResponse : ReceiveEnvelope<ConfigResponseBody>
        {};

        struct GetCombinedConfigResponse : ReceiveEnvelope<CombinedStateResponseBody>
        {};

        struct SubscribeToConfigRequest : SendEnvelope<SubscribeConfigRequestBody>
        {
            explicit SubscribeToConfigRequest(const char* target);
        };

        struct UnsubscribeFromConfigRequest : SendEnvelope<SubscribeConfigRequestBody>
        {
            explicit UnsubscribeFromConfigRequest(const char* target);
        };

        struct ConfigUpdateResponse : ReceiveEnvelope<ConfigUpdateBody>
		{
		};
    }
}
#endif