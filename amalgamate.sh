#!/bin/bash

OUTPUT_HEADER=gamelink_single.hpp
# These msut be topologically sorted. Generally this means that
# config.h, then serialization.h, envelope.h, then subscription.h followed by all other files.
INPUT_HEADERS=(
    ./schema/serialization.h 
    ./schema/envelope.h 
    ./schema/subscription.h 

    ./schema/authentication.h 
    ./schema/broadcast.h
    ./schema/datastream.h
    ./schema/poll.h 
    ./schema/purchase.h
    ./schema/schema.h 
    ./schema/state.h 
    
    ./include/gamelink.h
)

INPUT_SOURCES=(C:/muxy/gamelink-cpp/schema/authentication.cpp C:/muxy/gamelink-cpp/schema/datastream.cpp C:/muxy/gamelink-cpp/schema/envelope.cpp C:/muxy/gamelink-cpp/schema/poll.cpp C:/muxy/gamelink-cpp/schema/purchase.cpp C:/muxy/gamelink-cpp/schema/state.cpp C:/muxy/gamelink-cpp/src/gamelink.cpp C:/muxy/gamelink-cpp/src/gamelink_authentication.cpp C:/muxy/gamelink-cpp/src/gamelink_datastream.cpp C:/muxy/gamelink-cpp/src/gamelink_poll.cpp C:/muxy/gamelink-cpp/src/gamelink_purchases.cpp C:/muxy/gamelink-cpp/src/gamelink_state.cpp)
echo "/* This file is automatically generated from multiple sources. Do not modify */" >  $OUTPUT_HEADER
echo "" >> $OUTPUT_HEADER
echo "" >> $OUTPUT_HEADER
echo "#ifndef MUXY_GAMELINK_SINGLE_HPP" >> $OUTPUT_HEADER
echo "#define MUXY_GAMELINK_SINGLE_HPP" >> $OUTPUT_HEADER
echo "" >> $OUTPUT_HEADER

# Config begin
echo "#ifndef MUXY_GAMELINK_CONFIG_H"  >> $OUTPUT_HEADER
echo "#define MUXY_GAMELINK_CONFIG_H" >> $OUTPUT_HEADER
echo "" >> $OUTPUT_HEADER

echo "#ifndef MUXY_NO_JSON_INCLUDE" >> $OUTPUT_HEADER
cat ./third_party/nlohmann/json.hpp >> $OUTPUT_HEADER
echo "#endif" >> $OUTPUT_HEADER

cat ./include/config.h | sed 's/#include ".*"//g' | sed 's/#pragma once//g'  >> $OUTPUT_HEADER

echo "#endif" >> $OUTPUT_HEADER
# Config end

for file in  ${INPUT_HEADERS[@]}; do 
    # Remove any includes in quotes, and pragma once before amalgamating.
    cat $file | sed 's/#include ".*"//g' | sed 's/#pragma once//g'  >> $OUTPUT_HEADER
    echo "" >> $OUTPUT_HEADER
done
echo "#endif" >> $OUTPUT_HEADER

echo "#ifdef MUXY_GAMELINK_SINGLE_IMPL" >> $OUTPUT_HEADER

for file in ${INPUT_SOURCES[@]}; do 
    # Remove any includes in quotes before amalgamating
    cat $file | sed 's/#include ".*"//g' >> $OUTPUT_HEADER
    echo "" >> $OUTPUT_HEADER
done

echo "#endif" >> $OUTPUT_HEADER
