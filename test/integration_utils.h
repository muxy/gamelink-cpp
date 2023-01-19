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

	int Request(const char* method, const char* endpoint, const nlohmann::json* input, nlohmann::json* output);
	int RequestURL(const char *method, const char* url, nlohmann::json* output);
	gamelink::SDK sdk;

	void Sleep(int seconds = 1);
	void FlushSDKMessages();

	int64_t UnixNow();
	std::string Base64(const std::string& input);
	std::string Decode64(const std::string& input);

	void ForceDisconnect();
	void Connect();
	void Reconnect();

	std::string jwt;
	std::string signature;
	std::string client;
private:
	void LoadEnvironment();

	std::string target;

	std::string targetDomain;
	std::string targetPrefix;

	std::string authenticationHeader;
	std::unique_ptr<WebsocketConnection> connection;
	std::unique_ptr<std::thread> runner;
	std::atomic<bool> done;
};
#endif
