#include "catch2/catch.hpp"
#include "util.h"

#include "schema/authentication.h"

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

    gamelink::schema::AuthenticateWithPINRequest auth("not-a-client-id", "1234");
    SerializeEqual(auth, R"({
        "action": "authenticate", 
        "params": {
            "request_id": 65535
        },
        "data": {
            "pin": "1234", 
            "client_id": "not-a-client-id"
        }
    })");

    gamelink::schema::AuthenticateWithJWTRequest jwtAuth("not-a-client-id", "eyJhbG...");
    jwtAuth.params.request_id = 42;

    SerializeEqual(jwtAuth, R"({
        "action": "authenticate", 
        "params": {
            "request_id": 42
        },
        "data": {
            "jwt": "eyJhbG...",
            "client_id": "not-a-client-id"
        }
    })");
}

TEST_CASE("Authentication Deserialization", "[auth][deserialization]")
{
    gamelink::schema::AuthenticateResponse resp;
    Deserialize(R"({
        "meta": {
            "request_id": 152, 
            "action": "authenticate", 
            "target": "", 
            "timestamp": 1583777666077
        }, 
        "data": {
            "jwt": "eyJhbG..."
        }
    })", resp);

    REQUIRE(resp.meta.action == "authenticate");
    REQUIRE(resp.meta.request_id == 152);
    REQUIRE(resp.meta.target == "");
    REQUIRE(resp.meta.timestamp == 1583777666077);
    REQUIRE(resp.data.jwt == "eyJhbG...");
}