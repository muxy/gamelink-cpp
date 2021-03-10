#include <chrono>
#include <iostream>
#include <thread>

#define GAMELINK_DEBUG 1

#include "src/gamelink.hpp"
#include <websocket.h>

using gamelink::SDK;
using gamelink::Send;

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
				websocket.send(nlohmann::json::parse(send->data));
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

		case 's':
			sdk.subscribeTo("thing", "");
			break;

		case 'u':
			sdk.unsubscribeFrom("thing", "");
			break;

		case 'b':
			sdk.broadcast("hi world", {});
			break;

		case 'c':
			sdk.createPoll("vote-and-win", "Who's your favorite?", {"Me", "Him", "Her"});
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
