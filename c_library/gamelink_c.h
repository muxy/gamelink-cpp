#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	typedef uint16_t MGL_RequestId;
	static const MGL_RequestId ANY_REQUEST_ID = 0xFFFF;

	struct MuxyGameLink
	{
		void* SDK;
	};
	// Any functions taking void* expect the actual struct pointer and not the Obj void* member, do not use .Obj anywhere
	struct MGL_Error
	{
		const void* Obj;
	};
	struct MGL_Payload
	{
		const void* Obj;
	};
	struct MGL_Schema_User
	{
		const void* Obj;
	};
	struct MGL_Schema_AuthenticateResponse
	{
		const void* Obj;
	};
	struct MGL_Schema_DatastreamUpdate
	{
		const void* Obj;
	};
	struct MGL_Schema_DatastreamEvent
	{
		const void* Obj;
	};
	struct MGL_Schema_TwitchPurchaseBitsResponse
	{
		const void* Obj;
	};

	typedef enum
	{
		MGL_SCHEMA_RESPONSE_AUTHENTICATE,
		MGL_SCHEMA_RESPONSE_TWITCHPURCHASEBITS,
	} MGL_SCHEMA_RESPONSE_TYPE;

	MuxyGameLink MuxyGameLink_Make(void);
	void MuxyGameLink_Kill(MuxyGameLink GameLink);

	void MuxyGameLink_ForeachPayload(MuxyGameLink GameLink, void (*Callback)(void* UserData, MGL_Payload Payload), void* UserData);
	size_t MuxyGameLink_Payload_GetSize(MGL_Payload Payload);
	const char* MuxyGameLink_Payload_GetData(MGL_Payload Payload);

	// GetFirstError may not need MGL_SCHEMA_RESPONSE_TYPE in later versions of the library, as we will internally tag structs with the
	// RespType so the burden isn't on the caller
	MGL_Error MuxyGameLink_Schema_GetFirstError(void* Resp, MGL_SCHEMA_RESPONSE_TYPE RespType);
	bool MuxyGameLink_Error_IsValid(MGL_Error Error);
	uint32_t MuxyGameLink_Error_GetCode(MGL_Error Error);
	const char* MuxyGameLink_Error_GetTitle(MGL_Error Error);
	const char* MuxyGameLink_Error_GetDetail(MGL_Error Error);

	bool MuxyGameLink_ReceiveMessage(MuxyGameLink GameLink, const char* Bytes, uint32_t Length);

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

	bool MuxyGameLink_IsAuthenticated(MuxyGameLink GameLink);

	MGL_Schema_User MuxyGameLink_GetUser(MuxyGameLink GameLink);
	const char* MuxyGameLink_Schema_User_GetJWT(MGL_Schema_User User);
	const char* MuxyGameLink_Schema_User_GetRefreshToken(MGL_Schema_User User);

	MGL_RequestId MuxyGameLink_SetState(MuxyGameLink GameLink, const char* Target, const char* JsonString);
	MGL_RequestId MuxyGameLink_SendBroadcast(MuxyGameLink GameLink, const char* Target, const char* JsonString);

	MGL_RequestId MuxyGameLink_SubscribeToDatastream(MuxyGameLink GameLink);
	MGL_RequestId MuxyGameLink_UnsubscribeFromDatastream(MuxyGameLink GameLink);
	// Do not store MGL_Schema_DatastreamEvent's or MGL_Schema_DatastreamUpdate's anywhere, they may be invalidated at the moment the
	// callback finishes.
	uint32_t MuxyGameLink_OnDatastream(MuxyGameLink GameLink,
									   void (*Callback)(void* UserData, MGL_Schema_DatastreamUpdate DatastreamUpdate),
									   void* UserData);
	void MuxyGameLink_DetachOnDatastream(MuxyGameLink GameLink, uint32_t OnDatastreamHandle);

	size_t MuxyGameLink_Schema_DatastreamUpdate_GetEventCount(MGL_Schema_DatastreamUpdate DatastreamUpdate);
	MGL_Schema_DatastreamEvent MuxyGameLink_Schema_DatastreamUpdate_GetEventAt(MGL_Schema_DatastreamUpdate DatastreamUpdate,
																			   size_t AtIndex);

	const char* MuxyGameLink_Schema_DatastreamEvent_ToJson(MGL_Schema_DatastreamEvent DatastreamEvent);
	int64_t MuxyGameLink_Schema_DatastreamEvent_GetTimestamp(MGL_Schema_DatastreamEvent DatastreamEvent);

	MGL_RequestId MuxyGameLink_SubscribeToSKU(MuxyGameLink GameLink, const char* SKU);
	MGL_RequestId MuxyGameLink_UnsubscribeFromSKU(MuxyGameLink GameLink, const char* SKU);
	MGL_RequestId MuxyGameLink_SubscribeToAllPurchases(MuxyGameLink GameLink);
	MGL_RequestId MuxyGameLink_UnsubscribeFromAllPurchases(MuxyGameLink GameLink);
	uint32_t MuxyGameLink_OnTwitchPurchaseBits(MuxyGameLink GameLink,
											   void (*Callback)(void* UserData, MGL_Schema_TwitchPurchaseBitsResponse TPBResp),
											   void* UserData);
	void MuxyGameLink_DetachOnTwitchPurchaseBits(MuxyGameLink GameLink, uint32_t id);

	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetId(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetSKU(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetDisplayName(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetUserId(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetUserName(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	int32_t MuxyGameLink_Schema_TwitchPurchaseBits_GetCost(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	int64_t MuxyGameLink_Schema_TwitchPurchaseBits_GetTimestamp(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);
	const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetAdditionalJson(MGL_Schema_TwitchPurchaseBitsResponse TPBResp);

#ifdef __cplusplus
}
#endif
