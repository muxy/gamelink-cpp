#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_STATE_H
#define MUXY_GAMELINK_SCHEMA_STATE_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		struct GetPollRequestBody
		{
			string pollId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetPollRequestBody, "poll_id", pollId);
		};

		struct CreatePollRequestBody
		{
			string pollId;
			string prompt;
			std::vector<string> options;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(CreatePollRequestBody, "poll_id", pollId, "prompt", prompt, "options", options);
		};

		template<typename T>
		struct CreatePollWithUserDataRequestBody
		{
			string pollId;
			string prompt;
			std::vector<string> options;
			T userData;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_4(CreatePollWithUserDataRequestBody,
												"poll_id",
												pollId,
												"prompt",
												prompt,
												"options",
												options,
												"user_data",
												userData);
		};

		struct PollResponseBody
		{
			string prompt;
			std::vector<string> options;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(PollResponseBody, "prompt", prompt, "options", options);
		};

		template<typename T>
		struct PollWithUserDataResponseBody
		{
			string prompt;
			std::vector<string> options;
			T userData;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(PollWithUserDataResponseBody, "prompt", prompt, "options", options, "user_data", userData);
		};

		struct PollUpdateBody
		{
			string pollId;

			PollResponseBody poll;
			std::vector<int> results;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(PollUpdateBody, "topic_id", pollId, "poll", poll, "results", results);
		};

		template<typename T>
		struct PollWithUserDataUpdateBody
		{
			string pollId;

			PollWithUserDataResponseBody<T> poll;
			std::vector<int> results;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(PollWithUserDataUpdateBody, "topic_id", pollId, "poll", poll, "results", results);
		};

		struct GetPollRequest : SendEnvelope<GetPollRequestBody>
		{
			explicit GetPollRequest(const string& pollId);
		};

		struct CreatePollRequest : SendEnvelope<CreatePollRequestBody>
		{
			CreatePollRequest(const string& pollId, const string& prompt, const std::vector<string>& options);
		};

		template<typename T>
		struct CreatePollWithUserDataRequest : SendEnvelope<CreatePollWithUserDataRequestBody<T>>
		{
			CreatePollWithUserDataRequest(const string& pollId, const string& prompt, const std::vector<string>& options, T& userData)
			{
				this->action = string("create");
				this->params.target = string("poll");

				this->data.pollId = pollId;
				this->data.prompt = prompt;
				this->data.options = options;

				this->data.userData = userData;
			}
		};

		struct SubscribePollRequest : SendEnvelope<SubscribeTopicRequestBody>
		{
			SubscribePollRequest(const string& pollId);
		};

		struct PollUpdateResponse : ReceiveEnvelope<PollUpdateBody>
		{
			PollUpdateResponse()
			{};
			
			PollUpdateResponse(const string& pollId,
							   const string& prompt,
							   const std::vector<string>& options,
							   const std::vector<int>& results);
		};

		template<typename T>
		struct PollWithUserDataUpdateResponse : ReceiveEnvelope<PollWithUserDataUpdateBody<T>>
		{
		};
	}
}

#endif
