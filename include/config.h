#pragma once

#define MUXY_GAMELINK_VERSION_MAJOR 0
#define MUXY_GAMELINK_VERSION_MINOR 0
#define MUXY_GAMELINK_VERSION_PATCH 1

#if defined MUXY_GAMELINK_EXPORT_SYMBOLS && !defined MUXY_GAMELINK_API
#	if defined(_WIN32) || defined(_WIN64)
#		define MUXY_GAMELINK_API __declspec(dllexport)
#	else
#		define MUXY_GAMELINK_API
#	endif
#endif

// Support custom string types.
#ifndef MUXY_GAMELINK_CUSTOM_STRING_TYPE
#	include <string>
#	define MUXY_GAMELINK_CUSTOM_STRING_TYPE std::string
#else
#	define MUXY_GAMELINK_REQUIRES_STRING_JSON_OVERLOAD 1
#endif

#ifdef MUXY_GAMELINK_CUSTOM_STRING_INCLUDE
#include MUXY_GAMELINK_CUSTOM_STRING_INCLUDE
#endif

// Support custom lock types. 
#ifndef MUXY_GAMELINK_CUSTOM_LOCK_TYPE
#include <mutex>
#define MUXY_GAMELINK_CUSTOM_LOCK_TYPE std::mutex
#endif

#ifdef MUXY_GAMELINK_CUSTOM_LOCK_INCLUDE
#include MUXY_GAMELINK_CUSTOM_LOCK_INCLUDE
#endif

namespace gamelink
{
	/// This can be controlled by defining `MUXY_GAMELINK_CUSTOM_STRING_TYPE`.
	/// By default, MUXY_GAMELINK_CUSTOM_STRING_TYPE is std::string
	/// This string should
	///     * Have a default constructor that creates an empty ("") string.
	///     * Be Copy constructable and copy assignable.
	///     * Be Move constructable and move assignable.
	///     * Provide a constructor from a null-terminated c-string of chars
	///     * Provide const operator== with another const reference of the string type. 
	///     * Provide a const .size() that returns an integer of the length of the string, 
	///       excluding any null terminator. This should return an uint32_t.
	///     * Provide a const .c_str() that returns a pointer to the first element of a
	///       null-terminated array of utf8 encoded chars.
	typedef MUXY_GAMELINK_CUSTOM_STRING_TYPE string;

	/// This can be controlled by defining `MUXY_GAMELINK_CUSTOM_LOCK_TYPE`
	/// By default, MUXY_GAMELINK_CUSTOM_LOCK_TYPE is std::mutex
	/// This lock type should
	///     * Have a default constructor that creates a valid, unlocked lock.
	///     * May be uncopyable and unmovable.
	/// 	* Provide .lock(), .unlock() and bool .try_lock()
	/// 	* .try_lock() should true upon lock aquisition, and false otherwise.
	///     * Does not have to be recursive
	typedef MUXY_GAMELINK_CUSTOM_LOCK_TYPE lock;
}

// These includes are removed during amalgamation, but are here for non-amalgamated builds.
#include "third_party/nlohmann/json.hpp"
#include "config_footer.h"