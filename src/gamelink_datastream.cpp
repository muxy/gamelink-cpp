#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::SubscribeToDatastream()
	{
		schema::SubscribeDatastreamRequest payload;
		return queuePayload(payload);
	}

	RequestId SDK::UnsubscribeFromDatastream()
	{
		schema::UnsubscribeDatastreamRequest payload;
		return queuePayload(payload);
	}

	uint32_t SDK::OnDatastream(std::function<void(const schema::DatastreamUpdate&)> callback)
	{
		return _onDatastreamUpdate.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnDatastream(void (*callback)(void*, const schema::DatastreamUpdate&), void* user)
	{
		return _onDatastreamUpdate.set(callback, user, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnDatastream(uint32_t id)
	{
		if (_onDatastreamUpdate.validateId(id))
		{
			_onDatastreamUpdate.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnDatastream");
		}
	}
}