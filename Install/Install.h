#pragma once
#ifndef INSTALLER_INSTALL_H
#define INSTALLER_INSTALL_H

#include <Helpers/Helpers.h>
#include <Config/Config.h>
#include <Logger/Logger.h>
#include <Install/Screens.h>
#include <Install/Download.h>
#include <Install/Settings.h>

class Install {
private:
    Helpers _helpers;
    Config *_config;
    Logger _logger;
    Screens _screens;
    Download _download;
    Items *_items;
    Settings _settings;
    bool _cloud_main = false;
    bool _cloud_client = false;
    std::string _main_ip;

    // Sistemin ihtiyac duydugu dizin ve startup configler
    // Last Percent 25
    void initSystem()
    {
        _helpers.Println("System configuring...", _helpers.CYAN);
        _helpers.Println("Creating directories...", _helpers.YELLOW);
        _logger.Info("System configuring...");
        _logger.Info("Creating directories...");
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "apps");
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "backup");
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "tmp");
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "etc");
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "migration");
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "var/log");

        _helpers.Println("Creating system configurations...", _helpers.YELLOW);
        _logger.Info("Creating system configurations...");

        std::string panel_conf_content = _config->getValue("panel.socket.key") + " " + _config->getValue("panel.socket.iv") + " " + _config->getValue("panel.database.password");
        std::string system_conf_content = "[server]\n"
                                          "address = " + _helpers.GetLocalIpAddress() + "\n"
                                                                                        "[update]\n"
                                                                                        "time = 00:00\n"
                                                                                        "random = 0\n"
                                                                                        "auto = 1\n"
                                                                                        "[statistics]\n"
                                                                                        "time = 00:00\n"
                                                                                        "random = 0\n";

        _helpers.writeNewFile(_config->getValue("panel.config.file"), panel_conf_content);
        _helpers.writeNewFile(_config->getValue("panel.system.conf"), system_conf_content);
        _helpers.writeNewFile(_config->getValue("panel.sqlpass.file"), _config->getValue("panel.sqlpass"));

        if(_config->getValue("system.os").compare("centos") == 0) {
            _helpers.Println("Disabling default packages...", _helpers.YELLOW);
            _logger.Info("Disabling default packages...");
            _helpers.RunCmd("yum remove -y unixODBC-utf16", _logger.debug);
            _helpers.RunCmd("yum remove -y unixODBC-utf16-devel", _logger.debug);
            _helpers.RunCmd("yum remove -y postfix", _logger.debug);



            // yum upgrade ca-certificates --disablerepo=epel
            _helpers.Println("Upgrading certificates...", _helpers.YELLOW);
            _logger.Info("Upgrading certificates...");
            std::string mysql_yum_repo_content;
            std::string nginx_yum_repo_content;
            std::string mssql_yum_repo_content;
            std::string epel_yum_repo_content;
            std::string repi_yum_repo_content;
            _helpers.RunCmd("yum upgrade -y ca-certificates --disablerepo=epel", _logger.debug);
            if(_config->getValue("system.version") == "8") {
                repi_yum_repo_content = "[remi-safe]\n";
                repi_yum_repo_content += "name=Safe Remi's RPM repository for Enterprise Linux 7 - $basearch\n";
                repi_yum_repo_content += "baseurl=http://rpms.remirepo.net/enterprise/8/safe/$basearch/\n";
                repi_yum_repo_content += "#mirrorlist=https://rpms.remirepo.net/enterprise/8/safe/httpsmirror\n";
                repi_yum_repo_content += "#mirrorlist=http://cdn.remirepo.net/enterprise/8/safe/mirror\n";
                repi_yum_repo_content += "enabled=1\n";
                repi_yum_repo_content += "gpgcheck=0\n";
                repi_yum_repo_content += "gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-remi\n";
                repi_yum_repo_content += "\n";

                epel_yum_repo_content = "[epel]\n";
                epel_yum_repo_content += "name=Extra Packages for Enterprise Linux 8 - $basearch\n";
                epel_yum_repo_content += "baseurl=http://download.fedoraproject.org/pub/epel/8/Everything/$basearch\n";
                epel_yum_repo_content += "failovermethod=priority\n";
                epel_yum_repo_content += "enabled=1\n";
                epel_yum_repo_content += "gpgcheck=0\n";
                epel_yum_repo_content += "gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-EPEL-8";

                nginx_yum_repo_content = "[nginx]\n";
                nginx_yum_repo_content += "name=nginx repo\n";
                nginx_yum_repo_content += "baseurl=http://nginx.org/packages/centos/8/x86_64/\n";
                nginx_yum_repo_content += "enabled=1\n";
                nginx_yum_repo_content += "gpgcheck=0\n";

                mssql_yum_repo_content = "[packages-microsoft-com-prod]\n";
                mssql_yum_repo_content += "name=packages-microsoft-com-prod\n";
                mssql_yum_repo_content += "baseurl=http://packages.microsoft.com/rhel/8/prod/\n";
                mssql_yum_repo_content += "enabled=1\n";
                mssql_yum_repo_content += "gpgcheck=0\n";
                mssql_yum_repo_content += "gpgkey=http://packages.microsoft.com/keys/microsoft.asc";
            }

            _helpers.Println("Initializing new repositories...", _helpers.YELLOW);
            _logger.Info("Initializing new repositories...");
            _helpers.writeNewFile("/etc/yum.repos.d/nginx.repo", nginx_yum_repo_content);
            _helpers.writeNewFile("/etc/yum.repos.d/epel.repo", epel_yum_repo_content);
            _helpers.writeNewFile("/etc/yum.repos.d/remi-safe.repo", repi_yum_repo_content);
            _helpers.writeNewFile("/etc/yum.repos.d/microsoft.repo", mssql_yum_repo_content);

            _helpers.RunCmd("rpm -ivh https://dev.mysql.com/get/mysql80-community-release-el8-1.noarch.rpm", _logger.debug);

            _helpers.Println("Installing the required packages...", _helpers.YELLOW);
            _logger.Info("Installing the required packages...");
            _helpers.RunCmd(_config->getValue("system.install.command") + " libpq", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libpng", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " perl", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " gzip", _logger.debug);
//            _helpers.RunCmd(_config->getValue("system.install.command") + " mcrypt.x86_64", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libmcrypt.x86_64", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " openssl", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libxslt", _logger.debug);
//            _helpers.RunCmd(_config->getValue("system.install.command") + " libpqxx", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libtool-ltdl", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libpng", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libpng12", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " php-odbc", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " freetds", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " bind-utils", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " cronie", _logger.debug);
//            _helpers.RunCmd(_config->getValue("system.install.command") + " sqlite3", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " sqlite", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libicu", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " compat-openssl*", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libpcap0.8", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libXext", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libXrender", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " xorg-x11-fonts-75dpi", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " xorg-x11-fonts-Type1", _logger.debug);
            // cyrus-sasl.x86_64  cyrus-sasl-lib.x86_64
            // perl gzip cyrus-sasl.x86_64 mcrypt.x86_64 libmcrypt.x86_64 cyrus-sasl-lib.x86_64
            // openssl libxslt libpqxx libtool-ltdl libpng
            // libpng12 php-odbc freetds bind-utils
            // cronie sqlite3 sqlite
        }
        else if(_config->getValue("system.os").compare("debian") == 0) {
            _helpers.RunCmd("apt-get clean", _logger.debug);
            _helpers.UpdateSystem(_logger.debug);
            // openssl debconf-utils sasl2-bin libsasl2-2 dnsutils gzip
            // libxslt1.1 libpq5 libmcrypt4 libltdl7 libpng12-0 sqlite3
            // apt install apt-rdepends
            _helpers.Println("Installing the required packages...", _helpers.YELLOW);
            _logger.Info("Installing the required packages...");
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes openssl", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes debconf-utils", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes sasl2-bin", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libsasl2-2", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes dnsutils", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes gzip", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libxslt1.1", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes apt-rdepends", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libxslt1.1", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libpq5", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libmcrypt4", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libltdl7", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes sqlite3", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libpcap0.8", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes fontconfig", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes xfonts-75dpi", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes xfonts-base", _logger.debug);

            std::string distro;
            if(_config->getValue("system.version").compare("10") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libpng16-16", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libcurl4", _logger.debug);
                distro = "buster";
            }

            if(_items->getMySqlStatus() == "X") {
                _helpers.Println("MySQL repo initializing", _helpers.YELLOW);
                _logger.Info("MySQL repo initializing");
                std::string mysql_apt_source = "deb http://repo.mysql.com/apt/debian/ "+ distro +" mysql-apt-config\n";
                mysql_apt_source += "deb http://repo.mysql.com/apt/debian/ "+ distro +" mysql-5.7\n";
                mysql_apt_source += "deb http://repo.mysql.com/apt/debian/ "+ distro +" mysql-tools\n";
                _helpers.writeNewFile("/etc/apt/sources.list.d/mysql.list", mysql_apt_source);
            }

            std::string general_apt_source = "deb http://ftp.de.debian.org/debian/ "+ distro +" main contrib non-free\n";
            if(_items->getApacheNginxStatus() == "X") {
                std::string nginx_apt_source = "deb http://nginx.org/packages/debian/ "+ distro +" nginx\n";
                _helpers.writeNewFile("/etc/apt/sources.list.d/nginx.list", nginx_apt_source);
            }


            _helpers.writeNewFile("/etc/apt/sources.list.d/non-free.list", general_apt_source);
            _helpers.RunCmd("apt-get clean", _logger.debug);
            _helpers.UpdateSystem(_logger.debug);
        }
        else if(_config->getValue("system.os").compare("ubuntu") == 0) {
            _helpers.Println("Installing the required packages...", _helpers.YELLOW);
            _logger.Info("Installing the required packages...");
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes aptitude", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes openssl", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes debconf-utils", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes sasl2-bin", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libsasl2-2", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes dnsutils", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes gzip", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libxslt1.1", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes apt-rdepends", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libpq5", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libmcrypt4", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libltdl7", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libpng16-16", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes sqlite3", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libcurl4", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libpcap0.8", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes fontconfig", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes libxrender1", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes xfonts-75dpi", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes xfonts-base", _logger.debug);

            std::string distro = "focal";

            if(_items->getMySqlStatus() == "X") {
                if(_config->getValue("system.version").compare("20") == 0) {
                    std::string mysql_apt_source = "deb http://repo.mysql.com/apt/ubuntu/ "+ distro +" mysql-apt-config\n";
                    mysql_apt_source += "deb http://repo.mysql.com/apt/ubuntu/ "+ distro +" mysql-8.0\n";
                    mysql_apt_source += "deb http://repo.mysql.com/apt/ubuntu/ "+ distro +" mysql-tools\n";
                    _helpers.writeNewFile("/etc/apt/sources.list.d/mysql.list", mysql_apt_source);
                }
            }

            std::string nginx_apt_source = "deb http://nginx.org/packages/ubuntu/ "+ distro +" nginx\n";
            _helpers.writeNewFile("/etc/apt/sources.list.d/nginx.list", nginx_apt_source);
            _helpers.RunCmd("apt-get clean", _logger.debug);
            _helpers.UpdateSystem(_logger.debug);
        }

        if(!_cloud_client) {
            _helpers.RunCmd("groupadd fpanel", _logger.debug);
            _helpers.RunCmd("useradd -g fpanel -d /usr/local/fpanel/httpd/htdocs -s /sbin/nologin fpanel", _logger.debug);
        }
    }

    // MySQL MariaDB yada PostgreSQL
    void installDatabase()
    {
        if(_items->getMySqlStatus() == "X") {
            _helpers.Println("MySQL Database Servers installing...", _helpers.CYAN);
            _logger.Info("MySQL Database Servers installing...");

            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd("dnf module disable mysql -y", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + "  mysql-community-server", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive " + _config->getValue("system.install.command") + " --force-yes mysql-community-server", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive " + _config->getValue("system.install.command") + " --force-yes mysql-community-server", _logger.debug);
            }

            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
                _helpers.RunCmd("apt-get clean", _logger.debug);
                _helpers.UpdateSystem(_logger.debug);
            }
        }
        else if(_items->getMariaDbStatus() == "X") {
            _helpers.Println("MariaDB Database Servers installing...", _helpers.CYAN);
            _logger.Info("MariaDB Database Servers installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " mariadb-server", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive " + _config->getValue("system.install.command") + " mariadb-server", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive " + _config->getValue("system.install.command") + " mariadb-server", _logger.debug);
            }
        }
        if(_items->getPostgreStatus() == "X") {
            _helpers.Println("PostgreSQL Database Servers installing...", _helpers.CYAN);
            _logger.Info("PostgreSQL Database Servers installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd("dnf install -y https://download.postgresql.org/pub/repos/yum/reporpms/EL-8-x86_64/pgdg-redhat-repo-latest.noarch.rpm", _logger.debug);
                _helpers.RunCmd("dnf -qy module disable postgresql", _logger.debug);
                _helpers.RunCmd("dnf install -y postgresql12 postgresql12-server", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " postgresql-12 postgresql-client-12", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " postgresql-12 postgresql-client-12", _logger.debug);
            }
        }
    }

    // Apache Nginx yada lighttpd
    void installHttpd()
    {
        std::string mod_packages;
        if(_items->getApacheStatus() == "X") {
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.Println("Apache Server installing...", _helpers.CYAN);
                _logger.Info("Apache Server installing...");
                _helpers.RunCmd(_config->getValue("system.install.command") + " httpd", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " mod_ssl", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " python3-mod_wsgi", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                if(_helpers.IsFileExists("/etc/apt/sources.list.d/php.list")) {
                    _helpers.MoveFile("/etc/apt/sources.list.d/php.list", "/tmp/.php.list");
                    _helpers.UpdateSystem(_logger.debug);
                }
                _helpers.Println("Apache Server installing...", _helpers.CYAN);
                _logger.Info("Apache Server installing...");
                _helpers.RunCmd(_config->getValue("system.install.command") + " apache2", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-bw", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-authz-unixgroup", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-perl2", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-python", _logger.debug);
//                    _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-ruid2 ", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.Println("Apache Server installing...", _helpers.CYAN);
                _logger.Info("Apache Server installing...");
                _helpers.RunCmd(_config->getValue("system.install.command") + " apache2", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-bw", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-authz-unixgroup", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-perl2", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-python", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-ruid2 ", _logger.debug);
            }
        }
        else if(_items->getApacheNginxStatus() == "X") {
            _helpers.Println("Nginx Server installing...", _helpers.CYAN);
            _logger.Info("Nginx Server installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " nginx nginx-all-modules.noarch httpd-tools", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                if(_helpers.IsFileExists("/etc/apt/sources.list.d/php.list")) {
                    _helpers.MoveFile("/etc/apt/sources.list.d/php.list", "/tmp/.php.list");
                    _helpers.UpdateSystem(_logger.debug);
                }
                _helpers.RunCmd(_config->getValue("system.install.command") + " nginx-extras apache2-utils", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " nginx-extras apache2-utils", _logger.debug);
            }

            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
                _helpers.RemoveFile("/etc/apt/sources.list.d/nginx.list");
                _helpers.RunCmd("apt-get clean", _logger.debug);
                _helpers.UpdateSystem(_logger.debug);
            }
        }
    }

    // Bind yada PowerDns!!!
    void installDns()
    {
        if(_items->getBindStatus() == "X") {
            _helpers.Println("DNS Servers installing...", _helpers.CYAN);
            _logger.Info("DNS Servers installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + "  bind", _logger.debug);
                // bind
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " bind9", _logger.debug);
                // bind9
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " bind9", _logger.debug);
                // bind9
            }
        }
    }

    // Postfix Dovecot
    void installMail()
    {
        std::string pkg_extension = _config->getValue("system.package.extension");
        if(_items->getPostfixStatus() == "X") {
            _helpers.Println("Mail Servers installing...", _helpers.CYAN);
            _logger.Info("Mail Servers installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                // rpm -i postfix
                // rpm -i dovecot
                // cyrus-sasl libnsl2 libdb
                _helpers.RunCmd(_config->getValue("system.install.command") + " cyrus-sasl", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libnsl2", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " libdb", _logger.debug);
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "dovecot" + pkg_extension)) {
                    _logger.Error("dovecot" + pkg_extension + " is not exists");
                }
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "dovecot" + pkg_extension, _logger.debug);
                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "dovecot" + pkg_extension);
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "postfix" + pkg_extension)) {
                    _logger.Error("postfix" + pkg_extension + " is not exists");
                }
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "postfix" + pkg_extension, _logger.debug);
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "postfwd" + pkg_extension, _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " cpanminus", _logger.debug);

                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "postfix" + pkg_extension);
                _helpers.RemoveFile(_config->getValue("panel.dir") + "mail/etc/makedefs.out");
                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "postfwd" + pkg_extension);
                _helpers.CreateDirectory("/var/lib/dovecot");

                _helpers.RunCmd("chown dovecot:dovecot /var/lib/dovecot", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " pypolicyd-spf", _logger.debug);//postfix-policyd-spf-python

            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " dovecot-core dovecot-imapd dovecot-sqlite dovecot-pop3d", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "postfwd" + pkg_extension, _logger.debug);
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "postfix" + pkg_extension)) {
                    _logger.Error("postfix" + pkg_extension + " is not exists");
                }
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") +"postfix" + pkg_extension, _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " cpanminus", _logger.debug);

                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "postfix" + pkg_extension);
                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "postfwd" + pkg_extension);
                _helpers.RemoveFile(_config->getValue("panel.dir") + "mail/etc/makedefs.out");

                _helpers.RunCmd(_config->getValue("system.install.command") + " postfix-policyd-spf-python", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " dovecot-core dovecot-imapd dovecot-sqlite dovecot-pop3d", _logger.debug);
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "postfix" + pkg_extension)) {
                    _logger.Error("postfix" + pkg_extension + " is not exists");
                }

                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "postfix" + pkg_extension, _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " postfix-policyd-spf-python", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.install.command") + " cpanminus", _logger.debug);
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "postfwd" + pkg_extension, _logger.debug);

                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "postfix" + pkg_extension);
                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "postfwd" + pkg_extension);
                _helpers.RemoveFile(_config->getValue("panel.dir") + "mail/etc/makedefs.out");
            }

            //For postfwd common part
            _helpers.RunCmd("cpanm -n Net::Server::Daemonize", _logger.debug);
            _helpers.RunCmd("cpanm -n Net::Server::Multiplex", _logger.debug);
            _helpers.RunCmd("cpanm -n Net::Server::PreFork", _logger.debug);
            _helpers.RunCmd("cpanm -n Net::DNS", _logger.debug);
            _helpers.RunCmd("cpanm -n IO::Multiplex", _logger.debug);
        }
    }

    // Spamassassin versionlari
    void installSpamAssassin()
    {
        if(_items->getPostfixStatus() == "X") {
            if(_items->getSpamAssassinStatus() == "X") {
                _helpers.Println("Spamassassin Service installing...", _helpers.CYAN);
                _logger.Info("Spamassassin Service installing...");
                if(_config->getValue("system.os").compare("centos") == 0) {
                    _helpers.RunCmd(_config->getValue("system.install.command") + "  ruby", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + "  spamassassin", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + "  perl-Razor-Agent", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + "  pyzor.noarch", _logger.debug);
                    _helpers.RunCmd("ln -s /etc/mail/spamassassin /etc/spamassassin", _logger.debug);
                    _helpers.RunCmd("mkdir -p /var/lib/spamassassin/.spamassassin", _logger.debug);
                    _helpers.RunCmd("chown -R spamd:spamd /var/lib/spamassassin", _logger.debug);
                }
                else if(_config->getValue("system.os").compare("debian") == 0) {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " spamassassin", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + " spamc", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + " razor", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + " pyzor", _logger.debug);
                }
                else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " spamassassin", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + " spamc", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + " razor", _logger.debug);
                    _helpers.RunCmd(_config->getValue("system.install.command") + " pyzor", _logger.debug);
                }
            }
        }
    }

    // Mailman versionlari
    void installMailman()
    {
        if(_items->getPostfixStatus() == "X") {
            if(_items->getMailManStatus() == "X") {
                _helpers.Println("Mailman Service installing...", _helpers.CYAN);
                _logger.Info("Mailman Service installing...");
                if(_config->getValue("system.os").compare("centos") == 0) {
                }
                else if(_config->getValue("system.os").compare("debian") == 0) {
                }
                else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                }
            }
        }
    }

    // Php versionlari
    void installPhp()
    {
        if(_items->getPhpStatus() == "X") {
            _helpers.Println("PHP Binaries installing...", _helpers.CYAN);
            _logger.Info("PHP Binaries installing...");
            if((_config->getValue("system.os").compare("debian") == 0)) {
                if(_helpers.IsFileExists("/tmp/.php.list")) {
                    _helpers.MoveFile("/tmp/.php.list", "/etc/apt/sources.list.d/php.list");
                    _helpers.UpdateSystem(_logger.debug);
                }
                else {
                    _helpers.Println("PHP source list not found...", _helpers.RED);
                    _logger.Error("PHP source list not found...");
                }
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd("add-apt-repository -y ppa:ondrej/php", _logger.debug);
                _helpers.RunCmd("apt-get update", _logger.debug);
                _helpers.UpdateSystem(_logger.debug);
            }

            bool fpm = false;
            if(_items->getApacheNginxStatus() == "X") {
                fpm = true;
                _helpers.Println("PHP FPM Binaries installing...", _helpers.MAGENTA);
                _logger.Info("PHP FPM Binaries installing...");
            }
            if(_config->getValue("system.os").compare("centos") == 0) {
                if(_items->getPhp56Status() == "X") {
                    std::vector<std::string> list = {"php56-php-mysql ",
                                                     "php56-php-mysqlnd ",
                                                     "php56-php-gd ",
                                                     "php56-php-imap ",
                                                     "php56-php-mcrypt ",
                                                     "php56-php-cli ",
                                                     "php56-php-dba ",
                                                     "php56-php-json ",
                                                     "php56-php-mbstring ",
                                                     "php56-php-mysqlnd ",
                                                     "php56-php-opcache ",
                                                     "php56-php-soap ",
                                                     "php56-php-pdo ",
                                                     "php56-php-pgsql ",
                                                     "php56-php-gettext ",
                                                     "php56-php-xml ",
                                                     "php56-php-xmlrpc ",
                                                     "php56-php-libzip"};

                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " php56-php-fpm", _logger.debug);
                    }
                    else {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " php56 php56-php", _logger.debug);
                    }
                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " " + *t, _logger.debug);
                    }

                }
                if(_items->getPhp72Status() == "X") {
                    std::vector<std::string> list = {"php72-php-mysql ",
                                                     "php72-php-gd ",
                                                     "php72-php-imap "
                                                     "php72-php-mysqlnd ",
                                                     "php72-php-cli ",
                                                     "php72-php-dba ",
                                                     "php72-php-json ",
                                                     "php72-php-mbstring ",
                                                     "php72-php-mysqlnd ",
                                                     "php72-php-opcache ",
                                                     "php72-php-soap ",
                                                     "php72-php-pdo ",
                                                     "php72-php-pgsql ",
                                                     "php72-php-gettext ",
                                                     "php72-php-xml ",
                                                     "php72-php-xmlrpc ",
                                                     "php72-libzip"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " php72-php-fpm", _logger.debug);
                    }
                    else {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " php72 php72-php", _logger.debug);
                    }

                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " " + *t, _logger.debug);
                    }
                }
                if(_items->getPhp74Status() == "X") {
                    std::vector<std::string> list = {"php74-php-mysql ",
                                                     "php74-php-gd ",
                                                     "php74-php-imap ",
                                                     "php74-php-mysqlnd ",
                                                     "php74-php-cli ",
                                                     "php74-php-dba ",
                                                     "php74-php-json ",
                                                     "php74-php-mbstring ",
                                                     "php74-php-mysqlnd ",
                                                     "php74-php-opcache ",
                                                     "php74-php-soap ",
                                                     "php74-php-pdo ",
                                                     "php74-php-pgsql ",
                                                     "php74-php-gettext ",
                                                     "php74-php-xml ",
                                                     "php74-php-xmlrpc ",
                                                     "php74-libzip"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " php74-php-fpm", _logger.debug);
                    }
                    else {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " php74 php74-php", _logger.debug);
                    }

                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " " + *t, _logger.debug);
                    }
                }
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                std::string mod_packages;
                if(_items->getApacheStatus() == "X") {
                    if(_items->getPhp74Status() == "X") {
                        mod_packages += " libapache2-mod-php7.4";
                    }
                    if(_items->getPhp72Status() == "X") {
                        mod_packages += " libapache2-mod-php7.2";
                    }
                    if(_items->getPhp56Status() == "X") {
                        mod_packages += " libapache2-mod-php5.6";
                    }
                }
                if(_items->getPhp56Status() == "X") {
                    std::vector<std::string> list = {"php5.6", "php5.6-sqlite", "php5.6-cgi", "php5.6-gd", "php5.6-mysql", "php5.6-mysqlnd", "php5.6-pgsql", "php5.6-gettext", "php5.6-mbstring", "php5.6-bcmath", "php5.6-bz2", "php5.6-curl", "php5.6-json", "php5.6-opcache", "php5.6-sqlite3", "php5.6-xml", "php5.6-xsl", "php5.6-zip", "php5.6-mcrypt"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes php5.6-fpm ", _logger.debug);
                    }

                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes " + *t, _logger.debug);
                    }
                }
                if(_items->getPhp72Status() == "X") {
                    std::vector<std::string> list = {"php7.2", "php7.2-sqlite", "php7.2-cgi", "php7.2-gd", "php7.2-mysql", "php7.2-mysqlnd", "php7.2-pgsql", "php7.2-gettext", "php7.2-mbstring", "php7.2-bcmath", "php7.2-bz2", "php7.2-curl", "php7.2-json", "php7.2-opcache", "php7.2-sqlite3", "php7.2-xml", "php7.2-xsl", "php7.2-zip"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes php7.2-fpm ", _logger.debug);
                    }
                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes " + *t, _logger.debug);
                    }
                }
                if(_items->getPhp74Status() == "X") {
                    std::vector<std::string> list = {"php7.4", "php7.4-sqlite", "php7.4-cgi", "php7.4-gd", "php7.4-mysql", "php7.4-mysqlnd", "php7.4-pgsql", "php7.4-gettext", "php7.4-mbstring", "php7.4-bcmath", "php7.4-bz2", "php7.4-curl", "php7.4-json", "php7.4-opcache", "php7.4-sqlite3", "php7.4-xml", "php7.4-xsl", "php7.4-zip"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes php7.4-fpm ", _logger.debug);
                    }
                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes " + *t, _logger.debug);
                    }
                }
                _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes " + mod_packages, _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                if(_items->getPhp56Status() == "X") {
                    std::vector<std::string> list = {"php5.6", "php5.6-sqlite", "php5.6-cgi", "php5.6-gd", "php5.6-mysql", "php5.6-mysqlnd", "php5.6-pgsql", "php5.6-gettext", "php5.6-mbstring", "php5.6-bcmath", "php5.6-bz2", "php5.6-curl", "php5.6-json", "php5.6-opcache", "php5.6-sqlite3", "php5.6-xml", "php5.6-xsl", "php5.6-zip", "php5.6-mcrypt"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes php5.6-fpm ", _logger.debug);
                    }
                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes " + *t, _logger.debug);
                    }
                }
                if(_items->getPhp72Status() == "X") {
                    std::vector<std::string> list = {"php7.2", "php7.2-sqlite", "php7.2-cgi", "php7.2-gd", "php7.2-mysql", "php7.2-mysqlnd", "php7.2-pgsql", "php7.2-gettext", "php7.2-mbstring", "php7.2-bcmath", "php7.2-bz2", "php7.2-curl", "php7.2-json", "php7.2-opcache", "php7.2-sqlite3", "php7.2-xml", "php7.2-xsl", "php7.2-zip"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes php7.2-fpm ", _logger.debug);
                    }
                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes " + *t, _logger.debug);
                    }
                }
                if(_items->getPhp74Status() == "X") {
                    std::vector<std::string> list = {"php7.4", "php7.4-sqlite", "php7.4-cgi", "php7.4-gd", "php7.4-mysql", "php7.4-mysqlnd", "php7.4-pgsql", "php7.4-gettext", "php7.4-mbstring", "php7.4-bcmath", "php7.4-bz2", "php7.4-curl", "php7.4-json", "php7.4-opcache", "php7.4-sqlite3", "php7.4-xml", "php7.4-xsl", "php7.4-zip"};
                    if(fpm) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes php7.4-fpm ", _logger.debug);
                    }
                    for (std::vector<std::string>::iterator t= list.begin(); t!= list.end(); ++t) {
                        _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes " + *t, _logger.debug);
                    }
                }
            }

            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
