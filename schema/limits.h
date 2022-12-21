#ifndef MUXY_GAMELINK_LIMITS_H
#define MUXY_GAMELINK_LIMITS_H

#include "schema/game_metadata.h"

namespace gamelink
{
	namespace limits
	{

		static const uint32_t POLL_MAX_PROMPT_SIZE = 256;
		static const uint32_t POLL_MAX_OPTIONS = 10;
		static const uint32_t POLL_MAX_OPTION_NAME_SIZE = 128;
		
		static const uint32_t METADATA_MAX_GAME_NAME_SIZE = 256;
		static const uint32_t METADATA_MAX_GAME_LOGO_SIZE = 1024 * 1024 * 1;


		inline bool VerifyPoll(const string& prompt, const std::vector<string>& options)
		{
			if (options.size() > POLL_MAX_OPTIONS || prompt.size() > POLL_MAX_PROMPT_SIZE)
			{
				return false;
			}

			for (auto opt : options)
			{
				if (opt.size() > POLL_MAX_OPTION_NAME_SIZE)
				{
					return false;
				}
			}

			return true;
		}

		inline bool VerifyGameMetadata(const gamelink::GameMetadata& meta) 
		{
			if (meta.game_name.size() > METADATA_MAX_GAME_NAME_SIZE || meta.game_logo.size() > METADATA_MAX_GAME_LOGO_SIZE)
			{
				return false;
			}

			return true;
		}
	}
}
#endif