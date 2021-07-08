#include "gamelink.h"
#include "gamelink_c.h"

struct MGL_GenericPointer { const void *Obj; };

MuxyGameLink MuxyGameLink_Make(void)
{
    MuxyGameLink GameLink;
	GameLink.SDK = new gamelink::SDK();

	return GameLink;
}

void MuxyGameLink_Kill(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    delete SDK;
}

// Payload begin

void MuxyGameLink_ForeachPayload(MuxyGameLink GameLink, void (*Callback)(void*, MGL_Payload), void* UserData)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    SDK->ForeachPayload([Callback, UserData](const gamelink::Payload* Payload)
	{
        MGL_Payload WPayload;
        WPayload.Obj = Payload;

        Callback(UserData, WPayload);
	});
}

uint32_t MuxyGameLink_Payload_GetSize(MGL_Payload Payload)
{
    const gamelink::Payload *MGLPayload = static_cast<const gamelink::Payload*>(Payload.Obj);
    return MGLPayload->data.size();
}

const char* MuxyGameLink_Payload_GetData(MGL_Payload Payload)
{
    const gamelink::Payload *MGLPayload = static_cast<const gamelink::Payload*>(Payload.Obj);
    return MGLPayload->data.c_str();
}

// Payload end


// Error begin
MGL_Error MuxyGameLink_Schema_GetFirstError(void *Resp, MGL_SCHEMA_RESPONSE_TYPE RespType)
{
    MGL_GenericPointer *GP = (MGL_GenericPointer*) Resp;
    const gamelink::schema::Error* MGLErr = NULL;

    switch (RespType)
    {            
        case MGL_SCHEMA_RESPONSE_AUTHENTICATE:
        {
            const gamelink::schema::AuthenticateResponse* Auth = static_cast<const gamelink::schema::AuthenticateResponse*>(GP->Obj);
            MGLErr = gamelink::FirstError(*Auth);
            break;
        }
        case MGL_SCHEMA_RESPONSE_TWITCHPURCHASEBITS:
        {
            const gamelink::schema::TwitchPurchaseBitsResponse<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponse<nlohmann::json>*>(GP->Obj);
            MGLErr = gamelink::FirstError(*TPB);
            break;
        }

        default: break;
    }

    MGL_Error WErr;
    WErr.Obj = MGLErr;

    return WErr;
}

bool MuxyGameLink_Error_IsValid(MGL_Error Error)
{
    return Error.Obj != nullptr;
}

uint32_t MuxyGameLink_Error_GetCode(MGL_Error Error)
{
    const gamelink::schema::Error *GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
    return GLError->number;
}
const char* MuxyGameLink_Error_GetTitle(MGL_Error Error)
{
    const gamelink::schema::Error *GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
    return GLError->title.c_str();
}
const char* MuxyGameLink_Error_GetDetail(MGL_Error Error)
{
    const gamelink::schema::Error *GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
    return GLError->detail.c_str();
}

// Error end

bool MuxyGameLink_ReceiveMessage(MuxyGameLink GameLink, const char* Bytes, uint32_t Length)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->ReceiveMessage(Bytes, Length);
}

MGL_RequestId MuxyGameLink_AuthenticateWithPIN(MuxyGameLink GameLink,
                                  const char* ClientId, const char* PIN,
								  void (*Callback)(void*, MGL_Schema_AuthenticateResponse),
								  void* UserData)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    MGL_RequestId res = SDK->AuthenticateWithPIN(ClientId, PIN, [Callback, UserData](const gamelink::schema::AuthenticateResponse& AuthResp)
    {
        MGL_Schema_AuthenticateResponse WAuthResp;
        WAuthResp.Obj = &AuthResp;

        Callback(UserData, WAuthResp);
    });

    return res;
}

MGL_RequestId MuxyGameLink_AuthenticateWithRefreshToken(MuxyGameLink GameLink, 
                                                        const char* ClientId, const char* RefreshToken,
								                        void (*Callback)(void*, MGL_Schema_AuthenticateResponse),
								                        void* UserData)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    MGL_RequestId res = SDK->AuthenticateWithRefreshToken(ClientId, RefreshToken, [Callback, UserData](const gamelink::schema::AuthenticateResponse& AuthResp)
    {
        MGL_Schema_AuthenticateResponse WAuthResp;
        WAuthResp.Obj = &AuthResp;

        Callback(UserData, WAuthResp);
    });

    return res;
}

bool MuxyGameLink_IsAuthenticated(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK); 
    return SDK->IsAuthenticated();
}

MGL_Schema_User MuxyGameLink_GetUser(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK); 
    MGL_Schema_User User;
    User.Obj = SDK->GetUser();

    return User;
}

const char* MuxyGameLink_Schema_User_GetJWT(MGL_Schema_User User)
{
    const gamelink::schema::User* MGLUser = static_cast<const gamelink::schema::User*>(User.Obj);
    if (MGLUser) 
    {
        return MGLUser->GetRefreshToken().c_str();
    }

    return "";
}

const char* MuxyGameLink_Schema_User_GetRefreshToken(MGL_Schema_User User)
{
    const gamelink::schema::User* MGLUser = static_cast<const gamelink::schema::User*>(User.Obj);
    if (MGLUser) 
    {
        return MGLUser->GetJWT().c_str();
    }
    
    return "";
}

MGL_RequestId MuxyGameLink_SetState(MuxyGameLink GameLink, const char *Target, const char *JsonString)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK); 
    nlohmann::json Json = nlohmann::json(JsonString);
    return SDK->SetState(Target, Json);
}

