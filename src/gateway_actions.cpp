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

	struct ActionSetMaxCount
	{
		string ID;
		int Count; 

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ActionSetMaxCount, "id", ID, "count", Count);
	};

	void SDK::SetActionMaximumCount(const string& id, int32_t count)
	{
		gamelink::schema::SendEnvelope<ActionSetMaxCount> msg;
		if (count < -1)
		{
			Base.InvokeOnDebugMessage("Invalid count passed into SDK::SetActionMaximumInventory: count must be greater than zero or ACTION_INFINITE_USES");
			return;
		}

		msg.action = "set";
		msg.params.target = "gatewayActionState";
		msg.data.ID = id;
		msg.data.Count = count;

		Base.queuePayload(msg);
	}

	struct ActionSetCount
	{
		string ID;
		int Inventory;

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ActionSetCount, "id", ID, "inventory", Inventory);
	};

	void SDK::SetActionCount(const string& id, int32_t inventory)
	{
		gamelink::schema::SendEnvelope<ActionSetCount> msg;
		if (inventory < -1)
		{
			Base.InvokeOnDebugMessage("Invalid count passed into SDK::SetActionInventory: inventory must be greater than zero or ACTION_INFINITE_USES");
			return;
		}

		msg.action = "set";
		msg.params.target = "gatewayActionState";
		msg.data.ID = id;
		msg.data.Inventory = inventory;

		Base.queuePayload(msg);
	}

	struct DeltaCount
	{
		int Change;

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(DeltaCount, "change", Change);
	};

	struct ActionChangeCount
	{
		string ID;
		DeltaCount Delta;

		MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(ActionChangeCount, "id", ID, "delta_inventory", Delta);
	};

	void SDK::IncrementActionCount(const string& id, int32_t change)
	{
		gamelink::schema::SendEnvelope<ActionChangeCount> msg;
		msg.action = "set";
		msg.params.target = "gatewayActionState";
		msg.data.ID = id;
		msg.data.Delta.Change = change;

		Base.queuePayload(msg);
	}

	void SDK::DecrementActionCount(const string& id, int32_t change)
	{
		gamelink::schema::SendEnvelope<ActionChangeCount> msg;
		
		msg.action = "set";
		msg.params.target = "gatewayActionState";
		msg.data.ID = id;
		msg.data.Delta.Change = -change;

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