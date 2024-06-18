#include "gateway.h"

namespace gateway
{
	MUXY_GAMELINK_SERIALIZE_3(GameText, "icon", Icon, "label", Label, "value", Value);

	void SDK::SetGameTexts(const GameTexts& texts)
	{
		Base.UpdateStateWithArray(gamelink::StateTarget::Channel,
			gamelink::Operation::Add,
			gamelink::string("/game/text/game_data"),
			texts.Texts.data(),
			texts.Texts.data() + texts.Texts.size());
	}

	void SDK::SetGameText(int index, const GameText& text)
	{
		char pathBuffer[1024];
		int written = snprintf(pathBuffer, 1024, "/game/text/game_data/%d", index);
		pathBuffer[written] = '\0';

		gamelink::string path(pathBuffer);

		Base.UpdateStateWithObject(
			gamelink::StateTarget::Channel,
			gamelink::Operation::Add,
			pathBuffer,
			text);
	}

	void SDK::SetGameVector4(const string& label, const float* values)
	{
		char pathBuffer[1024];
		int written = snprintf(pathBuffer, 1024, "/game/text/game_vectors/%s", label.c_str());
		pathBuffer[written] = '\0';

		gamelink::string path(pathBuffer);

		Base.UpdateStateWithArray(
			gamelink::StateTarget::Channel,
			gamelink::Operation::Add,
			pathBuffer,
			values, values + 4);
	}

	void SDK::SetGameVector4WithComponents(const string& label, float x, float y, float z, float w)
	{
		float arr[] = {
			x, y, z, w
		};

		SetGameVector4(label, arr);
	}
}