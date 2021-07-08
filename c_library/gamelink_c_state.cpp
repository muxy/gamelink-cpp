#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;

const char* STATE_TARGET_CHANNEL = schema::STATE_TARGET_CHANNEL;
const char* STATE_TARGET_EXTENSION = schema::STATE_TARGET_EXTENSION;

MGL_RequestId MuxyGameLink_SetState(MuxyGameLink GameLink, const char* Target, const char* JsonString)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	nlohmann::json Json = nlohmann::json(JsonString);
	return Instance->SetState(Target, Json);
}

MGL_RequestId MuxyGameLink_GetState(MuxyGameLink GameLink,
									const char* Target,
									void (*Callback)(void* UserData, MGL_Schema_StateResponse StateResp),
									void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->GetState(Target, [Callback, UserData](const schema::GetStateResponse<nlohmann::json>& StateResponse) {
		MGL_Schema_StateResponse Response;
		Response.Obj = &StateResponse;

		Callback(UserData, Response);
	});
}

char* MuxyGameLink_Schema_StateResponse_MakeJson(MGL_Schema_StateResponse Response)
{
	const schema::GetStateResponse<nlohmann::json>* StateResponse =
		static_cast<const schema::GetStateResponse<nlohmann::json>*>(Response.Obj);
	return strdup(StateResponse->data.state.dump().c_str());
}

void MuxyGameLink_Schema_StateResponse_KillJson(char* Json)
{
	free(Json);
}

MGL_RequestId
MuxyGameLink_UpdateStateWithInteger(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, int64_t Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithInteger(Target, Operation, Path, Value);
}

MGL_RequestId
MuxyGameLink_UpdateStateWithDouble(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, double Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithDouble(Target, Operation, Path, Value);
}

MGL_RequestId
MuxyGameLink_UpdateStateWithString(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, const char* Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithString(Target, Operation, Path, gamelink::string(Value));
}

MGL_RequestId
MuxyGameLink_UpdateStateWithLiteral(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, const char* Json)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithLiteral(Target, Operation, Path, gamelink::string(Json));
}

MGL_RequestId MuxyGameLink_UpdateStateWithNull(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithNull(Target, Operation, Path);
}

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

MGL_RequestId MuxyGameLink_OnStateUpdate(MuxyGameLink GameLink,
										 void (*Callback)(void* UserData, MGL_Schema_StateUpdateResponse UpdateResp),
										 void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->OnStateUpdate([Callback, UserData](const schema::SubscribeStateUpdateResponse<nlohmann::json>& UpdateResponse) {
		MGL_Schema_StateUpdateResponse Response;
		Response.Obj = &UpdateResponse;

		Callback(UserData, Response);
	});
}

const char* MuxyGameLink_Schema_StateUpdateResponse_GetTarget(MGL_Schema_StateUpdateResponse Response)
{
	const schema::SubscribeStateUpdateResponse<nlohmann::json>* UpdateResponse =
		static_cast<const schema::SubscribeStateUpdateResponse<nlohmann::json>*>(Response.Obj);
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

char* MuxyGameLink_Schema_StateUpdateResponse_MakeJson(MGL_Schema_StateUpdateResponse Response)
{
	const schema::SubscribeStateUpdateResponse<nlohmann::json>* UpdateResponse =
		static_cast<const schema::SubscribeStateUpdateResponse<nlohmann::json>*>(Response.Obj);
	return strdup(UpdateResponse->data.state.dump().c_str());
}

void MuxyGameLink_Schema_StateUpdateResponse_KillJson(char* Json)
{
	free(Json);
}