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

	uint32_t SDK::OnTransaction(std::function<void(const schema::TransactionResponse&)> callback)
	{
		return _onTransaction.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnTransaction(void (*callback)(void*, const schema::TransactionResponse&), void* ptr)
	{
		return _onTransaction.set(callback, ptr, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnTransaction(uint32_t id)
	{
		if (_onTransaction.validateId(id))
		{
			_onTransaction.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into DetachOnTransaction");
		}
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