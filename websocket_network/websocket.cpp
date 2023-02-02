#include "websocket.h"

#include <curl/curl.h>
#include <mutex>
#include <thread>
#include <iostream>
#include <atomic>

#define LOGF(msg) std::cerr << "[DEBUG] " << msg << "\n"

struct Message
{
	std::vector<char> data;
	bool binary;
	bool first;
	bool final;

	bool disconnect = false;
};

struct Impl
{
	CURLM* multi = nullptr;
	bool done = false;

	CURL* connection;
	char curlErrorBuffer[CURL_ERROR_SIZE];

	bool connected = false;

	std::mutex lock;
	std::vector<std::unique_ptr<Message>> messages;

	std::vector<char> messageFragment;

	std::function<void (const char*, uint32_t)> onMessage;
};

size_t writeResponse(char* ptr, size_t size, size_t nmemb, void* data)
{
	Impl* impl = static_cast<Impl*>(data);

	size_t oldSize = impl->messageFragment.size();
	impl->messageFragment.resize(impl->messageFragment.size() + size * nmemb);

	char* start = impl->messageFragment.data() + oldSize;
	memcpy(start, ptr, size * nmemb);
	
	curl_ws_frame* frame = curl_ws_meta(impl->connection);
	if (frame->bytesleft > 0)
	{
		return size * nmemb;
	}

	impl->onMessage(impl->messageFragment.data(), static_cast<uint32_t>(impl->messageFragment.size()));
	impl->messageFragment.clear();

	return size * nmemb;
}

size_t writeHeader(char* buffer, size_t size, size_t nitems, void* userdata)
{
	std::string headerView(buffer, size * nitems);
	if (headerView == "\r\n")
	{
		Impl* impl = static_cast<Impl*>(userdata);
		impl->connected = true;

		LOGF("Connection established");
	}

	return size * nitems;
}

std::atomic<int> refcount;

static void panic(const char* msg)
{
	std::cerr << msg << "\n";
	std::exit(1);
}

WebsocketConnection::WebsocketConnection(const std::string& url, uint16_t port)
{
	impl = new Impl();
	impl->multi = curl_multi_init();
	if (!impl->multi)
	{
		panic("Null response from curl_multi_init");
	}

	impl->done = false;

	CURL* curl = curl_easy_init();

	std::string protocolURL = "wss://" + url;
	curl_easy_setopt(curl, CURLOPT_URL, protocolURL.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, impl);

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeader);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, impl);

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, impl->curlErrorBuffer);
	impl->curlErrorBuffer[0] = '\0';

	impl->connection = curl;
	curl_multi_add_handle(impl->multi, curl);

	LOGF("Add connection");
}

WebsocketConnection::~WebsocketConnection()
{
	LOGF("Cleaning up connection");
	impl->done = true;
	if (impl->connection)
	{
		curl_multi_remove_handle(impl->multi, impl->connection);
		curl_easy_cleanup(impl->connection);
		impl->connection = nullptr;
	}

	curl_multi_cleanup(impl->multi);

	delete impl;
}

int WebsocketConnection::run()
{
	if (impl->done)
	{
		return 1;
	}

	int running = 0;
	CURLMcode err = curl_multi_perform(impl->multi, &running);
	if (err != 0)
	{
		LOGF("Error while perform: " << err);
		return err;
	}

	int remain = 0;
	while (CURLMsg* msg = curl_multi_info_read(impl->multi, &remain))
	{
		if (msg->msg == CURLMSG_DONE)
		{
			if (msg->easy_handle != impl->connection)
			{
				LOGF("Connection mismatch?");
			}

			curl_multi_remove_handle(impl->multi, impl->connection);
			curl_easy_cleanup(impl->connection);
			impl->connection = nullptr;

			LOGF("Removing connection");
			if (msg->data.result != CURLE_OK)
			{
				LOGF("code=" << msg->data.result << " err=" << impl->curlErrorBuffer);
			}
		}
	}

	std::lock_guard<std::mutex> lock(impl->lock);
	if (!impl->connection || !impl->connected)
	{
		return 0;
	}

	for (auto it = impl->messages.begin(); it != impl->messages.end(); ++it)
	{
		Message* msg = it->get();
		unsigned int flags = CURLWS_TEXT;
		if (msg->binary)
		{
			flags = CURLWS_BINARY;
		}

		if (msg->disconnect)
		{
			flags = CURLWS_CLOSE;
		}

		size_t sent = 0;
		CURLcode result = curl_ws_send(
			impl->connection, 
			msg->data.data(), 
			msg->data.size(), 
			&sent, 
			0, 
			flags
		);

		if (result != CURLE_OK)
		{
			return 1;
		}
	}

	impl->messages.clear();
	return 0;
}

void WebsocketConnection::terminate()
{
	std::unique_ptr<Message> msg = std::unique_ptr<Message>(new Message());
	msg->binary = true;
	msg->disconnect = true;

	std::lock_guard<std::mutex> lock(impl->lock);
	impl->messages.push_back(std::move(msg));
}

void WebsocketConnection::send(const char* bytes, uint32_t length)
{
	std::unique_ptr<Message> msg = std::unique_ptr<Message>(new Message());
	msg->binary = false;
	msg->data = std::vector<char>(bytes, bytes + length);
	
	std::lock_guard<std::mutex> lock(impl->lock);
	impl->messages.push_back(std::move(msg));
}

void WebsocketConnection::onMessage(std::function<void(const char *, uint32_t)> cb)
{
	impl->onMessage = std::move(cb);
}

void WebsocketConnection::read()
{}