#include "gamelink.h"
#include "gamelink_c.h"

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

bool MuxyGameLink_ReceiveMessage(MuxyGameLink GameLink, const char* Bytes, uint32_t Length)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->ReceiveMessage(Bytes, Length);
}

void MuxyGameLink_HandleReconnect(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->HandleReconnect();
}

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
	return MGLPayload->Length();
}

const char* MuxyGameLink_Payload_GetData(MGL_Payload Payload)
{
	const gamelink::Payload* MGLPayload = static_cast<const gamelink::Payload*>(Payload.Obj);
	return MGLPayload->Data();
}

void MuxyGameLink_WaitForResponse(MuxyGameLink GameLink, MGL_RequestId Request)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->WaitForResponse(Request);
}