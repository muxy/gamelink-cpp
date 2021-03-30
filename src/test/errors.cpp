#include "catch2/catch.hpp"
#include "util.h"

#include "../gamelink.hpp"

TEST_CASE("SDK Returns False On Invalid JSON", "[sdk][errors]")
{
	gamelink::SDK sdk;
	auto success = sdk.ReceiveMessage("Invalid JSON");
	REQUIRE(!success);
}
