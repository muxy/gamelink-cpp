#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::SubscribeToSKU(const string& sku)
	{
		schema::SubscribeTransactionsRequest payload(sku);
		return queuePayload(payload);
	}

	RequestId SDK::SubscribeToAllPurchases()
	{
		return SubscribeToSKU("*");
	}

	RequestId SDK::UnsubscribeFromSKU(const string& sku)
	{
		schema::UnsubscribeTransactionsRequest payload(sku);
		return queuePayload(payload);
	}

	RequestId SDK::UnsubscribeFromAllPurchases()
	{
		return UnsubscribeFromSKU("*");
	}

	uint32_t SDK::OnTwitchPurchaseBits(std::function<void(const schema::TwitchPurchaseBitsResponse<nlohmann::json>&)> callback)
	{
		return _onTwitchPurchaseBits.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnTwitchPurchaseBits(void (*callback)(void*, const schema::TwitchPurchaseBitsResponse<nlohmann::json>&), void* ptr)
	{
		return _onTwitchPurchaseBits.set(callback, ptr, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnTwitchPurchaseBits(uint32_t id)
	{
		if (_onTwitchPurchaseBits.validateId(id))
		{
			_onTwitchPurchaseBits.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnTwitchPurchaseBits");
		}
	}
}