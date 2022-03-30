#ifndef MUXY_GAMELINK_CONSTS_H
#define MUXY_GAMELINK_CONSTS_H
namespace gamelink
{
	enum class Operation
	{
		Add = 0,
		Remove,
		Replace,
		Copy,
		Move,
		Test
	};

	static const char* OPERATION_STRINGS[] = {"add", "remove", "replace", "copy", "move", "test"};

	enum class StateTarget
	{
		Channel = 0,
		Extension
	};

	enum class ConfigTarget
	{
		Channel = 0,
		Extension,
		Combined
	};
	static const char* TARGET_STRINGS[] = {"channel", "extension", "combined"};
}
#endif