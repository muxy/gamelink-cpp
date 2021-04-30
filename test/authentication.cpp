#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

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
    })",
				resp);

	REQUIRE(resp.meta.action == "authenticate");
	REQUIRE(resp.meta.request_id == 152);
	REQUIRE(resp.meta.target == "");
	REQUIRE(resp.meta.timestamp == 1583777666077);
	REQUIRE(resp.data.jwt == "eyJhbG...");
}

TEST_CASE("SDK PIN Authentication", "[sdk][authentication][pin]")
{
	gamelink::SDK sdk;
	std::string jwt = "test-jwt";

	// Test initial state
	REQUIRE(!sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser() == NULL);
	REQUIRE(!sdk.HasPayloads());

	// Verify generated auth request
	uint32_t calls = 0;
	sdk.AuthenticateWithPIN("client_id", "pin", [&](const gamelink::schema::AuthenticateResponse& resp) {
		REQUIRE(sdk.IsAuthenticated());
		REQUIRE(resp.data.jwt == "test-jwt");
		calls++;
	});

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(
			JSONEquals(send->data, R"({"action":"authenticate","data":{"client_id":"client_id","pin":"pin"},"params":{"request_id":1}})"));
	});

	REQUIRE(!sdk.HasPayloads());

	// Verify state after successful auth
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt"
		}
	})";

	sdk.ReceiveMessage(msg, strlen(msg));

	REQUIRE(sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser()->GetJWT() == gamelink::string(jwt.c_str()));
	REQUIRE(!sdk.HasPayloads());
	REQUIRE(calls == 1);
}

TEST_CASE("SDK JWT Authentication", "[sdk][authentication][jwt]")
{
	gamelink::SDK sdk;
	std::string jwt = "test-jwt";

	// Test initial state
	REQUIRE(!sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser() == NULL);
	REQUIRE(!sdk.HasPayloads());

	// Verify generated auth request
	uint32_t calls = 0;
	sdk.AuthenticateWithJWT("client_id", jwt.c_str(), [&](const gamelink::schema::AuthenticateResponse& resp) {
		REQUIRE(resp.data.jwt == "test-jwt");
		calls++;
	});

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({"action":"authenticate","data":{"client_id":"client_id","jwt":"test-jwt"},"params":{"request_id":1}})"));
	});

	REQUIRE(!sdk.HasPayloads());

	// Verify state after successful auth
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt"
		}
	})";
	sdk.ReceiveMessage(msg, strlen(msg));

	REQUIRE(sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser()->GetJWT() == gamelink::string(jwt.c_str()));
	REQUIRE(!sdk.HasPayloads());
}
