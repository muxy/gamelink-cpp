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
}