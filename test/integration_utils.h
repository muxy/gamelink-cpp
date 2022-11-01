#pragma once
#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include <nlohmann/json.hpp>
#include <atomic>
#include <thread>
#include <string>
#include <memory>

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
	std::string target;

	std::string targetDomain;
	std::string targetPrefix;

	std::string authenticationHeader;
	std::unique_ptr<WebsocketConnection> connection;
	std::unique_ptr<std::thread> runner;
	std::atomic<bool> done;
};
#endif
