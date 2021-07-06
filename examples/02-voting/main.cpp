#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#define GAMELINK_DEBUG 1
#define MUXY_GAMELINK_SINGLE_IMPL

#include "gamelink_single.hpp"

#include "load_configuration.h"
#include "websocket_network/websocket.h"

int main()
{
	examples::Configuration cfg = examples::LoadConfiguration();
	gamelink::SDK sdk;

	// Set up network connection
	std::string url = gamelink::WebsocketConnectionURL(cfg.clientID, gamelink::CONNECTION_STAGE_SANDBOX);
	WebsocketConnection websocket(url, 80);
	websocket.onMessage([&](const char* bytes, uint32_t len) { sdk.ReceiveMessage(bytes, len); });

	// Setup the debug logger.
	sdk.OnDebugMessage([](const std::string& str) { std::cerr << "!    " << str << "\n"; });

	bool done = false;

	// Set up SDK event handlers
	sdk.OnPollUpdate([&](const gamelink::schema::PollUpdateResponse& pollResponse) {
		std::cout << "Got poll info, id=" << pollResponse.data.poll.pollId << " prompt=" << pollResponse.data.poll.prompt << "\n";
		for (uint32_t i = 0; i < pollResponse.data.poll.options.size(); i++)
		{
			std::cout << "Option[" << i << "]: " << pollResponse.data.poll.options[i] << " = " << pollResponse.data.results[i] << "\n";
		}

		done = true;
	});

	std::string refresh = examples::LoadRefresh();

	sdk.AuthenticateWithRefreshToken(cfg.clientID, refresh);
	sdk.CreatePoll("vote-and-win", "Who's your favorite?", {"Me", "Him", "Her"});
	sdk.SubscribeToPoll("vote-and-win");

	sdk.ForeachPayload([&](const gamelink::Payload* send) { websocket.send(send->data.c_str(), send->data.size()); });

	while (!done)
	{
		websocket.run();
	}

	return 0;
}
