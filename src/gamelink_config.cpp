#include "gamelink.h"

namespace gamelink
{
    RequestId SDK::GetConfig(const char* target, std::function<void(const schema::GetConfigResponse&)> callback)
    {
        schema::GetConfigRequest req(target);
        RequestId id = queuePayload(req);

        _onGetConfig.set(callback, id, detail::CALLBACK_ONESHOT);
        return id;
    }

	RequestId SDK::GetConfig(const char* target, void (*callback)(void *, const schema::GetConfigResponse&), void* user)
    {
        schema::GetConfigRequest req(target);
        RequestId id = queuePayload(req);

        _onGetConfig.set(callback, user, id, detail::CALLBACK_ONESHOT);
        return id;
    }

    RequestId SDK::GetCombinedConfig(std::function<void (const schema::GetCombinedConfigResponse&)> callback)
    {
        schema::GetConfigRequest req("combined");
        RequestId id = queuePayload(req);

        _onGetCombinedConfig.set(callback, id, detail::CALLBACK_ONESHOT);
        return id;
    }

    RequestId SDK::GetCombinedConfig(void (*callback)(void *, const schema::GetCombinedConfigResponse&), void* user)
    {
        schema::GetConfigRequest req("combined");
        RequestId id = queuePayload(req);

        _onGetCombinedConfig.set(callback, user, id, detail::CALLBACK_ONESHOT);
        return id;
    }

    RequestId SDK::SubscribeToConfigurationChanges(const char* target)
    {
        schema::SubscribeToConfigRequest req(target);
        return queuePayload(req);
    }

    RequestId SDK::UnsubscribeFromConfigurationChanges(const char* target)
    {
        schema::UnsubscribeFromConfigRequest req(target);
        return queuePayload(req);
    }

    RequestId SDK::SetChannelConfig(const nlohmann::json& js)
    {
        schema::SetConfigRequest req(js);
        return queuePayload(req);
    }

    uint32_t SDK::OnConfigUpdate(std::function<void(const schema::ConfigUpdateResponse&)> callback)
    {
        return _onConfigUpdate.set(callback, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
    }

    uint32_t SDK::OnConfigUpdate(void (*callback)(void*, const schema::ConfigUpdateResponse&), void* user)
    {
        return _onConfigUpdate.set(callback, user, ANY_REQUEST_ID, detail::CALLBACK_PERSISTENT);
    }

    void SDK::DetachOnConfigUpdate(uint32_t id)
    {
        if (_onConfigUpdate.validateId(id))
        {
            _onConfigUpdate.remove(id);
        }
        else
        {
            _onDebugMessage.invoke("Invalid ID passed into DetachOnConfigUpdate");
        }
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

	RequestId SDK::UpdateChannelConfigWithInteger(const char * operation, const string& path, int64_t i)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = schema::atomFromInteger(i);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithDouble(const char * operation, const string& path, double d)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = schema::atomFromDouble(d);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithBoolean(const char * operation, const string& path, bool b)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = schema::atomFromBoolean(b);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithString(const char * operation, const string& path, const string& str)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = schema::atomFromString(str);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithLiteral(const char * operation, const string& path, const string& str)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = schema::atomFromLiteral(str);

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithNull(const char * operation, const string& path)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = schema::atomNull();

		return UpdateChannelConfig(&op, &op + 1);
	}

	RequestId SDK::UpdateChannelConfigWithJson(const char * operation, const string& path, const nlohmann::json& js)
	{
		schema::PatchOperation op;
		op.operation = operation;
		op.path = path;
		op.value = schema::atomFromLiteral(string(js.dump().c_str()));

		return UpdateChannelConfig(&op, &op + 1);
	}
}