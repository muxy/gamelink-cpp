#include "integration_utils.h"
#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include <curl/curl.h>
#include "catch2/catch.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <ctime>

const char * IntegrationBaseUrl = "https://%s.api.muxy.io/v1/e/%s";

// Writer functions
inline size_t writeFunction(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	std::vector<char>* resp = reinterpret_cast<std::vector<char>*>(userdata);
	resp->insert(resp->end(), ptr, ptr + size * nmemb);

	return size * nmemb;
}

IntegrationTestFixture::IntegrationTestFixture()
	:done(false)
{
	LoadEnvironment();

	// Init CURL and websocket connection, setup thread runner for websockets
	curl_global_init(CURL_GLOBAL_ALL);

	Reconnect();

	// Get the authorization token by doing either token flow or refresh token.
	std::stringstream ss;
	ss << "Authorization: " << client << " " << jwt;
	authenticationHeader = ss.str();

	// This is static so other tests can use the refresh token flow.
	static std::string refreshToken;
	if (refreshToken.empty())
	{
		nlohmann::json resp;
		Request("POST", "gamelink/token", nullptr, &resp);

		std::string token = resp["token"];

		gamelink::RequestId req = sdk.AuthenticateWithPIN(gamelink::string(client.c_str()), gamelink::string(token.c_str()));
		sdk.WaitForResponse(req);
	}
	else
	{
		gamelink::RequestId req = sdk.AuthenticateWithRefreshToken(gamelink::string(client.c_str()), gamelink::string(refreshToken.c_str()));
		sdk.WaitForResponse(req);
	}

	// Store the refresh token
	sdk.OnAuthenticate().Add([&](const gamelink::schema::AuthenticateResponse& response) {
		const gamelink::schema::Error* err = gamelink::FirstError(response);
		if (err)
		{
			std::cout << "Couldn't authenticate. " << response.errors[0].title.c_str() << ": " << response.errors[0].detail.c_str() << "\n";
			return;
		}

		refreshToken = std::string(response.data.refresh.c_str());
	});

	// Wait for authentication.
	while (!sdk.IsAuthenticated())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

IntegrationTestFixture::~IntegrationTestFixture()
{
	done.store(true);
	runner->join();
	curl_global_cleanup();
}

void IntegrationTestFixture::FlushSDKMessages()
{
	while (sdk.HasPayloads())
	{
		std::this_thread::yield();
	}
}

void IntegrationTestFixture::Sleep(int seconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * seconds));
}

int64_t IntegrationTestFixture::UnixNow()
{
	return static_cast<int64_t>(std::time(nullptr));
}

void IntegrationTestFixture::ForceDisconnect()
{
	if (runner)
	{
		done = true;
		runner->join();

		runner.reset(nullptr);
		done = false;
	}

	if (connection)
	{
		connection.reset(nullptr);
	}
}

void IntegrationTestFixture::LoadEnvironment()
{
	// Get environment variables
	const char * jwtenv = std::getenv("MUXY_INTEGRATION_JWT");
	if (jwtenv)
	{
		jwt = jwtenv;
	}
	REQUIRE(jwt.size());

	const char * clientEnv = std::getenv("MUXY_INTEGRATION_ID");
	if (clientEnv)
	{
		client = clientEnv;
	}
	REQUIRE(client.size());

	target = "sandbox";
	const char * targetEnv = std::getenv("MUXY_INTEGRATION_TARGET");
	if (targetEnv)
	{
		target = targetEnv;
	}
}

void IntegrationTestFixture::Reconnect()
{
	ForceDisconnect();

	char buffer[256];
	int output = snprintf(buffer, 256, "%s.gamelink.muxy.io/%d.%d.%d/%s",
		target.c_str(),
		MUXY_GAMELINK_VERSION_MAJOR, MUXY_GAMELINK_VERSION_MINOR, MUXY_GAMELINK_VERSION_PATCH,
		client.c_str());

	REQUIRE(output > 0);
	REQUIRE(output < 256);

	gamelink::string url = gamelink::string(buffer);
	connection = std::unique_ptr<WebsocketConnection>(new WebsocketConnection(url.c_str(), 80));
	std::cerr << "!<   connect: " << url.c_str() << "\n";

	connection->onMessage([&](const char* bytes, uint32_t len)
	{
		sdk.ReceiveMessage(bytes, len);
	});

	sdk.OnDebugMessage([](const gamelink::string& str)
	{
		std::cerr <<"!<   " << str.c_str() << "\n";
	});

	runner = std::unique_ptr<std::thread>(new std::thread([&]()
	{
		while (!done)
		{
			sdk.ForeachPayload([&](const gamelink::Payload* send) {
				std::cerr << "!>   send len=" << send->Length() << " msg=" << std::string(send->Data(), send->Data() + send->Length()) << "\n";
				connection->send(send->Data(), send->Length());
			});

			connection->run();
		}
	}));
}

int IntegrationTestFixture::Request(const char* method, const char* endpoint, const nlohmann::json* input, nlohmann::json* output)
{
	CURL* curl = curl_easy_init();
	REQUIRE(curl);

	curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	if (!authenticationHeader.empty())
	{
		headers = curl_slist_append(headers, authenticationHeader.c_str());
	}

	char buffer[256];
	int writtenBytes = snprintf(buffer, 256, "https://%s.api.muxy.io/v1/e/%s",
		target.c_str(),
		endpoint);

	REQUIRE(writtenBytes > 0);
	REQUIRE(writtenBytes < 256);

	std::string url = std::string(buffer);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	std::string body;
	if (input)
	{
		body = input->dump();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		std::cerr << "#>   http " << method << " endpoint=" << url << " body=" << body << "\n";
	}
	else
	{
		std::cerr << "#>   http " << method << " endpoint=" << url << "\n";
	}

	std::vector<char> response;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(curl);
	REQUIRE(!res);

	if (output)
	{
		std::string responseString(response.begin(), response.end());
		nlohmann::json parsed = nlohmann::json::parse(responseString);

		*output = parsed;
	}

	int code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

	std::cerr << "#<   http " << method << " recv=" << code << " " << std::string(response.begin(), response.end()) << "\n";

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return code;
}
#endif
