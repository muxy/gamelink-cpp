#include "gamelink.h"
#include "gamelink_c.h"

using namespace gamelink;

MGL_RequestId MuxyGameLink_SubscribeToSKU(MuxyGameLink GameLink, const char* SKU)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->SubscribeToSKU(SKU);
}

MGL_RequestId MuxyGameLink_UnsubscribeFromSKU(MuxyGameLink GameLink, const char* SKU)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->UnsubscribeFromSKU(SKU);
}

MGL_RequestId MuxyGameLink_SubscribeToAllPurchases(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->SubscribeToAllPurchases();
}

MGL_RequestId MuxyGameLink_UnsubscribeFromAllPurchases(MuxyGameLink GameLink)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->UnsubscribeFromAllPurchases();
}

uint32_t MuxyGameLink_OnTransaction(MuxyGameLink GameLink, MGL_TransactionResponseCallback Callback, void* UserData)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    uint32_t res = SDK->OnTransaction([Callback, UserData](const gamelink::schema::TransactionResponse &TPBResp)
    {
        MGL_Schema_TransactionResponse WTPBResp;
        WTPBResp.Obj = &TPBResp;

        Callback(UserData, WTPBResp);
    });

    return res;
}

void MuxyGameLink_DetachOnTransaction(MuxyGameLink GameLink, uint32_t id)
{
    gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    SDK->DetachOnTransaction(id);
}

const char* MuxyGameLink_Schema_Transaction_GetId(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return TPB->data.id.c_str();
}

const char* MuxyGameLink_Schema_Transaction_GetSKU(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return TPB->data.sku.c_str();
}

const char* MuxyGameLink_Schema_Transaction_GetDisplayName(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return TPB->data.displayName.c_str();
}

const char* MuxyGameLink_Schema_Transaction_GetUserId(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return TPB->data.userId.c_str();
}

const char* MuxyGameLink_Schema_Transaction_GetUserName(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return TPB->data.userName.c_str();
}

int32_t MuxyGameLink_Schema_Transaction_GetCost(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return TPB->data.cost;
}

int64_t MuxyGameLink_Schema_Transaction_GetTimestamp(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return TPB->data.timestamp;
}

MGL_String MuxyGameLink_Schema_Transaction_GetJson(MGL_Schema_TransactionResponse TPBResp)
{
    const gamelink::schema::TransactionResponse *TPB = static_cast<const gamelink::schema::TransactionResponse*>(TPBResp.Obj);
    return MuxyGameLink_StrDup(TPB->data.additional.dump().c_str());
}

MGL_RequestId MuxyGameLink_GetOutstandingTransactions(MuxyGameLink GameLink, const char *SKU, MGL_GetOutstandingTransactionsResponseCallback Callback, void *UserData)
{
	gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
	return SDK->GetOutstandingTransactions(SKU, [Callback, UserData](const schema::GetOutstandingTransactionsResponse& Resp)
	{
		MGL_Schema_GetOutstandingTransactionsResponse WResp;
		WResp.Obj = &Resp;
		Callback(UserData, WResp);
	});
}

uint32_t MuxyGameLink_Schema_GetOutstandingTransactions_GetTransactionCount(MGL_Schema_GetOutstandingTransactionsResponse Resp)
{
    const gamelink::schema::GetOutstandingTransactionsResponseBody *Body = static_cast<const gamelink::schema::GetOutstandingTransactionsResponseBody*>(Resp.Obj);
    return Body->transactions.size();
}

MGL_Schema_TransactionResponse MuxyGameLink_Schema_GetOutstandingTransactions_GetTransactionAt(MGL_Schema_GetOutstandingTransactionsResponse Resp, uint32_t Index)
{
    const gamelink::schema::GetOutstandingTransactionsResponseBody *Body = static_cast<const gamelink::schema::GetOutstandingTransactionsResponseBody*>(Resp.Obj);
    MGL_Schema_TransactionResponse Res;
    Res.Obj = &Body->transactions[Index];
    return Res;
}

MGL_RequestId MuxyGameLink_RefundTransactionBySKU(MuxyGameLink GameLink, const char *SKU, const char *UserId)
{
	gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->RefundTransactionBySKU(SKU, UserId);
}

MGL_RequestId MuxyGameLink_RefundTransactionByID(MuxyGameLink GameLink, const char *TxId, const char *UserId)
{
	gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->RefundTransactionByID(TxId, UserId);
}

MGL_RequestId MuxyGameLink_ValidateTransaction(MuxyGameLink GameLink, const char *TxId, const char *Details)
{
	gamelink::SDK *SDK = static_cast<gamelink::SDK*>(GameLink.SDK);
    return SDK->ValidateTransaction(TxId, Details);
}