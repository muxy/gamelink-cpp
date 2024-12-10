#pragma once

#include "gamelink.h"

namespace gateway
{
	typedef gamelink::string string;
	typedef gamelink::RequestId RequestID;

	static const RequestID ANY_REQUEST_ID = gamelink::ANY_REQUEST_ID;
	static const RequestID REJECTED_REQUEST_ID = gamelink::REJECTED_REQUEST_ID;

	struct GameMetadata
	{
		string GameName;
		// Base64 encoded image
		string GameLogo;
		string Theme;
	};

	class Payload
	{
		friend class SDK;
	public:
		const char* GetData() const;
		uint32_t GetLength() const;

	private:
		Payload(const char* data, uint32_t length);

		const char* _Data;
		uint32_t _Length;
	};

	class AuthenticateResponse
	{
	public:
		AuthenticateResponse(string JWT, string RefreshToken, string TwitchName, bool DidError)
			: JWT(JWT)
			, RefreshToken(RefreshToken)
			, TwitchName(TwitchName)
			, DidError(DidError)
		{
		}

		/// Signed JWT. Will expire.
		string JWT;

		/// Refresh token, used to reauth after the JWT expires.
		string RefreshToken;

		/// Information about the channel the auth was done with
		string TwitchName;

		bool HasError() const
		{
			return DidError;
		}
	private:
		bool DidError;
	};

	enum class PollMode
	{
		Chaos = 0,
		Order = 1
	};

	enum class PollLocation
	{
		Default = 0,
	};

	struct PollUpdate
	{
		// Index of the winner, in [0, 32).
		// If one or more values are tied, returns the first one.
		int32_t Winner;

		// The number of votes cast for the winning vote.
		int32_t WinningVoteCount;

		// All results in the range [0, 32).
		std::vector<int32_t> Results;

		// How many votes were cast in total.
		int32_t Count;

		// The average value of all votes.
		double Mean;

		// True if this is the final update.
		bool IsFinal;
	};

	struct MatchPollUpdate
	{
		PollUpdate overall;
		std::unordered_map<string, PollUpdate> perChannel;
	};

	struct PollConfiguration
	{
		string Prompt;
		std::vector<string> Options;

		PollMode Mode = PollMode::Order;
		PollLocation Location = PollLocation::Default;

		// Duration of the poll, in seconds.
		// If set to a negative or zero duration, the poll lasts until a call
		// to StopPoll
		int32_t Duration = 0;

		// Arbitrary user data to send. Should be small.
		nlohmann::json UserData;

		// Called regularly as poll results are streamed in from the server
		std::function<void(const PollUpdate&)> OnUpdate;

		// Called after the poll completes. This is called right after
		std::function<void(const PollUpdate&)> OnComplete;
	};

	template<typename T>
	struct GenericMatchPollConfiguration
	{
		string Prompt;
		std::vector<string> Options;

		PollMode Mode = PollMode::Order;
		PollLocation Location = PollLocation::Default;

		// Duration of the poll, in seconds.
		// If set to a negative or zero duration, the poll lasts until a call
		// to StopPoll
		int32_t Duration = 0;

		// Arbitrary user data to send. Should be small.
		T UserData;

		// Called regularly as poll results are streamed in from the server
		std::function<void(const MatchPollUpdate&)> OnUpdate;

		// Called after the poll completes. This is called right after
		std::function<void(const MatchPollUpdate&)> OnComplete;
	};

	typedef GenericMatchPollConfiguration<nlohmann::json> MatchPollConfiguration;

	struct GamechangerTier
	{
		string IncrementalText;
		double IncrementalValue;

		string EffectText;
		double EffectValue;

		// In seconds
		int64_t TierDuration;

		int64_t TierThreshold;

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_6(GamechangerTier,
			"incremental_effect_text", IncrementalText,
			"incremental_effect_value", IncrementalValue,
			"effect_text", EffectText,
			"effect_value", EffectValue,
			"tier_duration", TierDuration,
			"tier_threshold", TierThreshold
		);
	};

	struct GamechangerPollData
	{
		string Name;
		std::vector<GamechangerTier> Tiers;

		// Shouldn't need to be changed, a marker to signal for special-case handling
		string Type = string("gamechanger");

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(GamechangerPollData,
			"name", Name,
			"type", Type,
			"tiers", Tiers
		);
	};

	typedef GenericMatchPollConfiguration<GamechangerPollData> GamechangerMatchPollConfiguration;

	enum class ActionCategory
	{
		Neutral = 0,
		Hinder = 1,
		Help = 2
	};

	enum class ActionState
	{
		Unavailable = 0,
		Available = 1,
		Hidden = 2,
	};

	static const int32_t ACTION_INFINITE_USES = 0xFFFF;
	struct Action
	{
		string ID;
		ActionCategory Category;
		ActionState State;
		int32_t Impact;

		string Name;
		string Description;
		string Icon;

		int32_t Count;
	};

	struct GameText
	{
		string Label;
		string Value;
		string Icon;
	};

	struct GameTexts
	{
		std::vector<GameText> Texts;
	};

	struct BitsUsed
	{
		string TransactionID;
		string SKU;
		int32_t Bits;

		string UserID;
		string Username;
	};

	struct ActionUsed
	{
		string TransactionID;
		string ActionID;
		int32_t Cost;

		string UserID;
		string Username;
	};

	class SDK
	{
	public:
		explicit SDK(string gameID);
		~SDK();

		// Used to retarget this SDK to a new client ID.
		void SetClientID(const gateway::string& id);

		// Not implemented. SDK is not copyable
		SDK(const SDK&) = delete;
		SDK& operator=(const SDK&) = delete;

		// Not implemented. SDK is not movable
		SDK(SDK&&) = delete;
		SDK& operator=(SDK&&) = delete;

