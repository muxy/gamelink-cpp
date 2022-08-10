#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;
MGL_RequestId MuxyGameLink_SubscribeToMatchmakingQueueInvite(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<SDK*>(GameLink.SDK);
	return SDK->SubscribeToMatchmakingQueueInvite();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromMatchmakingQueueInvite(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<SDK*>(GameLink.SDK);
	return SDK->UnsubscribeFromMatchmakingQueueInvite();
}

MGL_RequestId MuxyGameLink_ClearMatchmakingQueue(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<SDK*>(GameLink.SDK);
	return SDK->ClearMatchmakingQueue();
}

MGL_RequestId MuxyGameLink_RemoveMatchmakingEntry(MuxyGameLink GameLink, const char* Id)
{
	gamelink::SDK* SDK = static_cast<SDK*>(GameLink.SDK);
	return SDK->RemoveMatchmakingEntry(Id);
}

uint32_t MuxyGameLink_OnMatchmakingQueueInvite(MuxyGameLink GameLink, MGL_MatchmakingResponseCallback Callback, void* UserData)
{
	gamelink::SDK* SDK = static_cast<SDK*>(GameLink.SDK);
	return SDK->OnMatchmakingQueueInvite().Add([Callback, UserData](const schema::MatchmakingUpdate& UpdateResponse) {
		MGL_Schema_MatchmakingUpdateResponse Response;
		Response.Obj = &UpdateResponse;

		Callback(UserData, Response);
	});
}

uint32_t MuxyGameLink_DetachOnMatchmakingQueueInvite(MuxyGameLink GameLink, uint32_t Id)
{
	gamelink::SDK* SDK = static_cast<SDK*>(GameLink.SDK);
	return SDK->OnMatchmakingQueueInvite().Remove(Id);
}