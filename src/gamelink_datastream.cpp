#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::SubscribeToDatastream()
	{
		if (_subscriptionSets.canRegisterDatastream())
		{
			schema::SubscribeDatastreamRequest payload;
			RequestId req = queuePayload(payload);

			_subscriptionSets.registerDatastream();
			return req;
		}

		InvokeOnDebugMessage("SubscribeToDatastream: duplicated subscription call");
		return ANY_REQUEST_ID;
	}

	RequestId SDK::UnsubscribeFromDatastream()
	{
		schema::UnsubscribeDatastreamRequest payload;
		_subscriptionSets.unregisterDatastream();
		return queuePayload(payload);
	}

	Event<schema::DatastreamUpdate>& SDK::OnDatastreamUpdate()
	{
		return _onDatastreamUpdate;
	}
}