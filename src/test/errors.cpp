#include "catch2/catch.hpp"
#include "util.h"

#include "../gamelink.hpp"

TEST_CASE("SDK Throws On Invalid JSON Response", "[sdk][errors]")
{
	gamelink::SDK sdk;
	auto caught = false;

	try
	{
		sdk.ReceiveMessage("Invalid JSON");
	}
	catch (const std::exception& ex)
	{
		caught = true;
	}

	REQUIRE(caught);
}
