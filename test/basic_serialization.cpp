#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include "schema/authentication.h"

TEST_CASE("Can deserialize with missing properties", "[deserialize]")
{
	gamelink::schema::AuthenticateResponse resp;
	Deserialize(R"({
        "meta": {
            "request_id": 152
        },
        "data": {}
    })",
				resp);

	REQUIRE(resp.meta.action == "");
	REQUIRE(resp.meta.request_id == 152);
	REQUIRE(resp.meta.target == "");
	REQUIRE(resp.meta.timestamp == 0);
	REQUIRE(resp.data.jwt == "");
}

TEST_CASE("Can deserialize with empty body", "[deserialize]")
{
	gamelink::schema::ReceiveEnvelope<gamelink::schema::EmptyBody> resp;
	Deserialize(R"({ })", resp);
}

TEST_CASE("Can deserialize with ok body", "[deserialize]")
{
	gamelink::schema::ReceiveEnvelope<gamelink::schema::OKResponseBody> resp;
	Deserialize(R"({ 
        "data": {
            "ok": true
        }
    })",
				resp);

	REQUIRE(resp.data.ok == true);
}

TEST_CASE("Can deserialize errors array", "[deserialize]")
{
	gamelink::schema::AuthenticateResponse resp;
	Deserialize(R"({
        "meta": {
            "request_id": 152
        },
        "errors": [{
            "number": 403, 
            "title": "Unauthorized", 
            "detail": "Bad authentication attempt"
        }]
    })",
				resp);

	REQUIRE(resp.meta.action == "");
	REQUIRE(resp.meta.request_id == 152);
	REQUIRE(resp.meta.target == "");
	REQUIRE(resp.meta.timestamp == 0);
	REQUIRE(resp.errors.size() == 1);
	REQUIRE(resp.errors[0].number == 403);
	REQUIRE(resp.errors[0].title == "Unauthorized");
	REQUIRE(resp.errors[0].detail == "Bad authentication attempt");
}