		bool ReceiveMessage(const char* Bytes, uint32_t Length);
		bool HasPayloads() const;

		void OnDebugMessage(std::function<void (const gateway::string&)> message);
		void DetachOnDebugMessage();

		/// Type used in ForeachPayload below. Takes in the user pointer as the first argument,
		/// and a pointer to a payload as the second parameter.
		typedef void (*NetworkCallback)(void*, const Payload*);

		/// Invokes a function pointer for each avaliable payload.
		/// May invoke the callback zero times if there are no outstanding payloads.
		/// Does not internally sleep.
		///
		/// @param[in] cb Callback to be invoked for each avaliable payload
		/// @param[in] user User pointer that is passed into the callback
		void ForeachPayload(NetworkCallback Callback, void* User);

		template<typename CallbackType>
		void ForeachPayload(CallbackType Callback)
		{
			Base.ForeachPayload([=](const gamelink::Payload* Send) {
				gateway::Payload P(Send->Data(), Send->Length());
				Callback(&P);
			});
		}

		void HandleReconnect();

		RequestID AuthenticateWithPIN(const string& PIN, std::function<void(const AuthenticateResponse&)> Callback);
		RequestID AuthenticateWithRefreshToken(const string& JWT, std::function<void(const AuthenticateResponse&)> Callback);

		void Deauthenticate();

		/// Returns if an authentication message has been received.
		///
		/// @return true if an authentication message has been received.
		bool IsAuthenticated() const;

		// Polling

		// These start a poll with id='default', and should be used in the majority of cases.
		void StartPoll(const PollConfiguration& cfg);
		void StopPoll();

		// For multi-poll purposes, use these functions:
		void StartPollWithID(const string& id, const PollConfiguration& cfg);
		void StopPollWithID(const string& id);

		// Actions
		void SetActions(const Action* begin, const Action* end);
		void EnableAction(const string& id);
		void DisableAction(const string& id);
		void SetActionMaximumCount(const string& id, int32_t number);
		void SetActionCount(const string& id, int32_t number);

		void IncrementActionCount(const string& id, int32_t delta);
		void DecrementActionCount(const string& id, int32_t delta);

		// Game Data
		// SetGameTexts sets ordered key-value pairs.
		void SetGameTexts(const GameTexts& data);

		// Sets the game text at the given index.
		void SetGameText(int index, const GameText& text);

		// SetGameVector4 sets a key->vec4 of single precision floating point data.
		void SetGameVector4(const string& label, const float* ptr);
		void SetGameVector4WithComponents(const string& label, float x, float y, float z, float w);

		// Low level set game access. Sets a serializable array to the json path.
		template<typename T>
		void UpdateGameStatePathWithArray(const string& path, const T* begin, const T* end)
		{
			Base.UpdateStateWithArray(gamelink::StateTarget::Channel, gamelink::Operation::Add, path, begin, end);
		}

		// Low level set game access. Sets an serializable object to the json path.
		template<typename T>
		void UpdateGameStatePathWithObject(const string& path, const T& obj)
		{
			Base.UpdateStateWithObject(gamelink::StateTarget::Channel, gamelink::Operation::Add, path, obj);
		}

		RequestID SetGameMetadata(GameMetadata Meta);

		string GetProjectionSandboxURL(const string& projection, int major, int minor, int patch) const;
		string GetProjectionProductionURL(const string& projection, int major, int minor, int patch) const;

		string GetSandboxURL() const;
		string GetProductionURL() const;

		void OnBitsUsed(std::function<void (const gateway::BitsUsed&)> Callback);
		void OnActionUsed(std::function<void (const gateway::ActionUsed&)> Callback);

		void AcceptAction(const gateway::ActionUsed& used, const gamelink::string& Details);
		void RefundAction(const gateway::ActionUsed& used, const gamelink::string& Details);

		// Matches
		void CreateMatch(const string& match);
		void KeepMatchAlive(const string& match);
		void AddChannelsToMatch(const string& match, const string* start, const string* end);
		void RemoveChannelsFromMatch(const string& match, const string* start, const string* end);

		template<typename T>
		void RunMatchPoll(const string& match, const GenericMatchPollConfiguration<T>& cfg)
		{
			MatchPollConfiguration proxy;
			proxy.Prompt = cfg.Prompt;
			proxy.Options = cfg.Options;
			proxy.Mode = cfg.Mode;
			proxy.Location = cfg.Location;
			proxy.Duration = cfg.Duration;
			proxy.UserData = cfg.UserData;
			proxy.OnUpdate = cfg.OnUpdate;
			proxy.OnComplete = cfg.OnComplete;

			return RunMatchPoll(match, proxy);
		}

		void RunMatchPoll(const string& match, const MatchPollConfiguration& cfg);
		void StopMatchPoll(const string& match);

		template<typename T>
		void RunMatchPollWithID(const string& match, const string& id, const GenericMatchPollConfiguration<T>& cfg)
		{
			MatchPollConfiguration proxy;
			proxy.Prompt = cfg.Prompt;
			proxy.Options = cfg.Options;
			proxy.Mode = cfg.Mode;
			proxy.Location = cfg.Location;
			proxy.Duration = cfg.Duration;
			proxy.UserData = cfg.UserData;
			proxy.OnUpdate = cfg.OnUpdate;
			proxy.OnComplete = cfg.OnComplete;

			return RunMatchPollWithID(match, id, proxy);
		}

		void RunMatchPollWithID(const string& match, const string& id, const MatchPollConfiguration& cfg);
		void StopMatchPollWithID(const string& match, const string& id);
	private:
		gamelink::SDK Base;

		string GameID;
		string ClientID = string("i575hs2x9lb3u8hqujtezit03w1740");
	};
}
