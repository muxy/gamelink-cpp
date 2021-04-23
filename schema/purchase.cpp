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
    }
}
