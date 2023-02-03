#include "gateway.h"

namespace gateway
{
	Payload::Payload(string Data)
		: WaitingForResponse(ANY_REQUEST_ID)
		, _Data(Data)
	{
	}

	const char* Payload::GetData() const
	{
		return this->_Data.c_str();
	}

	uint32_t Payload::GetLength() const
	{
		return static_cast<uint32_t>(this->_Data.size());
	}

	SDK::SDK(gateway::string GameID)
		: GameID(std::move(GameID))
	{
	}

	SDK::~SDK() {}

	bool SDK::ReceiveMessage(const char* Bytes, uint32_t Length)
	{
		return Base.ReceiveMessage(Bytes, Length);
	}

	bool SDK::HasPayloads() const
	{
		return Base.HasPayloads();
	}

	void SDK::ForeachPayload(SDK::NetworkCallback Callback, void* User)
	{
		Base.ForeachPayload([&](const gamelink::Payload* Send) {
			gateway::Payload P(gateway::string(Send->Data(), Send->Length()));
			Callback(User, &P);
		});
	}

	void SDK::HandleReconnect()
	{
		Base.HandleReconnect();
	}

	RequestID SDK::AuthenticateWithPIN(const string& PIN, std::function<void(const gateway::AuthenticateResponse&)> Callback)
	{
		return Base.AuthenticateWithGameIDAndPIN(this->ClientID, this->GameID, PIN, [&](const gamelink::schema::AuthenticateResponse Resp) {
			gateway::AuthenticateResponse Auth(Resp.data.jwt, Resp.data.refresh, Resp.data.twitch_name, gamelink::FirstError(Resp) != NULL);
			Callback(Auth);
		});
	}

	RequestID SDK::AuthenticateWithRefreshToken(const string& JWT, std::function<void(const gateway::AuthenticateResponse&)> Callback)
	{
		return Base.AuthenticateWithGameIDAndRefreshToken(
			this->ClientID, this->GameID, JWT, [&](const gamelink::schema::AuthenticateResponse Resp) {
				gateway::AuthenticateResponse Auth(Resp.data.jwt, Resp.data.refresh, Resp.data.twitch_name,
												   gamelink::FirstError(Resp) != NULL);
				Callback(Auth);
			});
	}

	bool SDK::IsAuthenticated() const
	{
		return Base.IsAuthenticated();
	}

	RequestID SDK::SetGameMetadata(gateway::GameMetadata Meta)
	{
		gamelink::GameMetadata BaseMeta;
		BaseMeta.game_name = Meta.GameName;
		BaseMeta.game_logo = Meta.GameLogo;
		BaseMeta.theme = Meta.Theme;
		return Base.SetGameMetadata(BaseMeta);
	}

	string SDK::GetSandboxURL() const
	{
		return gamelink::WebsocketConnectionURL(ClientID, gamelink::ConnectionStage::Sandbox);
	}

	string SDK::GetProductionURL() const
	{
		return gamelink::WebsocketConnectionURL(ClientID, gamelink::ConnectionStage::Production);
	}
}
