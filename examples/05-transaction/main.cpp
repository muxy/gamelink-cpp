#define MUXY_GAMELINK_SINGLE_IMPL
#include "gamelink_single.hpp"

#include "load_configuration.h"
#include "websocket_network/websocket.h"

int main()
{
	examples::Configuration config = examples::LoadConfiguration();
	gamelink::SDK sdk;

	// Setup the connection
	std::string url = gamelink::WebsocketConnectionURL(config.clientID, gamelink::CONNECTION_STAGE_SANDBOX);
	WebsocketConnection connection(url, 80);
	
	connection.onMessage([&](const char* bytes, uint32_t len) { sdk.ReceiveMessage(bytes, len); });

	// Setup the debug logger.
	sdk.OnDebugMessage([](const std::string& str) { std::cerr << "!    " << str << "\n"; });

	std::string pin;
	while (pin.empty() || !std::cin)
	{
		std::cout << "Input PIN>\n";
		std::cin >> pin;
	}

	bool done = false;
	sdk.OnAuthenticate([&](const gamelink::schema::AuthenticateResponse& response) {
		const gamelink::schema::Error* err = gamelink::FirstError(response);
		if (err)
		{
			std::cout << "Couldn't authenticate. " << response.errors[0].title << ": " << response.errors[0].detail << "\n";
			done = true;
			return;
		}

		std::cout << "Authenticated with PIN, got jwt: " << response.data.jwt << "\n";
		examples::SaveRefresh(response.data.refresh);
	});

	sdk.OnTransaction([&](const gamelink::schema::TransactionResponse& response) {
		std::cout << "Got transaction: " << response.data.sku << " from " << response.data.userName << "\n";
		sdk.ValidateTransaction(response.data.muxyId, response.data.userId);
	});

	sdk.SubscribeToAllPurchases();
	sdk.AuthenticateWithPIN(config.clientID, pin);

	while (!done)
	{
		sdk.ForeachPayload([&](const gamelink::Payload* send) { connection.send(send->data.c_str(), send->data.size()); });
		connection.run();
	}
}