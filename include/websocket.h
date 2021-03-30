#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

struct Impl;

class WebsocketConnection
{
public:
	WebsocketConnection(const std::string& host, uint16_t port);
	~WebsocketConnection();

	// Noncopyable
	WebsocketConnection(const WebsocketConnection&) = delete;
	WebsocketConnection& operator=(const WebsocketConnection&) = delete;
	WebsocketConnection(WebsocketConnection&&) = delete;
	WebsocketConnection& operator=(const WebsocketConnection&&) = delete;

	int run();
	void terminate();

	// This function will block
	void send(const char * bytes, uint32_t length);
	void onMessage(std::function<void(const char *, uint32_t)> cb);
private:
	void read();

	Impl* impl;
};
