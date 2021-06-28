#define MUXY_GAMELINK_SINGLE_IMPL
#include "gamelink_single.hpp"

#include "load_configuration.h"
#include "websocket_network/websocket.h"

int main()
{
	examples::Configuration config = examples::LoadConfiguration();

	gamelink::SDK sdk;

	// Setup the connection
	WebsocketConnection connection("sandbox.gamelink.muxy.io", 80);
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
		if (response.errors.empty())
		{
			std::cout << "Authenticated with PIN, got jwt: " << response.data.jwt << "\n";
			examples::SaveRefresh(response.data.refresh);
		}
		else
		{
			std::cout << "Couldn't authenticate. " << response.errors[0].title << ": " << response.errors[0].detail << "\n";
		}
		done = true;
	});

	sdk.AuthenticateWithPIN(config.clientID, pin);
	sdk.ForeachPayload([&](const gamelink::Payload* send) { connection.send(send->data.c_str(), send->data.size()); });

	while (!done)
	{
		connection.run();
	}
}