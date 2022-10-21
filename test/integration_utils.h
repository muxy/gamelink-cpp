#pragma once
#define MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include <nlohmann/json.hpp>
#include <atomic>

#include "gamelink.h"
#include "websocket.h"

class IntegrationTestFixture
{
public:
	IntegrationTestFixture();
	~IntegrationTestFixture();

	int Request(const char* method, const char* endpoint,  const nlohmann::json* input, nlohmann::json* output);
	gamelink::SDK sdk;

	void Sleep(int seconds = 1);
	void FlushSDKMessages();

	int64_t UnixNow();

	void ForceDisconnect();
	void Reconnect();
private:
	void LoadEnvironment();

	std::string jwt;
	std::string client;

	std::string authenticationHeader;
	std::unique_ptr<WebsocketConnection> connection;
	std::unique_ptr<std::thread> runner;
	std::atomic<bool> done;
};
#endif