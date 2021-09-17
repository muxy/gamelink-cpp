#include "schema/purchase.h"

namespace gamelink
{
	namespace schema
	{
		SubscribeTransactionsRequest::SubscribeTransactionsRequest(const string& SKU)
		{
			action = string("subscribe");
			params.target = string("twitchPurchaseBits");
			data.sku = SKU;
		}

		UnsubscribeTransactionsRequest::UnsubscribeTransactionsRequest(const string& SKU)
		{
			action = string("unsubscribe");
			params.target = string("twitchPurchaseBits");
			data.sku = SKU;
		}

		GetOutstandingTransactionsRequest::GetOutstandingTransactionsRequest(const string& sku)
		{
			action = string("get");
			params.target = string("transaction");
			data.sku = sku;
		}

		RefundTransactionRequest::RefundTransactionRequest(const string& transactionId, const string& userId)
		{
			action = string("refund");
			params.target = string("transaction");
			data.transactionId = string(transactionId);
			data.userId = string(userId);
		}

		RefundTransactionBySKURequest::RefundTransactionBySKURequest(const string& sku, const string& userId)
		{
			action = string("refund");
			params.target = string("transaction");
			data.SKU = string(sku);
			data.userId = string(userId);
		}

		ValidateTransactionRequest::ValidateTransactionRequest(const string& transactionId, const string& details)
		{
			action = string("validate");
			params.target = string("transaction");
			data.transactionId = string(transactionId);
			data.details = string(details);
		}
	}
}
