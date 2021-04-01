#pragma once
#ifndef MUXY_GAMELINK_EXAMPLE_LOAD_CONFIGURATION_H
#define MUXY_GAMELINK_EXAMPLE_LOAD_CONFIGURATION_H
#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

namespace examples
{
    struct Configuration
    {
        std::string clientID;
        std::string channelID;
    };

    inline Configuration LoadConfiguration()
    {
        std::ifstream config("../config.json", std::ios::binary);
        if (config)
        {
            config.seekg(0, std::ios::end);
            size_t length = config.tellg();
            config.seekg(0, std::ios::beg);

            std::vector<char> bytes(length);
            config.read(&bytes[0], bytes.size());
            bytes.push_back('\0');

            nlohmann::json value = nlohmann::json::parse(bytes.begin(), bytes.end(), nullptr, false);
            if (!value.is_discarded())
            {
                Configuration cfg;
                cfg.clientID = value["client_id"];
                cfg.channelID = value["channel_id"];
                return cfg;
            }
        }
        
        std::cout << "Couldn't load config.json\n";
        std::terminate();

        return Configuration();
    }

    inline void SaveJWT(const std::string& jwt)
    {
        std::ofstream jwtFile("../jwt", std::ios::trunc);
        jwtFile << jwt;
    }

    inline std::string LoadJWT() 
    {
        std::ifstream jwtFile("../jwt");
        if (!jwtFile)
        {
            std::cout << "Couldn't load saved jwt. Run the authenticate demo first\n";
            std::terminate();
        }

        std::string result;
        jwtFile >> result;

        return result;
    }
}
#endif
