#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_POLL_H
#define MUXY_GAMELINK_SCHEMA_POLL_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	/// PollConfiguration is used in the ConfigurePoll APIs to control
	/// details of how a poll is held.
	struct MUXY_GAMELINK_API PollConfiguration
	{
		PollConfiguration();

		/// When userIdVoting is true, only users that have shared their ID will
		/// be able to vote.
		bool userIdVoting;

		/// distinctOptionsPerUser controls how many options a user can vote for.
		/// Must be in the range [1, 258].
		int distinctOptionsPerUser;

		/// totalVotesPerUser controls how many votes any user can cast.
		/// Must be in the range [1, 1024]
		int totalVotesPerUser;

		/// votesPerOption controls how many votes per option a user can cast.
		/// Must be in the range [1, 1024]
		int votesPerOption;

		/// When disabled is true, the poll will not accept any votes.
		bool disabled;

		/// startsAt should be a unix timestamp, in seconds, at which point the poll
		/// will become enabled and will be able to be voted on.
		/// If no startAt time is desired, set this to 0.
		int64_t startsAt;

		/// endsAt should be a unix timestamp, in seconds, at which point the poll
		/// will become disabled.
		/// If no endsAt time is desired, set this to 0.
		int64_t endsAt;

		/// startsIn will set a relative startsAt, in seconds from now, at which point the poll
		/// will become enabled and will be able to be voted on.
		/// if startsAt is non-zero, this field has no effect.
		int64_t startsIn;

		/// endsIn will set a relative endsAt, in seconds from now, at which point the poll
		/// will become disabled.
		/// if endsAt is non-zero, this field has no effect.
		int64_t endsIn;

		/// Arbitrary user data field.
		/// For legacy reasons, this isn't part of the config schema, but rather
		/// promoted to the request body when used as part of CreatePollWithConfigurationRequest
		nlohmann::json userData;

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_9(PollConfiguration,
			"userIDVoting", userIdVoting,
			"distinctOptionsPerUser", distinctOptionsPerUser,
			"totalVotesPerUser", totalVotesPerUser,
			"votesPerOption", votesPerOption,
			"disabled", disabled,
			"startsAt", startsAt,
			"endsAt", endsAt,
			"startsIn", startsIn,
			"endsIn", endsIn);
	};

	namespace schema
	{
		struct MUXY_GAMELINK_API GetPollRequestBody
		{
			/// The Poll ID to get
			string pollId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetPollRequestBody, "poll_id", pollId);
		};

		struct MUXY_GAMELINK_API DeletePollRequestBody
		{
			string pollId;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(DeletePollRequestBody, "poll_id", pollId);
		};

		struct MUXY_GAMELINK_API CreatePollRequestBody
		{
			/// The Poll ID to create. Poll IDs are scoped to the current channel.
			string pollId;

			/// The poll prompt
			string prompt;

			/// A list of answers to the prompt. Maximum 64.
			std::vector<string> options;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(CreatePollRequestBody, "poll_id", pollId, "prompt", prompt, "options", options);
		};

		struct MUXY_GAMELINK_API CreatePollWithConfigurationRequestBody
		{
			/// The Poll ID to create. Poll IDs are scoped to the current channel.
			string pollId;

			/// The poll prompt
			string prompt;

			/// A list of answers to the prompt. Maximum 64.
			std::vector<string> options;

			/// Configuration information
			PollConfiguration configuration;

			// Arbitrary user data
			nlohmann::json userData;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_5(CreatePollWithConfigurationRequestBody,
				"poll_id", pollId,
				"prompt", prompt,
				"options", options,
				"config", configuration,
				"user_data", userData);
		};

		struct MUXY_GAMELINK_API PollResponseBody
		{
			/// The Poll ID that the update is for
			string pollId;

			/// The prompt for the poll.
			string prompt;

			/// The current status of the poll.
			string status;

			/// A list of answers to the prompt. Maximum 64.
			std::vector<string> options;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_4(PollResponseBody, "poll_id", pollId, "prompt", prompt, "status", status, "options", options);
		};

		template<typename T>
		struct MUXY_GAMELINK_API PollWithUserDataResponseBody
		{
			/// The Poll ID that the update is for
			string pollId;

			/// The prompt for the poll.
			string prompt;

			/// A list of answers to the prompt. Maximum 64.
			std::vector<string> options;

			/// The current status of the poll.
			string status;

			/// Arbitrary user data type. Must be marked up with the MUXY_GAMELINK_SERIALIZE macro.
			T userData;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_5(PollWithUserDataResponseBody, "poll_id", pollId, "prompt", prompt, "options", options, "status", status, "user_data", userData);
		};

		// Note that this is the same as PollUpdateBody, and is provided for consistency with each
		// endpoint having their own envelope with body.
		struct MUXY_GAMELINK_API GetPollResponseBody
		{
			/// The poll information
			PollResponseBody poll;

			/// Array of vote counts for each poll option. This array may not be the same size as the
			/// options array.
			std::vector<int> results;

			/// Arithmetic mean of all poll values, including ones that outside the [0, 32) range.
			double mean;

			/// Sum of all poll values, including ones that outside the [0, 32) range.
			double sum;

			/// Number of responses, including ones that outside the [0, 32) range.
			int32_t count;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_5(GetPollResponseBody, "poll", poll, "results", results, "mean", mean, "sum", sum, "count", count);
		};

		struct MUXY_GAMELINK_API PollUpdateBody
		{
			/// The poll information
			PollResponseBody poll;

			/// Array of vote counts for each poll option. This array may not be the same size as the
			/// options array.
			std::vector<int> results;

			/// Arithmetic mean of all poll values, including ones that outside the [0, 32) range.
			double mean;

			/// Sum of all poll values, including ones that outside the [0, 32) range.
			double sum;

			/// Number of responses, including ones that outside the [0, 32) range.
			int32_t count;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_5(PollUpdateBody,
				"poll", poll,
				"results", results,
				"mean", mean,
				"sum", sum,
				"count", count
			);
		};

		template<typename T>
		struct PollWithUserDataUpdateBody
		{
			/// The Poll ID that this update represents
			string pollId;

			/// The poll prompt and options.
			PollWithUserDataResponseBody<T> poll;

			/// An array of vote counts for each poll option. This array may not be the same
			/// size as the options array.
			std::vector<int> results;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(PollWithUserDataUpdateBody, "topic_id", pollId, "poll", poll, "results", results);
		};

		struct MUXY_GAMELINK_API GetPollRequest : SendEnvelope<GetPollRequestBody>
		{
			/// Creates a GetPoll request
			/// @param[in] pollId The ID of the poll to get. Maximum size 64 characters.
			explicit GetPollRequest(const string& pollId);
		};

		struct MUXY_GAMELINK_API CreatePollRequest : SendEnvelope<CreatePollRequestBody>
		{
			/// Creates a CreatePoll request.
			/// @param[in] pollId The ID of the poll to create. This can overwrite existing polls if the same
			///                   id is specified.
			/// @param[in] prompt The prompt for the poll to create.
			/// @param[in] options vector of options for the poll.
			CreatePollRequest(const string& pollId, const string& prompt, const std::vector<string>& options);
		};

		struct MUXY_GAMELINK_API CreatePollWithConfigurationRequest : SendEnvelope<CreatePollWithConfigurationRequestBody>
		{
			/// Creates a CreatePoll request with configuration
			/// @param[in] pollId The ID of the poll to create. This can overwrite existing polls if the same
			///                   id is specified.
			/// @param[in] prompt The prompt for the poll to create.
			/// @param[in] config PollConfiguration for the poll.
			/// @param[in] options vector of options for the poll.
			CreatePollWithConfigurationRequest(const string& pollId, const string& prompt, const PollConfiguration& config, const std::vector<string>& options);
		};

		struct MUXY_GAMELINK_API SubscribePollRequest : SendEnvelope<SubscribeTopicRequestBody>
		{
			/// Creates a SubscribePollRequest.
			/// @param[in] pollId The ID of the poll to subscribe to updates for.
			explicit SubscribePollRequest(const string& pollId);
		};

		struct MUXY_GAMELINK_API UnsubscribePollRequest : SendEnvelope<UnsubscribeTopicRequestBody>
		{
			/// Creates an UnsubscribePollRequest.
			/// @param[in] pollId The ID of the poll to subscribe to updates for.
			explicit UnsubscribePollRequest(const string& pollId);
		};

		struct ExpireConfig
		{
			int32_t endsAt;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(ExpireConfig, "endsAt", endsAt);
		};

		struct DisableConfig
		{
			bool disabled;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(DisableConfig, "disabled", disabled);
		};

		template<typename Config>
		struct PollReconfigureBody
		{
			string pollId;
			Config config;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(PollReconfigureBody, "poll_id", pollId, "config", config);
		};

		struct MUXY_GAMELINK_API ExpirePollRequest : SendEnvelope<PollReconfigureBody<ExpireConfig>>
		{
			/// Creates an ExpirePollRequest.
			/// @param[in] pollId The ID of the poll expire
			explicit ExpirePollRequest(const string& pollId);
		};

		struct MUXY_GAMELINK_API SetPollDisabledStatusRequest : SendEnvelope<PollReconfigureBody<DisableConfig>>
		{
			/// Creates an ExpirePollRequest.
			/// @param[in] pollId The ID of the poll set the status of.
			/// @param[in] disabled The new disabled state.
			SetPollDisabledStatusRequest(const string& pollId, bool disabled);
		};

		struct MUXY_GAMELINK_API PollUpdateResponse : ReceiveEnvelope<PollUpdateBody>
		{
		};

		struct MUXY_GAMELINK_API GetPollResponse : ReceiveEnvelope<GetPollResponseBody>
		{
		};

		template<typename T>
		struct PollWithUserDataUpdateResponse : ReceiveEnvelope<PollWithUserDataUpdateBody<T>>
		{
		};

		struct MUXY_GAMELINK_API DeletePollRequest : SendEnvelope<DeletePollRequestBody>
		{
			/// Creates a DeletePoll request
			/// @param[in] pollId 	The ID of the poll to be deleted.
			explicit DeletePollRequest(const string& pollId);
		};
	}
}

#endif
