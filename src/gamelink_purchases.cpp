#include "gamelink.h"

namespace gamelink
{
	RequestId SDK::SubscribeToSKU(const string& sku)
	{
		if (_subscriptionSets.canRegisterSKU(sku))
		{
			schema::SubscribeTransactionsRequest payload(sku);
			RequestId req = queuePayload(payload);

			_subscriptionSets.registerSKU(sku);
			return req;
		}

		char buffer[128];
		snprintf(buffer, 128, "SubscribeToSKU: duplicated subscription call with target=%s", sku.c_str());
		InvokeOnDebugMessage(buffer);

		return ANY_REQUEST_ID;
	}

	RequestId SDK::SubscribeToAllPurchases()
	{
		return SubscribeToSKU("*");
	}

	RequestId SDK::UnsubscribeFromSKU(const string& sku)
	{
		schema::UnsubscribeTransactionsRequest payload(sku);
		_subscriptionSets.unregisterSKU(sku);
		return queuePayload(payload);
	}

	RequestId SDK::UnsubscribeFromAllPurchases()
	{
		return UnsubscribeFromSKU("*");
	}

	Event<schema::TransactionResponse>& SDK::OnTransaction()
	{
		return _onTransaction;
	}

	RequestId SDK::GetOutstandingTransactions(const string& sku, std::function<void (const schema::GetOutstandingTransactionsResponse&)> callback)
	{
		schema::GetOutstandingTransactionsRequest payload(sku);
		RequestId id = queuePayload(payload);

		_onGetOutstandingTransactions.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::GetOutstandingTransactions(const string& sku, void (*callback)(void*, const schema::GetOutstandingTransactionsResponse&), void* ptr)
	{
		schema::GetOutstandingTransactionsRequest payload(sku);
		RequestId id = queuePayload(payload);

		_onGetOutstandingTransactions.set(callback, ptr, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::RefundTransactionBySKU(const string& sku, const string& userid)
	{
		schema::RefundTransactionBySKURequest payload(sku, userid);
		return queuePayload(payload);
	}

	RequestId SDK::RefundTransactionByID(const string& txid, const string& userid)
	{
		schema::RefundTransactionRequest payload(txid, userid);
		return queuePayload(payload);
	}

	RequestId SDK::ValidateTransaction(const string& txid, const string& details)
	{
		schema::ValidateTransactionRequest payload(txid, details);
		return queuePayload(payload);
	}
}