#include "gateway.h"

#ifndef MUXY_GATEWAY_WITHOUT_PNG_ENCODER
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_STATIC
#include "third_party/stb/stb_image_write.h"

// Suppress warnings.
namespace gateway 
{
	void unused() {
		(void)stbi_write_png_to_func;
		(void)stbi_write_bmp_to_func;
		(void)stbi_write_tga_to_func;
		(void)stbi_write_jpg_to_func;
		(void)stbi_flip_vertically_on_write;
	}
}
#endif

namespace gateway
{
	
#ifndef MUXY_GATEWAY_WITHOUT_PNG_ENCODER
	string base64EncodeWithImageURLPrefix(const void* voidIn, uint32_t length)
	{
		uint32_t outputLength = length / 3 * 4;
		if (outputLength > 1024 * 1024) 
		{
			return string("");
		}

		const uint8_t* in = static_cast<const uint8_t*>(voidIn);
		static const char* codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
		uint32_t o = 0;

		const char* prefix = "data:image/png;base64,";
		uint32_t prefixSize = strlen(prefix);

		char* out = new char[length / 3 * 4 + 4 + prefixSize];
		memcpy(out, prefix, prefixSize);

		o += prefixSize;

		for (uint32_t i = 0; i < length; i += 3)
		{
			// Read 6 bits.
			uint32_t bits = (in[i] & 0xFC) >> 2;
			out[o++] = codes[bits];

			// Read in 2 bits
			bits = (in[i] & 0x3) << 4;
			if (i + 1 < length)
			{
				// Read in 4 bits
				bits |= (in[i + 1] & 0xF0) >> 4;
				out[o++] = codes[bits];

				// Read in 4 bits
				bits = (in[i + 1] & 0xF) << 2;
				if (i + 2 < length)
				{
					// Read in 2 bits
					bits |= (in[i + 2] & 0xC0) >> 6;
					out[o++] = codes[bits];

					// Read in 6 bits
					bits = in[i + 2] & 0x3F;
					out[o++] = codes[bits];
				}
				else
				{
					out[o++] = codes[bits];
					out[o++] = '=';
				}
			}
			else
			{
				out[o++] = codes[bits];
				out[o++] = '=';
				out[o++] = '=';
			}
		}

		out[o] = '\0';
		string result = string(out, o - 1);

		delete [] out;
		return result;
	}

	string EncodeImageToBase64PNG(const void* untypedData, uint32_t w, uint32_t h, uint32_t components, uint32_t strideBytes, bool flipVertically)
	{
		if (!untypedData)
		{
			return string("");
		}

		const uint8_t* data = static_cast<const uint8_t*>(untypedData);
		if (components != 3 && components != 4)
		{
			return string("");
		}

		if (flipVertically)
		{
			// Copy out the rows into a big buffer, flipping vertically.
			uint8_t* buffer = new uint8_t[w * h * components]; 
			for (uint32_t y = 0; y < h; ++y)
			{
				memcpy(&buffer[components * w * (h - y - 1)], &data[strideBytes * y], w * components);
			}

			int outputLength = 0;
			unsigned char* png = stbi_write_png_to_mem(reinterpret_cast<const unsigned char*>(buffer), 
				static_cast<int>(strideBytes), 
				static_cast<int>(w),
				static_cast<int>(h), 
				components,
				&outputLength);

			string result = base64EncodeWithImageURLPrefix(png, outputLength);
			delete[] buffer;
			STBIW_FREE(png);
			return result;
		}
		else
		{
			int outputLength = 0;
			unsigned char* png = stbi_write_png_to_mem(reinterpret_cast<const unsigned char*>(data), 
				static_cast<int>(strideBytes), 
				static_cast<int>(w),
				static_cast<int>(h), 
				components,
				&outputLength);

			string result = base64EncodeWithImageURLPrefix(png, outputLength);
			STBIW_FREE(png);
			
			return result;
		}
	}

#endif

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

	bool SDK::ReceiveMessage(const char* Bytes, uint32_t Length)
	{
		return Base.ReceiveMessage(Bytes, Length);
	}

	bool SDK::HasPayloads() const
	{
		return Base.HasPayloads();
	}

	void SDK::OnDebugMessage(std::function<void (const gateway::string&)> callback)
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
		return Base.AuthenticateWithGameIDAndPIN(this->ClientID, this->GameID, PIN, [=](const gamelink::schema::AuthenticateResponse& Resp) {
			gateway::AuthenticateResponse Auth(Resp.data.jwt, Resp.data.refresh, Resp.data.twitch_name, gamelink::FirstError(Resp) != NULL);
			Callback(Auth);
		});
	}

	RequestID SDK::AuthenticateWithRefreshToken(const string& JWT, std::function<void(const gateway::AuthenticateResponse&)> Callback)
	{
		return Base.AuthenticateWithGameIDAndRefreshToken(
			this->ClientID, this->GameID, JWT, [=](const gamelink::schema::AuthenticateResponse& Resp) {
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

	void SDK::StopPoll()
	{
		Base.StopPoll("default");
	}

	void SDK::StartPoll(const PollConfiguration& cfg)
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

		Base.RunPoll(
			"default", 
			cfg.Prompt, 
			config, 
			cfg.Options, 
			[=](const gamelink::schema::PollUpdateResponse& response)
			{
				PollUpdate update;

				uint32_t idx = gamelink::GetPollWinnerIndex(response.data.results);
				update.Winner = static_cast<int>(idx);
				update.WinningVoteCount = response.data.results[idx];
				update.Results = response.data.results;
				update.IsFinal = false;
				update.Mean = response.data.mean;
				update.Count = response.data.count;

				cfg.OnUpdate(update);
			}, 
			[=](const gamelink::schema::PollUpdateResponse& response)
			{
				PollUpdate finish;

				uint32_t idx = gamelink::GetPollWinnerIndex(response.data.results);
				finish.Winner = static_cast<int>(idx);
				finish.WinningVoteCount = response.data.results[idx];
				finish.Results = response.data.results;
				finish.IsFinal = true;
				finish.Mean = response.data.mean;
				finish.Count = response.data.count;

				cfg.OnUpdate(finish);
			}
		);
	}
}
