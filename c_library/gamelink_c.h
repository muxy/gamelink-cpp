#pragma once
#ifndef MUXY_GAMELINK_C_H
#define MUXY_GAMELINK_C_H
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/*
		x_Make always requires a x_Kill call or you will have memory leaks!
		x_Init does not require a x_Kill call
		Example: MuxyGameLink_Schema_DatastreamEvent_MakeJson requires a call to MuxyGameLink_Schema_DatastreamEvent_KillJson
		If it were named MuxyGameLink_Schema_DatastreamEvent_InitJson it wouldn't require a kill call
	*/

	typedef uint16_t MGL_RequestId;
	static const MGL_RequestId ANY_REQUEST_ID = 0xFFFF;

	enum MGL_SCHEMA_RESPONSE_TYPE
	{
		MGL_SCHEMA_RESPONSE_AUTHENTICATE,
		MGL_SCHEMA_RESPONSE_TWITCHPURCHASEBITS,

		MGL_SCHEMA_ENSURE_32 = 0xFFFFFFFFu
	};

	typedef struct
	{
		void* SDK;
	} MuxyGameLink;
	
	/* 
		Any functions taking void* expect the actual struct pointer and not the Obj void* member, do not use .Obj anywhere.
		
		Do not store these structsm, their lifetimes are generally bound by the SDK lifetime or 
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
	} MGL_Schema_TwitchPurchaseBitsResponse;

	/*
		SDK Initialization functions
	*/
	MuxyGameLink MuxyGameLink_Make(void);
	void MuxyGameLink_Kill(MuxyGameLink GameLink);

	/*
		SDK network send and receive functions
	*/
	void MuxyGameLink_ForeachPayload(MuxyGameLink GameLink, void (*Callback)(void* UserData, MGL_Payload Payload), void* UserData);
	uint32_t MuxyGameLink_Payload_GetSize(MGL_Payload Payload);
	const char* MuxyGameLink_Payload_GetData(MGL_Payload Payload);
	bool MuxyGameLink_ReceiveMessage(MuxyGameLink GameLink, const char* Bytes, uint32_t Length);
	void MuxyGameLink_WaitForResponse(MuxyGameLink GameLink, MGL_RequestId Request);

	/* 
		SDK Error manipulation functions
		
		GetFirstError may not need MGL_SCHEMA_RESPONSE_TYPE in later versions of the library, as we will internally tag structs with the
		RespType so the burden isn't on the caller.
	*/
	MGL_Error MuxyGameLink_Schema_GetFirstError(void* Resp, MGL_SCHEMA_RESPONSE_TYPE RespType);
	bool MuxyGameLink_Error_IsValid(MGL_Error Error);
	uint32_t MuxyGameLink_Error_GetCode(MGL_Error Error);
	const char* MuxyGameLink_Error_GetTitle(MGL_Error Error);
	const char* MuxyGameLink_Error_GetDetail(MGL_Error Error);

	/*
		Authentication functions.

		The lifetime of MGL_Schema_AuthenticateResponse in the callback ends 
		as soon as the callback finishes executing. Copying this parameter
		is not supported.
	*/
	MGL_RequestId MuxyGameLink_AuthenticateWithPIN(MuxyGameLink GameLink,
												   const char* ClientId,
												   const char* PIN,
												   void (*Callback)(void* UserData, MGL_Schema_AuthenticateResponse AuthResp),
												   void* UserData);

	MGL_RequestId MuxyGameLink_AuthenticateWithRefreshToken(MuxyGameLink GameLink,
															const char* ClientId,
															const char* RefreshToken,
															void (*Callback)(void* UserData, MGL_Schema_AuthenticateResponse AuthResp),
															void* UserData);

	/*
		Authentication checking.
		The MGL_Schema_User object's lifetime is the same as the MuxyGameLink lifetime.
	*/
	bool MuxyGameLink_IsAuthenticated(MuxyGameLink GameLink);
	MGL_Schema_User MuxyGameLink_GetUser(MuxyGameLink GameLink);
	const char* MuxyGameLink_Schema_User_GetJWT(MGL_Schema_User User);
	const char* MuxyGameLink_Schema_User_GetRefreshToken(MGL_Schema_User User);

	/* 
		State functions
	*/
	extern const char * STATE_TARGET_CHANNEL;
	extern const char * STATE_TARGET_EXTENSION;

	MGL_RequestId MuxyGameLink_SetState(MuxyGameLink GameLink, const char* Target, const char* JsonString);
	MGL_RequestId MuxyGameLink_GetState(MuxyGameLink GameLink, const char* Target, void (*Callback)(void* UserData, MGL_Schema_StateResponse StateResp), void* UserData);

	char * MuxyGameLink_Schema_StateResponse_MakeJson(MGL_Schema_StateResponse);
	void MuxyGameLink_Schema_StateResponse_KillJson(char *);

	MGL_RequestId MuxyGameLink_UpdateStateWithInteger(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, int64_t Value);
	MGL_RequestId MuxyGameLink_UpdateStateWithDouble(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, double Value);
	MGL_RequestId MuxyGameLink_UpdateStateWithString(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, const char* Value);
	MGL_RequestId MuxyGameLink_UpdateStateWithLiteral(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, const char* Json);
	MGL_RequestId MuxyGameLink_UpdateStateWithNull(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path);

	MGL_RequestId MuxyGameLink_SubscribeToStateUpdates(MuxyGameLink GameLink, const char* Target);
	MGL_RequestId MuxyGameLink_UnsubscribeToStateUpdates(MuxyGameLink GameLink, const char* Target);

	MGL_RequestId MuxyGameLink_OnStateUpdate(MuxyGameLink GameLink, void (*Callback)(void* UserData, MGL_Schema_StateUpdateResponse UpdateResp), void* UserData);
	const char * MuxyGameLink_Schema_StateUpdateResponse_GetTarget(MGL_Schema_StateUpdateResponse Response);
	char * MuxyGameLink_Schema_StateUpdateResponse_MakeJson(MGL_Schema_StateUpdateResponse Response);
	void MuxyGameLink_Schema_StateUpdateResponse_KillJson(char *);

	/*
		Config functions
	*/
	extern const char* CONFIG_TARGET_CHANNEL;
	extern const char* CONFIG_TARGET_EXTENSION;

	MGL_RequestId MuxyGameLink_SetChannelConfig(MuxyGameLink GameLink, const char* JsonString);
	MGL_RequestId MuxyGameLink_GetConfig(MuxyGameLink GameLink, const char* Target, void (*Callback)(void* UserData, MGL_Schema_ConfigResponse StateResp));

	const char * MuxyGameLink_Schema_ConfigResponse_GetConfigID(MGL_Schema_ConfigResponse);
	char * MuxyGameLink_Schema_ConfigResponse_MakeJson(MGL_Schema_ConfigResponse);
	void MuxyGameLink_Schema_ConfigResponse_KillJson(char *);

	MGL_RequestId MuxyGameLink_UpdateChannelConfigWithInteger(MuxyGameLink GameLink, const char* Operation, const char* Path, int64_t Value);
	MGL_RequestId MuxyGameLink_UpdateChannelConfigWithDouble(MuxyGameLink GameLink, const char* Operation, const char* Path, double Value);
	MGL_RequestId MuxyGameLink_UpdateChannelConfigWithString(MuxyGameLink GameLink, const char* Operation, const char* Path, const char* Value);
	MGL_RequestId MuxyGameLink_UpdateChannelConfigWithLiteral(MuxyGameLink GameLink, const char* Operation, const char* Path, const char* Json);
	MGL_RequestId MuxyGameLink_UpdateChannelConfigWithNull(MuxyGameLink GameLink, const char* Operation, const char* Path);

	MGL_RequestId MuxyGameLink_SubscribeToConfigurationChanges(MuxyGameLink GameLink, const char* Target);
	MGL_RequestId MuxyGameLink_UnsubscribeToConfigurationChanges(MuxyGameLink GameLink, const char* Target);
	MGL_RequestId MuxyGameLink_OnConfigUpdate(MuxyGameLink GameLink, void (*Callback)(void* UserData, MGL_Schema_ConfigUpdateResponse UpdateResp), void* UserData);

	const char * MuxyGameLink_Schema_ConfigUpdateResponse_GetConfigID(MGL_Schema_ConfigUpdateResponse);
	char * MuxyGameLink_Schema_ConfigUpdateResponse_MakeJson(MGL_Schema_ConfigUpdateResponse);
	void MuxyGameLink_Schema_ConfigUpdateResponse_KillJson(char *);

	/* 
		Broadcast functions
	*/
	MGL_RequestId MuxyGameLink_SendBroadcast(MuxyGameLink GameLink, const char* Target, const char* JsonString);

	/*
		Datastream functions.

		The lifetime of MGL_Schema_DatastreamUpdate and derived MGL_Schema_DatastreamEvent end
		when the callback returns.
	*/
	MGL_RequestId MuxyGameLink_SubscribeToDatastream(MuxyGameLink GameLink);
	MGL_RequestId MuxyGameLink_UnsubscribeFromDatastream(MuxyGameLink GameLink);
	uint32_t MuxyGameLink_OnDatastream(MuxyGameLink GameLink,
									   void (*Callback)(void* UserData, MGL_Schema_DatastreamUpdate DatastreamUpdate),
									   void* UserData);
	void MuxyGameLink_DetachOnDatastream(MuxyGameLink GameLink, uint32_t OnDatastreamHandle);

	uint32_t MuxyGameLink_Schema_DatastreamUpdate_GetEventCount(MGL_Schema_DatastreamUpdate DatastreamUpdate);
	MGL_Schema_DatastreamEvent MuxyGameLink_Schema_DatastreamUpdate_GetEventAt(MGL_Schema_DatastreamUpdate DatastreamUpdate,
																			   uint32_t AtIndex);
	int64_t MuxyGameLink_Schema_DatastreamEvent_GetTimestamp(MGL_Schema_DatastreamEvent DatastreamEvent);
	char* MuxyGameLink_Schema_DatastreamEvent_MakeJson(MGL_Schema_DatastreamEvent DatastreamEvent);
	void MuxyGameLink_Schema_DatastreamEvent_KillJson(char* DatastreamEventJson);

	/*
		Purchase and SKU functions.
		
		The lifetime of a MGL_Schema_TwitchPurchaseBitsResponse ends when the callback returns.
	*/
	MGL_RequestId MuxyGameLink_SubscribeToSKU(MuxyGameLink GameLink, const char* SKU);
	MGL_RequestId MuxyGameLink_UnsubscribeFromSKU(MuxyGameLink GameLink, const char* SKU);
	MGL_RequestId MuxyGameLink_SubscribeToAllPurchases(MuxyGameLink GameLink);
	MGL_RequestId MuxyGameLink_UnsubscribeFromAllPurchases(MuxyGameLink GameLink);
	uint32_t MuxyGameLink_OnTwitchPurchaseBits(MuxyGameLink GameLink,
											   void (*Callback)(void* UserData, MGL_Schema_TwitchPurchaseBitsResponse TPBResp),
											   void* UserData);
	void MuxyGameLink_DetachOnTwitchPurchaseBits(MuxyGameLink GameLink, uint32_t id);

	/*
		These const char* strings have the same lifetime as the MGL_Schema_TwitchPurchaseBitsResponse.
	*/
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetId(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetSKU(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetDisplayName(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetUserId(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetUserName(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	int32_t MuxyGameLink_Schema_TwitchPurchaseBits_GetCost(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	int64_t MuxyGameLink_Schema_TwitchPurchaseBits_GetTimestamp(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);

	char* MuxyGameLink_Schema_TwitchPurchaseBits_MakeAdditionalJson(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	void MuxyGameLink_Schema_TwitchPurchaseBits_KillAdditionalJson(char* TwitchPurchaseBitsJson);

#ifdef __cplusplus
}
#endif

#endif
