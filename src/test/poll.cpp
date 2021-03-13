#include "catch2/catch.hpp"
#include "util.h"

#include "../gamelink.hpp"

TEST_CASE("SDK Poll Creation", "[sdk][poll][creation]")
{
	gamelink::SDK sdk;

	sdk.CreatePoll("test-poll", "Me or Them?", {"Me", "Them"});

	REQUIRE(sdk.HasSends());

	sdk.ForeachSend([](gamelink::Send* send) {
		REQUIRE(send->data == R"({"action":"create","data":{"options":["Me","Them"],"poll_id":"test-poll","prompt":"Me or Them?"},"params":{"request_id":65535,"target":"poll"}})");
	});

	REQUIRE(!sdk.HasSends());
}
