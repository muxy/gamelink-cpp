#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;

const char * CONFIG_TARGET_CHANNEL = schema::CONFIG_TARGET_CHANNEL;
const char * CONFIG_TARGET_EXTENSION = schema::CONFIG_TARGET_EXTENSION;

MGL_RequestId MuxyGameLink_SetChannelConfig(MuxyGameLink GameLink, const char *JsonString)
{
    SDK * Instance = static_cast<SDK*>(GameLink.SDK); 
    nlohmann::json Json = nlohmann::json(JsonString);
    return Instance->SetChannelConfig(Json);
}

MGL_RequestId MuxyGameLink_GetConfig(MuxyGameLink GameLink, const char* Target, void (*Callback)(void* UserData, MGL_Schema_ConfigResponse StateResp), void* UserData)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->GetConfig(Target, [Callback, UserData](const schema::GetConfigResponse& ConfigResponse)
    {
        MGL_Schema_ConfigResponse Response;
        Response.Obj = &ConfigResponse;

        Callback(UserData, Response);
    });
}

const char * MuxyGameLink_Schema_ConfigResponse_GetConfigID(MGL_Schema_ConfigResponse ConfigResponse)
{
    const schema::GetConfigResponse * Response = static_cast<const schema::GetConfigResponse*>(ConfigResponse.Obj);
    if (Response->data.configId == "channel")
    {
        return CONFIG_TARGET_CHANNEL;
    }
    else if (Response->data.configId == "extension")
    {
        return CONFIG_TARGET_EXTENSION;
    }

    // Should be unreachable.
    return CONFIG_TARGET_CHANNEL;
}

char * MuxyGameLink_Schema_ConfigResponse_MakeJson(MGL_Schema_ConfigResponse ConfigResponse)
{
    const schema::GetConfigResponse * Response = static_cast<const schema::GetConfigResponse*>(ConfigResponse.Obj);
    return strdup(Response->data.config.dump().c_str());
}

void MuxyGameLink_Schema_ConfigResponse_KillJson(char * Json)
{
    free(Json);
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithInteger(MuxyGameLink GameLink, const char* Operation, const char* Path, int64_t Value)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->UpdateChannelConfigWithInteger(Operation, Path, Value);
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithDouble(MuxyGameLink GameLink, const char* Operation, const char* Path, double Value)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->UpdateChannelConfigWithDouble(Operation, Path, Value);
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithString(MuxyGameLink GameLink, const char* Operation, const char* Path, const char* Value)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->UpdateChannelConfigWithString(Operation, Path, gamelink::string(Value));
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithLiteral(MuxyGameLink GameLink, const char* Operation, const char* Path, const char* Json)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->UpdateChannelConfigWithLiteral(Operation, Path, gamelink::string(Json));
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithNull(MuxyGameLink GameLink, const char* Operation, const char* Path)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->UpdateChannelConfigWithNull(Operation, Path);
}

MGL_RequestId MuxyGameLink_SubscribeToConfigurationChanges(MuxyGameLink GameLink, const char* Target)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->SubscribeToConfigurationChanges(Target);
}

MGL_RequestId MuxyGameLink_UnsubscribeToConfigurationChanges(MuxyGameLink GameLink, const char* Target)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->UnsubscribeFromConfigurationChanges(Target);
}

MGL_RequestId MuxyGameLink_OnConfigUpdate(MuxyGameLink GameLink, void (*Callback)(void* UserData, MGL_Schema_ConfigUpdateResponse UpdateResp), void* UserData)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->OnConfigUpdate([Callback, UserData](const schema::ConfigUpdateResponse& UpdateResponse)
    {
        MGL_Schema_ConfigUpdateResponse Response;
        Response.Obj = &UpdateResponse;

        Callback(UserData, Response);
    });
}

const char * MuxyGameLink_Schema_ConfigUpdateResponse_GetConfigID(MGL_Schema_ConfigUpdateResponse UpdateResponse)
{
    const schema::ConfigUpdateResponse * Response = static_cast<const schema::ConfigUpdateResponse*>(UpdateResponse.Obj);
    if (Response->data.topicId == "channel")
    {
        return CONFIG_TARGET_CHANNEL;
    }
    else if (Response->data.topicId == "extension")
    {
        return CONFIG_TARGET_EXTENSION;
    }

    // Should be unreachable.
    return CONFIG_TARGET_CHANNEL;
}

char * MuxyGameLink_Schema_ConfigUpdateResponse_MakeJson(MGL_Schema_ConfigUpdateResponse UpdateResponse)
{
    const schema::ConfigUpdateResponse * Response = static_cast<const schema::ConfigUpdateResponse*>(UpdateResponse.Obj);
    return strdup(Response->data.config.dump().c_str());
}

void MuxyGameLink_Schema_ConfigUpdateResponse_KillJson(char * Json)
{
    free(Json);
}


/*
MGL_RequestId MuxyGameLink_SubscribeToStateUpdates(MuxyGameLink GameLink, const char* Target)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->SubscribeToStateUpdates(Target);
}

MGL_RequestId MuxyGameLink_UnsubscribeToStateUpdates(MuxyGameLink GameLink, const char* Target)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->UnsubscribeFromStateUpdates(Target);
}

MGL_RequestId MuxyGameLink_OnStateUpdate(MuxyGameLink GameLink, void (*Callback)(void* UserData, MGL_Schema_StateUpdateResponse UpdateResp), void* UserData)
{
    SDK* Instance = static_cast<SDK*>(GameLink.SDK);
    return Instance->OnStateUpdate([Callback, UserData](const schema::SubscribeStateUpdateResponse<nlohmann::json>& UpdateResponse)
    {
        MGL_Schema_StateUpdateResponse Response;
        Response.Obj = &UpdateResponse;

        Callback(UserData, Response);
    });
}

const char * MuxyGameLink_Schema_StateUpdateResponse_GetTarget(MGL_Schema_StateUpdateResponse Response)
{
    const schema::SubscribeStateUpdateResponse<nlohmann::json> * UpdateResponse = static_cast<const schema::SubscribeStateUpdateResponse<nlohmann::json>*>(Response.Obj);
    if (UpdateResponse->data.topic_id == "channel")
    {
        // Make sure to return the global one declared in the C header.
        return STATE_TARGET_CHANNEL;
    }
    else if (UpdateResponse->data.topic_id == "extension")
    {
        return STATE_TARGET_EXTENSION;
    }

    // Should be unreachable.
    return STATE_TARGET_CHANNEL;
}

char * MuxyGameLink_Schema_StateUpdateResponse_MakeJson(MGL_Schema_StateUpdateResponse Response)
{
    const schema::SubscribeStateUpdateResponse<nlohmann::json> * UpdateResponse = static_cast<const schema::SubscribeStateUpdateResponse<nlohmann::json>*>(Response.Obj);
    return strdup(UpdateResponse->data.state.dump().c_str());
}

void MuxyGameLink_Schema_StateUpdateResponse_KillJson(char * Json)
{
    free(Json);
}*/