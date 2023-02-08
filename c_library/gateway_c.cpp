#include <stdio.h>
#include <stdlib.h>
#include "gateway_c.h"
#include "gateway.h"

MGW_SDK MGW_MakeSDK(const char* GameID)
{
    MGW_SDK Gateway;
	Gateway.SDK = new gateway::SDK(gateway::string(GameID));
    return Gateway;
}

void MGW_KillSDK(MGW_SDK Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
	delete SDK;
}

MGW_RequestID MGW_SDK_AuthenticateWithPIN(MGW_SDK Gateway, const char *PIN, MGW_AuthenticateResponseCallback Callback, void *User)
{
    MGW_AuthenticateResponse Auth;
    
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

MGW_RequestID MGW_SDK_AuthenticateWithRefreshToken(MGW_SDK Gateway, const char *RefreshToken, MGW_AuthenticateResponseCallback Callback, void *User)
{
    MGW_AuthenticateResponse Auth;
    
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

bool MGW_SDK_IsAuthenticated(MGW_SDK Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->IsAuthenticated();
}

MGW_RequestID MGW_SDK_SetGameMetadata(MGW_SDK Gateway, const MGW_GameMetadata *Meta)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    gateway::GameMetadata MD;
    MD.GameName = Meta->GameName;
    MD.GameLogo = Meta->GameLogo;
    MD.Theme = Meta->Theme;
    return SDK->SetGameMetadata(MD);
}

bool MGW_SDK_ReceiveMessage(MGW_SDK Gateway, const char *Bytes, uint32_t Length)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->ReceiveMessage(Bytes, Length);
}

bool MGW_SDK_HasPayloads(MGW_SDK Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->HasPayloads();
}

typedef struct
{
    MGW_PayloadCallback Callback;
    void *User;
} MGW_ForeachPayloadUserdata;

static void ForeachPayloadCallback(void *User, const gateway::Payload *Payload)
{
	MGW_ForeachPayloadUserdata* data = static_cast<MGW_ForeachPayloadUserdata*>(User);

    MGW_Payload PL;
    PL.Bytes = Payload->GetData();
    PL.Length = Payload->GetLength();

    data->Callback(data->User, &PL);
}

void MGW_SDK_ForeachPayload(MGW_SDK Gateway, MGW_PayloadCallback Callback, void *User)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	MGW_ForeachPayloadUserdata data;
	data.Callback = Callback;
	data.User = User;

    SDK->ForeachPayload(ForeachPayloadCallback, &data);
}

const char* MGW_SDK_GetSandboxURL(MGW_SDK Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->GetSandboxURL().c_str();
}

const char* MGW_SDK_GetProductionURL(MGW_SDK Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->GetProductionURL().c_str();
}

void MGW_SDK_StartPoll(MGW_SDK Gateway, MGW_PollConfiguration cfg)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	gateway::PollConfiguration config;
	config.Duration = cfg.Duration;
	config.Location = static_cast<gateway::PollLocation>(cfg.Location);
	config.Mode = static_cast<gateway::PollMode>(cfg.Mode);
	config.Prompt = gamelink::string(cfg.Prompt);
	
	for (size_t i = 0; i < cfg.OptionsCount; ++i)
	{
		config.Options.push_back(gamelink::string(cfg.Options[i]));
	}

	config.OnUpdate = [=](const gateway::PollUpdate& update)
	{
		MGW_PollUpdate upd;
		upd.Winner = update.Winner;
		upd.WinningVoteCount = update.WinningVoteCount;
		upd.ResultCount = update.Results.size();
		
		upd.Results = nullptr;
		if (!update.Results.empty())
		{
			upd.Results = update.Results.data();
		}

		upd.Count = update.Count;
		upd.Mean = update.Mean;
		upd.IsFinal = update.IsFinal;

		cfg.OnUpdate(cfg.User, &upd);
	};

	SDK->StartPoll(config);
}

void MGW_SDK_StopPoll(MGW_SDK Gateway)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
	SDK->StopPoll();
}