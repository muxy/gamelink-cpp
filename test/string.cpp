#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("HasPrefix works correctly", "[sdk]")
{
	REQUIRE(gamelink::HasPrefix(gamelink::string("aaaa"), gamelink::string("")));
	REQUIRE(gamelink::HasPrefix(gamelink::string("this is an ascii string"), gamelink::string("this is")));
	REQUIRE(gamelink::HasPrefix(gamelink::string("(╯°□°）╯︵ ┻━┻)"), gamelink::string("(╯°□°）")));
	REQUIRE(gamelink::HasPrefix(gamelink::string("😍😍😍"), gamelink::string("😍")));
	REQUIRE(gamelink::HasPrefix(gamelink::string("和製漢語"), gamelink::string("和")));

	REQUIRE(!gamelink::HasPrefix(gamelink::string("this is an ascii string"), gamelink::string("thisis")));
	REQUIRE(!gamelink::HasPrefix(gamelink::string("                this is an ascii string"), gamelink::string("this is")));
	REQUIRE(!gamelink::HasPrefix(gamelink::string(""), gamelink::string("thisis")));
	REQUIRE(!gamelink::HasPrefix(gamelink::string("😍😍😍"), gamelink::string("❤️")));
	REQUIRE(!gamelink::HasPrefix(gamelink::string("和製漢語"), gamelink::string("漢")));
}