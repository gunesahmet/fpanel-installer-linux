#ifndef INSTALLER_HEAD_H
#define INSTALLER_HEAD_H

#include <Helpers/Helpers.h>
#include <Config/Config.h>
#include <Logger/Logger.h>
class Head {
private:
    Helpers _helpers;
    Config *_config;
    Logger _logger;

public:
    void SetConfig(Config *config)
    {
        _config = config;
    }

    enum enums {
        defaultHeader,
        httpHeader,
        databaseHeader,
        dnsHeader,
        mailHeader,
        statisticsHeader,
        installHeader,
        continueHeader,
        completeHeader
    };

    enums hashit (std::string const& inString)
    {
        if (inString.compare("default") == 0) return defaultHeader;
        if (inString.compare("httpd") == 0) return httpHeader;
        if (inString.compare("database") == 0) return databaseHeader;
        if (inString.compare("dns") == 0) return dnsHeader;
        if (inString.compare("mail") == 0) return mailHeader;
        if (inString.compare("install") == 0) return installHeader;
        if (inString.compare("statistics") == 0) return statisticsHeader;
        if (inString.compare("continue") == 0) return continueHeader;
        if (inString.compare("complete") == 0) return completeHeader;
        return defaultHeader;
    }

    void setHeader(std::string headerType, int columns)
    {
        std::string hashes = "#" + std::string(columns-2, '#') + "#";
        std::cout << hashes;
        std::string defaultHead = "#      FPanel Install Script" + std::string(columns - 29, ' ') + "#";
        std::string httpHead = "#      FPanel Select HTTP Packages" + std::string(columns - 35, ' ') + "#";
        std::string databaseHead = "#      FPanel Select Database Packages" + std::string(columns - 39, ' ') + "#";
        std::string dnsHead = "#      FPanel Select DNS Packages" + std::string(columns - 34, ' ') + "#";
        std::string mailHead = "#      FPanel Select Mail Packages" + std::string(columns - 35, ' ') + "#";
        std::string statisticsHead = "#      FPanel Select Statistics Packages" + std::string(columns - 41, ' ') + "#";
        std::string continueHead = "#      Fpanel Installation - Please enter variables" + std::string(columns - 52, ' ') + "#";
        std::string installHead = "#      System installation started. Please wait while installation..." + std::string(columns - 70, ' ') + "#";
        std::string completeHead = "#      FPanel Installation Completed" + std::string(columns - 37, ' ') + "#";

        switch(hashit(headerType)) {
            case defaultHeader:
                std::cout << defaultHead.c_str();
                break;
            case httpHeader:
                std::cout << httpHead.c_str();
                break;
            case databaseHeader:
                std::cout << databaseHead.c_str();
                break;
            case dnsHeader:
                std::cout << dnsHead.c_str();
                break;
            case mailHeader:
                std::cout << mailHead.c_str();
                break;
            case installHeader:
                std::cout << installHead.c_str();
                break;
            case statisticsHeader:
                std::cout << statisticsHead.c_str();
                break;
            case continueHeader:
                std::cout << continueHead.c_str();
                break;
            case completeHeader:
                std::cout << completeHead.c_str();
                break;
        }
        std::cout << hashes;
    }
};
#endif //INSTALLER_HEAD_H
