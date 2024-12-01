#include "gateway.h"

namespace gateway
{
	void SDK::CreateMatch(const string& str)
	{
		Base.CreateMatch(str);
	}

	void SDK::KeepMatchAlive(const string& str)
	{
		Base.KeepMatchAlive(str);
	}

	void SDK::AddChannelsToMatch(const string& id, const string* start, const string* end)
	{
		std::vector<string> channels(start, end);
		Base.AddChannelsToMatch(id, channels);
	}

	void SDK::RemoveChannelsFromMatch(const string& id, const string* start, const string* end)
	{
		std::vector<string> channels(start, end);
		Base.RemoveChannelsFromMatch(id, channels);
	}

	void SDK::RunMatchPoll(const string& match, const MatchPollConfiguration& cfg)
	{
		RunMatchPollWithID(match, string("default"), cfg);
	}

	void SDK::StopMatchPoll(const string& match)
	{
		StopMatchPollWithID(match, string("default"));
	}

	void SDK::StopMatchPollWithID(const string& match, const string& id)
	{
		Base.StopMatchPoll(match, id);
	}

	void SDK::RunMatchPollWithID(const string& match, const string& id, const MatchPollConfiguration& cfg)
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

		Base.RunMatchPoll(
			match,
			id,
			cfg.Prompt,
			config,
			cfg.Options,
			[=](const gamelink::schema::MatchPollUpdate& response)
			{
				MatchPollUpdate matchUpdate;

				for (auto it = response.data.results.begin(); it != response.data.results.end(); ++it)
				{
					const gamelink::schema::MatchPollResult& upd = it->second;
					PollUpdate update;

					uint32_t idx = gamelink::GetPollWinnerIndex(upd.results);
					update.Winner = static_cast<int>(idx);
					update.WinningVoteCount = upd.results[idx];
					update.Results = upd.results;
					update.Mean = upd.mean;
					update.Count = upd.count;
					update.IsFinal = false;

					matchUpdate.perChannel.insert(std::make_pair(it->first, std::move(update)));
				}

				uint32_t idx = gamelink::GetPollWinnerIndex(response.data.overall.results);
				matchUpdate.overall.Winner = idx;
				matchUpdate.overall.WinningVoteCount = response.data.overall.results[idx];
				matchUpdate.overall.Results = response.data.overall.results;
				matchUpdate.overall.Mean = response.data.overall.mean;
				matchUpdate.overall.Count = response.data.overall.count;
				matchUpdate.overall.IsFinal = false;

				if (cfg.OnUpdate)
				{
					cfg.OnUpdate(matchUpdate);
				}
			},
			[=](const gamelink::schema::MatchPollUpdate& response)
			{
				MatchPollUpdate matchUpdate;

				for (auto it = response.data.results.begin(); it != response.data.results.end(); ++it)
				{
					const gamelink::schema::MatchPollResult& upd = it->second;
					PollUpdate update;

					uint32_t idx = gamelink::GetPollWinnerIndex(upd.results);
					update.Winner = static_cast<int>(idx);
					update.WinningVoteCount = upd.results[idx];
					update.Results = upd.results;
					update.Mean = upd.mean;
					update.Count = upd.count;
					update.IsFinal = true;

					matchUpdate.perChannel.insert(std::make_pair(it->first, std::move(update)));
				}

				uint32_t idx = gamelink::GetPollWinnerIndex(response.data.overall.results);
				matchUpdate.overall.Winner = idx;
				matchUpdate.overall.WinningVoteCount = response.data.overall.results[idx];
				matchUpdate.overall.Results = response.data.overall.results;
				matchUpdate.overall.Mean = response.data.overall.mean;
				matchUpdate.overall.Count = response.data.overall.count;
				matchUpdate.overall.IsFinal = true;

				if (cfg.OnUpdate)
				{
					cfg.OnUpdate(matchUpdate);
				}

				if (cfg.OnComplete)
				{
					cfg.OnComplete(matchUpdate);
				}
			}
		);
	}
}