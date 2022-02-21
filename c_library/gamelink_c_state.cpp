#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;

const char* STATE_TARGET_CHANNEL = schema::STATE_TARGET_CHANNEL;
const char* STATE_TARGET_EXTENSION = schema::STATE_TARGET_EXTENSION;

MGL_RequestId MuxyGameLink_SetState(MuxyGameLink GameLink, const char* Target, const char* JsonString)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	nlohmann::json Json = nlohmann::json::parse(JsonString, nullptr, false);
	if (!Json.is_discarded())
	{
		return Instance->SetState(Target, Json);
	}
	else
	{
		Instance->InvokeOnDebugMessage(gamelink::string("Couldn't parse state"));
		return gamelink::ANY_REQUEST_ID;
	}
}

MGL_RequestId MuxyGameLink_GetState(MuxyGameLink GameLink,
									const char* Target,
									MGL_StateResponseCallback Callback,
									void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->GetState(Target, [Callback, UserData](const schema::GetStateResponse<nlohmann::json>& StateResponse) {
		MGL_Schema_StateResponse Response;
		Response.Obj = &StateResponse;

		Callback(UserData, Response);
	});
}

MGL_String MuxyGameLink_Schema_StateResponse_GetJson(MGL_Schema_StateResponse Response)
{
	const schema::GetStateResponse<nlohmann::json>* StateResponse =
		static_cast<const schema::GetStateResponse<nlohmann::json>*>(Response.Obj);
	return MuxyGameLink_StrDup(StateResponse->data.state.dump().c_str());
}

MGL_RequestId MuxyGameLink_UpdateStateWithInteger(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, int64_t Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithInteger(Target, Operation, Path, Value);
}

MGL_RequestId MuxyGameLink_UpdateStateWithDouble(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, double Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithDouble(Target, Operation, Path, Value);
}

MGL_RequestId MuxyGameLink_UpdateStateWithString(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, const char* Value)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithString(Target, Operation, Path, gamelink::string(Value));
}

MGL_RequestId MuxyGameLink_UpdateStateWithLiteral(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path, const char* Json)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithLiteral(Target, Operation, Path, gamelink::string(Json));
}

MGL_RequestId MuxyGameLink_UpdateStateWithNull(MuxyGameLink GameLink, const char* Target, const char* Operation, const char* Path)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UpdateStateWithNull(Target, Operation, Path);
}

MGL_RequestId MuxyGameLink_UpdateStateWithPatchList(MuxyGameLink GameLink, const char* Target, const char* Operation, MGL_PatchList PList)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	return Instance->UpdateStateWithPatchList(Target, *Patch);
}

MGL_RequestId MuxyGameLink_SubscribeToStateUpdates(MuxyGameLink GameLink, const char* Target)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->SubscribeToStateUpdates(Target);
}

MGL_RequestId MuxyGameLink_UnsubscribeFromStateUpdates(MuxyGameLink GameLink, const char* Target)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UnsubscribeFromStateUpdates(Target);
}

uint32_t MuxyGameLink_OnStateUpdate(MuxyGameLink GameLink, MGL_StateUpdateResponseCallback Callback, void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->OnStateUpdate([Callback, UserData](const schema::SubscribeStateUpdateResponse<nlohmann::json>& UpdateResponse) {
		MGL_Schema_StateUpdateResponse Response;
		Response.Obj = &UpdateResponse;

		Callback(UserData, Response);
	});
}

void MuxyGameLink_DetachOnStateUpdate(MuxyGameLink GameLink, uint32_t Id)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	Instance->DetachOnStateUpdate(Id);
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

MGL_String MuxyGameLink_Schema_StateUpdateResponse_GetJson(MGL_Schema_StateUpdateResponse Response)
{
	const schema::SubscribeStateUpdateResponse<nlohmann::json>* UpdateResponse =
		static_cast<const schema::SubscribeStateUpdateResponse<nlohmann::json>*>(Response.Obj);
	return MuxyGameLink_StrDup(UpdateResponse->data.state.dump().c_str());
}


MGL_PatchList MuxyGameLink_PatchList_Make(MuxyGameLink GameLink) 
{
	MGL_PatchList PList;
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	PList.Obj = new gamelink::PatchList();
	return PList;
}
void MuxyGameLink_PatchList_Kill(MGL_PatchList PList) 
{
	delete PList.Obj;
}

void MuxyGameLink_PatchList_UpdateStateWithInteger(MGL_PatchList PList, const char* Operation, const char* Path, int64_t Val) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithInteger(Operation, Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithDouble(MGL_PatchList PList, const char* Operation, const char* Path, double Val) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithDouble(Operation, Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithBoolean(MGL_PatchList PList, const char* Operation, const char* Path, bool Val) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithDouble(Operation, Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithString(MGL_PatchList PList, const char* Operation, const char* Path, const char* Val) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithString(Operation, Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithLiteral(MGL_PatchList PList, const char* Operation, const char* Path, const char* Val) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithLiteral(Operation, Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithNull(MGL_PatchList PList, const char* Operation, const char* Path)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithNull(Operation, Path);
}

void MuxyGameLink_PatchList_UpdateStateWithJson(MGL_PatchList PList, const char* Operation, const char* Path, const char* Val) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithJson(Operation, Path, Val);
}

void MuxyGameLink_PatchList_UpdateStateWithEmptyArray(MGL_PatchList PList, const char* Operation, const char* Path) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->UpdateStateWithEmptyArray(Operation, Path);
}

bool MuxyGameLink_PatchList_Empty(MGL_PatchList PList)
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	return Patch->Empty();
}

void MuxyGameLink_PatchList_Clear(MGL_PatchList PList) 
{
	gamelink::PatchList* Patch = static_cast<gamelink::PatchList*>(PList.Obj);
	Patch->Clear();
}