#include "gateway.h"

namespace gateway
{	
	void SDK::OnBitsUsed(std::function<void (const gateway::BitsUsed&)> Callback)
	{
		Base.OnTransaction().AddUnique(gamelink::string("gamelink-bits"), [=](const gamelink::schema::TransactionResponse& resp)
		{
			if (resp.data.currency == "bits")
			{
				gateway::BitsUsed used;
				used.TransactionID = resp.data.id;
				used.SKU = resp.data.sku;
				used.Bits = resp.data.cost;

				Callback(used);
			}
		});

		Base.SubscribeToAllPurchases();
	}

	void SDK::OnCoinsUsed(std::function<void (const gateway::CoinsUsed&)> Callback)
	{
		Base.OnTransaction().AddUnique(gamelink::string("gamelink-coins"), [=](const gamelink::schema::TransactionResponse& resp)
		{
			if (resp.data.currency == "coins")
			{
				gateway::CoinsUsed used;
				used.TransactionID = resp.data.muxyId;
				used.Cost = resp.data.cost;
				used.SKU = resp.data.sku;

				Callback(used);
			}
		});

		Base.SubscribeToAllPurchases();
	}

	void SDK::ValidateTransaction(const gateway::CoinsUsed& used, const gamelink::string& Details)
	{
		Base.ValidateTransaction(used.TransactionID, Details);
	}
	void SDK::RefundTransaction(const gateway::CoinsUsed& used, const gamelink::string& Details)
	{
		Base.RefundTransactionByID(used.TransactionID, Details);
	}
}