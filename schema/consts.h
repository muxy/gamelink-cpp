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
		Test,

		OperationCount
	};

	inline bool IsValidOperation(Operation op)
	{
		return static_cast<int>(op) >= 0 && static_cast<int>(op) < static_cast<int>(Operation::OperationCount);
	}

	static const char* OPERATION_STRINGS[] = {"add", "remove", "replace", "copy", "move", "test"};

	enum class StateTarget
	{
		Channel = 0,
		Extension,

		StateCount
	};

	inline bool IsValidStateTarget(StateTarget st)
	{
		return static_cast<int>(st) >= 0 && static_cast<int>(st) < static_cast<int>(StateTarget::StateCount);
	}

	static const char* STATETARGET_STRINGS[] = {"channel", "extension"};

	enum class ConfigTarget
	{
		Channel = 0,
		Extension,
		Combined,

		ConfigTargetCount
	};

	inline bool IsValidConfigTarget(ConfigTarget ct)
	{
		return static_cast<int>(ct) >= 0 && static_cast<int>(ct) < static_cast<int>(ConfigTarget::ConfigTargetCount);
	}

	static const char* TARGET_STRINGS[] = {"channel", "extension", "combined"};
}
#endif