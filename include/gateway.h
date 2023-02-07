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

	private:
		explicit Payload(string data);

		/// Data to be sent.
		const string _Data;

	public:
		const char* GetData() const;
		uint32_t GetLength() const;
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
		};

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
		int Winner;

		// The number of votes cast for the winning vote.
		int WinningVoteCount;

		// All results in the range [0, 32).
		std::vector<int> Results;
		
		// How many votes were cast in total.
		int Count;

		// The average value of all votes.
		double Mean;

		// True iff this is a final update for the vote.
		bool IsFinal;
	};

	struct PollConfiguration
	{
		string Prompt;
		std::vector<string> Options;

		PollMode Mode = PollMode::Order;
		PollLocation Location = PollLocation::Default;
		int32_t Duration = 0;

		std::function<void(const PollUpdate&)> OnUpdate;
	};

	class SDK
	{
	public:
		explicit SDK(string gameID);
		~SDK();

		// Not implemented. SDK is not copyable
		SDK(const SDK&) = delete;
		SDK& operator=(const SDK&) = delete;

		// Not implemented. SDK is not movable
		SDK(SDK&&) = delete;
		SDK& operator=(SDK&&) = delete;

		bool ReceiveMessage(const char* Bytes, uint32_t Length);
		bool HasPayloads() const;

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

		void HandleReconnect();

		RequestID AuthenticateWithPIN(const string& PIN, std::function<void(const AuthenticateResponse&)> Callback);
		RequestID AuthenticateWithRefreshToken(const string& JWT, std::function<void(const AuthenticateResponse&)> Callback);

		/// Returns if an authentication message has been received.
		///
		/// @return true if an authentication message has been received.
		bool IsAuthenticated() const;

		// Polling
		void StartPoll(const PollConfiguration& cfg);
		void StopPoll();

		RequestID SetGameMetadata(GameMetadata Meta);

		string GetSandboxURL() const;
		string GetProductionURL() const;

	private:
		gamelink::SDK Base;
		string GameID;
		string ClientID = string("i575hs2x9lb3u8hqujtezit03w1740");
	};
}
