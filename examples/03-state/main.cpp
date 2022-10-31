#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#define GAMELINK_DEBUG 1
#define MUXY_GAMELINK_SINGLE_IMPL

#include "gamelink_single.hpp"

#include "load_configuration.h"
#include "websocket_network/websocket.h"

struct Spell
{
	std::string name;
	int level;

	MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(Spell, "name", name, "level", level);
};

struct Character
{
	std::string name;
	std::string characterClass;

	int str;
	int con;
	int dex;

	int intellect;
	int wis;
	int cha;

	std::vector<Spell> spellbook;

	MUXY_GAMELINK_SERIALIZE_INTRUSIVE_9(Character,
										"name",
										name,
										"characterClass",
										characterClass,
										"str",
										str,
										"con",
										con,
										"dex",
										dex,
										"int",
										intellect,
										"wis",
										wis,
										"cha",
										cha,
										"spellbook",
										spellbook);
};

int main()
{
	examples::Configuration cfg = examples::LoadConfiguration();
	gamelink::SDK sdk;

	// Set up network connection
	std::string url = gamelink::WebsocketConnectionURL(cfg.clientID, gamelink::ConnectionStage::Sandbox);
	WebsocketConnection websocket(url, 80);
	websocket.onMessage([&](const char* bytes, uint32_t len) { sdk.ReceiveMessage(bytes, len); });

	// Setup the debug logger.
	sdk.OnDebugMessage([](const std::string& str) { std::cerr << "!    " << str << "\n"; });

	bool done = false;
	std::string refresh = examples::LoadRefresh();

	sdk.AuthenticateWithRefreshToken(cfg.clientID, refresh);

	Character gandalf;
	gandalf.name = "Gandalf";
	gandalf.characterClass = "Wizard";
	gandalf.str = 10;
	gandalf.con = 8;
	gandalf.dex = 12;
	gandalf.intellect = 18;
	gandalf.wis = 18;
	gandalf.cha = 16;

	gandalf.spellbook.push_back({"Light", 0});
	gandalf.spellbook.push_back({"Shatter", 1});
	gandalf.spellbook.push_back({"Speak With Animals", 1});

	sdk.SetState(gamelink::StateTarget::Channel, gandalf);

	// Level up int
	sdk.UpdateStateWithInteger(gamelink::StateTarget::Channel, gamelink::Operation::Replace, "/int", 19);

	// Learn a useful spell
	Spell fireball { "Fireball", 3 };
	sdk.UpdateStateWithObject(gamelink::StateTarget::Channel, gamelink::Operation::Add, "/spellbook/3", fireball);

	// Actually, learn lightning bolt.
	Spell lightningBolt { "Lightning Bolt", 3 };
	sdk.UpdateStateWithObject(gamelink::StateTarget::Channel, gamelink::Operation::Replace, "/spellbook/3", lightningBolt);

	// Get state now:
	sdk.GetState(gamelink::StateTarget::Channel, [](const gamelink::schema::GetStateResponse<nlohmann::json>& state) {
		std::cout << " Got state: " << state.data.state["name"] << " has " << state.data.state["int"] << " int\n";
	});

	// Subscribe to state
	sdk.OnStateUpdate().Add([&](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& state) {
		std::cout << " Got a state update: " << state.data.state["name"] << " has " << state.data.state["int"] << " int\n";
		done = true;
	});

	sdk.SubscribeToStateUpdates(gamelink::StateTarget::Channel);

	// Forget a spell
	sdk.UpdateStateWithNull(gamelink::StateTarget::Channel, gamelink::Operation::Remove, "/spellbook/1");

	while (!done)
	{
		sdk.ForeachPayload([&](const gamelink::Payload* send) { websocket.send(send->Data(), send->Length()); });
		websocket.run();
	}

	return 0;
}
