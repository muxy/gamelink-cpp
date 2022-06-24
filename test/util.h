#pragma once
#ifndef GAMELINK_MUXY_TEST_UTIL_H
#define GAMELINK_MUXY_TEST_UTIL_H

#include "catch2/catch.hpp"
#include "constrained_types.h"
#include "gamelink.h"
#include "nlohmann/json.hpp"
#include <iostream>

template<typename T>
void SerializeEqual(const T& v, const std::string& in)
{
	nlohmann::json expected = nlohmann::json::parse(in, nullptr, false);
	REQUIRE(!expected.is_discarded());

	nlohmann::json input;
	to_json(input, v);

	REQUIRE(input["action"] == expected["action"]);
	REQUIRE(input["params"]["request_id"] == expected["params"]["request_id"]);

	if (expected["params"]["target"].is_string())
	{
		REQUIRE(input["params"]["target"] == expected["params"]["target"]);
	}

	REQUIRE(input["data"] == expected["data"]);
}

template<typename T>
void Deserialize(const std::string& in, T& out)
{
	nlohmann::json input = nlohmann::json::parse(in);
	from_json(input, out);
}

inline bool JSONEquals(const std::string& in, const std::string& expect)
{
	nlohmann::json input = nlohmann::json::parse(in, nullptr, false);
	REQUIRE(!input.is_discarded());

	nlohmann::json expected = nlohmann::json::parse(expect, nullptr, false);
	REQUIRE(!expected.is_discarded());

	// Ignore request_id parameter
	if (expected["params"].contains("request_id") && expected["params"]["request_id"] == 0xFFFF)
	{
		if (input["params"].contains("request_id"))
		{
			input["params"]["request_id"] = 0xFFFF;
		}
	}

	if (input != expected)
	{
		std::cerr << "Mismatch: (input)\n";
		std::cerr << input.dump(2) << "\n===== (expected): \n";
		std::cerr << expected.dump(2) << "\n\n";
		return false;
	}

	return true;
}

inline bool JSONEquals(const ConstrainedString& in, const ConstrainedString& expect)
{
	nlohmann::json input = nlohmann::json::parse(in.c_str(), nullptr, false);
	REQUIRE(!input.is_discarded());

	nlohmann::json expected = nlohmann::json::parse(expect.c_str(), nullptr, false);
	REQUIRE(!expected.is_discarded());

	// Ignore request_id parameter
	if (expected["params"].contains("request_id") && expected["params"]["request_id"] == 0xFFFF)
	{
		if (input["params"].contains("request_id"))
		{
			input["params"]["request_id"] = 0xFFFF;
		}
	}

	if (input != expected)
	{
		std::cerr << "Mismatch: (input)\n";
		std::cerr << input.dump(2) << "\n===== (expected): \n";
		std::cerr << expected.dump(2) << "\n\n";
		return false;
	}

	return true;
}

inline bool JSONEquals(const gamelink::Payload& in, const ConstrainedString& expect)
{
	ConstrainedString str(in.Data());

	return JSONEquals(str, expect);
}

inline void validateSinglePayload(gamelink::SDK& sdk, const std::string& p)
{
	REQUIRE(sdk.HasPayloads());

	uint32_t count = 0;
	sdk.ForeachPayload([p, &count](const gamelink::Payload* payload) {
		ConstrainedString str(reinterpret_cast<const char *>(payload->Data()));
		REQUIRE(JSONEquals(str, ConstrainedString(p.c_str())));
		count++;
	});

	REQUIRE(count == 1);
	REQUIRE(!sdk.HasPayloads());
}
#endif