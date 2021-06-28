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

    inline void SaveRefresh(const std::string& refresh)
    {
        std::ofstream refreshFile("../refresh", std::ios::trunc);
        refreshFile << refresh;
    }

    inline std::string LoadRefresh() 
    {
        std::ifstream refreshFile("../refresh");
        if (!refreshFile)
        {
            std::cout << "Couldn't load saved refresh. Run the authenticate demo first\n";
            std::terminate();
        }

        std::string result;
        refreshFile >> result;

        return result;
    }
}
#endif
