#include "gamelink_c.h"
#include "gamelink.h"

#include "gamelink_c_interop.h"

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
	gamelink::string URL = gamelink::ProjectionWebsocketConnectionURL(
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

MGL_RequestId MuxyGameLink_AuthenticateWithGameIDAndPIN(MuxyGameLink GameLink,
											   const char* ClientId,
											   const char* GameId,
											   const char* PIN,
                                               MGL_AuthenticateResponseCallback Callback,
											   void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	if (!GameId)
	{
		MGL_RequestId res = SDK->AuthenticateWithPIN(ClientId, PIN, C_CALLBACK(Callback, UserData, AuthenticateResponse));
		return res;
	}
	else
	{
		MGL_RequestId res = SDK->AuthenticateWithGameIDAndPIN(ClientId, gamelink::string(GameId), PIN, C_CALLBACK(Callback, UserData, AuthenticateResponse));
		return res;
	}
}

MGL_RequestId MuxyGameLink_AuthenticateWithPIN(MuxyGameLink GameLink,
													const char* ClientId,
													const char* PIN,
													MGL_AuthenticateResponseCallback Callback,
													void* UserData)
{
	return MuxyGameLink_AuthenticateWithGameIDAndPIN(GameLink, ClientId, NULL, PIN, Callback, UserData);
}

MGL_RequestId MuxyGameLink_AuthenticateWithRefreshTokenAndGameID(MuxyGameLink GameLink,
														const char* ClientId,
														const char* GameId,
														const char* RefreshToken,
                                                        MGL_AuthenticateResponseCallback Callback,
														void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	if (GameId)
	{
		MGL_RequestId res = SDK->AuthenticateWithGameIDAndRefreshToken(ClientId, GameId, RefreshToken,
														  C_CALLBACK(Callback, UserData, AuthenticateResponse));
		return res;
	} 
	else
	{
		MGL_RequestId res = SDK->AuthenticateWithRefreshToken(ClientId, RefreshToken,
														  C_CALLBACK(Callback, UserData, AuthenticateResponse));
		return res;
	}
}

MGL_RequestId MuxyGameLink_AuthenticateWithRefreshToken(MuxyGameLink GameLink,
													const char* ClientId,
													const char* RefreshToken,
													MGL_AuthenticateResponseCallback Callback,
													void* UserData)
{
	return MuxyGameLink_AuthenticateWithRefreshTokenAndGameID(GameLink, ClientId, NULL, RefreshToken, Callback, UserData);
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
		return MGLUser->GetJWT().c_str();
	}

	return "";
}

const char* MuxyGameLink_Schema_User_GetRefreshToken(MGL_Schema_User User)
{
	const gamelink::schema::User* MGLUser = static_cast<const gamelink::schema::User*>(User.Obj);
	if (MGLUser)
	{
		return MGLUser->GetRefreshToken().c_str();
	}

	return "";
}

const char* MuxyGameLink_Schema_User_GetTwitchName(MGL_Schema_User User)
{
	const gamelink::schema::User* MGLUser = static_cast<const gamelink::schema::User*>(User.Obj);
	if (MGLUser)
	{
		return MGLUser->GetTwitchName().c_str();
	}

	return "";
}

MGL_RequestId MuxyGameLink_SendBroadcast(MuxyGameLink GameLink, const char* Topic, const char* JsonString)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	nlohmann::json Json = nlohmann::json::parse(JsonString, nullptr, false);
	if (!Json.is_discarded())
	{
		return SDK->SendBroadcast(Topic, Json);
	}
	else
	{
		SDK->InvokeOnDebugMessage(gamelink::string("Couldn't parse broadcast"));
		return gamelink::ANY_REQUEST_ID;
	}
}

MGL_RequestId MuxyGameLink_SetGameMetadata(MuxyGameLink GameLink, const MGL_GameMetadata* Meta)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	gamelink::GameMetadata CPPMeta;
	CPPMeta.game_name = Meta->GameName;
	CPPMeta.game_logo = Meta->GameLogo;
	CPPMeta.theme = Meta->Theme;

	return SDK->SetGameMetadata(CPPMeta);
}