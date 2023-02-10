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

void MGW_SDK_SetActions(MGW_SDK Gateway, const MGW_Action* Begin, const MGW_Action* End)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	std::vector<gateway::Action> actions;
	while (Begin != End)
	{
		gateway::Action action;
		action.ID = gamelink::string(Begin->ID);
		action.Category = static_cast<gateway::ActionCategory>(Begin->Category);
		action.State = static_cast<gateway::ActionState>(Begin->State);
		action.Impact = Begin->Impact;
		action.Name = gamelink::string(Begin->Name);
		action.Description = gamelink::string(Begin->Description);
		action.Icon = gamelink::string(Begin->Icon);
		action.Count = Begin->Count;

		actions.emplace_back(std::move(action));

		Begin++;
	}

	if (!actions.empty())
	{
		SDK->SetActions(actions.data(), actions.data() + actions.size());
	}
}

void MGW_SDK_EnableAction(MGW_SDK Gateway, const char* id)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
	SDK->EnableAction(gamelink::string(id));
}

void MGW_SDK_DisableAction(MGW_SDK Gateway, const char* id)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
	SDK->DisableAction(gamelink::string(id));
}

void MGW_SDK_SetActionCount(MGW_SDK Gateway, const char* id, int count)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
	SDK->SetActionCount(gamelink::string(id), count);
}

void MGW_SDK_SetGameTexts(MGW_SDK Gateway, const MGW_GameTexts* Texts)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	gateway::GameTexts texts;
	for (size_t i = 0; i < Texts->TextsCount; ++i)
	{
		gateway::GameText entry;
		entry.Label = gateway::string(Texts->Texts[i].Label);
		entry.Value = gateway::string(Texts->Texts[i].Value);
		entry.Icon = gateway::string(Texts->Texts[i].Icon);

		texts.Texts.emplace_back(std::move(entry));
	}

	SDK->SetGameTexts(texts);
}

void MGW_SDK_OnBitsUsed(MGW_SDK Gateway, MGW_OnBitsUsedCallback Callback, void* User)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	SDK->OnBitsUsed([=](const gateway::BitsUsed& BitsUsed)
	{
		MGW_BitsUsed Used;
		Used.TransactionID = BitsUsed.TransactionID.c_str();
		Used.SKU = BitsUsed.SKU.c_str();
		Used.Bits = BitsUsed.Bits;

		Callback(User, &Used);
	});
}

void MGW_SDK_OnCoinsUsed(MGW_SDK Gateway, MGW_OnCoinsUsedCallback Callback, void* User)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	SDK->OnCoinsUsed([=](const gateway::CoinsUsed& BitsUsed)
	{
		MGW_CoinsUsed Used;
		Used.TransactionID = BitsUsed.TransactionID.c_str();
		Used.SKU = BitsUsed.SKU.c_str();
		Used.Cost = BitsUsed.Cost;

		Callback(User, &Used);
	});
}

void MGW_SDK_ValidateTransaction(MGW_SDK Gateway, MGW_CoinsUsed Coins, const char* Reason)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	gateway::CoinsUsed tx;
	tx.SKU = gateway::string(Coins.SKU);
	tx.TransactionID = gateway::string(Coins.TransactionID);
	tx.Cost = Coins.Cost;

	SDK->ValidateTransaction(tx, gateway::string(Reason));
}

void MGW_SDK_RefundTransaction(MGW_SDK Gateway, MGW_CoinsUsed Coins, const char* Reason)
{
	gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

	gateway::CoinsUsed tx;
	tx.SKU = gateway::string(Coins.SKU);
	tx.TransactionID = gateway::string(Coins.TransactionID);
	tx.Cost = Coins.Cost;

	SDK->RefundTransaction(tx, gateway::string(Reason));
}