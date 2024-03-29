#include "gamelink_c.h"
#include "gamelink.h"

MGL_Error MuxyGameLink_GetFirstError(void* Resp)
{
	const gamelink::schema::ReceiveEnvelopeCommon* Common = static_cast<gamelink::schema::ReceiveEnvelopeCommon*>(Resp);
	const gamelink::schema::Error* MGLErr = gamelink::FirstError(*Common);

	MGL_Error WErr;
	WErr.Obj = MGLErr;

	return WErr;
}

uint32_t MuxyGameLink_Error_IsValid(MGL_Error Error)
{
	return Error.Obj != nullptr ? 1 : 0;
}

uint32_t MuxyGameLink_Error_GetCode(MGL_Error Error)
{
	const gamelink::schema::Error* GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
	return GLError->number;
}

const char* MuxyGameLink_Error_GetTitle(MGL_Error Error)
{
	const gamelink::schema::Error* GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
	return GLError->title.c_str();
}

const char* MuxyGameLink_Error_GetDetail(MGL_Error Error)
{
	const gamelink::schema::Error* GLError = static_cast<const gamelink::schema::Error*>(Error.Obj);
	return GLError->detail.c_str();
}
