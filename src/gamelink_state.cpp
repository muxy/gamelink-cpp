#include "gamelink.h"

namespace gamelink
{
	uint32_t SDK::OnStateUpdate(std::function<void(const schema::SubscribeStateUpdateResponse<nlohmann::json>&)> callback)
	{
		return _onStateUpdate.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	uint32_t SDK::OnStateUpdate(void (*callback)(void*, const schema::SubscribeStateUpdateResponse<nlohmann::json>&), void* ptr)
	{
		return _onStateUpdate.set(callback, ptr, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
	}

	void SDK::DetachOnStateUpdate(uint32_t id)
	{
		if (_onStateUpdate.validateId(id))
		{
			_onStateUpdate.remove(id);
		}
		else
		{
			_onDebugMessage.invoke("Invalid ID passed into OnStateUpdate");
		}
	}

	RequestId SDK::SetState(const char* target, const nlohmann::json& value)
	{
		schema::SetStateRequest<nlohmann::json> payload(target, value);
		return queuePayload(payload);
	}

	RequestId SDK::GetState(const char* target)
	{
		schema::GetStateRequest payload(target);
		return queuePayload(payload);
	}

	RequestId SDK::GetState(const char* target, std::function<void(const schema::GetStateResponse<nlohmann::json>&)> callback)
	{
		schema::GetStateRequest payload(target);
		RequestId id = queuePayload(payload);
		_onGetState.set(callback, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::GetState(const char* target, void (*callback)(void*, const schema::GetStateResponse<nlohmann::json>&), void* user)
	{
		schema::GetStateRequest payload(target);

		RequestId id = queuePayload(payload);
		_onGetState.set(callback, user, id, detail::CALLBACK_ONESHOT);
		return id;
	}

	RequestId SDK::SubscribeToStateUpdates(const char* target)
	{
		schema::SubscribeStateRequest payload(target);
		return queuePayload(payload);
	}

	RequestId SDK::UpdateState(const char* target, const string& operation, const string& path, const schema::JsonAtom& atom)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = atom;

		return UpdateState(target, &op, &op + 1);
	}

	RequestId SDK::UpdateState(const char* target, const schema::PatchOperation* begin, const schema::PatchOperation* end)
	{
		schema::PatchStateRequest payload(target);
		std::vector<schema::PatchOperation> updates;
		updates.resize(end - begin);
		std::copy(begin, end, updates.begin());

		payload.data.state = std::move(updates);
		return queuePayload(payload);
	};
}