//                _helpers.RemoveFile("/etc/apt/sources.list.d/php.list");
                _helpers.RunCmd("apt-get clean", _logger.debug);
                _helpers.UpdateSystem(_logger.debug);
            }

            std::string pkg_extension = _config->getValue("system.package.extension");

            if(_items->getRoundCubeStatus() == "X") {
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "webmail_rc" + pkg_extension)) {
                    _logger.Error("webmail_rc" + pkg_extension + " is not exists");
                }
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "webmail_rc" + pkg_extension, _logger.debug);
            }
            if(_items->getdbMyAdminStatus() == "X") {
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "dbmyadmin" + pkg_extension)) {
                    _logger.Error("dbmyadmin" + pkg_extension + " is not exists");
                }
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "dbmyadmin" + pkg_extension, _logger.debug);
            }

            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "webmail_rc" + pkg_extension);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "dbmyadmin" + pkg_extension);
        }
    }

    // Perl
    void installPerl()
    {
        if(_items->getPerlStatus() == "X") {
            _helpers.Println("Perl Binaries installing...", _helpers.CYAN);
            _logger.Info("Perl Binaries installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " perl", _logger.debug);
                // perl
                if(_items->getApacheStatus() == "X") {
                    //libapache2-mod-perl2
                    _helpers.RunCmd(_config->getValue("system.install.command") + " mod_perl", _logger.debug);
                }
                else if (_items->getApacheNginxStatus() == "X") {
                    //nginx-mod-http-perl
                    _helpers.RunCmd(_config->getValue("system.install.command") + " nginx-mod-http-perl", _logger.debug);
                }
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                // perl
                _helpers.RunCmd(_config->getValue("system.install.command") + " perl", _logger.debug);
                if(_items->getApacheStatus() == "X") {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-perl2", _logger.debug);
                    //mod_perl
                }
                else if (_items->getApacheNginxStatus() == "X") {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " nginx-mod-http-perl", _logger.debug);
                    //nginx-mod-http-perl
                }
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                // perl
                _helpers.RunCmd(_config->getValue("system.install.command") + " perl", _logger.debug);
                if(_items->getApacheStatus() == "X") {
                    //mod_perl
                    _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-perl2", _logger.debug);
                }
                else if (_items->getApacheNginxStatus() == "X") {
                    //nginx-mod-http-perl
                    _helpers.RunCmd(_config->getValue("system.install.command") + " nginx-mod-http-perl", _logger.debug);
                }
            }
        }
    }

    // Python
    void installPython()
    {
        if(_items->getPythonStatus() == "X") {
            _helpers.Println("Python Binaries installing...", _helpers.CYAN);
            _logger.Info("Python Binaries installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " python2 python3", _logger.debug);
                // python
                if (_items->getApacheNginxStatus() == "X") {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " python-pip", _logger.debug);
                    //python-pip
                }
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                // python
                _helpers.RunCmd(_config->getValue("system.install.command") + " python", _logger.debug);
                if(_items->getApacheStatus() == "X") {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-wsgi libapache2-mod-python", _logger.debug);
                    //libapache2-mod-wsgi libapache2-mod-python
                }
                else if (_items->getApacheNginxStatus() == "X") {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " python-pip", _logger.debug);
                    //python-pip
                }
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd(_config->getValue("system.install.command") + " python", _logger.debug);
                // python
                if(_items->getApacheStatus() == "X") {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " libapache2-mod-wsgi libapache2-mod-python", _logger.debug);
                    //libapache2-mod-wsgi libapache2-mod-python
                }
                else if (_items->getApacheNginxStatus() == "X") {
                    _helpers.RunCmd(_config->getValue("system.install.command") + " python-pip", _logger.debug);
                    //python-pip
                }
            }
        }
    }

    // ProFtpd
    void installFtp()
    {
        if(_items->getProftpStatus() == "X") {
            _helpers.Println("FTP Servers installing...", _helpers.CYAN);
            _logger.Info("FTP Servers installing...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                // proftpd proftpd-utils
                _helpers.RunCmd(_config->getValue("system.install.command") + " proftpd proftpd-utils", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                // proftpd
                _helpers.RunCmd("echo \"proftpd-basic shared/proftpd/inetd_or_standalone select standalone\" | debconf-set-selections && " + _config->getValue("system.install.command") + " proftpd", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                // proftpd
                _helpers.RunCmd("echo \"proftpd-basic shared/proftpd/inetd_or_standalone select standalone\" | debconf-set-selections && " + _config->getValue("system.install.command") + " proftpd", _logger.debug);
            }
        }
    }

    // Panel paketleri
    void installPanel()
    {
        // panel-httpd
        // panel-redis
        // panel-php
        // panel-core
        // panel-htdoc
        // panel-configs
        std::string pkg_extension = _config->getValue("system.package.extension");

        if(!_cloud_client) {
            _helpers.Println("FPanel HTTP Package installing...", _helpers.CYAN);
            _logger.Info("FPanel HTTP Package installing...");
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "panel-httpd" + pkg_extension)) {
                _logger.Error("panel-httpd" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "panel-httpd" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "panel-httpd" + pkg_extension);

            _helpers.Println("FPanel Redis Package installing...", _helpers.CYAN);
            _logger.Info("FPanel Redis Package installing...");
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "panel-redis" + pkg_extension)) {
                _logger.Error("panel-redis" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "panel-redis" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "panel-redis" + pkg_extension);

            _helpers.Println("FPanel PHP Package installing...", _helpers.CYAN);
            _logger.Info("FPanel PHP Package installing...");
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "panel-php" + pkg_extension)) {
                _logger.Error("panel-php" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "panel-php" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "panel-php" + pkg_extension);

            _helpers.Println("FPanel Htdoc Package installing...", _helpers.CYAN);
            _logger.Info("FPanel Htdoc Package installing...");
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "panel-htdoc" + pkg_extension)) {
                _logger.Error("panel-htdoc" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "panel-htdoc" + pkg_extension, _logger.debug);
            _helpers.writeNewFile("/usr/local/fpanel/httpd/htdocs/.firstboot", "firstboot");
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "panel-htdoc" + pkg_extension);

            _helpers.RunCmd("chown -R fpanel:fpanel " + _config->getValue("panel.dir") + "httpd/htdocs/", _logger.debug);
        }

        _helpers.Println("FPanel Core Package installing...", _helpers.CYAN);
        _logger.Info("FPanel Core Package installing...");
        if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "panel-core" + pkg_extension)) {
            _logger.Error("panel-core" + pkg_extension + " is not exists");
        }
        _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "panel-core" + pkg_extension, _logger.debug);
        _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "panel-core" + pkg_extension);


        if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "wkhtmltox" + pkg_extension)) {
            _logger.Error("wkhtmltox" + pkg_extension + " is not exists");
        }
        _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "wkhtmltox" + pkg_extension, _logger.debug);
        _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "wkhtmltox" + pkg_extension);

        if(_helpers.IsFileExists("/usr/local/bin/wkhtmltoimage")) {
            _helpers.RunCmd("ln -s /usr/local/bin/wkhtmltoimage /usr/local/fpanel/core/sbin/wkhtmltoimage", _logger.debug);
        }

        _helpers.Println("FPanel Config Package installing...", _helpers.CYAN);
        _logger.Info("FPanel Config Package installing...");
        if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "panel-configs" + pkg_extension)) {
            _logger.Error("panel-configs" + pkg_extension + " is not exists");
        }
        _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "panel-configs" + pkg_extension, _logger.debug);
        _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "panel-configs" + pkg_extension);

        if(_items->getAwstatsStatus() == "X") {
            _helpers.Println("Awstats installing...", _helpers.CYAN);
            _logger.Info("Awstats installing...");
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "awstats" + pkg_extension)) {
                _logger.Error("awstats" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "awstats" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "awstats" + pkg_extension);
        }

        if(_items->getWebalizerStatus() == "X") {
            _helpers.Println("Webalizer installing...", _helpers.CYAN);
            _logger.Info("Webalizer installing...");
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "webalizer" + pkg_extension)) {
                _logger.Error("awstats" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "webalizer" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "webalizer" + pkg_extension);
        }



        _helpers.Println("ODBC packages installing...", _helpers.CYAN);
        _logger.Info("ODBC packages installing...");
        if(_config->getValue("system.os").compare("debian") == 0) {
//            apt-get install multiarch-support
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "libodbc1" + pkg_extension)) {
                _logger.Error("libodbc1" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "libodbc1" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "libodbc1" + pkg_extension);

            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "odbcinst" + pkg_extension)) {
                _logger.Error("odbcinst" + pkg_extension + " is not exists");
            }
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "odbcinst1debian2" + pkg_extension)) {
                _logger.Error("odbcinst1debian2" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "odbcinst*" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "odbcinst" + pkg_extension);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "odbcinst1debian2" + pkg_extension);

            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "unixodbc" + pkg_extension)) {
                _logger.Error("unixodbc" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "unixodbc" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "unixodbc" + pkg_extension);

            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "msodbcsql17" + pkg_extension)) {
                _logger.Error("msodbcsql17" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "msodbcsql17" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "msodbcsql17" + pkg_extension);
        }
        else if(_config->getValue("system.os").compare("ubuntu") == 0) {
            _helpers.RunCmd(_config->getValue("system.remove.command") + " --force-yes odbcinst1debian2 unixodbc-dev odbcinst", _logger.debug);

            if(_config->getValue("system.version").compare("20") == 0) {
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "multiarch-support" + pkg_extension)) {
                    _logger.Error("multiarch-support" + pkg_extension + " is not exists");
                }
                _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "multiarch-support" + pkg_extension, _logger.debug);
                _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "multiarch-support" + pkg_extension);
            }

            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "libodbc1" + pkg_extension)) {
                _logger.Error("libodbc1" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "libodbc1" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "libodbc1" + pkg_extension);

            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "odbcinst" + pkg_extension)) {
                _logger.Error("odbcinst" + pkg_extension + " is not exists");
            }
            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "odbcinst1debian2" + pkg_extension)) {
                _logger.Error("odbcinst1debian2" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "odbcinst*" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "odbcinst" + pkg_extension);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "odbcinst1debian2" + pkg_extension);

            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "unixodbc" + pkg_extension)) {
                _logger.Error("unixodbc" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "unixodbc" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "unixodbc" + pkg_extension);

            if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "msodbcsql17" + pkg_extension)) {
                _logger.Error("msodbcsql17" + pkg_extension + " is not exists");
            }
            _helpers.RunCmd("DEBIAN_FRONTEND=noninteractive ACCEPT_EULA=Y " + _config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "msodbcsql17" + pkg_extension, _logger.debug);
            _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "msodbcsql17" + pkg_extension);

            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes aptitude", _logger.debug);
        }
        else if(_config->getValue("system.os").compare("centos") == 0) {
            if(_items->getPythonStatus() == "X") {
                if(_items->getApacheStatus() == "X") {
                    _helpers.Println("Mod Python installing...", _helpers.CYAN);
                    _logger.Info("Mod Python installing...");
                    if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "mod_python" + pkg_extension)) {
                        _logger.Error("mod_python" + pkg_extension + " is not exists");
                    }
                    _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "mod_python" + pkg_extension, _logger.debug);
                    _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "mod_python" + pkg_extension);
                }
            }


            _helpers.RunCmd("ACCEPT_EULA=Y " + _config->getValue("system.install.command") + " msodbcsql17", _logger.debug);
            _helpers.RunCmd("ACCEPT_EULA=Y " + _config->getValue("system.install.command") + " mssql-tools", _logger.debug);
            _helpers.RunCmd("ACCEPT_EULA=Y " + _config->getValue("system.manuel_install.command") +
            " /usr/local/fpanel/tmp/odbcinst.deb " +
            " /usr/local/fpanel/tmp/mssql-tools.deb " +
            " /usr/local/fpanel/tmp/odbcinst1debian2.deb " +
            " /usr/local/fpanel/tmp/unixodbc.deb " +
            " /usr/local/fpanel/tmp/msodbcsql17.deb " +
            " /usr/local/fpanel/tmp/libodbc1.deb", _logger.debug);
        }

        if(_items->getPostfixStatus() == "X") {
            _helpers.RunCmd("chown autoresponder:autoresponder "+ _config->getValue("panel.dir") +"mail/etc/mail.db", _logger.debug);
            _helpers.RunCmd("chown autoresponder:autoresponder "+ _config->getValue("panel.dir") +"mail/etc/autoresponder.db", _logger.debug);
        }

        _helpers.RunCmd("systemctl daemon-reload", _logger.debug);
    }

    void initSystemSettings()
    {
        _settings.setConfig(_config)
                .setLogger(_logger)
                .setCloudMain(_cloud_main)
                .setCloudClient(_cloud_client)
                .setMainIp(_main_ip)
                .setItems(_items);
        _settings.InitSystemSettings();
    }

    void restartQ(std::string answer) {
        if ((answer == "Y") || (answer == "y") || (answer == "yes") || (answer == "Yes")) {
            _helpers.RunCmd("reboot", _logger.debug);
        }
    }
