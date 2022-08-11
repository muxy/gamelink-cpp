#include "gamelink.h"
#include "gamelink_c.h"
#include "gamelink_c_interop.h"

using namespace gamelink;

MGL_RequestId MuxyGameLink_SetChannelConfig(MuxyGameLink GameLink, const char* JsonString)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	nlohmann::json Json = nlohmann::json::parse(JsonString, nullptr, false);
	if (!Json.is_discarded())
	{
		return Instance->SetChannelConfig(Json);
	}
	else
	{
		Instance->InvokeOnDebugMessage(gamelink::string("Couldn't parse config"));
		return gamelink::ANY_REQUEST_ID;
	}
}

MGL_RequestId MuxyGameLink_GetConfig(MuxyGameLink GameLink,
									 MGL_ConfigTarget Target,
									 MGL_ConfigResponseCallback Callback,
									 void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->GetConfig(static_cast<ConfigTarget>(Target), [Callback, UserData](const schema::GetConfigResponse& ConfigResponse) {
		MGL_Schema_ConfigResponse Response;
		Response.Obj = &ConfigResponse;

		Callback(UserData, Response);
	});
}

MGL_String MuxyGameLink_Schema_ConfigResponse_GetJson(MGL_Schema_ConfigResponse ConfigResponse)
{
	const schema::GetConfigResponse* Response = static_cast<const schema::GetConfigResponse*>(ConfigResponse.Obj);
	return MuxyGameLink_StrDup(Response->data.config.dump().c_str());
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithInteger(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, int64_t Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateChannelConfigWithInteger(static_cast<gamelink::Operation>(Operation), Path, Value);
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithDouble(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, double Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateChannelConfigWithDouble(static_cast<gamelink::Operation>(Operation), Path, Value);
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithString(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, const char* Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateChannelConfigWithString(static_cast<gamelink::Operation>(Operation), Path, gamelink::string(Value));
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithLiteral(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path, const char* Json)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateChannelConfigWithLiteral(static_cast<gamelink::Operation>(Operation), Path, gamelink::string(Json));
}

MGL_RequestId MuxyGameLink_UpdateChannelConfigWithNull(MuxyGameLink GameLink, MGL_Operation Operation, const char* Path)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateChannelConfigWithNull(static_cast<gamelink::Operation>(Operation), Path);
}

MGL_RequestId MuxyGameLink_SubscribeToConfigurationChanges(MuxyGameLink GameLink, MGL_ConfigTarget Target)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->SubscribeToConfigurationChanges(static_cast<ConfigTarget>(Target));
}

MGL_RequestId MuxyGameLink_UnsubscribeToConfigurationChanges(MuxyGameLink GameLink, MGL_ConfigTarget Target)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UnsubscribeFromConfigurationChanges(static_cast<ConfigTarget>(Target));
}

uint32_t MuxyGameLink_OnConfigUpdate(MuxyGameLink GameLink, MGL_ConfigUpdateResponseCallback Callback, void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->OnConfigUpdate().Add(C_CALLBACK(Callback, UserData, ConfigUpdateResponse));
}

uint32_t MuxyGameLink_OnConfigUpdateUnique(MuxyGameLink GameLink, const char* Name, MGL_ConfigUpdateResponseCallback Callback, void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->OnConfigUpdate().AddUnique(gamelink::string(Name), C_CALLBACK(Callback, UserData, ConfigUpdateResponse));
}

void MuxyGameLink_DetachOnConfigUpdate(MuxyGameLink GameLink, uint32_t Id)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	Instance->OnConfigUpdate().Remove(Id);
}

MGL_ConfigTarget MuxyGameLink_Schema_ConfigUpdateResponse_GetConfigID(MGL_Schema_ConfigUpdateResponse UpdateResponse)
{
	const schema::ConfigUpdateResponse* Response = static_cast<const schema::ConfigUpdateResponse*>(UpdateResponse.Obj);
	if (Response->data.topicId == "channel")
	{
		return MGL_CONFIG_TARGET_CHANNEL;
	}
	else if (Response->data.topicId == "extension")
	{
		return MGL_CONFIG_TARGET_EXTENSION;
	}

	// Should be unreachable.
	return MGL_CONFIG_TARGET_CHANNEL;
}

MGL_String MuxyGameLink_Schema_ConfigUpdateResponse_GetJson(MGL_Schema_ConfigUpdateResponse UpdateResponse)
{
	const schema::ConfigUpdateResponse* Response = static_cast<const schema::ConfigUpdateResponse*>(UpdateResponse.Obj);
	return MuxyGameLink_StrDup(Response->data.config.dump().c_str());
}
