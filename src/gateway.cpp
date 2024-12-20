#include "gateway.h"

namespace gateway
{
	Payload::Payload(const char* data, uint32_t length)
		: _Data(data)
		, _Length(length)
	{
	}

	const char* Payload::GetData() const
	{
		return _Data;
	}

	uint32_t Payload::GetLength() const
	{
		return _Length;
	}

	SDK::SDK(gateway::string GameID)
		: GameID(std::move(GameID))
	{
	}

	SDK::~SDK() {}

	void SDK::SetClientID(const gateway::string& id)
	{
		ClientID = id;
	}

	bool SDK::ReceiveMessage(const char* Bytes, uint32_t Length)
	{
		return Base.ReceiveMessage(Bytes, Length);
	}

	bool SDK::HasPayloads() const
	{
		return Base.HasPayloads();
	}

	void SDK::OnDebugMessage(std::function<void(const gateway::string&)> callback)
	{
		return Base.OnDebugMessage(std::move(callback));
	}

	void SDK::DetachOnDebugMessage()
	{
		Base.DetachOnDebugMessage();
	}

	void SDK::ForeachPayload(SDK::NetworkCallback Callback, void* User)
	{
		Base.ForeachPayload([=](const gamelink::Payload* Send) {
			gateway::Payload P(Send->Data(), Send->Length());
			Callback(User, &P);
		});
	}

	void SDK::HandleReconnect()
	{
		Base.HandleReconnect();
	}

	RequestID SDK::AuthenticateWithPIN(const string& PIN, std::function<void(const gateway::AuthenticateResponse&)> Callback)
	{
		return Base.AuthenticateWithGameIDAndPIN(
			this->ClientID, this->GameID, PIN, [=](const gamelink::schema::AuthenticateResponse& Resp) {
				gateway::AuthenticateResponse Auth(Resp.data.jwt, Resp.data.refresh, Resp.data.twitch_name, Resp.data.twitch_id,
												   gamelink::FirstError(Resp) != NULL);
				Callback(Auth);
			});
	}

	RequestID SDK::AuthenticateWithRefreshToken(const string& JWT, std::function<void(const gateway::AuthenticateResponse&)> Callback)
	{
		return Base.AuthenticateWithGameIDAndRefreshToken(
			this->ClientID, this->GameID, JWT, [=](const gamelink::schema::AuthenticateResponse& Resp) {
				gateway::AuthenticateResponse Auth(Resp.data.jwt, Resp.data.refresh, Resp.data.twitch_name, Resp.data.twitch_id,
												   gamelink::FirstError(Resp) != NULL);
				Callback(Auth);
			});
	}

	void SDK::Deauthenticate()
	{
		Base.Deauthenticate();
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

	string SDK::GetProjectionSandboxURL(const string& projection, int major, int minor, int patch) const
	{
		return gamelink::ProjectionWebsocketConnectionURL(ClientID, gamelink::ConnectionStage::Sandbox, projection, major, minor, patch);
	}

	string SDK::GetProjectionProductionURL(const string& projection, int major, int minor, int patch) const
	{
		return gamelink::ProjectionWebsocketConnectionURL(ClientID, gamelink::ConnectionStage::Production, projection, major, minor, patch);
	}

	void SDK::StopPoll()
	{
		StopPollWithID(gateway::string("default"));
	}

	void SDK::StartPoll(const PollConfiguration& cfg)
	{
		StartPollWithID(gateway::string("default"), cfg);
	}

	void SDK::StopPollWithID(const gateway::string& id)
	{
		Base.StopPoll(id);
	}

	void SDK::StartPollWithID(const gateway::string& id, const PollConfiguration& cfg)
	{
		gamelink::PollConfiguration config;

		config.userIdVoting = true;
		if (cfg.Mode == PollMode::Chaos)
		{
			config.totalVotesPerUser = 1024;
			config.distinctOptionsPerUser = 258;
			config.votesPerOption = 1024;
		}
		else if (cfg.Mode == PollMode::Order)
		{
			config.totalVotesPerUser = 1;
			config.distinctOptionsPerUser = 1;
			config.votesPerOption = 1;
		}

		if (cfg.Duration > 0)
		{
			config.endsIn = cfg.Duration;
		}

		config.userData = cfg.UserData;

		Base.RunPoll(
			id, cfg.Prompt, config, cfg.Options,
			[=](const gamelink::schema::PollUpdateResponse& response) {
				PollUpdate update;

				uint32_t idx = gamelink::GetPollWinnerIndex(response.data.results);
				update.Winner = static_cast<int>(idx);
				update.WinningVoteCount = response.data.results[idx];
				update.Results = response.data.results;
				update.Mean = response.data.mean;
				update.Count = response.data.count;
				update.IsFinal = false;

				if (cfg.OnUpdate)
				{
					cfg.OnUpdate(update);
				}
			},
			[=](const gamelink::schema::PollUpdateResponse& response) {
				PollUpdate finish;

				uint32_t idx = gamelink::GetPollWinnerIndex(response.data.results);
				finish.Winner = static_cast<int>(idx);
				finish.WinningVoteCount = response.data.results[idx];
				finish.Results = response.data.results;
				finish.Mean = response.data.mean;
				finish.Count = response.data.count;
				finish.IsFinal = true;

				if (cfg.OnUpdate)
				{
					cfg.OnUpdate(finish);
				}

				if (cfg.OnComplete)
				{
					cfg.OnComplete(finish);
				}
			});
	}
}
