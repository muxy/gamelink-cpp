#pragma once
#ifndef GAMELINK_MUXY_TEST_UTIL_H
#define GAMELINK_MUXY_TEST_UTIL_H

#include "catch2/catch.hpp"
#include "nlohmann/json.hpp"
#include "constrained_string.h"
#include <iostream>

template<typename T>
void SerializeEqual(const T& v, const std::string& in)
{
    nlohmann::json expected = nlohmann::json::parse(in);
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
    nlohmann::json input = nlohmann::json::parse(in);
    nlohmann::json expected = nlohmann::json::parse(expect);
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
    nlohmann::json input = nlohmann::json::parse(in.c_str());
    nlohmann::json expected = nlohmann::json::parse(expect.c_str());
    if (input != expected)
    {
        std::cerr << "Mismatch: (input)\n";
        std::cerr << input.dump(2) << "\n===== (expected): \n";
        std::cerr << expected.dump(2) << "\n\n";
        return false;
    }

    return true;
}
#endif