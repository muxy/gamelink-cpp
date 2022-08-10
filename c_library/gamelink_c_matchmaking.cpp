#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;
MGL_RequestId MuxyGameLink_SubscribeToMatchmakingQueueInvite(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->SubscribeToMatchmakingQueueInvite();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromMatchmakingQueueInvite(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->UnsubscribeFromMatchmakingQueueInvite();
}

MGL_RequestId MuxyGameLink_ClearMatchmakingQueue(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->ClearMatchmakingQueue();
}

MGL_RequestId MuxyGameLink_RemoveMatchmakingEntry(MuxyGameLink GameLink, const char* Id)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->RemoveMatchmakingEntry(Id);
}

uint32_t MuxyGameLink_OnMatchmakingQueueInvite(MuxyGameLink GameLink, MGL_MatchmakingResponseCallback Callback, void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->OnMatchmakingQueueInvite().Add([Callback, UserData](const schema::MatchmakingUpdate& UpdateResponse) {
		MGL_Schema_MatchmakingUpdateResponse Response;
		Response.Obj = &UpdateResponse;
		Callback(UserData, Response);
	});
}

void MuxyGameLink_DetachOnMatchmakingQueueInvite(MuxyGameLink GameLink, uint32_t Id)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->OnMatchmakingQueueInvite().Remove(Id);
}

const char* MuxyGameLink_MatchmakingUpdate_GetTwitchUsername(MGL_Schema_MatchmakingUpdateResponse Resp) 
{
	const schema::MatchmakingUpdate* Response = static_cast<const schema::MatchmakingUpdate*>(Resp.Obj);
	return Response->data.twitchUsername.c_str();
}

const char* MuxyGameLink_MatchmakingUpdate_GetTwitchID(MGL_Schema_MatchmakingUpdateResponse Resp)
{
	const schema::MatchmakingUpdate* Response = static_cast<const schema::MatchmakingUpdate*>(Resp.Obj);
	return Response->data.twitchID.c_str();
}

int64_t MuxyGameLink_MatchmakingUpdate_GetTimestamp(MGL_Schema_MatchmakingUpdateResponse Resp) 
{
	const schema::MatchmakingUpdate* Response = static_cast<const schema::MatchmakingUpdate*>(Resp.Obj);
	return Response->data.timestamp;
}

bool MuxyGameLink_MatchmakingUpdate_IsFollower(MGL_Schema_MatchmakingUpdateResponse Resp)
{
	const schema::MatchmakingUpdate* Response = static_cast<const schema::MatchmakingUpdate*>(Resp.Obj);
	return Response->data.isFollower;
}

int MuxyGameLink_MatchmakingUpdate_GetSubscriptionTier(MGL_Schema_MatchmakingUpdateResponse Resp)
{
	const schema::MatchmakingUpdate* Response = static_cast<const schema::MatchmakingUpdate*>(Resp.Obj);
	return Response->data.subscriptionTier;
}

int MuxyGameLink_MatchmakingUpdate_GetBitsSpent(MGL_Schema_MatchmakingUpdateResponse Resp)
{
	const schema::MatchmakingUpdate* Response = static_cast<const schema::MatchmakingUpdate*>(Resp.Obj);
	return Response->data.bitsSpent;
}