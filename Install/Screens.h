#pragma once
#ifndef INSTALLER_SCREENS_H
#define INSTALLER_SCREENS_H
#include <iostream>
#include <Install/Items.h>
#include <Install/Head.h>
#include <Helpers/Helpers.h>

class Screens
{
private:
    int columns, rows;
    bool complete_install = false;

public:
    Helpers _helpers;
    Head _head;
    Items *items;
//    bool cloud_main = false;

    void InitializeInstanceFields(Items *_items) {
        items = _items;
        _helpers.getSizeOfTerminal(&columns, &rows);
        items->setHostname("#HOSTNAME#");
        //utils.showConsoleCursor(false);
    }

    void restartQ(std::string answer) {
        if ((answer == "Y") || (answer == "y") || (answer == "yes") || (answer == "Yes")) {
            _helpers.RunCmd("reboot", false);
        }
    }

    bool cloudScreen(bool no_option) {
        if(complete_install)
            return true;
        int line;
        _helpers.ClearScreen();
        _head.setHeader("default", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("If you continue then this server will be the main for your cloud system. Are you sure you want to do this?", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. Continue ?", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("9. Return ?", _helpers.YELLOW);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;

        if (std::cin.fail()) {
            line = 6;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }

        switch (line) {
            case 1:
                items->setApacheStatus(" ");
                items->setApacheNginxStatus(" ");
                items->setProftpStatus(" ");
                items->setPhpStatus(" ");
                items->setPhp56Status(" ");
                items->setPhp72Status(" ");
                items->setPhp74Status(" ");
                items->setDovecotStatus(" ");
                items->setPostfixStatus(" ");
                items->setMariaDbStatus(" ");
                items->setMySqlStatus("X");
                items->setPerlStatus(" ");
                items->setPythonStatus(" ");
                items->setSpamAssassinStatus(" ");
                items->setAwstatsStatus(" ");
                items->setWebalizerStatus(" ");
                items->setBindStatus(" ");
                items->setPostgreStatus(" ");
                items->setRoundCubeStatus(" ");
                items->setdbMyAdminStatus(" ");
                items->setBindStatus(" ");
                items->setMailManStatus(" ");
//                cloud_main = true;
                continueInstall(false);
                return true;
            case 9:
                mainScreen(false);
                return true;
            case 0:
                exit(EXIT_SUCCESS);
            default:
                cloudScreen(true);
                break;
        }
        return true;
    }

    void selectPackageHttp(bool no_option) {
        _helpers.ClearScreen();
        int line;
        _head.setHeader("http", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("What would you like to do? (Enter the number of your choice)", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. Apache       [" + items->getApacheStatus() + "]", _helpers.CYAN);
        _helpers.Println("2. Apache-Nginx [" + items->getApacheNginxStatus() + "]", _helpers.CYAN);
        _helpers.Println("3. Perl         [" + items->getPerlStatus() + "]", _helpers.CYAN);
        _helpers.Println("4. Python       [" + items->getPythonStatus() + "]", _helpers.CYAN);
        _helpers.Println("5. PHP 5.6      [" + items->getPhp56Status() + "]", _helpers.CYAN);
        _helpers.Println("6. PHP 7.2      [" + items->getPhp72Status() + "]", _helpers.CYAN);
        _helpers.Println("7. PHP 7.4      [" + items->getPhp74Status() + "]", _helpers.CYAN);

        _helpers.Println("", _helpers.RESET);
        _helpers.Println("9. Return ?", _helpers.YELLOW);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;
        if (std::cin.fail()) {
            line = 10;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }
        switch (line) {
            case 1:
                if (items->getApacheStatus() == "X") {
                    items->setApacheStatus(" ");
                    items->setPhpStatus(" ");
                }
                else {
                    items->setApacheStatus("X");
                    items->setApacheNginxStatus(" ");
                }
                selectPackageHttp(false);
                break;
            case 2:
                if (items->getApacheNginxStatus() == "X") {
                    items->setApacheNginxStatus(" ");
                    items->setPhpStatus(" ");
                }
                else {
                    items->setApacheNginxStatus("X");
                    items->setApacheStatus(" ");
//                    items->setPhpStatus("X");
                }
                selectPackageHttp(false);
                break;
            case 3:
                if (items->getPerlStatus() == "X") {
                    items->setPerlStatus(" ");
                    items->setWebalizerStatus(" ");
                    items->setAwstatsStatus(" ");
                }
                else
                    items->setPerlStatus("X");
                selectPackageHttp(false);
                break;
            case 4:
                if (items->getPythonStatus() == "X") {
                    items->setPythonStatus(" ");
                }
                else
                    items->setPythonStatus("X");
                selectPackageHttp(false);
                break;
            case 5:
                if (items->getPhp56Status() == "X") {
                    items->setPhp56Status(" ");
                }
                else {
                    items->setPhp56Status("X");
                }
                selectPackageHttp(false);
                break;
            case 6:
                if (items->getPhp72Status() == "X") {
                    items->setPhp72Status(" ");
                    if(items->getPhp74Status() != "X") {
                        items->setRoundCubeStatus(" ");
                        items->setdbMyAdminStatus(" ");
                    }
                }
                else {
                    items->setPhp72Status("X");
                    if(items->getPostfixStatus() == "X") {
                        items->setRoundCubeStatus("X");
                    }
                    if(items->getMySqlStatus() == "X" || items->getMariaDbStatus() == "X") {
                        items->setdbMyAdminStatus("X");
                    }
                }
                selectPackageHttp(false);
                break;
            case 7:
                if (items->getPhp74Status() == "X") {
                    items->setPhp74Status(" ");
                    if(items->getPhp72Status() != "X") {
                        items->setRoundCubeStatus(" ");
                        items->setdbMyAdminStatus(" ");
                    }
                }
                else {
                    items->setPhp74Status("X");
                    if(items->getPostfixStatus() == "X") {
                        items->setRoundCubeStatus("X");
                    }
                    if(items->getMySqlStatus() == "X" || items->getMariaDbStatus() == "X") {
                        items->setdbMyAdminStatus("X");
                    }
                }
                selectPackageHttp(false);
                break;
            case 9:
                standaloneScreen(false);
                break;
            case 0:
                //utils.showConsoleCursor(true);
                exit(EXIT_SUCCESS);
            default:
                selectPackageHttp(true);
                break;
        }
    }

    void selectPackageDatabase(bool no_option) {
        int line;
        _helpers.ClearScreen();
        _head.setHeader("database", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("What would you like to do? (Enter the number of your choice)", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. MySQL 5.7                 [" + items->getMySqlStatus() + "]", _helpers.CYAN);
        _helpers.Println("2. MariaDB                   [" + items->getMariaDbStatus() + "]", _helpers.CYAN);
        _helpers.Println("3. PostgreSQL                [" + items->getPostgreStatus() + "]", _helpers.CYAN);
        _helpers.Println("4. phpMyAdmin                [" + items->getdbMyAdminStatus() + "]", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("9. Return ?", _helpers.YELLOW);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;
        if (std::cin.fail()) {
            line = 6;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }
//        no_option = false;
        switch (line) {
            case 1:
                if (items->getMySqlStatus() == "X") {
                    items->setMySqlStatus(" ");
                    items->setMariaDbStatus("X");
                }
                else {
                    items->setMySqlStatus("X");
                    items->setMariaDbStatus(" ");
                }
                selectPackageDatabase(false);
                break;
            case 2:
                if (items->getMariaDbStatus() == "X") {
                    items->setMariaDbStatus(" ");
                    items->setMySqlStatus("X");
                }
                else {
                    items->setMariaDbStatus("X");
                    items->setMySqlStatus(" ");
                }
                selectPackageDatabase(false);
                break;
            case 3:
                if (items->getPostgreStatus() == "X")
                    items->setPostgreStatus(" ");
                else
                    items->setPostgreStatus("X");
                selectPackageDatabase(false);
                break;
            case 4:
                if (items->getdbMyAdminStatus() == "X")
                    items->setdbMyAdminStatus(" ");
                else {
                    items->setdbMyAdminStatus("X");
                    if (items->getMySqlStatus() != "X")
                        items->setMariaDbStatus("X");
                    else
                        items->setMySqlStatus("X");

                    if(items->getPhp72Status() != "X" || items->getPhp74Status() != "X") {
                        items->setPhp72Status("X");
                    }
                }
                selectPackageDatabase(false);
                break;
            case 9:
                standaloneScreen(false);
                break;
            case 0:
                exit(EXIT_SUCCESS);
            default:
                selectPackageDatabase(true);
                break;
        }
    }

    void selectPackageDns(bool no_option) {
        int line;
        _helpers.ClearScreen();
        _head.setHeader("dns", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("What would you like to do? (Enter the number of your choice)", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. Bind DNS Server       [" + items->getBindStatus() + "]", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("9. Return ?", _helpers.YELLOW);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;
        if (std::cin.fail()) {
            line = 2;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }
        switch (line) {
            case 1:
                if (items->getBindStatus() == "X") {
                    items->setBindStatus(" ");
                }
                else {
                    items->setBindStatus("X");
                }
                selectPackageDns(false);
                break;
            case 9:
                standaloneScreen(false);
                break;
            case 0:
                exit(EXIT_SUCCESS);
            default:
                selectPackageDns(true);
                break;
        }
    }

    void selectPackageMail(bool no_option) {
        int line;
        _helpers.ClearScreen();
        _head.setHeader("mail", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("What would you like to do? (Enter the number of your choice)", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. Postfix SMTP Server           [" + items->getPostfixStatus() + "]", _helpers.CYAN);
        _helpers.Println("2. Dovecot POP3 Server           [" + items->getPostfixStatus() + "]", _helpers.CYAN);
        _helpers.Println("3. Spam Assassin                 [" + items->getSpamAssassinStatus() + "]", _helpers.CYAN);
        _helpers.Println("4. MailMan                       [" + items->getMailManStatus() + "]", _helpers.CYAN);
        _helpers.Println("5. RoundCube WebMail             [" + items->getRoundCubeStatus() + "]", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("9. Return ?", _helpers.YELLOW);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;
        if (std::cin.fail()) {
            line = 5;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }
        switch (line) {
            case 1:
                if (items->getPostfixStatus() == "X") {
                    items->setPostfixStatus(" ");
                    items->setSpamAssassinStatus(" ");
                    items->setDovecotStatus(" ");
                }
                else {
                    items->setPostfixStatus("X");
                    items->setDovecotStatus("X");
                }
                selectPackageMail(false);
                break;
            case 2:
                if (items->getDovecotStatus() == "X") {
                    items->setPostfixStatus(" ");
                    items->setSpamAssassinStatus(" ");
                    items->setDovecotStatus(" ");
                }
                else {
                    items->setPostfixStatus("X");
                    items->setDovecotStatus("X");
                }
                selectPackageMail(false);
                break;
            case 3:
                if (items->getSpamAssassinStatus() == "X") {
                    items->setSpamAssassinStatus(" ");
                }
                else {
                    items->setPostfixStatus("X");
                    items->setDovecotStatus("X");
                    items->setSpamAssassinStatus("X");
                }
                selectPackageMail(false);
                break;
            case 4:
                if (items->getMailManStatus() == "X") {
                    items->setMailManStatus(" ");
                }
                else {
                    items->setPostfixStatus("X");
                    items->setDovecotStatus("X");
                    items->setMailManStatus("X");
                }
                selectPackageMail(false);
                break;
            case 5:
                if (items->getRoundCubeStatus() == "X") {
                    items->setRoundCubeStatus(" ");
                }
                else {
                    items->setPostfixStatus("X");
                    items->setDovecotStatus("X");
                    items->setRoundCubeStatus("X");
                    if(items->getPhp72Status() != "X" || items->getPhp74Status() != "X") {
                        items->setPhp72Status("X");
                    }
                }
                selectPackageMail(false);
                break;
            case 9:
                standaloneScreen(false);
                break;
            case 0:
                exit(EXIT_SUCCESS);
            default:
                selectPackageMail(true);
                break;
        }
    }

    void selectPackageStatistics(bool no_option) {
        int line;
        _helpers.ClearScreen();
        _head.setHeader("statistics", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("What would you like to do? (Enter the number of your choice)", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. Awstats             [" + items->getAwstatsStatus() + "]", _helpers.CYAN);
        _helpers.Println("2. Webalizer           [" + items->getWebalizerStatus() + "]", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("9. Return ?", _helpers.YELLOW);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;
        if (std::cin.fail()) {
            line = 5;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }
        switch (line) {
            case 1:
                if (items->getAwstatsStatus() == "X") {
                    items->setAwstatsStatus(" ");
                }
                else {
                    items->setAwstatsStatus("X");
                    items->setPerlStatus("X");
                }
                selectPackageStatistics(false);
                break;
            case 2:
                if (items->getWebalizerStatus() == "X") {
                    items->setWebalizerStatus(" ");
                }
                else {
                    items->setWebalizerStatus("X");
                    items->setPerlStatus("X");
                }
                selectPackageStatistics(false);
                break;
            case 9:
                standaloneScreen(false);
                break;
            case 0:
                exit(EXIT_SUCCESS);
            default:
                selectPackageStatistics(true);
                break;
        }
    }

    void complete(std::string db_root_password) {
        _helpers.ClearScreen();
//        bool no_hostname = true;
//        bool breaked = false;
        std::string answer;
        _head.setHeader("complete", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("Your SQL Password: " + db_root_password, _helpers.YELLOW);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("If you are using any firewall, don't forget to allow 88XX ports", _helpers.RED);
        _helpers.Println("Required to system restart", _helpers.CYAN);
        _helpers.Print("Do you want to restart now? Yes/Y: ", _helpers.CYAN);
//        fgetc(stdin);
        std::cin >> answer;
        restartQ(answer);
    }

    bool continueInstall(bool no_hostname)
    {
        if(!items->isSetHostname()) {
            _helpers.Println("Please enter the hostname on the previous screen.", _helpers.RED);
            _helpers.Println("Press enter to set hostname screen.", _helpers.YELLOW);
            fgetc(stdin);
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            setHostnameScreen(false);
            _helpers.ClearScreen();
            _head.setHeader("install", columns);
            _helpers.Println("", _helpers.RESET);
            return true;
        }
        _helpers.ClearScreen();
        std::string hostname = items->getHostname();
        std::string default_ip = _helpers.GetLocalIpAddress();
        _head.setHeader("continue", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("Your Default IP Address         : " + default_ip, _helpers.RED);
        _helpers.Println("Your Hostname                   : " + hostname, _helpers.RED);
        _helpers.Println("Please Enter for continue", _helpers.YELLOW);
        _helpers.Print("Or press CTRL+C to exit", _helpers.RED);

        fgetc(stdin);
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        complete_install = true;
        return true;
    }

    bool setHostnameScreen(bool no_option)
    {
        std::string answer_hostname;
        _helpers.ClearScreen();
        std::string answer;
        _head.setHeader("default", columns);
        _helpers.Println("", _helpers.RESET);
        if(no_option) {
            _helpers.Println("Please enter valid hostname", _helpers.RED);
        }
        _helpers.Println("Please set your machine hostname", _helpers.YELLOW);
        std::cin >> answer_hostname;
        if(!answer_hostname.empty()) {
            items->setHostname(answer_hostname);
        }
        else {
            setHostnameScreen(true);
        }
        return true;
    }

    bool standaloneScreen(bool no_option) {
        if(complete_install)
            return true;
        int line;
        _helpers.ClearScreen();
        _head.setHeader("default", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("What would you like to do? (Enter the number of your choice)", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. HTTP ?", _helpers.CYAN);
        _helpers.Println("2. Database ?", _helpers.CYAN);
        _helpers.Println("3. DNS ?", _helpers.CYAN);
        _helpers.Println("4. Mail ?", _helpers.CYAN);
        _helpers.Println("5. Statistics ?", _helpers.CYAN);
        _helpers.Println("6. Continue ?", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;

        if (std::cin.fail()) {
            line = 6;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }

        switch (line) {
            case 1:
                selectPackageHttp(false);
                standaloneScreen(false);
                break;
            case 2:
                selectPackageDatabase(false);
                standaloneScreen(false);
                break;
            case 3:
                selectPackageDns(false);
                standaloneScreen(false);
                break;
            case 4:
                selectPackageMail(false);
                standaloneScreen(false);
                break;
            case 5:
                selectPackageStatistics(false);
                standaloneScreen(false);
                break;
            case 6:
                continueInstall(false);
                return true;
            case 0:
                exit(EXIT_SUCCESS);
            default:
                standaloneScreen(true);
                break;
        }

        return true;
    }

    Items* mainScreen(bool no_option) {
        if (complete_install) {
            _helpers.ClearScreen();
            _head.setHeader("install", columns);
            _helpers.Println("", _helpers.RESET);
            return items;
        }
        _helpers.ClearScreen();
        int line;
        _head.setHeader("default", columns);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("What would you like to do? (Enter the number of your choice)", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("1. Standalone Mode Installation ?", _helpers.CYAN);
        _helpers.Println("2. Cloud Mode Installation ?", _helpers.CYAN);
//        _helpers.Println("3. Set Hostname", _helpers.CYAN);
        _helpers.Println("", _helpers.RESET);
        _helpers.Println("0. Quit ?", _helpers.RED);
        _helpers.Println("", _helpers.RESET);
        if (no_option)
            _helpers.Println("Wrong option, please choose a valid option.", _helpers.RED);
        _helpers.Print("Enter choise: ", _helpers.RESET);
        std::cin >> line;

        if (std::cin.fail()) {
            line = 3;
            std::cin.clear(); std::cin.ignore(INT_MAX, '\n');
        }

//        no_option = false;

        switch (line) {
            case 1:
                standaloneScreen(false);
                _helpers.ClearScreen();
                _head.setHeader("install", columns);
                _helpers.Println("", _helpers.RESET);
                return items;
            case 2:
                cloudScreen(false);
                _helpers.ClearScreen();
                _head.setHeader("install", columns);
                _helpers.Println("", _helpers.RESET);
                return items;
//            case 3:
//                setHostnameScreen(false);
//                _helpers.ClearScreen();
//                _head.setHeader("install", columns);
//                _helpers.Println("", _helpers.RESET);
//                mainScreen(false);
//                return items;
            case 0:
                exit(EXIT_SUCCESS);
            default:
                line = 3;
                mainScreen(true);
                break;
        }
        _helpers.ClearScreen();
        _head.setHeader("install", columns);
        _helpers.Println("", _helpers.RESET);
        return items;

    }
};

#endif //INSTALLER_SCREENS_H
