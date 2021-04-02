#pragma once
#ifndef MUXY_GAMELINK_CONFIG_H
#define MUXY_GAMELINK_CONFIG_H

// Support custom string types.
#ifndef MUXY_GAMELINK_CUSTOM_STRING_TYPE
#include <string>
#define MUXY_GAMELINK_CUSTOM_STRING_TYPE std::string
#endif

#ifndef NO_JSON_INCLUDE
#include <nlohmann/json.hpp>
#endif

namespace gamelink
{
	/// This can be controlled by defining `MUXY_GAMELINK_CUSTOM_STRING_TYPE`.
	/// By default, MUXY_GAMELINK_CUSTOM_STRING_TYPE is std::string
	/// This string should
	///     * Provide a constructor from a null-terminated c-string of chars
	///     * Provide operator== with another instance of the string type.
	///     * Provide a c_str() that returns a pointer to the first element of a
	///       null-terminated array of utf8 encoded chars.
	typedef MUXY_GAMELINK_CUSTOM_STRING_TYPE string;
}

#endif
