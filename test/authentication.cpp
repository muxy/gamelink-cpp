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

TEST_CASE("SDK doesn't authenticate on error", "[sdk][authentication]")
{
	gamelink::SDK sdk;

	REQUIRE(!sdk.IsAuthenticated());
	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"errors": [{
			"number": 403,
			"title": "Not authorized",
			"detail": "You're not authorized"
		}]
	})";

	bool ok = sdk.ReceiveMessage(msg, strlen(msg));
	REQUIRE(ok);
	REQUIRE(!sdk.IsAuthenticated());
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
	validateSinglePayload(sdk, R"({
		"action":"authenticate",
		"data": {
			"client_id":"client_id",
			"pin":"pin"
		},

		"params":{
			"request_id": 1
		}
	})");

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

	sdk.Deauthenticate();
	REQUIRE(!sdk.IsAuthenticated());
}

TEST_CASE("SDK Authentication callback", "[sdk][authentication]")
{
	gamelink::SDK sdk;
	std::string jwt = "test-jwt";

	// Test initial state
	REQUIRE(!sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser() == NULL);
	REQUIRE(!sdk.HasPayloads());

	uint32_t calls = 0;
	sdk.OnAuthenticate().Add([&](const gamelink::schema::AuthenticateResponse& resp)
	{
		calls++;
	});

	// Verify generated auth request
	sdk.AuthenticateWithPIN("client_id", "pin", [&](const gamelink::schema::AuthenticateResponse& resp) {
		REQUIRE(sdk.IsAuthenticated());
		REQUIRE(resp.data.jwt == "test-jwt");
		calls++;
	});

	REQUIRE(sdk.HasPayloads());
	validateSinglePayload(sdk, R"({
		"action":"authenticate",
		"data": {
			"client_id":"client_id",
			"pin":"pin"
		},

		"params":{
			"request_id": 1
		}
	})");

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

	REQUIRE(calls == 2);

	sdk.Deauthenticate();
	REQUIRE(!sdk.IsAuthenticated());
}

TEST_CASE("SDK Gateway auth", "[sdk][authentication]")
{
	{
		gamelink::SDK sdk;
		sdk.AuthenticateWithGameIDAndPIN("client_id", "game", "pin");

		REQUIRE(sdk.HasPayloads());
		validateSinglePayload(sdk, R"({
			"action":"authenticate",
			"data": {
				"client_id":"client_id",
				"pin":"pin",
				"game_id": "game"
			},

			"params":{
				"request_id": 1
			}
		})");
	}

	{
		gamelink::SDK sdk;
		sdk.AuthenticateWithGameIDAndRefreshToken("client_id", "game", "refresh");

		REQUIRE(sdk.HasPayloads());
		validateSinglePayload(sdk, R"({
			"action":"authenticate",
			"data": {
				"client_id":"client_id",
				"refresh":"refresh",
				"game_id": "game"
			},

			"params":{
				"request_id": 1
			}
		})");
	}

	{
		gamelink::SDK sdk;
		sdk.AuthenticateWithRefreshToken("client_id", "refresh");

		REQUIRE(sdk.HasPayloads());
		validateSinglePayload(sdk, R"({
			"action":"authenticate",
			"data": {
				"client_id":"client_id",
				"refresh":"refresh"
			},

			"params":{
				"request_id": 1
			}
		})");
	}
}


TEST_CASE("SDK Gateway auth fills user", "[sdk][authentication]")
{
	gamelink::SDK sdk;
	sdk.AuthenticateWithGameIDAndPIN("client_id", "game", "pin");

	REQUIRE(sdk.HasPayloads());
	validateSinglePayload(sdk, R"({
		"action":"authenticate",
		"data": {
			"client_id":"client_id",
			"pin":"pin",
			"game_id": "game"
		},

		"params":{
			"request_id": 1
		}
	})");

	const char* msg = R"({
		"meta": {
			"request_id": 1,
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt",
			"refresh": "some-refresh",
			"twitch_name": "notauser",
			"twitch_id": "123456"
		}
	})";

	sdk.ReceiveMessage(msg, strlen(msg));

	const gamelink::schema::User* usr = sdk.GetUser();
	REQUIRE(usr);
	REQUIRE(usr->GetJWT() == "test-jwt");
	REQUIRE(usr->GetRefreshToken() == "some-refresh");
	REQUIRE(usr->GetTwitchName() == "notauser");
	REQUIRE(usr->GetTwitchID() == "123456");
}