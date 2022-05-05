#pragma once
#ifndef MUXY_GAMELINK_C_H
#define MUXY_GAMELINK_C_H
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/*
		!!!!!! WARNING !!!!!!
		Any function that returns a MGL_String must be freed with MuxyGameLink_FreeString or you will have a memory leak!
	*/

#	ifdef MUXY_CLIB_EXPORT
#		ifdef _MSC_VER
#			define MUXY_CLIB_API __declspec(dllexport)
#		else
#			define MUXY_CLIB_API 
#		endif
#	else
#		define MUXY_CLIB_API 
#	endif

	typedef char* MGL_String;
	typedef uint16_t MGL_RequestId;
	static const MGL_RequestId ANY_REQUEST_ID = 0xFFFF;

	typedef struct
	{
		void* SDK;
	} MuxyGameLink;

	/*
		Do not store these structs, their lifetimes are generally bound by the SDK lifetime or
		the callback lifetime. Consult the documentation for more information.
	*/
	typedef struct
	{
		const void* Obj;
	} MGL_Error;

	typedef struct
	{
		const void* Obj;
	} MGL_Payload;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_User;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_StateResponse;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_StateUpdateResponse;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_ConfigResponse;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_ConfigUpdateResponse;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_AuthenticateResponse;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_DatastreamUpdate;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_DatastreamEvent;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_TransactionResponse;

	typedef struct
	{
		const void* Obj;
	} MGL_Schema_GetOutstandingTransactionsResponse;

	typedef struct 
	{
		const void* Obj;
	} MGL_Schema_GetPollResponse;

	typedef struct 
	{
		const void* Obj;
	} MGL_Schema_PollUpdateResponse;

	typedef struct
	{
		void* Obj;
	} MGL_PatchList;

	/*
		SDK Callbacks
	*/
	typedef void (*MGL_AuthenticateResponseCallback)(void* UserData, MGL_Schema_AuthenticateResponse AuthResp);
	typedef void (*MGL_PayloadCallback)(void* UserData, MGL_Payload Payload);
	typedef void (*MGL_DatastreamUpdateCallback)(void* UserData, MGL_Schema_DatastreamUpdate DatastreamUpdate);
	typedef void (*MGL_TransactionResponseCallback)(void* UserData, MGL_Schema_TransactionResponse TPBResp);
	typedef void (*MGL_GetPollResponseCallback)(void* UserData, MGL_Schema_GetPollResponse GPResp);
	typedef void (*MGL_GetOutstandingTransactionsResponseCallback)(void* UserData, MGL_Schema_GetOutstandingTransactionsResponse GOTResp);
	typedef void (*MGL_PollUpdateResponseCallback)(void* UserData, MGL_Schema_PollUpdateResponse PUResp);
	typedef void (*MGL_StateResponseCallback)(void* UserData, MGL_Schema_StateResponse StateResp);
	typedef void (*MGL_StateUpdateResponseCallback)(void* UserData, MGL_Schema_StateUpdateResponse StateResp);
	typedef void (*MGL_ConfigResponseCallback)(void* UserData, MGL_Schema_ConfigResponse ConfigResp);
	typedef void (*MGL_ConfigUpdateResponseCallback)(void* UserData, MGL_Schema_ConfigUpdateResponse ConfigResp);
	typedef void (*MGL_OnDebugMessageCallback)(void* UserData, const char *Message);

	/*
		Get URL for connection. Exposes the projection version.
		const char* clientId,
			ConnectionStage stage,
			const char* projection,
			int projectionMajor, int projectionMinor, int projectionPatch);

	*/
	typedef enum
	{
		MGL_CONNECTION_STAGE_PRODUCTION = 0,
		MGL_CONNECTION_STAGE_SANDBOX,

		MGL_CONNECTION_STAGE_ENSURE_32 = 0xFFFFFFFFu
	} MGL_ConnectionStage;

	MUXY_CLIB_API char * MuxyGameLink_ProjectionWebsocketConnectionURL(const char* clientID, MGL_ConnectionStage stage, const char* projection, int projectionMajor, int projectionMinor, int projectionPatch);

	/*
		SDK Initialization functions
	*/
	MUXY_CLIB_API MuxyGameLink MuxyGameLink_Make(void);
	MUXY_CLIB_API void MuxyGameLink_Kill(MuxyGameLink GameLink);

	/*
		SDK String functions
	*/
	MUXY_CLIB_API void MuxyGameLink_FreeString(MGL_String Str);
	MUXY_CLIB_API uint32_t MuxyGameLink_Strlen(MGL_String Str);
	MUXY_CLIB_API char* MuxyGameLink_StrDup(const char *Str);

	/*
		Debug message set
	*/
	MUXY_CLIB_API void MuxyGameLink_OnDebugMessage(MuxyGameLink GameLink, MGL_OnDebugMessageCallback Callback, void* UserData);
	MUXY_CLIB_API void MuxyGameLink_DetachOnDebugMessage(MuxyGameLink GameLink);

	/*
		SDK network send and receive functions
	*/
	MUXY_CLIB_API void MuxyGameLink_ForeachPayload(MuxyGameLink GameLink, MGL_PayloadCallback Callback, void* UserData);
	MUXY_CLIB_API uint32_t MuxyGameLink_Payload_GetSize(MGL_Payload Payload);
	MUXY_CLIB_API const char* MuxyGameLink_Payload_GetData(MGL_Payload Payload);
	MUXY_CLIB_API bool MuxyGameLink_ReceiveMessage(MuxyGameLink GameLink, const char* Bytes, uint32_t Length);
	MUXY_CLIB_API void MuxyGameLink_WaitForResponse(MuxyGameLink GameLink, MGL_RequestId Request);

	/*
		SDK Error manipulation functions
	*/
	MUXY_CLIB_API MGL_Error MuxyGameLink_Schema_GetFirstError(void* Resp);
	MUXY_CLIB_API bool MuxyGameLink_Error_IsValid(MGL_Error Error);
	MUXY_CLIB_API uint32_t MuxyGameLink_Error_GetCode(MGL_Error Error);
	MUXY_CLIB_API const char* MuxyGameLink_Error_GetTitle(MGL_Error Error);
	MUXY_CLIB_API const char* MuxyGameLink_Error_GetDetail(MGL_Error Error);

	/*
		Authentication functions.

		The lifetime of MGL_Schema_AuthenticateResponse in the callback ends
		as soon as the callback finishes executing. Copying this parameter
		is not supported.
	*/
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_AuthenticateWithPIN(MuxyGameLink GameLink,
												   const char* ClientId,
												   const char* PIN,
												   MGL_AuthenticateResponseCallback Callback,
												   void* UserData);

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_AuthenticateWithRefreshToken(MuxyGameLink GameLink,
															const char* ClientId,
															const char* RefreshToken,
															MGL_AuthenticateResponseCallback Callback,
															void* UserData);

	/*
		Authentication checking.
		The MGL_Schema_User object's lifetime is the same as the MuxyGameLink lifetime.
	*/
	MUXY_CLIB_API bool MuxyGameLink_IsAuthenticated(MuxyGameLink GameLink);
	MUXY_CLIB_API MGL_Schema_User MuxyGameLink_GetSchemaUser(MuxyGameLink GameLink);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_User_GetJWT(MGL_Schema_User User);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_User_GetRefreshToken(MGL_Schema_User User);

	/*
		State functions
	*/

	typedef enum
	{
		MGL_STATE_TARGET_CHANNEL = 0,
		MGL_STATE_TARGET_EXTENSION,
	} MGL_StateTarget;

	typedef enum
	{
		MGL_CONFIG_TARGET_CHANNEL = 0,
		MGL_CONFIG_TARGET_EXTENSION,
		MGL_CONFIG_TARGET_COMBINED,
	} MGL_ConfigTarget;

	typedef enum
	{
		MGL_OPERATION_ADD = 0,
		MGL_OPERATION_REMOVE,
		MGL_OPERATION_REPLACE,
		MGL_OPERATION_COPY,
		MGL_OPERATION_MOVE,
		MGL_OPERATION_TEST
	} MGL_Operation;

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SetState(MuxyGameLink GameLink, MGL_StateTarget Target, const char* JsonString);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_GetState(MuxyGameLink GameLink,
										MGL_StateTarget Target,
										MGL_StateResponseCallback Callback,
										void* UserData);

	MUXY_CLIB_API MGL_String MuxyGameLink_Schema_StateResponse_GetJson(MGL_Schema_StateResponse);

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateStateWithInteger(MuxyGameLink GameLink, MGL_StateTarget Target, MGL_Operation Operation, const char* Path, int64_t Value);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateStateWithDouble(MuxyGameLink GameLink, MGL_StateTarget Target, MGL_Operation Operation, const char* Path, double Value);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateStateWithString(MuxyGameLink GameLink,
													 MGL_StateTarget Target,
													 MGL_Operation Operation,
													 const char* Path,
													 const char* Value);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateStateWithLiteral(MuxyGameLink GameLink,
													  MGL_StateTarget Target,
													  MGL_Operation Operation,
													  const char* Path,
													  const char* Json);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateStateWithNull(MuxyGameLink GameLink, MGL_StateTarget Target, MGL_Operation Operation, const char* Path);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateStateWithPatchList(MuxyGameLink GameLink, MGL_StateTarget Target, MGL_PatchList PList);

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SubscribeToStateUpdates(MuxyGameLink GameLink, MGL_StateTarget Target);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UnsubscribeFromStateUpdates(MuxyGameLink GameLink, MGL_StateTarget Target);

	MUXY_CLIB_API uint32_t MuxyGameLink_OnStateUpdate(MuxyGameLink GameLink, MGL_StateUpdateResponseCallback Callback, void* UserData);
	MUXY_CLIB_API void MuxyGameLink_DetachOnStateUpdate(MuxyGameLink GameLink, uint32_t Id);
	MUXY_CLIB_API MGL_StateTarget MuxyGameLink_Schema_StateUpdateResponse_GetTarget(MGL_Schema_StateUpdateResponse Response);
	MUXY_CLIB_API MGL_String MuxyGameLink_Schema_StateUpdateResponse_GetJson(MGL_Schema_StateUpdateResponse Response);

	/*
		Config functions
	*/

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SetChannelConfig(MuxyGameLink GameLink, const char* JsonString);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_GetConfig(MuxyGameLink GameLink, MGL_ConfigTarget Target, MGL_ConfigResponseCallback Callback, void* UserData);

	MUXY_CLIB_API MGL_ConfigTarget MuxyGameLink_Schema_ConfigResponse_GetConfigID(MGL_Schema_ConfigResponse);
	MUXY_CLIB_API MGL_String MuxyGameLink_Schema_ConfigResponse_GetJson(MGL_Schema_ConfigResponse);

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateChannelConfigWithInteger(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, int64_t Value);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateChannelConfigWithDouble(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, double Value);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateChannelConfigWithString(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, const char* Value);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateChannelConfigWithLiteral(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, const char* Json);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UpdateChannelConfigWithNull(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path);

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SubscribeToConfigurationChanges(MuxyGameLink GameLink, MGL_ConfigTarget Target);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UnsubscribeToConfigurationChanges(MuxyGameLink GameLink, MGL_ConfigTarget Target);
	MUXY_CLIB_API uint32_t MuxyGameLink_OnConfigUpdate(MuxyGameLink GameLink, MGL_ConfigUpdateResponseCallback Callback, void* UserData);
	MUXY_CLIB_API void MuxyGameLink_DetachOnConfigUpdate(MuxyGameLink GameLink, uint32_t Id);

	/*
		Broadcast functions
	*/
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SendBroadcast(MuxyGameLink GameLink, const char* Target, const char* JsonString);

	/*
		Datastream functions.

		The lifetime of MGL_Schema_DatastreamUpdate and derived MGL_Schema_DatastreamEvent end
		when the callback returns.
	*/
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SubscribeToDatastream(MuxyGameLink GameLink);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UnsubscribeFromDatastream(MuxyGameLink GameLink);
	MUXY_CLIB_API uint32_t MuxyGameLink_OnDatastream(MuxyGameLink GameLink, MGL_DatastreamUpdateCallback Callback, void* UserData);
	MUXY_CLIB_API void MuxyGameLink_DetachOnDatastream(MuxyGameLink GameLink, uint32_t OnDatastreamHandle);

	MUXY_CLIB_API uint32_t MuxyGameLink_Schema_DatastreamUpdate_GetEventCount(MGL_Schema_DatastreamUpdate DatastreamUpdate);
	MUXY_CLIB_API MGL_Schema_DatastreamEvent MuxyGameLink_Schema_DatastreamUpdate_GetEventAt(MGL_Schema_DatastreamUpdate DatastreamUpdate, uint32_t AtIndex);
	MUXY_CLIB_API int64_t MuxyGameLink_Schema_DatastreamEvent_GetTimestamp(MGL_Schema_DatastreamEvent DatastreamEvent);
	MUXY_CLIB_API MGL_String MuxyGameLink_Schema_DatastreamEvent_GetJson(MGL_Schema_DatastreamEvent DatastreamEvent);

	/*
		Purchase and SKU functions.

		The lifetime of a MGL_Schema_TransactionResponse ends when the callback returns.
	*/
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SubscribeToSKU(MuxyGameLink GameLink, const char* SKU);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UnsubscribeFromSKU(MuxyGameLink GameLink, const char* SKU);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SubscribeToAllPurchases(MuxyGameLink GameLink);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UnsubscribeFromAllPurchases(MuxyGameLink GameLink);
	MUXY_CLIB_API uint32_t MuxyGameLink_OnTransaction(MuxyGameLink GameLink, MGL_TransactionResponseCallback Callback, void* UserData);
	MUXY_CLIB_API void MuxyGameLink_DetachOnTransaction(MuxyGameLink GameLink, uint32_t id);

	MUXY_CLIB_API const char* MuxyGameLink_Schema_Transaction_GetId(MGL_Schema_TransactionResponse TPBResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_Transaction_GetSKU(MGL_Schema_TransactionResponse TPBResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_Transaction_GetDisplayName(MGL_Schema_TransactionResponse TPBResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_Transaction_GetUserId(MGL_Schema_TransactionResponse TPBResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_Transaction_GetUserName(MGL_Schema_TransactionResponse TPBResp);
	MUXY_CLIB_API int32_t MuxyGameLink_Schema_Transaction_GetCost(MGL_Schema_TransactionResponse TPBResp);
	MUXY_CLIB_API int64_t MuxyGameLink_Schema_Transaction_GetTimestamp(MGL_Schema_TransactionResponse TPBResp);
	MUXY_CLIB_API MGL_String MuxyGameLink_Schema_Transaction_GetJson(MGL_Schema_TransactionResponse TPBResp);

	MUXY_CLIB_API MGL_RequestId MuxyGameLink_GetOutstandingTransactions(MuxyGameLink GameLink, const char *SKU, MGL_GetOutstandingTransactionsResponseCallback Callback, void *UserData);
	MUXY_CLIB_API uint32_t MuxyGameLink_Schema_GetOutstandingTransactionsResponse_GetTransactionCount(MGL_Schema_GetOutstandingTransactionsResponse Resp);
	MUXY_CLIB_API MGL_Schema_TransactionResponse MuxyGameLink_Schema_GetOutstandingTransactionsResponse_GetTransactionAt(MGL_Schema_GetOutstandingTransactionsResponse Resp, uint32_t Index);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_RefundTransactionBySKU(MuxyGameLink GameLink, const char *SKU, const char *UserId);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_RefundTransactionByID(MuxyGameLink GameLink, const char *TxId, const char *UserId);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_ValidateTransaction(MuxyGameLink GameLink, const char *TxId, const char *Details);
	
	/*
		Polling functions
	*/
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_CreatePoll(MuxyGameLink GameLink, const char *PollId, const char *Prompt, const char **Options, uint32_t OptionsCount);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_SubscribeToPoll(MuxyGameLink GameLink, const char *PollId);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_UnsubscribeFromPoll(MuxyGameLink GameLink, const char *PollId);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_DeletePoll(MuxyGameLink GameLink, const char *PollId);
	MUXY_CLIB_API MGL_RequestId MuxyGameLink_GetPoll(MuxyGameLink GameLink, const char *PollId, MGL_GetPollResponseCallback Callback, void *UserData);
	MUXY_CLIB_API uint32_t MuxyGameLink_OnPollUpdate(MuxyGameLink GameLink, MGL_PollUpdateResponseCallback Callback, void *UserData);
	MUXY_CLIB_API void MuxyGameLink_DetachOnPollUpdate(MuxyGameLink GameLink, uint32_t Id);

	MUXY_CLIB_API const char* MuxyGameLink_Schema_GetPollResponse_GetPollId(MGL_Schema_GetPollResponse PResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_GetPollResponse_GetPrompt(MGL_Schema_GetPollResponse PResp);
	MUXY_CLIB_API uint32_t MuxyGameLink_Schema_GetPollResponse_GetOptionCount(MGL_Schema_GetPollResponse PResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_GetPollResponse_GetOptionAt(MGL_Schema_GetPollResponse PResp, uint32_t Index);
	MUXY_CLIB_API uint32_t MuxyGameLink_Schema_GetPollResponse_GetResultCount(MGL_Schema_GetPollResponse PResp);
	MUXY_CLIB_API int32_t MuxyGameLink_Schema_GetPollResponse_GetResultAt(MGL_Schema_GetPollResponse PResp, uint32_t Index);
	MUXY_CLIB_API double MuxyGameLink_Schema_GetPollResponse_GetMean(MGL_Schema_GetPollResponse PResp);
	MUXY_CLIB_API double MuxyGameLink_Schema_GetPollResponse_GetSum(MGL_Schema_GetPollResponse PResp);
	MUXY_CLIB_API int32_t MuxyGameLink_Schema_GetPollResponse_GetCount(MGL_Schema_GetPollResponse PResp);

	MUXY_CLIB_API const char* MuxyGameLink_Schema_PollUpdateResponse_GetPollId(MGL_Schema_PollUpdateResponse PResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_PollUpdateResponse_GetPrompt(MGL_Schema_PollUpdateResponse PResp);
	MUXY_CLIB_API uint32_t MuxyGameLink_Schema_PollUpdateResponse_GetOptionCount(MGL_Schema_PollUpdateResponse PResp);
	MUXY_CLIB_API const char* MuxyGameLink_Schema_PollUpdateResponse_GetOptionAt(MGL_Schema_PollUpdateResponse PResp, uint32_t Index);
	MUXY_CLIB_API uint32_t MuxyGameLink_Schema_PollUpdateResponse_GetResultCount(MGL_Schema_PollUpdateResponse PResp);
	MUXY_CLIB_API int32_t MuxyGameLink_Schema_PollUpdateResponse_GetResultAt(MGL_Schema_PollUpdateResponse PResp, uint32_t Index);
	MUXY_CLIB_API double MuxyGameLink_Schema_PollUpdateResponse_GetMean(MGL_Schema_PollUpdateResponse PResp);
	MUXY_CLIB_API double MuxyGameLink_Schema_PollUpdateResponse_GetSum(MGL_Schema_PollUpdateResponse PResp);
	MUXY_CLIB_API int32_t MuxyGameLink_Schema_PollUpdateResponse_GetCount(MGL_Schema_PollUpdateResponse PResp);

	/*
		PatchList functions
	*/

	MUXY_CLIB_API MGL_PatchList MuxyGameLink_PatchList_Make(void);
	MUXY_CLIB_API void MuxyGameLink_PatchList_Kill(MGL_PatchList PList);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithInteger(MGL_PatchList PList, MGL_Operation Operation, const char* Path, int64_t Val);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithDouble(MGL_PatchList PList, MGL_Operation Operation, const char* Path, double Val);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithBoolean(MGL_PatchList PList, MGL_Operation Operation, const char* Path, bool Val);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithString(MGL_PatchList PList, MGL_Operation Operation, const char* Path, const char* Val);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithLiteral(MGL_PatchList PList, MGL_Operation Operation, const char* Path, const char* Val);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithNull(MGL_PatchList PList, MGL_Operation Operation, const char* Path);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithJson(MGL_PatchList PList, MGL_Operation Operation, const char* Path, const char* Val);
	MUXY_CLIB_API void MuxyGameLink_PatchList_UpdateStateWithEmptyArray(MGL_PatchList PList, MGL_Operation Operation, const char* Path);
	MUXY_CLIB_API bool MuxyGameLink_PatchList_Empty(MGL_PatchList PList);
	MUXY_CLIB_API void MuxyGameLink_PatchList_Clear(MGL_PatchList PList);

#ifdef __cplusplus
}
#endif

#endif
