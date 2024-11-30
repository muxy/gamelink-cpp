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

				std::vector<int32_t> overall;
				overall.resize(32);

				for (auto it = response.data.results.begin(); it != response.data.results.end(); ++it)
				{
					const gamelink::schema::PollUpdateBody& upd = it->second;
					PollUpdate update;

					uint32_t idx = gamelink::GetPollWinnerIndex(upd.results);
					update.Winner = static_cast<int>(idx);
					update.WinningVoteCount = upd.results[idx];
					update.Results = upd.results;
					update.Mean = upd.mean;
					update.Count = upd.count;
					update.IsFinal = false;

					for (size_t i = 0; i < upd.results.size(); ++i)
					{
						if (i < overall.size())
						{
							overall[i] += upd.results[i];
						}
					}

					matchUpdate.perChannel.insert(std::make_pair(it->first, std::move(update)));
				}

				uint32_t idx = gamelink::GetPollWinnerIndex(overall);
				matchUpdate.overall.Winner = idx;
				matchUpdate.overall.WinningVoteCount = overall[idx];
				matchUpdate.overall.Results = std::move(overall);

				double accumulator = 0;
				uint32_t count = 0;
				for (size_t i = 0; i < matchUpdate.overall.Results.size(); ++i)
				{
					count += matchUpdate.overall.Results[i];
					accumulator += matchUpdate.overall.Results[i] * i;
				}

				matchUpdate.overall.Mean = accumulator / static_cast<double>(count);
				matchUpdate.overall.Count = count;
				matchUpdate.overall.IsFinal = false;

				if (cfg.OnUpdate)
				{
					cfg.OnUpdate(matchUpdate);
				}
			},
			[=](const gamelink::schema::MatchPollUpdate& response)
			{
				MatchPollUpdate matchFinish;

				std::vector<int32_t> overall;
				overall.resize(32);

				for (auto it = response.data.results.begin(); it != response.data.results.end(); ++it)
				{
					const gamelink::schema::PollUpdateBody& upd = it->second;
					PollUpdate update;

					uint32_t idx = gamelink::GetPollWinnerIndex(upd.results);
					update.Winner = static_cast<int>(idx);
					update.WinningVoteCount = upd.results[idx];
					update.Results = upd.results;
					update.Mean = upd.mean;
					update.Count = upd.count;
					update.IsFinal = false;

					for (size_t i = 0; i < upd.results.size(); ++i)
					{
						if (i < overall.size())
						{
							overall[i] += upd.results[i];
						}
					}

					matchFinish.perChannel.insert(std::make_pair(it->first, std::move(update)));
				}

				uint32_t idx = gamelink::GetPollWinnerIndex(overall);
				matchFinish.overall.Winner = idx;
				matchFinish.overall.WinningVoteCount = overall[idx];
				matchFinish.overall.Results = std::move(overall);

				double accumulator = 0;
				uint32_t count = 0;
				for (size_t i = 0; i < matchFinish.overall.Results.size(); ++i)
				{
					count += matchFinish.overall.Results[i];
					accumulator += matchFinish.overall.Results[i] * i;
				}

				matchFinish.overall.Mean = accumulator / static_cast<double>(count);
				matchFinish.overall.Count = count;
				matchFinish.overall.IsFinal = true;


				if (cfg.OnUpdate)
				{
					cfg.OnUpdate(matchFinish);
				}

				if (cfg.OnComplete)
				{
					cfg.OnComplete(matchFinish);
				}
			}
		);
	}
}