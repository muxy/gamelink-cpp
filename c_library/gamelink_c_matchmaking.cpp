#include "gamelink.h"
#include "gamelink_c.h"

#include "gamelink_c_interop.h"

using namespace gamelink;

MGL_RequestId MuxyGameLink_ClearMatchmakingQueue(MuxyGameLink GameLink)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->ClearMatchmakingQueue();
}

MGL_RequestId MuxyGameLink_RemoveMatchmakingEntry(MuxyGameLink GameLink, const char* id)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->RemoveMatchmakingEntry(gamelink::string(id));
}

MGL_RequestId MuxyGameLink_SubscribeToMatchmakingQueueInvite(MuxyGameLink GameLink)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->SubscribeToMatchmakingQueueInvite();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromMatchmakingQueueInvite(MuxyGameLink GameLink)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->UnsubscribeFromMatchmakingQueueInvite();
}

uint32_t MuxyGameLink_OnMatchmakingQueueInvite(MuxyGameLink GameLink, MGL_MatchmakingUpdateCallback Callback, void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->OnMatchmakingQueueInvite().Add(C_CALLBACK(Callback, UserData, MatchmakingUpdate));
}

uint32_t MuxyGameLink_OnMatchmakingQueueInviteUnique(MuxyGameLink GameLink, const char* Name, MGL_MatchmakingUpdateCallback Callback, void* UserData)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	return Instance->OnMatchmakingQueueInvite().AddUnique(gamelink::string(Name), C_CALLBACK(Callback, UserData, MatchmakingUpdate));
}

void MuxyGameLink_DetachOnMatchmakingQueueInvite(MuxyGameLink GameLink, uint32_t Handle)
{
	SDK* Instance = static_cast<SDK*>(GameLink.SDK);
	Instance->OnMatchmakingQueueInvite().Remove(Handle);
}

MGL_String MuxyGameLink_MatchmakingUpdate_GetData(MGL_Schema_MatchmakingUpdate obj)
{
	const schema::MatchmakingUpdate* Object = static_cast<const schema::MatchmakingUpdate *>(obj.Obj);
	return MuxyGameLink_StrDup(Object->data.data.dump().c_str());
}

const char* MuxyGameLink_MatchmakingUpdate_GetTwitchUsername(MGL_Schema_MatchmakingUpdate obj)
{
	const schema::MatchmakingUpdate* Object = static_cast<const schema::MatchmakingUpdate *>(obj.Obj);
	return Object->data.twitchUsername.c_str();
}

const char* MuxyGameLink_MatchmakingUpdate_GetTwitchID(MGL_Schema_MatchmakingUpdate obj)
{
	const schema::MatchmakingUpdate* Object = static_cast<const schema::MatchmakingUpdate *>(obj.Obj);
	return Object->data.twitchID.c_str();
}

int64_t MuxyGameLink_MatchmakingUpdate_GetTimestamp(MGL_Schema_MatchmakingUpdate obj)
{
	const schema::MatchmakingUpdate* Object = static_cast<const schema::MatchmakingUpdate *>(obj.Obj);
	return Object->data.timestamp;
}

int MuxyGameLink_MatchmakingUpdate_GetIsFollower(MGL_Schema_MatchmakingUpdate obj)
{
	const schema::MatchmakingUpdate* Object = static_cast<const schema::MatchmakingUpdate *>(obj.Obj);
	return Object->data.isFollower;
}

int MuxyGameLink_MatchmakingUpdate_GetSubscriptionTier(MGL_Schema_MatchmakingUpdate obj)
{
	const schema::MatchmakingUpdate* Object = static_cast<const schema::MatchmakingUpdate *>(obj.Obj);
	return Object->data.subscriptionTier;
}

int MuxyGameLink_MatchmakingUpdate_GetBitsSpent(MGL_Schema_MatchmakingUpdate obj)
{
	const schema::MatchmakingUpdate* Object = static_cast<const schema::MatchmakingUpdate *>(obj.Obj);
	return Object->data.bitsSpent;
}