public:
    void SetConfig(Config *config)
    {
        _config = config;
    }

    void SetLogger(Logger logger)
    {
        _logger = logger;
    }

    void SetCloudMain(bool cloud_main)
    {
        _cloud_main = cloud_main;
    }

    void SetCloudClient(bool cloud_client)
    {
        _cloud_client = cloud_client;
    }

    void SetMainIp(std::string main_ip)
    {
        _main_ip = main_ip;
    }

    void Start(bool fast)
    {
//        //@DEBUG
//        _settings.InitSystemSettings();
//        //@DEBUG

        // Screen initializing
        _items = new Items();
        _items->setApacheStatus(_config->getValue("apache"));
        _items->setApacheNginxStatus(_config->getValue("apache-nginx"));
        _items->setPhpStatus(_config->getValue("php"));
        _items->setPerlStatus(_config->getValue("perl"));
        _items->setPythonStatus(_config->getValue("python"));
        _items->setPhp56Status(_config->getValue("php56"));
        _items->setPhp72Status(_config->getValue("php72"));
        _items->setPhp74Status(_config->getValue("php74"));
        _items->setMySqlStatus(_config->getValue("mysql"));
        _items->setMariaDbStatus(_config->getValue("mariadb"));
        _items->setPostgreStatus(_config->getValue("postgresql"));
        _items->setBindStatus(_config->getValue("bind"));
        _items->setPostfixStatus(_config->getValue("postfix"));
        _items->setProftpStatus(_config->getValue("proftpd"));
        _items->setDovecotStatus(_config->getValue("dovecot"));
        _items->setSpamAssassinStatus(_config->getValue("spamassassin"));
        _items->setMailManStatus(_config->getValue("mailman"));
        _items->setRoundCubeStatus(_config->getValue("roundcube"));
        _items->setdbMyAdminStatus(_config->getValue("dbmyadmin"));
        _items->setAwstatsStatus(_config->getValue("awstats"));
        _items->setWebalizerStatus(_config->getValue("webalizer"));


        if(!fast) {
            _screens.InitializeInstanceFields(_items);
            _screens.mainScreen(false);
            system("setterm -cursor off"); // TODO ac
        }
        else {
            _items->setHostname(_config->getValue("hostname"));
        }


        _download.SetConfig(_config);
        _download.SetLogger(_logger);
        _download.SetItems(_items);

        _download.Start();

        initSystem();

        installDatabase();

        installHttpd();

        installPhp();

        installPerl();

        installPython();

        installFtp();

        installDns();

        installMail();

        installSpamAssassin();

        installMailman();

        _helpers.Println("FPanel packages installing...", _helpers.CYAN);
        _logger.Info("FPanel packages installing...");
        installPanel();

        _helpers.Println("Configuring system settings...", _helpers.CYAN);
        _logger.Info("Configuring system settings...");
        initSystemSettings();

        _helpers.Println("Installation completed...", _helpers.CYAN);
        _logger.Info("Installation completed...");

        if(!_cloud_client) {
            if(!fast) {
                system("setterm -cursor on");
                _screens.complete(_config->getValue("panel.sqlpass"));
            }
            else {
                _helpers.ClearScreen();
                std::string answer;
                _helpers.Println("", _helpers.RESET);
                _helpers.Println("Your SQL Password: " + _config->getValue("panel.sqlpass"), _helpers.YELLOW);
                _helpers.Println("", _helpers.RESET);
                _helpers.Println("If you use any firewall don't forget allow to 88XX ports", _helpers.RED);
                _helpers.Println("Required to system restart", _helpers.CYAN);
                _helpers.Print("Do you want to restart now? Yes/Y: ", _helpers.CYAN);
                std::cin >> answer;
                restartQ(answer);
            }
        }
    }
};
#endif //INSTALLER_INSTALL_H
