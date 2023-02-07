#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

TEST_CASE("SDK stores its client ID", "[sdk]")
{
	gamelink::SDK sdk;
	sdk.AuthenticateWithPIN("cool-game-id", "foobar");
	REQUIRE(gamelink::string(sdk.GetClientId()) == gamelink::string("cool-game-id"));
}

TEST_CASE("Connection URL generation", "[sdk]")
{
	gamelink::string sbox = WebsocketConnectionURL("client", gamelink::ConnectionStage::Sandbox);
	gamelink::string prod = WebsocketConnectionURL("client", gamelink::ConnectionStage::Production);

	char buffer[1024];
	snprintf(buffer, 1024, "gamelink.muxy.io/%d.%d.%d/client", MUXY_GAMELINK_VERSION_MAJOR, MUXY_GAMELINK_VERSION_MINOR,
			 MUXY_GAMELINK_VERSION_PATCH);
	gamelink::string expected(buffer);
	REQUIRE(prod == expected);

	snprintf(buffer, 1024, "gamelink.sandbox.muxy.io/%d.%d.%d/client", MUXY_GAMELINK_VERSION_MAJOR, MUXY_GAMELINK_VERSION_MINOR,
			 MUXY_GAMELINK_VERSION_PATCH);
	expected = gamelink::string(buffer);
	REQUIRE(sbox == expected);
}

TEST_CASE("Connection URL generation too big", "[sdk]")
{
	char buffer[1024];
	for (uint32_t i = 0; i < 250; ++i)
	{
		buffer[i] = 'A';
	}
	buffer[250] = '\0';

	gamelink::string toobig(buffer);

	gamelink::string empty = WebsocketConnectionURL(toobig, gamelink::ConnectionStage::Sandbox);
	REQUIRE(empty == "");
}

TEST_CASE("Connection URL generation invalid", "[sdk]")
{
	gamelink::string empty = WebsocketConnectionURL("foobar", static_cast<gamelink::ConnectionStage>(42));
	REQUIRE(empty == "");
}

TEST_CASE("ProjectionConnection URL generation", "[sdk]")
{
	gamelink::string sbox = ProjectionWebsocketConnectionURL("client", gamelink::ConnectionStage::Sandbox, "csharp", 1, 2, 3);
	gamelink::string prod = ProjectionWebsocketConnectionURL("client", gamelink::ConnectionStage::Production, "python", 6, 7, 8);

	char buffer[1024];
	snprintf(buffer, 1024, "gamelink.muxy.io/%d.%d.%d/python/6.7.8/client", MUXY_GAMELINK_VERSION_MAJOR, MUXY_GAMELINK_VERSION_MINOR,
			 MUXY_GAMELINK_VERSION_PATCH);
	gamelink::string expected(buffer);
	REQUIRE(prod == expected);

	snprintf(buffer, 1024, "gamelink.sandbox.muxy.io/%d.%d.%d/csharp/1.2.3/client", MUXY_GAMELINK_VERSION_MAJOR,
			 MUXY_GAMELINK_VERSION_MINOR, MUXY_GAMELINK_VERSION_PATCH);
	expected = gamelink::string(buffer);
	REQUIRE(sbox == expected);
}

TEST_CASE("ProjectionConnection URL generation too big", "[sdk]")
{
	char buffer[1024];
	for (uint32_t i = 0; i < 250; ++i)
	{
		buffer[i] = 'A';
	}
	buffer[250] = '\0';

	gamelink::string toobig(buffer);

	gamelink::string empty = ProjectionWebsocketConnectionURL(toobig, gamelink::ConnectionStage::Sandbox, "lua", 0, 0, 0);
	REQUIRE(empty == "");
}

TEST_CASE("ProjectionConnection URL generation invalid", "[sdk]")
{
	gamelink::string empty = ProjectionWebsocketConnectionURL("foobar", static_cast<gamelink::ConnectionStage>(42), "rust", 4, 3, 2);
	REQUIRE(empty == "");

	empty = ProjectionWebsocketConnectionURL("foobar", gamelink::ConnectionStage::Production, "rust", -10, 4, 2);
	REQUIRE(empty == "");
}

TEST_CASE("C-style foreach payload is the same as callback style", "[sdk]")
{
	std::vector<std::string> lines;

	{
		gamelink::SDK sdk;
		sdk.AuthenticateWithPIN("gameid", "foobar");
		sdk.GetState(gamelink::StateTarget::Channel);
		sdk.CreatePoll("jellybeans", "How many jelly beans in the jar?", {"1", "100"});
		sdk.ForeachPayload([&](const gamelink::Payload* payload) { lines.push_back(std::string(payload->Data(), payload->Length())); });
	}

	std::vector<std::string> clines;
	{
		gamelink::SDK sdk;
		sdk.AuthenticateWithPIN("gameid", "foobar");
		sdk.GetState(gamelink::StateTarget::Channel);
		sdk.CreatePoll("jellybeans", "How many jelly beans in the jar?", {"1", "100"});

		sdk.ForeachPayload(
			[](void* data, const gamelink::Payload* payload) {
				std::vector<std::string>* lines = static_cast<std::vector<std::string>*>(data);
				lines->push_back(std::string(payload->Data(), payload->Length()));
			},
			&clines);
	}

	REQUIRE(lines.size() == clines.size());
	for (size_t i = 0; i < lines.size(); ++i)
	{
		REQUIRE(lines[i] == clines[i]);
	}
}

TEST_CASE("Computing the poll winner index", "[sdk]")
{
	{
		std::vector<int> counts = {1, 10, 0};
		uint32_t idx = gamelink::GetPollWinnerIndex(counts);
		REQUIRE(idx == 1);
	}

	{
		std::vector<int> counts = {0, 0, 0, 0};
		uint32_t idx = gamelink::GetPollWinnerIndex(counts);

		REQUIRE(idx == 0);
	}

	{
		std::vector<int> counts = {0, 1, 1, 0};
		uint32_t idx = gamelink::GetPollWinnerIndex(counts);

		REQUIRE(idx == 1);
	}

	{
		std::vector<int> counts = {1, 2, 3, 4};
		uint32_t idx = gamelink::GetPollWinnerIndex(counts);

		REQUIRE(idx == 3);
	}

	{
		std::vector<int> counts = {4, 3, 2, 1};
		uint32_t idx = gamelink::GetPollWinnerIndex(counts);

		REQUIRE(idx == 0);
	}
}