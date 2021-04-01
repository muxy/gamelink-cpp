#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#define GAMELINK_DEBUG 1
#define MUXY_GAMELINK_SINGLE_IMPL

#include "fmt/ranges.h"
#include "gamelink_single.hpp"

#include "load_configuration.h"
#include "websocket_network/websocket.h"

int main()
{
	gamelink::SDK sdk;

	// Set up network connection
	WebsocketConnection websocket("sandbox.gamelink.muxy.io", 80);
	websocket.onMessage([&](const char* bytes, uint32_t len) { 
		sdk.ReceiveMessage(bytes, len); 
	});

	bool done = false;

	// Set up SDK event handlers
	sdk.OnPollUpdate([&](gamelink::schema::PollUpdateResponse pollResponse) {
		// Print the poll's results
		fmt::print("{}\n", pollResponse.data.poll.options);

		int i = 0;
		for (auto option : pollResponse.data.poll.options)
		{
			fmt::print("{}: {}\n", option, pollResponse.data.results[i]);
			i++;
		}

		done = true;
	});

	examples::Configuration cfg = examples::LoadConfiguration();
	std::string jwt = examples::LoadJWT();

	sdk.AuthenticateWithJWT(cfg.clientID, jwt);
	sdk.CreatePoll("vote-and-win", "Who's your favorite?", {"Me", "Him", "Her"});
	sdk.SubscribeToPoll("vote-and-win");

	sdk.ForeachSend([&](const gamelink::Send * send)
    {
        websocket.send(send->data.c_str(), send->data.size());
    });

	while (!done)
	{
		websocket.run();
	}

	return 0;
}
