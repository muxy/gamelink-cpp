#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_STATE_H
#define MUXY_GAMELINK_SCHEMA_STATE_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		namespace bodies
		{
			struct GetPollBody
			{
				string pollId;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetPollBody, "poll_id", pollId);
			};

			struct CreatePollBody
			{
				string pollId;
				string prompt;
				std::vector<string> options;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(CreatePollBody, "poll_id", pollId, "prompt", prompt, "options", options);
			};

			template<typename T>
			struct CreateUserDataPollBody
			{
				string pollId;
				string prompt;
				std::vector<string> options;
				T userData;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_4(CreateUserDataPollBody,
													"poll_id",
													pollId,
													"prompt",
													prompt,
													"options",
													options,
													"user_data",
													userData);
			};

			struct PollBody
			{
				string prompt;
				std::vector<string> options;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(PollBody, "prompt", prompt, "options", options);
			};

			template<typename T>
			struct UserDataPollBody
			{
				string prompt;
				std::vector<string> options;
				T userData;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(UserDataPollBody, "prompt", prompt, "options", options, "user_data", userData);
			};

			struct PollUpdateBody
			{
				string pollId;

				PollBody poll;
				std::vector<int> results;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(PollUpdateBody, "topic_id", pollId, "poll", poll, "results", results);
			};

			template<typename T>
			struct UserDataPollUpdateBody
			{
				string pollId;

				UserDataPollBody<T> poll;
				std::vector<int> results;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(UserDataPollUpdateBody, "topic_id", pollId, "poll", poll, "results", results);
			};
		}

		struct GetPollRequest : SendEnvelope<bodies::GetPollBody>
		{
			GetPollRequest(const string& pollId);
		};

		struct CreatePollRequest : SendEnvelope<bodies::CreatePollBody>
		{
			CreatePollRequest(const string& pollId, const string& prompt, const std::vector<string>& options);
		};

		template<typename T>
		struct CreateUserDataPollRequest : SendEnvelope<bodies::CreateUserDataPollBody<T>>
		{
			CreateUserDataPollRequest(const string& pollId, const string& prompt, const std::vector<string>& options, T& userData)
			{
				this->action = string("create");
				this->params.target = string("poll");

				this->data.pollId = pollId;
				this->data.prompt = prompt;
				this->data.options = options;

				this->data.userData = userData;
			}
		};

		struct SubscribePollRequest : SendEnvelope<bodies::SubscribeTopicBody>
		{
			SubscribePollRequest(const string& pollId);
		};

		struct PollUpdateResponse : ReceiveEnvelope<bodies::PollUpdateBody>
		{
			PollUpdateResponse(){};
			PollUpdateResponse(const string& pollId,
							   const string& prompt,
							   const std::vector<string>& options,
							   const std::vector<int>& results);
		};

		template<typename T>
		struct UserDataPollUpdateResponse : ReceiveEnvelope<bodies::UserDataPollUpdateBody<T>>
		{
		};
	}
}

#endif
