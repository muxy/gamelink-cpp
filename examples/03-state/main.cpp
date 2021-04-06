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
	gamelink::SDK sdk;

	// Set up network connection
	WebsocketConnection websocket("localhost", 5050);
	websocket.onMessage([&](const char* bytes, uint32_t len) { sdk.ReceiveMessage(bytes, len); });

	// Setup the debug logger.
	sdk.OnDebugMessage([](const std::string& str) { std::cerr << "!    " << str << "\n"; });

	bool done = false;
	examples::Configuration cfg = examples::LoadConfiguration();
	std::string jwt = examples::LoadJWT();

	sdk.AuthenticateWithJWT(cfg.clientID, jwt);

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

	sdk.SetState(gamelink::schema::STATE_TARGET_CHANNEL, gandalf);

	// Level up int
	sdk.UpdateState(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/int", gamelink::schema::atomFromInteger(19));

	// Learn a useful spell
	sdk.UpdateState(gamelink::schema::STATE_TARGET_CHANNEL, "add", "/spellbook/-",
					gamelink::schema::atomFromLiteral(R"({ "name": "Fireball", "level": 3 })"));

	// Subscribe to state
	sdk.OnStateUpdate([&](const gamelink::schema::SubscribeStateUpdateResponse<nlohmann::json>& state) {
		std::cout << " Got a state update: " << state.data.state["name"] << " has " << state.data.state["int"] << " int\n";
	});

	sdk.SubscribeToStateUpdates(gamelink::schema::STATE_TARGET_CHANNEL);

	// Forget a spell
	sdk.UpdateState(gamelink::schema::STATE_TARGET_CHANNEL, "remove", "/spellbook/1", gamelink::schema::atomNull());

	sdk.ForeachPayload([&](const gamelink::Payload* send) { websocket.send(send->data.c_str(), send->data.size()); });
	while (!done)
	{
		websocket.run();
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	return 0;
}
