#include "catch2/catch.hpp"
#include "gamelink.h"

TEST_CASE("Broadcast returns true from ReceiveMessage", "[sdk]")
{
	gamelink::SDK sdk;
	std::string msg = R"({"meta":{"request_id":16,"action":"broadcast","target":"","timestamp":1624574713916905},"data":{"ok":true}})";
	bool ok = sdk.ReceiveMessage(msg.data(), msg.size());

	REQUIRE(ok);
}
