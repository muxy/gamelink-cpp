#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::GetDrops(const string& status, std::function<void (const schema::GetDropsResponse&)> callback)
	{
		schema::GetDropsRequest payload(status);
		RequestId id = queuePayload(payload);

		_onGetDrops.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::GetDrops(const string& status, void (*callback)(void*, const schema::GetDropsResponse&), void* ptr)
	{
		schema::GetDropsRequest payload(status);
		RequestId id = queuePayload(payload);

		_onGetDrops.set(callback, ptr, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::ValidateDrop(const string& id)
	{
		schema::ValidateDropsRequest request(id);
		return queuePayload(request);
	}
}