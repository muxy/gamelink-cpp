#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_MATCHES_H
#define MUXY_GAMELINK_SCHEMA_MATCHES_H
#include "schema/envelope.h"
#include "schema/subscription.h"
#include "schema/poll.h"

namespace gamelink
{
    namespace schema
    {
        struct CreateMatchRequestBody
        {
            gamelink::string matchId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(CreateMatchRequestBody,
				"id", matchId
			);
        };

        struct MUXY_GAMELINK_API CreateMatchRequest : SendEnvelope<CreateMatchRequestBody>
        {
			explicit inline CreateMatchRequest(const string& id)
			{
				this->action = string("create");
                this->params.target = string("match");

				this->data.matchId = id;
			}
        };

		struct KeepMatchAliveBody
        {
            gamelink::string matchId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(KeepMatchAliveBody,
				"id", matchId
			);
        };

        struct MUXY_GAMELINK_API KeepMatchAliveRequest : SendEnvelope<KeepMatchAliveBody>
        {
			explicit inline KeepMatchAliveRequest(const string& id)
			{
				this->action = string("keepalive");
                this->params.target = string("match");

				this->data.matchId = id;
			}
        };

        struct AddOrRemoveMatchChannelsRequestBody
        {
            gamelink::string matchId;
            std::vector<gamelink::string> channels;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(AddOrRemoveMatchChannelsRequestBody,
				"id", matchId,
				"channel_ids", channels
			);
        };

        struct MUXY_GAMELINK_API AddMatchChannelsRequest : SendEnvelope<AddOrRemoveMatchChannelsRequestBody>
        {
			explicit inline AddMatchChannelsRequest(const string& id, const std::vector<string>& channels)
			{
				this->action = string("add_channels");
                this->params.target = string("match");

				this->data.matchId = id;
				this->data.channels = channels;
			}
        };

        struct MUXY_GAMELINK_API RemoveMatchChannelsRequest : SendEnvelope<AddOrRemoveMatchChannelsRequestBody>
        {
			explicit inline RemoveMatchChannelsRequest(const string& id, const std::vector<string>& channels)
			{
				this->action = string("remove_channels");
                this->params.target = string("match");

				this->data.matchId = id;
				this->data.channels = channels;
			}
        };

        struct CreateMatchPollRequestBody
        {
            gamelink::string matchId;
            CreatePollWithConfigurationRequestBody poll;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(CreateMatchPollRequestBody,
				"id", matchId,
				"poll", poll
			);
        };

        struct MUXY_GAMELINK_API CreateMatchPollRequest : SendEnvelope<CreateMatchPollRequestBody>
        {
			explicit inline CreateMatchPollRequest(const string& match, const CreatePollWithConfigurationRequestBody& poll)
			{
				this->action = string("create");
                this->params.target = string("match_poll");

				this->data.matchId = match;
				this->data.poll = poll;
			}
        };


        struct DeleteMatchPollRequestBody
        {
            gamelink::string matchId;
            gamelink::string pollId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(DeleteMatchPollRequestBody,
				"id", matchId,
				"poll_id", pollId
			);
        };

		struct MUXY_GAMELINK_API DeleteMatchPollRequest : SendEnvelope<DeleteMatchPollRequestBody>
        {
			explicit inline DeleteMatchPollRequest(const string& match, const string& poll)
			{
				this->action = string("delete");
                this->params.target = string("match_poll");

				this->data.matchId = match;
				this->data.pollId = poll;
			}
        };

		template<typename Config>
		struct ReconfigureMatchPollRequestBody
        {
            gamelink::string matchId;
			gamelink::string pollId;
			Config config;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(ReconfigureMatchPollRequestBody,
				"id", matchId,
				"poll_id", pollId,
				"config", config
			);
        };

        struct MUXY_GAMELINK_API ExpireMatchPollRequest : SendEnvelope<ReconfigureMatchPollRequestBody<ExpireConfig>>
        {
			explicit inline ExpireMatchPollRequest(const string& id, const string& pollId)
			{
				this->action = string("reconfigure");
                this->params.target = string("match_poll");

				this->data.matchId = id;
				this->data.pollId = pollId;
				this->data.config.endsAt = -1;
			}
        };

		 struct MUXY_GAMELINK_API SetMatchPollDisableRequest : SendEnvelope<ReconfigureMatchPollRequestBody<DisableConfig>>
        {
			explicit inline SetMatchPollDisableRequest(const string& id, const string& pollId, bool status)
			{
				this->action = string("reconfigure");
                this->params.target = string("match_poll");

				this->data.matchId = id;
				this->data.pollId = pollId;
				this->data.config.disabled = status;
			}
        };

        struct MUXY_GAMELINK_API SubscribeMatchPollRequest : SendEnvelope<SubscribeTopicRequestBody>
        {
            explicit inline SubscribeMatchPollRequest(const string& matchId)
			{
				this->action = string("subscribe");
				this->params.target = string("match_poll");
				this->data.topic_id = string(matchId);
			}
        };

        struct MUXY_GAMELINK_API UnsubscribeMatchPollRequest : SendEnvelope<UnsubscribeTopicRequestBody>
        {
            explicit inline UnsubscribeMatchPollRequest(const string& matchId)
			{
				this->action = string("unsubscribe");
				this->params.target = string("match_poll");
				this->data.topic_id = string(matchId);
			}
        };

		struct MatchPollUpdateInformation
		{
			string matchId;
			string pollId;
			string status;

			std::unordered_map<gamelink::string, PollUpdateBody> results;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_4(MatchPollUpdateInformation,
				"match_id", matchId,
				"poll_id", pollId,
				"status", status,
				"results", results
			);
		};

		struct MUXY_GAMELINK_API MatchPollUpdate : ReceiveEnvelope<MatchPollUpdateInformation>
		{};

		struct MatchPollUpdateInformationInternal
		{
			string matchId;
			string pollId;
			string status;

			nlohmann::json results;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_4(MatchPollUpdateInformationInternal,
				"match_id", matchId,
				"poll_id", pollId,
				"status", status,
				"results", results
			);
		};

		struct MUXY_GAMELINK_API MatchPollUpdateInternal : ReceiveEnvelope<MatchPollUpdateInformationInternal>
		{};

		template<typename T>
        struct BroadcastMatchRequestBody
        {
            string matchId;
            string topic;

			T data;

            MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(BroadcastMatchRequestBody,
                "match_id", matchId,
                "topic", topic,
                "data", data
            );
        };

        template<typename T>
        struct BroadcastMatchRequest : SendEnvelope<BroadcastMatchRequestBody<T>>
        {
            BroadcastMatchRequest(const string& matchId, const string& topic, const T& data)
            {
                this->action = string("broadcast");
                this->params.target = string("match");

                this->data.matchId = matchId;
                this->data.topic = topic;
                this->data.data = data;
            }
        };

        struct MUXY_GAMELINK_API BroadcastMatchResponse : ReceiveEnvelope<OKResponseBody>
        {};
    }
}

#endif