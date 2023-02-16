#include <stdio.h>
#include <stdlib.h>
#include "gateway_c.h"
#include "gateway.h"
#include "gamelink_c.h"

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
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->AuthenticateWithPIN(PIN, [=](const gateway::AuthenticateResponse Resp)
    {
        MGW_AuthenticateResponse Auth;
        Auth.JWT          = Resp.JWT.c_str();
        Auth.RefreshToken = Resp.RefreshToken.c_str();
        Auth.TwitchName   = Resp.TwitchName.c_str();
        Auth.HasError     = Resp.HasError();
        Callback(User, &Auth);
    });
}

MGW_RequestID MGW_SDK_AuthenticateWithRefreshToken(MGW_SDK Gateway, const char *RefreshToken, MGW_AuthenticateResponseCallback Callback, void *User)
{
    
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return SDK->AuthenticateWithRefreshToken(RefreshToken, [=](const gateway::AuthenticateResponse Resp)
    {
        MGW_AuthenticateResponse Auth;
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

void MGW_SDK_HandleReconnect(MGW_SDK Gateway)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    SDK->HandleReconnect();
}

void MGW_SDK_OnDebugMessage(MGW_SDK Gateway, MGW_DebugMessageCallback Callback, void* UserData)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    SDK->OnDebugMessage([=](const gateway::string& msg)
    {
        Callback(UserData, msg.c_str());
    });
}

void MGW_SDK_DetachOnDebugMessage(MGW_SDK Gateway)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    SDK->DetachOnDebugMessage();
}

MGW_RequestID MGW_SDK_SetGameMetadata(MGW_SDK Gateway, MGW_GameMetadata Meta)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    gateway::GameMetadata MD;

    if (Meta.GameName)
    {
        MD.GameName = Meta.GameName;
    }

    if (Meta.GameLogo)
    {
        MD.GameLogo = Meta.GameLogo;
    }

    if (Meta.Theme)
    {
        MD.Theme = Meta.Theme;
    }

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

MGW_String MGW_SDK_GetSandboxURL(MGW_SDK Gateway)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return MuxyGameLink_StrDup(SDK->GetSandboxURL().c_str());
}

MGW_String MGW_SDK_GetProductionURL(MGW_SDK Gateway)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    return MuxyGameLink_StrDup(SDK->GetProductionURL().c_str());
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

void MGW_SDK_SetActions(MGW_SDK Gateway, const MGW_Action* Actions, uint64_t Count)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    
    if (!Actions || Count == 0)
    {
        return;
    }

    std::vector<gateway::Action> GatewayActions;
    for (uint64_t i = 0; i < Count; ++i)
    {
        gateway::Action Action;

        if (Actions[i].ID)
        {
            Action.ID = gamelink::string(Actions[i].ID);
        }

        if (Actions[i].Name)
        {
            Action.Name = gamelink::string(Actions[i].Name);
        }

        if (Actions[i].Description)
        {
            Action.Description = gamelink::string(Actions[i].Description);
        }

        if (Actions[i].Icon)
        {
            Action.Icon = gamelink::string(Actions[i].Icon);
        }

        Action.Category = static_cast<gateway::ActionCategory>(Actions[i].Category);
        Action.State = static_cast<gateway::ActionState>(Actions[i].State);
        Action.Impact = Actions[i].Impact;
        Action.Count = Actions[i].Count;

        GatewayActions.emplace_back(std::move(Action));
    }

    if (!GatewayActions.empty())
    {
        SDK->SetActions(GatewayActions.data(), GatewayActions.data() + GatewayActions.size());
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

void MGW_SDK_SetActionCount(MGW_SDK Gateway, const char* id, int32_t count)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    SDK->SetActionCount(gamelink::string(id), count);
}

void MGW_SDK_SetGameTexts(MGW_SDK Gateway, const MGW_GameText* Texts, uint64_t count)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);
    if (!Texts || count == 0)
    {
        return;
    }

    gateway::GameTexts texts;
    for (size_t i = 0; i < count; ++i)
    {
        gateway::GameText entry;
        if (Texts[i].Label)
        {
            entry.Label = gateway::string(Texts[i].Label);
        }

        if (Texts[i].Value)
        {
            entry.Value = gateway::string(Texts[i].Value);
        }

        if (Texts[i].Icon)
        {
            entry.Icon = gateway::string(Texts[i].Icon);
        }

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

void MGW_SDK_OnActionUsed(MGW_SDK Gateway, MGW_OnActionUsedCallback Callback, void* User)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

    SDK->OnActionUsed([=](const gateway::ActionUsed& BitsUsed)
    {
        MGW_ActionUsed Used;
        Used.TransactionID = BitsUsed.TransactionID.c_str();
        Used.SKU = BitsUsed.SKU.c_str();
        Used.Cost = BitsUsed.Cost;

        Callback(User, &Used);
    });
}

void MGW_SDK_ValidateTransaction(MGW_SDK Gateway, MGW_ActionUsed Coins, const char* Reason)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

    gateway::ActionUsed tx;
    tx.SKU = gateway::string(Coins.SKU);
    tx.TransactionID = gateway::string(Coins.TransactionID);
    tx.Cost = Coins.Cost;

    SDK->ValidateActionTransaction(tx, gateway::string(Reason));
}

void MGW_SDK_RefundTransaction(MGW_SDK Gateway, MGW_ActionUsed Coins, const char* Reason)
{
    gateway::SDK* SDK = static_cast<gateway::SDK*>(Gateway.SDK);

    gateway::ActionUsed tx;
    tx.SKU = gateway::string(Coins.SKU);
    tx.TransactionID = gateway::string(Coins.TransactionID);
    tx.Cost = Coins.Cost;

    SDK->RefundActionTransaction(tx, gateway::string(Reason));
}

void MGW_FreeString(MGW_String Str)
{
    free(Str);
}
