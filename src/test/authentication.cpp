#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("SDK PIN Authentication", "[sdk][authentication][pin]")
{
	gamelink::SDK sdk;
	std::string jwt = "test-jwt";

	// Test initial state
	REQUIRE(!sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser() == NULL);
	REQUIRE(!sdk.HasPayloads());

	// Verify generated auth request
	sdk.AuthenticateWithPIN("client_id", "pin");

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({"action":"authenticate","data":{"client_id":"client_id","pin":"pin"},"params":{"request_id":65535}})"));
	});

	REQUIRE(!sdk.HasPayloads());

	// Verify state after successful auth
	const char * msg = R"({
		"meta": {
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt"
		}
	})";

	sdk.ReceiveMessage(msg, strlen(msg));

	REQUIRE(sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser()->GetJWT() == jwt);
	REQUIRE(!sdk.HasPayloads());
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
	sdk.AuthenticateWithJWT("client_id", jwt);

	REQUIRE(sdk.HasPayloads());

	sdk.ForeachPayload([](const gamelink::Payload* send) {
		REQUIRE(JSONEquals(send->data,
						   R"({"action":"authenticate","data":{"client_id":"client_id","jwt":"test-jwt"},"params":{"request_id":65535}})"));
	});

	REQUIRE(!sdk.HasPayloads());

	// Verify state after successful auth
	const char * msg = R"({
		"meta": {
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt"
		}
	})";
	sdk.ReceiveMessage(msg, strlen(msg));

	REQUIRE(sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser()->GetJWT() == jwt);
	REQUIRE(!sdk.HasPayloads());
}
