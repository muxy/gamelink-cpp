#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("Can generate connection URL", "[sdk]")
{
	gamelink::string str = gamelink::WebsocketConnectionURL("maybe-valid-client-id", gamelink::ConnectionStage::Production);
	REQUIRE(str.size() > 0);
}

TEST_CASE("Generation fails on bad inputs", "[sdk]")
{
	gamelink::string str =
		gamelink::WebsocketConnectionURL(gamelink::string("maybe-valid-client-id"), static_cast<gamelink::ConnectionStage>(234));
	REQUIRE(str.size() == 0);

	char bigString[300];
	for (uint32_t i = 0; i < 300; ++i)
	{
		bigString[i] = 'a';
	}
	bigString[299] = '\0';

	str = gamelink::WebsocketConnectionURL(gamelink::string(bigString), gamelink::ConnectionStage::Sandbox);
	REQUIRE(str.size() == 0);
}
