#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#define GAMELINK_DEBUG 1
#define MUXY_GAMELINK_SINGLE_IMPL

#include "fmt/ranges.h"
#include "gamelink_single.hpp"
#include <websocket.h>

using gamelink::SDK;
using gamelink::Send;
using gamelink::schema::PollUpdateResponse;

int main()
{
	SDK sdk;

	// Set up network connection
	WebsocketConnection websocket("sandbox.gamelink.muxy.io", 80);
	websocket.onMessage([&](const char* bytes, uint32_t len) { sdk.ReceiveMessage(bytes, len); });

	// Set up SDK event handlers
	sdk.OnPollUpdate([](PollUpdateResponse pollResponse) {
		// Print the poll's results
		fmt::print("For the poll \"{}\", results:\n", pollResponse.data.poll.prompt);
		fmt::print("{}\n", pollResponse.data.poll.options);

		int i = 0;
		for (auto option : pollResponse.data.poll.options)
		{
			fmt::print("{}: {}\n", option, pollResponse.data.results[i]);
			i++;
		}
	});

	auto done = false;

	// Create thread to send Gamelink messages to server
	auto loop = [&]() {
		while (!done)
		{
			websocket.run();

			sdk.ForeachSend([&](const Send* send) {
#ifdef GAMELINK_DEBUG
				fmt::print("outgoing> {}\n", send->data);
#endif

				websocket.send(send->data.c_str(), send->data.size());
			});

			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
	};

	std::thread sdkLoop(loop);

	std::string client_id, pin;

	auto extId = getenv("EXTENSION_ID");

	if (extId == NULL)
	{
		std::cout << "Extension client ID: ";
		std::cin >> client_id;
	}
	else
	{
		client_id = std::string(extId);
		std::cout << "Extension client ID: " << client_id << std::endl;
	}

	std::cout << "Authorization PIN: ";
	std::cin >> pin;

	sdk.AuthenticateWithPIN(client_id, pin);

	std::cout << "Awaiting auth response..";
	while (!sdk.IsAuthenticated())
	{
		std::cout << ".";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	sdk.CreatePoll("vote-and-win", "Who's your favorite?", {"Me", "Him", "Her"});

	sdk.SubscribeToPoll("vote-and-win");

	sdkLoop.join();
	websocket.terminate();

	return 0;
}
