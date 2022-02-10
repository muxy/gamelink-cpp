#include "gamelink_c.h"
#include "gamelink.h"

char* MuxyGameLink_StrDup(const char *Str)
{
    char *Dst = NULL;
    if (Str)
    {
        Dst = (char*) malloc(strlen(Str) + 1);
		if (!Dst)
		{
			return nullptr;
		}

        strcpy(Dst, Str);
    }
    else
    {
        Dst = (char*) malloc(1);  
		if (!Dst)
		{
			return nullptr;
		}

        Dst[0] = '\0';
    }

    return Dst;
}

char* MuxyGameLink_ProjectionWebsocketConnectionURL(const char* clientID, MGL_ConnectionStage stage, const char* projection, int projectionMajor, int projectionMinor, int projectionPatch)
{
	gamelink::string URL = gamelink::detail::ProjectionWebsocketConnectionURL(
		gamelink::string(clientID),
		static_cast<gamelink::ConnectionStage>(stage), 
		gamelink::string(projection), 
		projectionMajor, projectionMinor, projectionPatch);

	return MuxyGameLink_StrDup(URL.c_str());
}


MuxyGameLink MuxyGameLink_Make(void)
{
	MuxyGameLink GameLink;
	GameLink.SDK = new gamelink::SDK();

	return GameLink;
}

void MuxyGameLink_Kill(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	delete SDK;
}

void MuxyGameLink_FreeString(MGL_String Str)
{
    free(Str);
}

uint32_t MuxyGameLink_Strlen(MGL_String Str)
{
	if (!Str)
	{
		return 0;
	}

	return static_cast<uint32_t>(strlen(Str));
}

void MuxyGameLink_OnDebugMessage(MuxyGameLink GameLink, MGL_OnDebugMessageCallback Callback, void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	SDK->OnDebugMessage([=](const gamelink::string& DebugMessage)
	{
		Callback(UserData, DebugMessage.c_str());
	});
}

void MuxyGameLink_DetachOnDebugMessage(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	SDK->DetachOnDebugMessage();
}

// Payload begin
void MuxyGameLink_ForeachPayload(MuxyGameLink GameLink, MGL_PayloadCallback Callback, void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	SDK->ForeachPayload([Callback, UserData](const gamelink::Payload* Payload) {
		MGL_Payload WPayload;
		WPayload.Obj = Payload;

		Callback(UserData, WPayload);
	});
}

uint32_t MuxyGameLink_Payload_GetSize(MGL_Payload Payload)
{
	const gamelink::Payload* MGLPayload = static_cast<const gamelink::Payload*>(Payload.Obj);
	return MGLPayload->data.size();
}

const char* MuxyGameLink_Payload_GetData(MGL_Payload Payload)
{
	const gamelink::Payload* MGLPayload = static_cast<const gamelink::Payload*>(Payload.Obj);
	return MGLPayload->data.c_str();
}
// Payload end

// Error begin
MGL_Error MuxyGameLink_Schema_GetFirstError(void* Resp)
{
	const gamelink::schema::ReceiveEnvelopeCommon* Common = static_cast<gamelink::schema::ReceiveEnvelopeCommon*>(Resp);
	const gamelink::schema::Error* MGLErr = gamelink::FirstError(*Common);
	
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
	const gamelink::schema::Error* GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
	return GLError->number;
}

const char* MuxyGameLink_Error_GetTitle(MGL_Error Error)
{
	const gamelink::schema::Error* GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
	return GLError->title.c_str();
}

const char* MuxyGameLink_Error_GetDetail(MGL_Error Error)
{
	const gamelink::schema::Error* GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
	return GLError->detail.c_str();
}

// Error end

bool MuxyGameLink_ReceiveMessage(MuxyGameLink GameLink, const char* Bytes, uint32_t Length)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->ReceiveMessage(Bytes, Length);
}

void MuxyGameLink_WaitForResponse(MuxyGameLink GameLink, MGL_RequestId Request)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->WaitForResponse(Request);
}

