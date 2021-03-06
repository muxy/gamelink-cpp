#include "gamelink.h"

namespace gamelink
{
	Event<schema::SubscribeStateUpdateResponse<nlohmann::json> >& SDK::OnStateUpdate()
	{
		return _onStateUpdate;
	}

	RequestId SDK::SetState(StateTarget target, const nlohmann::json& value)
	{
		schema::SetStateRequest<nlohmann::json> payload(target, value);
		return queuePayload(payload);
	}

	RequestId SDK::ClearState(StateTarget target)
	{
		return SetState(target, nlohmann::json::object());
	}

	RequestId SDK::GetState(StateTarget target)
	{
		schema::GetStateRequest payload(target);
		return queuePayload(payload);
	}

	RequestId SDK::GetState(StateTarget target, std::function<void(const schema::GetStateResponse<nlohmann::json>&)> callback)
	{
		schema::GetStateRequest payload(target);
		RequestId id = queuePayload(payload);
		_onGetState.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::GetState(StateTarget target, void (*callback)(void*, const schema::GetStateResponse<nlohmann::json>&), void* user)
	{
		schema::GetStateRequest payload(target);

		RequestId id = queuePayload(payload);
		_onGetState.set(callback, user, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::SubscribeToStateUpdates(StateTarget target)
	{
		if (!IsValidStateTarget(target))
		{
			InvokeOnDebugMessage("SubscribeToStateUpdates: target value out of bounds");
			return ANY_REQUEST_ID;
		}

		if (_subscriptionSets.canRegisterStateUpdate(target))
		{
			schema::SubscribeStateRequest payload(target);
			RequestId req = queuePayload(payload);

			_subscriptionSets.registerStateUpdates(target);
			return req;
		}

		char buffer[128];
		snprintf(buffer, 128, "SubscribeToStateUpdates: duplicated subscription call with target=%s", STATETARGET_STRINGS[static_cast<int>(target)]);
		InvokeOnDebugMessage(buffer);

		return ANY_REQUEST_ID;
	}

	RequestId SDK::UnsubscribeFromStateUpdates(StateTarget target)
	{
		schema::UnsubscribeStateRequest payload(target);
		_subscriptionSets.unregisterStateUpdates(target);

		return queuePayload(payload);
	}

	RequestId SDK::UpdateState(StateTarget target, const schema::PatchOperation* begin, const schema::PatchOperation* end)
	{
		schema::PatchStateRequest payload(target);
		std::vector<schema::PatchOperation> updates;
		updates.resize(end - begin);
		std::copy(begin, end, updates.begin());

		payload.data.state = std::move(updates);
		return queuePayload(payload);
	}

	RequestId SDK::UpdateStateWithInteger(StateTarget target, Operation operation, const string& path, int64_t i)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromInteger(i);

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateStateWithDouble(StateTarget target, Operation operation, const string& path, double d)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromDouble(d);

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateStateWithBoolean(StateTarget target, Operation operation, const string& path, bool b)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromBoolean(b);

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateStateWithString(StateTarget target, Operation operation, const string& path, const string& str)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromString(str);

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateStateWithLiteral(StateTarget target, Operation operation, const string& path, const string& str)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromLiteral(str);

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateStateWithNull(StateTarget target, Operation operation, const string& path)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomNull();

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateStateWithJson(StateTarget target, Operation operation, const string& path, const nlohmann::json& js)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromLiteral(string(js.dump().c_str()));

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateStateWithPatchList(StateTarget target, const PatchList& list)
	{
		if (list.operations.empty())
		{
			return ANY_REQUEST_ID;
		}

		return UpdateState(target, list.operations.data(), list.operations.data() + list.operations.size());
	}

	// PatchList implementation
	PatchList::PatchList()
	{}

	PatchList::PatchList(uint32_t preallocate)
	{
		operations.reserve(preallocate);
	}

	void PatchList::UpdateState(const schema::PatchOperation* begin, const schema::PatchOperation* end)
	{
		lock.lock();;
		for (const schema::PatchOperation* b = begin; b != end; ++b)
		{
			operations.push_back(*b);
		}
		lock.unlock();
	}

	void PatchList::UpdateStateWithInteger(Operation operation, const string& path, int64_t i)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromInteger(i);

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	void PatchList::UpdateStateWithDouble(Operation operation, const string& path, double d)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromDouble(d);

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	void PatchList::UpdateStateWithBoolean(Operation operation, const string& path, bool b)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromBoolean(b);

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	void PatchList::UpdateStateWithString(Operation operation, const string& path, const string& s)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromString(s);

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	void PatchList::UpdateStateWithLiteral(Operation operation, const string& path, const string& js)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromLiteral(js);

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	void PatchList::UpdateStateWithNull(Operation operation, const string& path)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomNull();

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	void PatchList::UpdateStateWithJson(Operation operation, const string& path, const nlohmann::json& js)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromLiteral(string(js.dump().c_str()));

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	void PatchList::UpdateStateWithEmptyArray(Operation operation, const string& path)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromLiteral("[]");

		lock.lock();
		operations.emplace_back(std::move(op));
		lock.unlock();
	}

	bool PatchList::Empty() const
	{
		return this->operations.empty();
	}

	void PatchList::Clear()
	{
		lock.lock();
		this->operations.clear();
		lock.unlock();
	}
}