#pragma once
#ifndef MUXY_GATEWAY_C_H
#define MUXY_GATEWAY_C_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/*
		!!!!!! WARNING !!!!!!
		Any function that returns a MGW_String must be freed with MGW_FreeString or you will have a memory leak!
	*/

#ifdef MUXY_CLIB_EXPORT
#	ifdef _MSC_VER
#		define MUXY_GW_CLIB_API __declspec(dllexport)
#	else
#		define MUXY_GW_CLIB_API
#	endif
#else
#	define MUXY_GW_CLIB_API
#endif

	typedef char* MGW_String;
	typedef uint16_t MGW_RequestID;
	static const MGW_RequestID MGW_ANY_REQUEST_ID = UINT16_MAX;
	static const MGW_RequestID MGW_REJECTED_REQUEST_ID = UINT16_MAX - 1;

	typedef struct
	{
		void* SDK;
	} MGW_SDK;

	typedef struct
	{
		const char* JWT;
		const char* RefreshToken;
		const char* TwitchName;
		uint32_t HasError;
	} MGW_AuthenticateResponse;

	typedef struct
	{
		const char* GameName;
		// Base64 encoded image
		const char* GameLogo;
		const char* Theme;
	} MGW_GameMetadata;

	typedef struct
	{
		const char* Bytes;
		size_t Length;
	} MGW_Payload;

	typedef void (*MGW_AuthenticateResponseCallback)(void* User, const MGW_AuthenticateResponse* AuthResp);
	typedef void (*MGW_PayloadCallback)(void* User, const MGW_Payload* Payload);
	typedef void (*MGW_DebugMessageCallback)(void* User, const char* msg);

	MUXY_GW_CLIB_API MGW_SDK MGW_MakeSDK(const char* GameID);
	MUXY_GW_CLIB_API void MGW_KillSDK(MGW_SDK Gateway);

	MUXY_GW_CLIB_API MGW_RequestID MGW_SDK_AuthenticateWithPIN(MGW_SDK Gateway, const char* PIN, MGW_AuthenticateResponseCallback Callback, void* User);
	MUXY_GW_CLIB_API MGW_RequestID MGW_SDK_AuthenticateWithRefreshToken(MGW_SDK Gateway, const char* RefreshToken, MGW_AuthenticateResponseCallback Callback, void* User);
	MUXY_GW_CLIB_API void MGW_SDK_Deauthenticate(MGW_SDK Gateway);
	MUXY_GW_CLIB_API uint32_t MGW_SDK_IsAuthenticated(MGW_SDK Gateway);

	MUXY_GW_CLIB_API void MGW_SDK_OnDebugMessage(MGW_SDK Gateway, MGW_DebugMessageCallback Callback, void* UserData);
	MUXY_GW_CLIB_API void MGW_SDK_DetachOnDebugMessage(MGW_SDK Gateway);

	MUXY_GW_CLIB_API void MGW_SDK_HandleReconnect(MGW_SDK Gateway);

	MUXY_GW_CLIB_API MGW_RequestID MGW_SDK_SetGameMetadata(MGW_SDK Gateway, MGW_GameMetadata Meta);

	MUXY_GW_CLIB_API uint32_t MGW_SDK_ReceiveMessage(MGW_SDK Gateway, const char* Bytes, uint32_t Length);
	MUXY_GW_CLIB_API uint32_t MGW_SDK_HasPayloads(MGW_SDK Gateway);
	MUXY_GW_CLIB_API void MGW_SDK_ForeachPayload(MGW_SDK Gateway, MGW_PayloadCallback Callback, void* User);

	MUXY_GW_CLIB_API MGW_String MGW_SDK_GetSandboxURL(MGW_SDK Gateway);
	MUXY_GW_CLIB_API MGW_String MGW_SDK_GetProductionURL(MGW_SDK Gateway);

	MUXY_GW_CLIB_API MGW_String MGW_SDK_GetProjectionSandboxURL(MGW_SDK Gateway, const char* projection, int major, int minor, int patch);
	MUXY_GW_CLIB_API MGW_String MGW_SDK_GetProjectionProductionURL(MGW_SDK Gateway, const char* projection, int major, int minor, int patch);

	MUXY_GW_CLIB_API void MGW_FreeString(MGW_String Str);

	// Polling.
	static const int32_t MGW_POLL_LOCATION_DEFAULT = 0;
	static const int32_t MGW_POLL_MODE_CHAOS = 0;
	static const int32_t MGW_POLL_MODE_ORDER = 1;

	typedef struct
	{
		int32_t Winner;
		int32_t WinningVoteCount;

		const int32_t* Results;
		uint64_t ResultCount;

		int32_t Count;
		double Mean;

		uint32_t IsFinal;
	} MGW_PollUpdate;

	typedef void (*MGW_PollUpdateCallback)(void*, MGW_PollUpdate*);

	typedef struct
	{
		const char* Prompt;

		int32_t Location;
		int32_t Mode;

		const char** Options;
		uint64_t OptionsCount;

		int32_t Duration;

		MGW_PollUpdateCallback OnUpdate;
		void* User;
	} MGW_PollConfiguration;

	MUXY_GW_CLIB_API void MGW_SDK_StartPoll(MGW_SDK Gateway, MGW_PollConfiguration config);
	MUXY_GW_CLIB_API void MGW_SDK_StopPoll(MGW_SDK Gateway);

	static const int32_t MGW_ACTIONSTATE_UNAVAILABLE = 0;
	static const int32_t MGW_ACTIONSTATE_AVAILABLE = 1;
	static const int32_t MGW_ACTIONSTATE_HIDDEN = 2;

	static const int32_t MGW_ACTIONCATEGORY_NEUTRAL = 0;
	static const int32_t MGW_ACTIONCATEGORY_HINDER = 1;
	static const int32_t MGW_ACTIONCATEGORY_HELP = 2;

	static const int32_t MGW_ACTION_INFINITE_USES = -1;

	typedef struct
	{
		const char* ID;
		int32_t Category;
		int32_t State;
		int32_t Impact;

		const char* Name;
		const char* Description;
		const char* Icon;

		int32_t Count;
	} MGW_Action;

	MUXY_GW_CLIB_API void MGW_SDK_SetActions(MGW_SDK Gateway, const MGW_Action* Actions, uint64_t Count);
	MUXY_GW_CLIB_API void MGW_SDK_EnableAction(MGW_SDK Gateway, const char* id);
	MUXY_GW_CLIB_API void MGW_SDK_DisableAction(MGW_SDK Gateway, const char* id);
	MUXY_GW_CLIB_API void MGW_SDK_SetActionMaximumCount(MGW_SDK Gateway, const char* id, int32_t count);
	MUXY_GW_CLIB_API void MGW_SDK_SetActionCount(MGW_SDK Gateway, const char* id, int32_t count);
	MUXY_GW_CLIB_API void MGW_SDK_IncrementActionCount(MGW_SDK Gateway, const char* id, int32_t count);
	MUXY_GW_CLIB_API void MGW_SDK_DecrementActionCount(MGW_SDK Gateway, const char* id, int32_t count);

	typedef struct
	{
		const char* Label;
		const char* Value;
		const char* Icon;
	} MGW_GameText;

	MUXY_GW_CLIB_API void MGW_SDK_SetGameTexts(MGW_SDK Gateway, const MGW_GameText* Texts, uint64_t Count);

	typedef struct
	{
		const char* TransactionID;
		const char* SKU;
		int32_t Bits;

		const char* UserID;
		const char* Username;
	} MGW_BitsUsed;

	typedef struct
	{
		const char* TransactionID;
		const char* ActionID;
		int32_t Cost;

		const char* UserID;
		const char* Username;
	} MGW_ActionUsed;

	typedef void (*MGW_OnBitsUsedCallback)(void*, MGW_BitsUsed*);
	typedef void (*MGW_OnActionUsedCallback)(void*, MGW_ActionUsed*);

	MUXY_GW_CLIB_API void MGW_SDK_OnBitsUsed(MGW_SDK Gateway, MGW_OnBitsUsedCallback Callback, void* User);
	MUXY_GW_CLIB_API void MGW_SDK_OnActionUsed(MGW_SDK Gateway, MGW_OnActionUsedCallback Callback, void* User);

	MUXY_GW_CLIB_API void MGW_SDK_AcceptAction(MGW_SDK Gateway, MGW_ActionUsed Coins, const char* Reason);
	MUXY_GW_CLIB_API void MGW_SDK_RefundAction(MGW_SDK Gateway, MGW_ActionUsed Coins, const char* Reason);

#ifdef __cplusplus
}
#endif
#endif