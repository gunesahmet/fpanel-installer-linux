#pragma once
#ifndef INSTALLER_CONFIG_H
#define INSTALLER_CONFIG_H

#include <iostream>
#include <map>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>

#include <Logger/Logger.h>
#include <Helpers/Helpers.h>
#include <Ini/inireader.h>
#include <Crypto/Crypto.h>

class Config {
private:
    Logger _logger;
    Helpers _helpers;
    std::map<std::string, std::string> configurations;
    std::mutex conf_mutex;
public:
    Config()
    {
        configurations = std::map<std::string, std::string>();
    }

    ~Config()
    {
        configurations.clear();
    }

    void setLogger(Logger logger)
    {
        _logger = logger;
    }

    void init(std::string system_type)
    {
        std::lock_guard<std::mutex> lockGuard(conf_mutex);

//        _helpers.Print("System Type: ", Helpers::colour::RESET);
//        _helpers.Println(system_type, Helpers::colour::CYAN);
        if(system_type.empty()) {
            _helpers.Print("Unsupported system.", Helpers::colour::RED);
            _helpers.Println(" For detail please look in log file.", Helpers::colour::RESET);
            _logger.Error("Unsupported system. Please install Debian, Centos, Ubuntu or Windows.");
            exit(1);
        }
        _helpers.trim(system_type);

        configurations.empty();
        configurations.clear();
        std::string panel_directory = "/usr/local/fpanel/";
        std::string panel_configs_directory = "/usr/local/fpanel/configs/";
        std::string panel_system_conf = "/usr/local/fpanel/etc/system.conf";
        std::string panel_tmp_directory = panel_directory + "tmp/";
        std::string panel_template_directory = panel_directory + "core/templates/";
        std::string panel_database_admin_password = _helpers.GenerateHex(16);
        std::string panel_socket_key = _helpers.GenerateHex(16);
        std::string panel_socket_iv = _helpers.GenerateHex(16);
        std::string panel_sql_pass = _helpers.GenerateHex(6);

        if(!_helpers.IsDirExist(panel_directory)) {
            _helpers.CreateDirectory(panel_directory);
            _helpers.CreateDirectory(panel_tmp_directory);
            _helpers.CreateDirectory(panel_configs_directory);
            _helpers.CreateDirectory(panel_configs_directory + "domains/");
            _helpers.CreateDirectory(panel_configs_directory + "domains/nginx");
            _helpers.CreateDirectory(panel_configs_directory + "domains/apache");
        }
        std::string pkg_install_command;
        std::string pkg_remove_command;
        std::string pkg_manual_install_command;
        std::string pkg_extension;
        std::string source_list;
        std::string pkg_force_install_command;

        std::string pretty_name;
        std::string os_version;
        std::string os_id;

        std::string apache_user;
        std::string nogroup;

        os_id = system_type;
        pretty_name = _helpers.CmdOutput("lsb_release -ds");
        std::vector<std::string> arr;
        _helpers.Split(arr, _helpers.CmdOutput("lsb_release -rs"), ".");
        os_version = arr[0];

        bool valid = false;

        _helpers.trim(os_id);
        _helpers.trim(os_version);
        if(os_id.compare("debian") == 0) {
            if(os_version.compare("10") == 0) {
                valid = true;
            }
            else {
                _helpers.Println("Unsupported version. Your current OS: " + os_id + " Version: " + os_version, Helpers::colour::RED);
                _logger.Error("Unsupported version. Please install Debian (Buster/Stretch), Centos (7/8), Ubuntu (Xenial/Bionic/Focal) or Windows (2008/2012/2016 Core/Standard).");
                exit(2);
            }
        }
        if(os_id.compare("centos") == 0) {
            if(os_version != "8") {
                _helpers.Println("Unsupported version. Your current version: " + os_version, Helpers::colour::RED);
                _logger.Error("Unsupported version. Please install Debian (Buster/Stretch), Centos (7/8), Ubuntu (Xenial/Bionic/Focal) or Windows (2008/2012/2016 Core/Standard).");
                exit(2);
            }
            valid = true;
        }

        if(os_id.compare("ubuntu") == 0) {
            if(os_version.compare("20") != 0) {
                _helpers.Println("Unsupported version. Your current version: " + os_version, Helpers::colour::RED);
                _logger.Error("Unsupported version. Please install Debian (Buster/Stretch), Centos (7/8), Ubuntu (Xenial/Bionic/Focal) or Windows (2008/2012/2016 Core/Standard).");
                exit(2);
            }
            valid = true;
        }

        if(!valid) {
            _helpers.Print("Unsupported system.", Helpers::colour::RED);
            _helpers.Println(" For detail please look in log file.", Helpers::colour::RESET);
            _logger.Error("Unsupported system. Please install Debian, Centos, Ubuntu or Windows.");
            exit(2);
        }

        _helpers.CreateDirectory("/var/log/fpanel/");


        if(os_id.compare("debian") == 0) {
            pkg_install_command = "apt-get install -y -qq ";
            pkg_remove_command = "apt-get autoremove -y -qq ";
            pkg_manual_install_command = "dpkg -i --force-overwrite ";
            pkg_extension = ".deb";
            source_list = "/etc/apt/sources.list";
            pkg_force_install_command = "dpkg -i --force-overwrite ";
            apache_user = "www-data";
            nogroup = "nobody";
        }
        else if(os_id.compare("centos") == 0) {
            pkg_install_command = "yum install -y --nogpgcheck ";
            pkg_remove_command = "yum remove -y ";
            pkg_manual_install_command = "rpm -ivh ";
            pkg_extension = ".rpm";
            source_list = "/etc/yum.repos.d/CentOS-Vault.repo";
            pkg_force_install_command = "rpm -ivh --force --nodeps ";
            apache_user = "apache";
            nogroup = "nogroup";
        }
        else if(os_id.compare("ubuntu") == 0) {
            pkg_install_command = "apt-get install -y -qq ";
            pkg_remove_command = "apt-get autoremove -y -qq ";
            pkg_manual_install_command = "dpkg -i --force-overwrite ";
            pkg_extension = ".deb";
            source_list = "/etc/apt/sources.list";
            pkg_force_install_command = "dpkg -i --force-overwrite ";
            apache_user = "www-data";
            nogroup = "nobody";
        }


        std::string panel_http_server = "repo.fpanel.net";
        std::string panel_test_http_server = "test.repo.fpanel.net";
        std::string panel_distro_packages_server = "/linux/" + os_id + "/" + os_version + ".x/";
        std::string vhosts_dir = "/var/www";

        configurations.insert({ "panel.dir", panel_directory });
        configurations.insert({ "panel.system.conf", panel_system_conf });
        configurations.insert({ "panel.tmp.directory", panel_tmp_directory });
        configurations.insert({ "panel.template.directory", panel_template_directory });
        configurations.insert({ "panel.config.file", panel_directory + "etc/panel.conf" });
        configurations.insert({ "panel.sqlpass", panel_sql_pass });
        configurations.insert({ "panel.sqlpass.file", panel_directory + "etc/.sql_pass" });
        configurations.insert({ "panel.database.password", panel_database_admin_password });
        configurations.insert({ "panel.socket.key", panel_socket_key });
        configurations.insert({ "panel.socket.iv", panel_socket_iv });
        configurations.insert({ "vhosts.dir", vhosts_dir });

        configurations.insert({ "panel.http.server.address", panel_http_server });
        configurations.insert({ "panel.http.test.server.address", panel_test_http_server });
        configurations.insert({ "panel.http.server.distro", panel_distro_packages_server });

        configurations.insert({ "system.os", os_id });
        configurations.insert({ "system.version", os_version });
        configurations.insert({ "system.pretty_name", pretty_name });

        configurations.insert({ "system.source.list", source_list });
        configurations.insert({ "system.package.extension", pkg_extension });
        configurations.insert({ "system.apache.user", apache_user });

        configurations.insert({ "system.install.command", pkg_install_command });
        configurations.insert({ "system.remove.command", pkg_remove_command });
        configurations.insert({ "system.manuel_install.command", pkg_manual_install_command });
        configurations.insert({ "system.force.install.command", pkg_force_install_command });



        configurations.insert({ "apache", "X" });
        configurations.insert({ "apache-nginx", " " });
        configurations.insert({ "perl", "X" });
        configurations.insert({ "python", "X" });
        configurations.insert({ "php56", " " });
        configurations.insert({ "php72", "X" });
        configurations.insert({ "php74", " " });
        configurations.insert({ "mysql", "X" });
        configurations.insert({ "proftpd", "X" });
        configurations.insert({ "mariadb", " " });
        configurations.insert({ "postgresql", " " });
        configurations.insert({ "bind", "X" });
        configurations.insert({ "postfix", "X" });
        configurations.insert({ "dovecot", "X" });
        configurations.insert({ "spamassassin", " " });
        configurations.insert({ "mailman", " " });
        configurations.insert({ "roundcube", "X" });
        configurations.insert({ "dbmyadmin", "X" });
        configurations.insert({ "awstats", "X" });
        configurations.insert({ "webalizer", "X" });
        configurations.insert({ "php", "X" });
        configurations.insert({ "hostname", "" });
        configurations.insert({ "main-ip", "" });

    }

    void setValue(std::string key, std::string val) {
        configurations[key] = val;
    }

    std::string getValue(std::string key)
    {
        std::lock_guard<std::mutex> lockGuard(conf_mutex);
        return configurations.find(key)->second;
    }
};


#endif//FPANEL_CORE_CONFIG_H
