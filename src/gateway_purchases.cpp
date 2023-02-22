#include "gateway.h"

namespace gateway
{	
	void SDK::OnBitsUsed(std::function<void (const gateway::BitsUsed&)> Callback)
	{
		Base.OnTransaction().AddUnique(gamelink::string("gateway-bits"), [=](const gamelink::schema::TransactionResponse& resp)
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

	void SDK::OnActionUsed(std::function<void (const gateway::ActionUsed&)> Callback)
	{
		Base.OnTransaction().AddUnique(gamelink::string("gateway-actions"), [=](const gamelink::schema::TransactionResponse& resp)
		{
			if (resp.data.currency == "coins")
			{
				gateway::ActionUsed used;

				used.TransactionID = resp.data.muxyId;
				used.Cost = resp.data.cost;
				used.SKU = resp.data.sku;
				used.UserID = resp.data.userId;
				used.UserNickname = resp.data.userName;

				Callback(used);
			}
		});

		Base.SubscribeToAllPurchases();
	}

	void SDK::AcceptAction(const gateway::ActionUsed& used, const gamelink::string& Details)
	{
		Base.ValidateTransaction(used.TransactionID, Details);
	}

	void SDK::RefundAction(const gateway::ActionUsed& used, const gamelink::string& Details)
	{
		Base.RefundTransactionByID(used.TransactionID, used.UserID);
	}
}