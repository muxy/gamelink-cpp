#include "gamelink.h"

namespace gamelink
{
    RequestId SDK::GetConfig(ConfigTarget target, std::function<void(const schema::GetConfigResponse&)> callback)
    {
        schema::GetConfigRequest req(target);
        RequestId id = queuePayload(req);

        _onGetConfig.set(callback, id, detail::CALLBACK_ONESHOT);
        return id;
    }

	RequestId SDK::GetConfig(ConfigTarget target, void (*callback)(void*, const schema::GetConfigResponse&), void* user)
    {
        schema::GetConfigRequest req(target);
        RequestId id = queuePayload(req);

        _onGetConfig.set(callback, user, id, detail::CALLBACK_ONESHOT);
        return id;
    }

    RequestId SDK::GetCombinedConfig(std::function<void (const schema::GetCombinedConfigResponse&)> callback)
    {
        schema::GetConfigRequest req(ConfigTarget::Combined);
        RequestId id = queuePayload(req);

        _onGetCombinedConfig.set(callback, id, detail::CALLBACK_ONESHOT);
        return id;
    }

    RequestId SDK::GetCombinedConfig(void (*callback)(void *, const schema::GetCombinedConfigResponse&), void* user)
    {
		schema::GetConfigRequest req(ConfigTarget::Combined);
        RequestId id = queuePayload(req);

        _onGetCombinedConfig.set(callback, user, id, detail::CALLBACK_ONESHOT);
        return id;
    }

    RequestId SDK::SubscribeToConfigurationChanges(ConfigTarget target)
    {
		if (!IsValidConfigTarget(target))
		{
			InvokeOnDebugMessage("SubscribeToConfigurationChanges: target value out of bounds");
			return ANY_REQUEST_ID;
		}

		if (_subscriptionSets.canRegisterConfigurationChanges(target))
		{
			schema::SubscribeToConfigRequest payload(target);
			RequestId req = queuePayload(payload);

			_subscriptionSets.registerConfigurationChanges(target);
			return req;
		}

		char buffer[128];
		snprintf(buffer, 128, "SubscribeToConfigurationChanges: duplicated subscription call with target=%s", TARGET_STRINGS[static_cast<int>(target)]);
		InvokeOnDebugMessage(buffer);

		return ANY_REQUEST_ID;
    }

    RequestId SDK::UnsubscribeFromConfigurationChanges(ConfigTarget target)
    {
        schema::UnsubscribeFromConfigRequest req(target);
		_subscriptionSets.unregisterConfigurationChanges(target);
        return queuePayload(req);
    }

    RequestId SDK::SetChannelConfig(const nlohmann::json& js)
    {
        schema::SetConfigRequest req(js);
        return queuePayload(req);
    }

	Event<schema::ConfigUpdateResponse>& SDK::OnConfigUpdate()
	{
		return _onConfigUpdate;
	}

    RequestId SDK::UpdateChannelConfig(const schema::PatchOperation* begin, const schema::PatchOperation* end)
	{
		schema::PatchConfigRequest payload;
		std::vector<schema::PatchOperation> updates;
		updates.resize(end - begin);
		std::copy(begin, end, updates.begin());

		payload.data.config = std::move(updates);
		return queuePayload(payload);
	}

	RequestId SDK::UpdateChannelConfigWithInteger(Operation operation, const string& path, int64_t i)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromInteger(i);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithDouble(Operation operation, const string& path, double d)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromDouble(d);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithBoolean(Operation operation, const string& path, bool b)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromBoolean(b);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithString(Operation operation, const string& path, const string& str)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromString(str);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithLiteral(Operation operation, const string& path, const string& str)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromLiteral(str);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithNull(Operation operation, const string& path)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomNull();

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithJson(Operation operation, const string& path, const nlohmann::json& js)
	{
		schema::PatchOperation op;
		op.operation = OPERATION_STRINGS[static_cast<int>(operation)];
		op.path = path;
		op.value = schema::atomFromLiteral(string(js.dump().c_str()));

		return UpdateChannelConfig(&op, &op + 1);
	}
}