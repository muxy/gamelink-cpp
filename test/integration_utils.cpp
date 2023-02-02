#include "integration_utils.h"
#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include <curl/curl.h>
#include "catch2/catch.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <ctime>

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
}

IntegrationTestFixture::~IntegrationTestFixture()
{
	done.store(true);
	if (runner)
	{
		runner->join();
		curl_global_cleanup();
	}
}

void IntegrationTestFixture::Connect()
{
	REQUIRE(jwt.size());
	REQUIRE(client.size());

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

std::string IntegrationTestFixture::Decode64(const std::string& input)
{
	if (input.empty())
	{
		return "";
	}

	std::string copy = input;
	while (copy.size() % 4 != 0)
	{
		copy = copy + "=";
	}

	const std::string Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::array<uint32_t, 256> lut;
	for (size_t i = 0; i < Alphabet.size(); i++)
	{
		lut[Alphabet[i]] = i;
		lut['='] = 0;
	}

	std::vector<char> output;

	// Cut out 4 characters at a time.
	for (size_t i = 0; i < copy.size(); i += 4)
	{
		uint32_t full =
			(lut[copy[i + 0]] << 6 * 3) |
			(lut[copy[i + 1]] << 6 * 2) |
			(lut[copy[i + 2]] << 6 * 1) |
			(lut[copy[i + 3]] << 6 * 0);

		// Decode 3 bytes.
		char a = static_cast<char>((full >> 16) & 0xFF);
		char b = static_cast<char>((full >> 8) & 0xFF);
		char c = static_cast<char>((full >> 0) & 0xFF);

		output.push_back(a);
		output.push_back(b);
		output.push_back(c);
	}

	uint32_t extraBytes = 0;
	if (copy[copy.size() - 1] == '=' && copy[copy.size() - 2] == '=')
	{
		extraBytes = 2;
	}
	else if (copy[copy.size() - 1] == '=')
	{
		extraBytes = 1;
	}

	for (uint32_t i = 0; i < extraBytes; ++i)
	{
		output.pop_back();
	}

	return std::string(output.begin(), output.end());
}

std::string IntegrationTestFixture::Base64(const std::string& input)
{
	const std::string Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	std::vector<char> copy(input.begin(), input.end());

	// Add bytes
	size_t addedBytes = 0;
	while (copy.size() % 3 != 0)
	{
		copy.push_back(0);
		addedBytes++;
	}

	std::vector<char> output;

	// Encode 3 bytes at a time, left to right.
	for (size_t i = 0; i < copy.size(); i += 3)
	{
		uint32_t full =
			static_cast<uint32_t>(copy[i + 0]) << 16 |
			static_cast<uint32_t>(copy[i + 1]) << 8 |
			static_cast<uint32_t>(copy[i + 2]) << 0;

		// Cut out 6 bits at a time.
		uint8_t a = (full >> 6 * 3) & 0x3F;
		uint8_t b = (full >> 6 * 2) & 0x3F;
		uint8_t c = (full >> 6 * 1) & 0x3F;
		uint8_t d = (full >> 6 * 0) & 0x3F;

		output.push_back(Alphabet[a]);
		output.push_back(Alphabet[b]);
		output.push_back(Alphabet[c]);
		output.push_back(Alphabet[d]);
	}

	// Pad output with =s
	for (size_t i = 0; i < addedBytes; ++i)
	{
		output[output.size() - 1 - i] = '=';
	}

	return std::string(output.begin(), output.end());
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

	const char * clientEnv = std::getenv("MUXY_INTEGRATION_ID");
	if (clientEnv)
	{
		client = clientEnv;
	}

	target = "sandbox";
	targetDomain = "sandbox.muxy.io";
	const char * targetEnv = std::getenv("MUXY_INTEGRATION_TARGET");
	if (targetEnv)
	{
		if (std::string(targetEnv) != "production")
		{
			target = std::string(targetEnv);
			targetDomain = target + ".muxy.io";
		} else
		{
			target = "production";
			targetDomain = "muxy.io";
		}
	}
	REQUIRE(targetDomain.size());

	targetPrefix = "";
	const char * prefixEnv = std::getenv("MUXY_INTEGRATION_OLD_URLS");
	if (prefixEnv && std::string(prefixEnv) == "true")
	{
		targetPrefix = target + ".";
		targetDomain = "muxy.io";
	}

	signature = "";
	const char * signatureEnv = std::getenv("MUXY_SIGNATURE");
	if (signatureEnv)
	{
		signature = signatureEnv;
	}
}

void IntegrationTestFixture::Reconnect()
{
	ForceDisconnect();

	char buffer[256];
	int output = snprintf(buffer, 256, "%sgamelink.%s/%d.%d.%d/%s",
		targetPrefix.c_str(),
		targetDomain.c_str(),
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
	int writtenBytes = snprintf(buffer, 256, "https://%sapi.%s/v1/e/%s",
		targetPrefix.c_str(),
		targetDomain.c_str(),
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

int IntegrationTestFixture::RequestURL(const char* method, const char* url, nlohmann::json* output)
{
	CURL* curl = curl_easy_init();
	REQUIRE(curl);

	std::vector<char> response;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	std::cerr << "#>   http " << method << " url=" << url << "\n";

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	curl_easy_setopt(curl, CURLOPT_URL, url);

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

	curl_easy_cleanup(curl);
	return code;
}

#endif
