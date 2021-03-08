#pragma once
#ifndef MUXY_GAMELINK_CONFIG_H
#define MUXY_GAMELINK_CONFIG_H

#ifndef MUXY_GAMELINK_CUSTOM_STRING_TYPE
#include <string>
#define MUXY_GAMELINK_CUSTOM_STRING_TYPE std::string
#endif

namespace gamelink
{
    namespace schema
    {
        typedef MUXY_GAMELINK_CUSTOM_STRING_TYPE string;
    }
} 

#endif