#include "catch2/catch.hpp"
#include "util.h"

#include "../gamelink.hpp"

TEST_CASE("SDK Authentication", "[sdk][authentication]")
{
	gamelink::SDK sdk;
	std::string jwt = "test-jwt";

	// Test initial state
	REQUIRE(!sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser() == NULL);
	REQUIRE(!sdk.HasSends());

	// Verify generated auth request
	sdk.AuthenticateWithPIN("client_id", "pin");

	REQUIRE(sdk.HasSends());

	sdk.ForeachSend([](gamelink::Send* send) {
		REQUIRE(send->data == R"({"action":"authenticate","data":{"client_id":"client_id","pin":"pin"},"params":{"request_id":65535}})");
	});

	REQUIRE(!sdk.HasSends());

	// Verify state after successful auth
	sdk.ReceiveMessage(R"({
		"meta": {
			"action": "authenticate"
		},

		"data": {
			"jwt": "test-jwt"
		}
	})");

	REQUIRE(sdk.IsAuthenticated());
	REQUIRE(sdk.GetUser()->GetJWT() == jwt);
	REQUIRE(!sdk.HasSends());
}
