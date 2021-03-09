#include "catch2/catch.hpp"
#include "nlohmann/json.hpp"

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
