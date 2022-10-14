#pragma once
#ifdef MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS
#include <curl/curl.h>
#include "catch2/catch.hpp"
#include "nlohmann/json.hpp"
#include <iostream>
#include <sstream>
#include <string>

const char * IntegrationBaseUrl = "https://sandbox.api.muxy.io/v1/e/";

inline size_t writeFunction(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	std::vector<char>* resp = reinterpret_cast<std::vector<char>*>(userdata);
	resp->insert(resp->end(), ptr, ptr + size * nmemb);

	return size * nmemb;
}

// Overloads to handle nlohmann::json values and generic structs.
inline std::string getBody(const nlohmann::json* value)
{
	return value->dump();
}

template<typename T>
std::string getBody(const T* value)
{
	using nlohmann::to_json;

	nlohmann::json js;
	to_json(js, *value);

	return js.dump();
}

// Overloads to handle nlohmann::json values and generic structs.
inline void writeOutput(nlohmann::json* out, nlohmann::json* parsed)
{
	*out = *parsed;
}

template<typename T>
void writeOutput(T* out, nlohmann::json* parsed)
{
	using nlohmann::from_json;
	from_json(*parsed, *out);
}

template<typename Input, typename Output>
bool request(const char* method, const char* endpoint, const char* authHeader, const Input* input, Output* output)
{
	CURL* curl = curl_easy_init();
	REQUIRE(curl);

	curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	if (authHeader)
	{
		headers = curl_slist_append(headers, authHeader);
	}

	std::stringstream ss;
	ss << IntegrationBaseUrl << endpoint;

	std::string url = ss.str();

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	if (input)
	{
		std::string body = getBody(input);
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

		writeOutput(output, &parsed);
	}

	std::cerr << "#<   http " << method << " recv=" << std::string(response.begin(), response.end()) << "\n";

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return 0;
}
#endif