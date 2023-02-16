#include "gateway.h"

namespace gateway
{	
	NLOHMANN_JSON_SERIALIZE_ENUM(ActionCategory, {
		{ ActionCategory::Neutral, "neutral" }, 
		{ ActionCategory::Hinder, "hinder" }, 
		{ ActionCategory::Help, "help" },
	});

	NLOHMANN_JSON_SERIALIZE_ENUM(ActionState, {
		{ ActionState::Unavailable, "unavailable" }, 
		{ ActionState::Available, "available" }, 
		{ ActionState::Hidden, "hidden" },
	});

	MUXY_GAMELINK_SERIALIZE_8(Action, 
		"id", ID, 
		"category", Category, 
		"state", State, 
		"impact", Impact, 
		"name", Name, 
		"description", Description, 
		"icon", Icon, 
		"count", Count);

	struct ActionsList
	{
		std::vector<Action> actions;
		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(ActionsList, "actions", actions);
	};

	void SDK::SetActions(const Action* begin, const Action* end)
	{
		gamelink::schema::SendEnvelope<ActionsList> actionsList;

		actionsList.action = "set";
		actionsList.params.target = "gatewayActions";
		actionsList.data.actions = std::vector<Action>(begin, end);

		Base.queuePayload(actionsList);
	}

	struct ActionSetCount
	{
		string ID;
		int Count; 

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ActionSetCount, "id", ID, "count", Count);
	};

	void SDK::SetActionCount(const string& id, int32_t count)
	{
		gamelink::schema::SendEnvelope<ActionSetCount> msg;
		if (count < -1)
		{
			Base.InvokeOnDebugMessage("Invalid count passed into SDK::SetActionCount: count must be greater than zero or ACTION_INFINITE_USES");
			return;
		}

		msg.action = "set";
		msg.params.target = "gatewayActionState";
		msg.data.ID = id;
		msg.data.Count = count;

		Base.queuePayload(msg);
	}

	struct ActionSetState
	{
		string ID;
		ActionState State; 

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ActionSetState, "id", ID, "state", State);
	};

	void SDK::EnableAction(const string& id)
	{
		gamelink::schema::SendEnvelope<ActionSetState> msg;

		msg.action = "set";
		msg.params.target = "gatewayActionState";
		msg.data.ID = id;
		msg.data.State = ActionState::Available;

		Base.queuePayload(msg);
	}

	void SDK::DisableAction(const string& id)
	{
		gamelink::schema::SendEnvelope<ActionSetState> msg;

		msg.action = "set";
		msg.params.target = "gatewayActionState";
		msg.data.ID = id;
		msg.data.State = ActionState::Unavailable;

		Base.queuePayload(msg);
	}
}