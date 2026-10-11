#pragma once
#ifndef INSTALLER_ITEMS_H
#define INSTALLER_ITEMS_H

#include <iostream>

class Items
{
    std::string apache_status;
    std::string apache_nginx_status;
    std::string proftpd_status;

    std::string php_status;
    std::string php56_status;
    std::string php72_status;
    std::string php74_status;
    std::string php_fpm_status;

    std::string perl_status;
    std::string python_status;
    std::string mysql_server;
    std::string mariadb_server;
    std::string postgresql_status;
    std::string bind_status;
    std::string postfix_status;
    std::string dovecot_status;
    std::string spam_assasin_stastus;
    std::string mailman_status;
    std::string old_hostname;
    std::string roundcube_status;
    std::string dbmyadmin_status;
    std::string awstats_status;
    std::string webalizer_status;
    bool is_set_hostname = false;

public:
    std::string getApacheStatus()
    {
        return apache_status;
    }

    void setApacheStatus(std::string value)
    {
        apache_status = value;
    }

    std::string getApacheNginxStatus()
    {
        return apache_nginx_status;
    }

    void setApacheNginxStatus(std::string value)
    {
        apache_nginx_status = value;
    }

    std::string getProftpStatus()
    {
        return proftpd_status;
    }

    void setProftpStatus(std::string value)
    {
        proftpd_status = value;
    }

    std::string getPhpStatus()
    {
        return php_status;
    }

    void setPhpStatus(std::string value)
    {
        php_status = value;
    }

    std::string getPythonStatus()
    {
        return python_status;
    }

    void setPythonStatus(std::string value)
    {
        python_status = value;
    }

    std::string getPerlStatus()
    {
        return perl_status;
    }

    void setPerlStatus(std::string value)
    {
        perl_status = value;
    }

    std::string getMySqlStatus()
    {
        return mysql_server;
    }

    void setMySqlStatus(std::string value)
    {
        mysql_server = value;
    }

    std::string getMariaDbStatus()
    {
        return mariadb_server;
    }

    void setMariaDbStatus(std::string value)
    {
        mariadb_server = value;
    }

    std::string getPostgreStatus()
    {
        return postgresql_status;
    }

    void setPostgreStatus(std::string value)
    {
        postgresql_status = value;
    }

    std::string getBindStatus()
    {
        return bind_status;
    }

    void setBindStatus(std::string value)
    {
        bind_status = value;
    }

    std::string getPostfixStatus()
    {
        return postfix_status;
    }

    void setPostfixStatus(std::string value)
    {
        postfix_status = value;
    }

    std::string getDovecotStatus()
    {
        return dovecot_status;
    }

    void setDovecotStatus(std::string value)
    {
        dovecot_status = value;
    }

    std::string getSpamAssassinStatus()
    {
        return spam_assasin_stastus;
    }

    void setSpamAssassinStatus(std::string value)
    {
        spam_assasin_stastus = value;
    }

    std::string getHostname()
    {
        return old_hostname;
    }

    void setHostname(std::string value)
    {
        if(value != "#HOSTNAME#") {
            is_set_hostname = true;
        }
        old_hostname = value;
    }

    bool isSetHostname()
    {
        return is_set_hostname;
    }

    std::string getMailManStatus() {
        return mailman_status;
    }

    void setMailManStatus(std::string value) {
        mailman_status = value;
    }


    std::string getRoundCubeStatus() {
        return roundcube_status;
    }

    void setRoundCubeStatus(std::string value) {
        roundcube_status = value;
    }

    std::string getPhp56Status() {
        return php56_status;
    }

    void setPhp56Status(std::string value) {
        php56_status = value;
    }

    std::string getPhp72Status() {
        return php72_status;
    }

    void setPhp72Status(std::string value) {
        php72_status = value;
    }

    std::string getPhp74Status() {
        return php74_status;
    }

    void setPhp74Status(std::string value) {
        php74_status = value;
    }

    std::string getdbMyAdminStatus() {
        return dbmyadmin_status;
    }

    void setdbMyAdminStatus(std::string value) {
        dbmyadmin_status = value;
    }

    std::string getAwstatsStatus() {
        return awstats_status;
    }

    void setAwstatsStatus(std::string value) {
        awstats_status = value;
    }

    std::string getWebalizerStatus() {
        return webalizer_status;
    }

    void setWebalizerStatus(std::string value) {
        webalizer_status = value;
    }
};

#endif //INSTALLER_ITEMS_H
