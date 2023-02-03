#include <stdio.h>
#include <stdlib.h>
#include "gateway_c.h"
#include "gateway.h"


MuxyGateway MuxyGateway_Make(void)
{
    MuxyGateway Gateway;
	Gateway.SDK = new gateway::SDK();
    return Gateway;
}

void MuxyGateway_Kill(MuxyGateway Gateway)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(Gateway.SDK);
	delete SDK;
}

GW_RequestID MuxyGateway_AuthenticateWithPIN(MuxyGateway Gateway, const char *PIN, GW_AuthenticateResponseCallback Callback, void *User)
{
    GW_AuthenticateResponse Auth;
    
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->AuthenticateWithPIN(PIN, [&](const gateway::AuthenticateResponse Resp)
    {
        Auth.JWT          = Resp.JWT.c_str();
        Auth.RefreshToken = Resp.RefreshToken.c_str();
        Auth.TwitchName   = Resp.TwitchName.c_str();
        Auth.HasError     = Resp.HasError();
        Callback(User, &Auth);
    });
}

GW_RequestID MuxyGateway_AuthenticateWithRefreshToken(MuxyGateway Gateway, const char *RefreshToken, GW_AuthenticateResponseCallback Callback, void *User)
{
    GW_AuthenticateResponse Auth;
    
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->AuthenticateWithRefreshToken(RefreshToken, [&](const gateway::AuthenticateResponse Resp)
    {
        Auth.JWT          = Resp.JWT.c_str();
        Auth.RefreshToken = Resp.RefreshToken.c_str();
        Auth.TwitchName   = Resp.TwitchName.c_str();
        Auth.HasError     = Resp.HasError();
        Callback(User, &Auth);
    });
}

bool MuxyGateway_IsAuthenticated(MuxyGateway Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->IsAuthenticated();
}

GW_RequestID MuxyGateway_SetGameMetadata(MuxyGateway Gateway, const GW_GameMetadata *Meta)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    gateway::GameMetadata MD;
    MD.GameName = Meta->GameName;
    MD.GameLogo = Meta->GameLogo;
    MD.Theme = Meta->Theme;
    return SDK->SetGameMetadata(MD);
}

bool MuxyGateway_ReceiveMessage(MuxyGateway Gateway, const char *Bytes, uint32_t Length)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->ReceiveMessage(Bytes, Length);
}

bool MuxyGateway_HasPayloads(MuxyGateway Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->HasPayloads();
}

typedef struct
{
    GW_PayloadCallback Callback;
    void *User;
} GW_PayloadUserdata;

static void Gateway_ForeachPayload(void *User, const gateway::Payload *Payload)
{
    GW_PayloadUserdata *UD = (GW_PayloadUserdata*) User;

    GW_Payload PL;
    PL.Bytes = Payload->GetData();
    PL.Length = Payload->GetLength();

    UD->Callback(UD->User, &PL);
}

void MuxyGateway_ForeachPayload(MuxyGateway Gateway, GW_PayloadCallback Callback, void *User)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    SDK->ForeachPayload(Gateway_ForeachPayload, NULL);
}

const char* MuxyGateway_GetSandboxURL(MuxyGateway Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->GetSandboxURL().c_str();
}

const char* MuxyGateway_GetProductionURL(MuxyGateway Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->GetProductionURL().c_str();
}
