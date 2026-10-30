#include "gateway.h"

namespace gateway
{	
	MUXY_GAMELINK_SERIALIZE_3(GameText, "icon", Icon, "label", Label, "value", Value);

	void SDK::SetGameTexts(const GameTexts& texts)
	{
		GameText empty;
		const GameText* begin = texts.Texts.empty() ? &empty : texts.Texts.data();
		Base.UpdateStateWithArray(gamelink::StateTarget::Channel, 
			gamelink::Operation::Add, 
			gamelink::string("/game/text/game_data"), 
			begin,
			begin + texts.Texts.size());
	}
}
