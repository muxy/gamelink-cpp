#include <chrono>
#include <iostream>
#include <thread>

#define GAMELINK_DEBUG 1

#include "fmt/ranges.h"
#include "src/gamelink.hpp"
#include <websocket.h>

using gamelink::SDK;
using gamelink::Send;
using gamelink::schema::PollUpdateResponse;

int main()
{
	SDK sdk;

	// Set up network connection
	WebsocketConnection websocket("sandbox.gamelink.muxy.io", 80);
	websocket.onMessage([&](nlohmann::json json) {
		auto jstring = json.dump();

#ifdef GAMELINK_DEBUG
		fmt::print("gamelink> {}\n", json.dump(2));
#endif

		sdk.ReceiveMessage(jstring);
	});

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

			sdk.ForeachSend([&](Send* send) {
#ifdef GAMELINK_DEBUG
				fmt::print("outgoing> {}\n", send->data);
#endif

				nlohmann::json value = nlohmann::json::parse(send->data, nullptr, false);
				if (value.is_discarded())
				{
#ifdef GAMELINK_DEBUG
					fmt::print("! bad json\n");
#endif
				}
				else
				{
					websocket.send(value);
				}
			});

			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
	};

	std::thread sdkLoop(loop);

	std::string client_id, pin;

	std::cout << "Extension client ID: ";
	std::cin >> client_id;

	std::cout << "Authorization PIN: ";
	std::cin >> pin;

	sdk.AuthenticateWithPIN(client_id, pin);

	std::cout << "Awaiting auth response..";
	while (!sdk.IsAuthenticated())
	{
		std::cout << ".";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	while (!done)
	{
		std::cout << "> ";
		switch (getchar())
		{
		case 'a':
			std::cout << sdk.GetUser()->GetJWT() << "\n";
			break;

		case 'c':
			sdk.CreatePoll("vote-and-win", "Who's your favorite?", {"Me", "Him", "Her"});
			break;

		case 'p':
			sdk.SubscribeToPoll("vote-and-win");
			break;

		case 'q':
			fmt::print("Bye\n");
			done = true;
			break;

		default:
			break;
		}
	}

	sdkLoop.join();
	websocket.terminate();

	return 0;
}