MGL_RequestId MuxyGameLink_SendBroadcast(MuxyGameLink GameLink, const char *Target, const char *JsonString)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK); 
    nlohmann::json Json = nlohmann::json(JsonString);
    return SDK->SendBroadcast(Target, Json);
}

MGL_RequestId MuxyGameLink_SubscribeToDatastream(MuxyGameLink GameLink)
{  
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK); 
    return SDK->SubscribeToDatastream();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromDatastream(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK); 
    return SDK->UnsubscribeFromDatastream();
}

uint32_t MuxyGameLink_OnDatastream(MuxyGameLink GameLink, void (*Callback)(void*, MGL_Schema_DatastreamUpdate), void* UserData)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);

	uint32_t res = SDK->OnDatastream([Callback, UserData](const gamelink::schema::DatastreamUpdate& DatastreamUpdate) 
    {
        MGL_Schema_DatastreamUpdate WDatastreamUpdate;
        WDatastreamUpdate.Obj = &DatastreamUpdate;

        Callback(UserData, WDatastreamUpdate);
    });

    return res;
}

void MuxyGameLink_DetachOnDatastream(MuxyGameLink GameLink, uint32_t OnDatastreamHandle)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    SDK->DetachOnDatastream(OnDatastreamHandle);
}

uint32_t MuxyGameLink_Schema_DatastreamUpdate_GetEventCount(MGL_Schema_DatastreamUpdate DatastreamUpdate)
{
    const gamelink::schema::DatastreamUpdateBody *DSU = static_cast<const gamelink::schema::DatastreamUpdateBody*>(DatastreamUpdate.Obj);
    return DSU->events.size();
}

MGL_Schema_DatastreamEvent MuxyGameLink_Schema_DatastreamUpdate_GetEventAt(MGL_Schema_DatastreamUpdate DatastreamUpdate, uint32_t AtIndex)
{
    const gamelink::schema::DatastreamUpdateBody *DSU = static_cast<const gamelink::schema::DatastreamUpdateBody*>(DatastreamUpdate.Obj);

    // Careful, if the vec resizes and the pointer changes, there will be old references to it. 
    // User should handle data in the callback, make copies / use the data / do whatever 
    MGL_Schema_DatastreamEvent WDSEvent;  
    WDSEvent.Obj = &DSU->events[AtIndex];

    return WDSEvent;
}

char* MuxyGameLink_Schema_DatastreamEvent_MakeJson(MGL_Schema_DatastreamEvent DatastreamEvent)
{
    const gamelink::schema::DatastreamEvent *DSE = static_cast<const gamelink::schema::DatastreamEvent*>(DatastreamEvent.Obj);
    char *EventJson = strdup(DSE->event.dump().c_str());

    return EventJson;
}

void MuxyGameLink_Schema_DatastreamEvent_KillJson(char* DatastreamEventJson)
{
    free(DatastreamEventJson);
}

int64_t	MuxyGameLink_Schema_DatastreamEvent_GetTimestamp(MGL_Schema_DatastreamEvent DatastreamEvent)
{
    const gamelink::schema::DatastreamEvent *DSE = static_cast<const gamelink::schema::DatastreamEvent*>(DatastreamEvent.Obj);
    return DSE->timestamp;
}

MGL_RequestId MuxyGameLink_SubscribeToSKU(MuxyGameLink GameLink, const char* SKU)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->SubscribeToSKU(SKU);
}

MGL_RequestId MuxyGameLink_UnsubscribeFromSKU(MuxyGameLink GameLink, const char* SKU)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->UnsubscribeFromSKU(SKU);
}

MGL_RequestId MuxyGameLink_SubscribeToAllPurchases(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->SubscribeToAllPurchases();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromAllPurchases(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->UnsubscribeFromAllPurchases();
}

uint32_t MuxyGameLink_OnTwitchPurchaseBits(MuxyGameLink GameLink, void (*Callback)(void*, MGL_Schema_TwitchPurchaseBitsResponse), void* UserData)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    uint32_t res = SDK->OnTwitchPurchaseBits([Callback, UserData](const gamelink::schema::TwitchPurchaseBitsResponse<nlohmann::json> &TPBResp)
    {
        MGL_Schema_TwitchPurchaseBitsResponse WTPBResp;
        WTPBResp.Obj = &TPBResp;

        Callback(UserData, WTPBResp);
    });

    return res;
}

void MuxyGameLink_DetachOnTwitchPurchaseBits(MuxyGameLink GameLink, uint32_t id)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    SDK->DetachOnTwitchPurchaseBits(id);
}

const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetId(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return TPB->id.c_str();
}

const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetSKU(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return TPB->sku.c_str();
}

const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetDisplayName(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return TPB->displayName.c_str();
}

const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetUserId(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return TPB->userId.c_str();
}

const char* MuxyGameLink_Schema_TwitchPurchaseBits_GetUserName(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return TPB->userName.c_str();
}

int32_t MuxyGameLink_Schema_TwitchPurchaseBits_GetCost(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return TPB->cost;
}

int64_t MuxyGameLink_Schema_TwitchPurchaseBits_GetTimestamp(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return TPB->timestamp;
}

char* MuxyGameLink_Schema_TwitchPurchaseBits_MakeAdditionalJson(MGL_Schema_TwitchPurchaseBitsResponse TPBResp)
{
    const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>* TPB = static_cast<const gamelink::schema::TwitchPurchaseBitsResponseBody<nlohmann::json>*>(TPBResp.Obj);
    return strdup(TPB->additional.dump().c_str());
}

void MuxyGameLink_Schema_TwitchPurchaseBits_KillAdditionalJson(char* Json)
{
    free(Json);
}