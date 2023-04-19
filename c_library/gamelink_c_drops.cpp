#include "gamelink.h"
#include "gamelink_c.h"
#include "gamelink_c_interop.h"

MGL_RequestId MuxyGameLink_GetDrops(MuxyGameLink GameLink, const char* Status, MGL_DropsCallback Callback, void* User)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->GetDrops(Status, [Callback, User](const gamelink::schema::GetDropsResponse& Resp) {
		MGL_GetDropsResponse WDropResp;
		WDropResp.Obj = &Resp;
		Callback(User, WDropResp);
	});
}

MGL_RequestId MuxyGameLink_ValidateDrop(MuxyGameLink GameLink, const char* DropId) 
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->ValidateDrop(DropId);
}

MGL_Drop MuxyGameLink_GetDropsResponse_GetAt(MGL_GetDropsResponse DropResp, uint64_t index)
{
	const gamelink::schema::GetDropsResponse* Response = static_cast<const gamelink::schema::GetDropsResponse*>(DropResp.Obj);
	MGL_Drop Drop;
	Drop.Obj = &Response->data.drops[index];
	return Drop;
}

uint64_t MuxyGameLink_GetDropsResponse_GetCount(MGL_GetDropsResponse DropResp) 
{
	const gamelink::schema::GetDropsResponse* Response = static_cast<const gamelink::schema::GetDropsResponse*>(DropResp.Obj);
	return Response->data.drops.size();
}

const char* MuxyGameLink_Drop_GetId(MGL_Drop Drop) 
{
	const gamelink::schema::Drop* Response = static_cast<const gamelink::schema::Drop*>(Drop.Obj);
	return Response->id.c_str();
}

const char* MuxyGameLink_Drop_GetBenefitId(MGL_Drop Drop) 
{
	const gamelink::schema::Drop* Response = static_cast<const gamelink::schema::Drop*>(Drop.Obj);
	return Response->benefitId.c_str();
}

const char* MuxyGameLink_Drop_GetUserId(MGL_Drop Drop) 
{
	const gamelink::schema::Drop* Response = static_cast<const gamelink::schema::Drop*>(Drop.Obj);
	return Response->userId.c_str();
}

const char* MuxyGameLink_Drop_GetStatus(MGL_Drop Drop) 
{
	const gamelink::schema::Drop* Response = static_cast<const gamelink::schema::Drop*>(Drop.Obj);
	return Response->status.c_str();
}
const char* MuxyGameLink_Drop_GetService(MGL_Drop Drop)
{
	const gamelink::schema::Drop* Response = static_cast<const gamelink::schema::Drop*>(Drop.Obj);
	return Response->service.c_str();
}

const char* MuxyGameLink_Drop_GetUpdatedAt(MGL_Drop Drop)
{
	const gamelink::schema::Drop* Response = static_cast<const gamelink::schema::Drop*>(Drop.Obj);
	return Response->updatedAt.c_str();
}