MGL_RequestId MuxyGameLink_AuthenticateWithPIN(MuxyGameLink GameLink,
											   const char* ClientId,
											   const char* PIN,
                                               MGL_AuthenticateResponseCallback Callback,
											   void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	MGL_RequestId res =
		SDK->AuthenticateWithPIN(ClientId, PIN, [Callback, UserData](const gamelink::schema::AuthenticateResponse& AuthResp) {
			MGL_Schema_AuthenticateResponse WAuthResp;
			WAuthResp.Obj = &AuthResp;

			Callback(UserData, WAuthResp);
		});

	return res;
}

MGL_RequestId MuxyGameLink_AuthenticateWithRefreshToken(MuxyGameLink GameLink,
														const char* ClientId,
														const char* RefreshToken,
                                                        MGL_AuthenticateResponseCallback Callback,
														void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	MGL_RequestId res = SDK->AuthenticateWithRefreshToken(ClientId, RefreshToken,
														  [Callback, UserData](const gamelink::schema::AuthenticateResponse& AuthResp) {
															  MGL_Schema_AuthenticateResponse WAuthResp;
															  WAuthResp.Obj = &AuthResp;

															  Callback(UserData, WAuthResp);
														  });

	return res;
}

bool MuxyGameLink_IsAuthenticated(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->IsAuthenticated();
}

MGL_Schema_User MuxyGameLink_GetSchemaUser(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
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

MGL_RequestId MuxyGameLink_SendBroadcast(MuxyGameLink GameLink, const char* Target, const char* JsonString)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	nlohmann::json Json = nlohmann::json::parse(JsonString, nullptr, false);
	if (!Json.is_discarded())
	{
		return SDK->SendBroadcast(Target, Json);
	}
	else
	{
		SDK->InvokeOnDebugMessage(gamelink::string("Couldn't parse broadcast"));
		return gamelink::ANY_REQUEST_ID;
	}
}

MGL_RequestId MuxyGameLink_SubscribeToDatastream(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->SubscribeToDatastream();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromDatastream(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->UnsubscribeFromDatastream();
}

uint32_t MuxyGameLink_OnDatastream(MuxyGameLink GameLink, MGL_DatastreamUpdateCallback Callback, void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);

	uint32_t res = SDK->OnDatastream([Callback, UserData](const gamelink::schema::DatastreamUpdate& DatastreamUpdate) {
		MGL_Schema_DatastreamUpdate WDatastreamUpdate;
		WDatastreamUpdate.Obj = &DatastreamUpdate;

		Callback(UserData, WDatastreamUpdate);
	});

	return res;
}

void MuxyGameLink_DetachOnDatastream(MuxyGameLink GameLink, uint32_t OnDatastreamHandle)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	SDK->DetachOnDatastream(OnDatastreamHandle);
}

uint32_t MuxyGameLink_Schema_DatastreamUpdate_GetEventCount(MGL_Schema_DatastreamUpdate DatastreamUpdate)
{
	const gamelink::schema::DatastreamUpdate* DSU = static_cast<const gamelink::schema::DatastreamUpdate*>(DatastreamUpdate.Obj);
	return DSU->data.events.size();
}

MGL_Schema_DatastreamEvent MuxyGameLink_Schema_DatastreamUpdate_GetEventAt(MGL_Schema_DatastreamUpdate DatastreamUpdate, uint32_t AtIndex)
{
	const gamelink::schema::DatastreamUpdate* DSU = static_cast<const gamelink::schema::DatastreamUpdate*>(DatastreamUpdate.Obj);

	MGL_Schema_DatastreamEvent WDSEvent;
	WDSEvent.Obj = &DSU->data.events[AtIndex];

	return WDSEvent;
}

MGL_String MuxyGameLink_Schema_DatastreamEvent_GetJson(MGL_Schema_DatastreamEvent DatastreamEvent)
{
    const gamelink::schema::DatastreamEvent *DSE = static_cast<const gamelink::schema::DatastreamEvent*>(DatastreamEvent.Obj);
    return MuxyGameLink_StrDup(DSE->event.dump().c_str());
}

int64_t	MuxyGameLink_Schema_DatastreamEvent_GetTimestamp(MGL_Schema_DatastreamEvent DatastreamEvent)
{
    const gamelink::schema::DatastreamEvent *DSE = static_cast<const gamelink::schema::DatastreamEvent*>(DatastreamEvent.Obj);
    return DSE->timestamp;
}
