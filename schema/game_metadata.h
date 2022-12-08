#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_GAME_METADATA_H
#define MUXY_GAMELINK_SCHEMA_GAME_METADATA_H
#include "schema/envelope.h"

namespace gamelink
{
	struct GameMetadata
	{
		string game_name;
		// Base64 encoded image
		string game_logo;
	};

	namespace schema
	{
		struct SetGameMetadataRequestBody
		{
			string game_name;
			string game_logo;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(SetGameMetadataRequestBody, "game_name", game_name, "game_logo", game_logo);
		};

		struct SetGameMetadataRequest : SendEnvelope<SetGameMetadataRequestBody>
		{
			SetGameMetadataRequest(const GameMetadata& meta)
			{
				this->action = string("set");
				this->params.target = string("gameMetadata");

				this->data.game_name = meta.game_name;
				this->data.game_logo = meta.game_logo;
			}
		};

		struct MUXY_GAMELINK_API SetGameMetadataResponse : ReceiveEnvelope<OKResponseBody>
		{
		};
	}
}

#endif