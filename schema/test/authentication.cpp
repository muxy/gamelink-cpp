#include "catch2/catch.hpp"

#include "schema/authentication.h"

template<typename T>
void SerializeEqual(const T& v, const std::string& in)
{
    nlohmann::json parsed = nlohmann::json::parse(in);
    nlohmann::json input; 
    to_json(input, v);

    REQUIRE(input["action"] == parsed["action"]);
    REQUIRE(input["params"]["request_id"] == parsed["params"]["request_id"]);
    REQUIRE(input["params"]["target"] == parsed["params"]["target"]);
    REQUIRE(input["data"] == parsed["data"]);
}

TEST_CASE("Authentication Serialization", "[auth][serialization]")
{
    gamelink::schema::SubscribeAuthenticationRequest req;
    SerializeEqual(req, R"({
        "action": "subscribe", 
        "params": {
            "request_id": 65535, 
            "target": "authentication"
        }
    })");
}