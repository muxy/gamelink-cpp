#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"
#include "gamelink_c.h"
#include <iostream>

struct ParityFixture
{
    ParityFixture()
    {
        c = MuxyGameLink_Make();
    }

    ~ParityFixture()
    {
        EnsureParity();
        MuxyGameLink_Kill(c);
    }

    void EnsureParity()
    {
        std::vector<std::string> cppPayloads;
        cpp.ForeachPayload([&](const gamelink::Payload* payload)
        {
            cppPayloads.push_back(payload->data.c_str());
        });

        std::vector<std::string> cPayloads;
        MuxyGameLink_ForeachPayload(c, [](void* ptr, MGL_Payload payload)
        {
            std::vector<std::string> * payloads = static_cast<std::vector<std::string>*>(ptr);
            payloads->push_back(
                std::string(
                    MuxyGameLink_Payload_GetData(payload),
                    MuxyGameLink_Payload_GetSize(payload)));
        }, &cPayloads);

        REQUIRE(cppPayloads.size() == cPayloads.size());
        for (size_t i = 0; i < cppPayloads.size(); ++i)
        {
            REQUIRE(JSONEquals(cppPayloads[i], cPayloads[i]));
        }
    }

    MuxyGameLink c;
    gamelink::SDK cpp;
};

struct NoopCallback
{
    template<typename T>
    void operator()(const T& t)
    {}
};

template<typename T>
struct CallbackSecondParameterType
{};

template<typename T>
struct CallbackSecondParameterType<void (*)(void*, T)>
{
    typedef T type;
};

struct NoopCCallback
{
    template<typename T>
    static void invoke(void*, T)
    {}

    template<typename T>
    operator T () const
    {
        return &NoopCCallback::invoke< typename CallbackSecondParameterType<T>::type >;
    }
};

// Generic noop callback.
#define C_NOOP() NoopCCallback()
#define CPP_NOOP() NoopCallback()

TEST_CASE_METHOD(ParityFixture, "Authentication parity", "[sdk][c]")
{
    cpp.AuthenticateWithPIN("client-id", "pin123",  CPP_NOOP());
    MuxyGameLink_AuthenticateWithPIN(c, "client-id", "pin123", C_NOOP(), nullptr);

    cpp.AuthenticateWithRefreshToken("client-id", "refresh-token", CPP_NOOP());
    MuxyGameLink_AuthenticateWithRefreshToken(c, "client-id", "refresh-token", C_NOOP(), nullptr);
}

TEST_CASE_METHOD(ParityFixture, "State operation parity", "[sdk][c]")
{
    const char * state = R"({ "hello": "world" })";
    cpp.SetState(gamelink::schema::STATE_TARGET_CHANNEL, nlohmann::json::parse(state));
    MuxyGameLink_SetState(c, STATE_TARGET_CHANNEL, state);

    cpp.GetState(gamelink::schema::STATE_TARGET_CHANNEL);
    MuxyGameLink_GetState(c, STATE_TARGET_CHANNEL, C_NOOP(), nullptr);

    cpp.GetState(gamelink::schema::STATE_TARGET_EXTENSION);
    MuxyGameLink_GetState(c, STATE_TARGET_EXTENSION, C_NOOP(), nullptr);

    cpp.SubscribeToStateUpdates(gamelink::schema::STATE_TARGET_CHANNEL);
    cpp.SubscribeToStateUpdates(gamelink::schema::STATE_TARGET_EXTENSION);
    MuxyGameLink_SubscribeToStateUpdates(c, STATE_TARGET_CHANNEL);
    MuxyGameLink_SubscribeToStateUpdates(c, STATE_TARGET_EXTENSION);

    cpp.UnsubscribeFromStateUpdates(gamelink::schema::STATE_TARGET_EXTENSION);
    cpp.UnsubscribeFromStateUpdates(gamelink::schema::STATE_TARGET_CHANNEL);
    MuxyGameLink_UnsubscribeFromStateUpdates(c, STATE_TARGET_EXTENSION);
    MuxyGameLink_UnsubscribeFromStateUpdates(c, STATE_TARGET_CHANNEL);

    cpp.UpdateStateWithInteger(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/matter", 42);
    cpp.UpdateStateWithDouble(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/matter", 12.5);
    cpp.UpdateStateWithString(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/matter", "123456");
    cpp.UpdateStateWithLiteral(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/matter", "[1, 2, 3]");
    cpp.UpdateStateWithNull(gamelink::schema::STATE_TARGET_CHANNEL, "replace", "/matter");

    MuxyGameLink_UpdateStateWithInteger(c, STATE_TARGET_CHANNEL, "replace", "/matter", 42);
    MuxyGameLink_UpdateStateWithDouble(c, STATE_TARGET_CHANNEL, "replace", "/matter", 12.5);
    MuxyGameLink_UpdateStateWithString(c, STATE_TARGET_CHANNEL, "replace", "/matter", "123456");
    MuxyGameLink_UpdateStateWithLiteral(c, STATE_TARGET_CHANNEL, "replace", "/matter", "[1, 2, 3]");
    MuxyGameLink_UpdateStateWithNull(c, STATE_TARGET_CHANNEL, "replace", "/matter");
}

TEST_CASE_METHOD(ParityFixture, "Config operation parity", "[sdk][c]")
{
    const char * state = R"({ "hello": "world" })";
    cpp.SetChannelConfig(nlohmann::json::parse(state));
    MuxyGameLink_SetChannelConfig(c, state);

    cpp.GetConfig(gamelink::schema::CONFIG_TARGET_CHANNEL, CPP_NOOP());
    MuxyGameLink_GetConfig(c, CONFIG_TARGET_CHANNEL, C_NOOP(), nullptr);

    cpp.GetConfig(gamelink::schema::CONFIG_TARGET_EXTENSION, CPP_NOOP());
    MuxyGameLink_GetConfig(c, CONFIG_TARGET_EXTENSION, C_NOOP(), nullptr);

    cpp.SubscribeToConfigurationChanges(gamelink::schema::CONFIG_TARGET_CHANNEL);
    cpp.SubscribeToConfigurationChanges(gamelink::schema::CONFIG_TARGET_EXTENSION);
    MuxyGameLink_SubscribeToConfigurationChanges(c, CONFIG_TARGET_CHANNEL);
    MuxyGameLink_SubscribeToConfigurationChanges(c, CONFIG_TARGET_EXTENSION);

    cpp.UpdateChannelConfigWithInteger("replace", "/matter", 42);
    cpp.UpdateChannelConfigWithDouble("replace", "/matter", 12.5);
    cpp.UpdateChannelConfigWithString("replace", "/matter", "123456");
    cpp.UpdateChannelConfigWithLiteral("replace", "/matter", "[1, 2, 3]");
    cpp.UpdateChannelConfigWithNull("replace", "/matter");

    MuxyGameLink_UpdateChannelConfigWithInteger(c, "replace", "/matter", 42);
    MuxyGameLink_UpdateChannelConfigWithDouble(c, "replace", "/matter", 12.5);
    MuxyGameLink_UpdateChannelConfigWithString(c, "replace", "/matter", "123456");
    MuxyGameLink_UpdateChannelConfigWithLiteral(c,"replace", "/matter", "[1, 2, 3]");
    MuxyGameLink_UpdateChannelConfigWithNull(c, "replace", "/matter");
}

TEST_CASE_METHOD(ParityFixture, "Broadcast parity", "[sdk][c]")
{
    const char * message = R"({"update": "again"})";
    cpp.SendBroadcast("topic", nlohmann::json::parse(message));
    MuxyGameLink_SendBroadcast(c, "topic", message);
}