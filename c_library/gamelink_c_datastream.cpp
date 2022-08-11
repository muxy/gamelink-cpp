#include "gamelink_c.h"
#include "gamelink.h"

#include "gamelink_c_interop.h"

MGL_RequestId MuxyGameLink_SubscribeToDatastream(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->SubscribeToDatastream();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromDatastream(MuxyGameLink GameLink)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->UnsubscribeFromDatastream();
}

uint32_t MuxyGameLink_OnDatastream(MuxyGameLink GameLink, MGL_DatastreamUpdateCallback Callback, void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->OnDatastreamUpdate().Add(C_CALLBACK(Callback, UserData, DatastreamUpdate));
}

uint32_t MuxyGameLink_OnDatastreamUnique(MuxyGameLink GameLink, const char* Name, MGL_DatastreamUpdateCallback Callback, void* UserData)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->OnDatastreamUpdate().AddUnique(gamelink::string(Name), C_CALLBACK(Callback, UserData, DatastreamUpdate));
}

void MuxyGameLink_DetachOnDatastream(MuxyGameLink GameLink, uint32_t OnDatastreamHandle)
{
	gamelink::SDK* SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	SDK->OnDatastreamUpdate().Remove(OnDatastreamHandle);
}

uint32_t MuxyGameLink_Schema_DatastreamUpdate_GetEventCount(MGL_Schema_DatastreamUpdate DatastreamUpdate)
{
	const gamelink::schema::DatastreamUpdate* DSU = static_cast<const gamelink::schema::DatastreamUpdate*>(DatastreamUpdate.Obj);
	return DSU->data.events.size();
}

MGL_Schema_DatastreamEvent MuxyGameLink_Schema_DatastreamUpdate_GetEventAt(MGL_Schema_DatastreamUpdate DatastreamUpdate, uint32_t AtIndex)
{
	const gamelink::schema::DatastreamUpdate* DSU = static_cast<const gamelink::schema::DatastreamUpdate*>(DatastreamUpdate.Obj);

	MGL_Schema_DatastreamEvent WDSEvent;
	WDSEvent.Obj = &DSU->data.events[AtIndex];

	return WDSEvent;
}

MGL_String MuxyGameLink_Schema_DatastreamEvent_GetJson(MGL_Schema_DatastreamEvent DatastreamEvent)
{
    const gamelink::schema::DatastreamEvent *DSE = static_cast<const gamelink::schema::DatastreamEvent*>(DatastreamEvent.Obj);
    return MuxyGameLink_StrDup(DSE->event.dump().c_str());
}

int64_t	MuxyGameLink_Schema_DatastreamEvent_GetTimestamp(MGL_Schema_DatastreamEvent DatastreamEvent)
{
    const gamelink::schema::DatastreamEvent *DSE = static_cast<const gamelink::schema::DatastreamEvent*>(DatastreamEvent.Obj);
    return DSE->timestamp;
}
