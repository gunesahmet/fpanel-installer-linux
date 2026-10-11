#pragma once
#ifndef CORE_COMMANDS_H
#define CORE_COMMANDS_H
#include <iostream>
#include <Helpers/Helpers.h>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Maps/Licence.h>
#include <Maps/LicenceRequests.h>
#include <Crypto/Crypto.h>

class Commands {
    bool debug;
    Helpers helpers;
    Logger logger;
    Crypto crypto;
    Config *_config;
    Licence *_INSTALLER_map;
    LicenceRequests *_INSTALLER_requests;
    std::string INSTALLER_result = "{\"created_date\":\"none\",\"expire_date\":\"\",\"status\":0,\"domains\":1,\"clients\":1}";
    std::string default_INSTALLER_result = "{\"created_date\":\"none\",\"expire_date\":\"\",\"status\":0,\"domains\":1,\"clients\":1}";
public:
    std::string Run(const std::string &request)
    {
        std::string cmd_result;
        if (request.find("licencemng --check") != std::string::npos) {
            if (debug) {
                logger.Debug("Licence checking...");
            }
            if(_INSTALLER_map->getTotalTry() > 4)
                cmd_result = default_INSTALLER_result;
            else
                cmd_result = INSTALLER_result;
        }
        else if (request.find("licencemng --force-check") != std::string::npos) {
            if (debug) {
                logger.Debug("Licence force check");
            }
            std::string enc_cmd_result = crypto.Encrypt(_INSTALLER_requests->getLast(), _config->getValue("secret.key"), _config->getValue("secret.iv"));

            requestLicence(enc_cmd_result);

            if(_INSTALLER_map->getTotalTry() > 4)
                cmd_result = default_INSTALLER_result;
            else
                cmd_result = INSTALLER_result;
        }
        else {
            std::string command = _config->getValue("root_path") + request;
            cmd_result = helpers.CmdOutput(command.c_str());
            _helpers.trim(cmd_result);
        }
        return cmd_result;
    }
};
#endif //CORE_COMMANDS_H
