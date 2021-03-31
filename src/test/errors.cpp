#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("SDK Returns False On Invalid JSON", "[sdk][errors]")
{
	gamelink::SDK sdk;
	const char * bytes = "Invalid JSON";
	auto success = sdk.ReceiveMessage(bytes, strlen(bytes));
	REQUIRE(!success);
}
