#pragma once
#ifndef INCLUDE_MUXY_GATEWAY_H
#define INCLUDE_MUXY_GATEWAY_H

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

		// Base64 encoded image, PNG, JPG or SVG
		string GameLogo;
		string Theme;
	};

#ifndef MUXY_GATEWAY_WITHOUT_PNG_ENCODER
	/// Encodes an image as packed RGB(A) data into a base64 data url.
	/// @param image - Pointer to array of bytes in RGB or RGBA format, 8 bits per component.
	/// @param width - How many pixels wide the image is.
	/// @param height - How many pixels high the image is.
	/// @param components - 3 for RGB, 4 for RGBA. Other values are not supported.
	/// @param strideBytes - How many bytes a row of pixels are. May be larger than width * component.
	/// @param flipVertically - Set to true to flip the Y axis of the image efore encoding.
	/// @return Returns a data url in the form "data:image/png;base64,<base64encoded image>"
	/// @note This function is pretty slow, avoid calling this function on the main thread.
	string EncodeImageToBase64PNG(const void* image, uint32_t width, uint32_t height, uint32_t components, uint32_t strideBytes, bool flipVertically);
#endif

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

	static const int32_t ACTION_INFINITE_USES = -1;
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

		/// Returns if an authentication message has been received.
		///
		/// @return true if an authentication message has been received.
		bool IsAuthenticated() const;

		// Polling
		void StartPoll(const PollConfiguration& cfg);
		void StopPoll();

		// Actions
		void SetActions(const Action* begin, const Action* end);
		void EnableAction(const string& id);
		void DisableAction(const string& id);
		void SetActionCount(const string& id, int32_t number);

		// Game Data
		void SetGameTexts(const GameTexts& data);

		RequestID SetGameMetadata(GameMetadata Meta);

		string GetSandboxURL() const;
		string GetProductionURL() const;

		void OnBitsUsed(std::function<void (const gateway::BitsUsed&)> Callback);
		void OnActionUsed(std::function<void (const gateway::ActionUsed&)> Callback);

		void AcceptAction(const gateway::ActionUsed& used, const gamelink::string& Details);
		void RefundAction(const gateway::ActionUsed& used, const gamelink::string& Details);
	private:
		gamelink::SDK Base;
		string GameID;
		string ClientID = string("i575hs2x9lb3u8hqujtezit03w1740");
	};
}

#endif
