#pragma once
#ifndef INSTALLER_SETTINGS_H
#define INSTALLER_SETTINGS_H
#include <Helpers/Helpers.h>
#include <Config/Config.h>
#include <Logger/Logger.h>
#include <Install/Items.h>
#include <vector>

class Settings {
private:
    Helpers _helpers;
    Logger _logger;
    Config *_config;
    Items *_items;
    std::string _default_ip_addr;
    std::string _new_hostname;
    std::string _old_hostname;
    bool _cloud_main = false;
    bool _cloud_client = false;
    std::string _main_ip;

    void initMysqlSettings()
    {
        if(_items->getMySqlStatus() == "X") {
            _helpers.Println("MySQL Settings Configuring...", _helpers.YELLOW);
            _logger.Info("MySQL Settings Configuring...");
            std::string mysql_reset_query;
            std::string mysql_reset_query_second;
            std::string mysql_init;
            std::vector<std::string> arr;
            _helpers.Split(arr, _helpers.CmdOutput("mysql -V | awk '{print $3}'"), ".");
            std::string mysql_version = arr[0];

            if(_config->getValue("system.os").compare("centos") == 0) {
                // echo "default_authentication_plugin=mysql_native_password" >> /etc/my.cnf
                _helpers.RunCmd("echo \"default_authentication_plugin=mysql_native_password\" >> /etc/my.cnf", true);
                std::string retval = _helpers.CmdOutput("cat /etc/my.cnf | grep \"^default_authentication_plugin\" > /dev/null;RETVAL=$?;echo $RETVAL");
                _helpers.trim(retval);
                if(retval.compare("0") != 0) {
                    _helpers.RunCmd("sed -i 's/# default-authentication-plugin=mysql_native_password/default-authentication-plugin=mysql_native_password/g' /etc/my.cnf", true);
                    retval = _helpers.CmdOutput("cat /etc/my.cnf | grep \"^default_authentication_plugin\" > /dev/null;RETVAL=$?;echo $RETVAL");
                    _helpers.trim(retval);
                    if(retval.compare("0") != 0) {
                        _helpers.Println("Retval: " + retval, _helpers.RED);
                        _helpers.Println("MySQL Server Configuring failed...", _helpers.RED);
                        _logger.Error("MySQL Server Configuring failed...");
                        exit(1);
                    }
                }

                if(_cloud_client) {
                    retval = _helpers.CmdOutput("cat /etc/my.cnf | grep \"^bind-address\" > /dev/null;RETVAL=$?;echo $RETVAL");
                    _helpers.trim(retval);
                    if(retval.compare("0") == 0) {
                        _helpers.CmdOutput("sed -i 's/^bind-address.*/bind-address = 0.0.0.0/g' /etc/my.cnf");
                    }
                    else {
                        _helpers.CmdOutput("sed -i 's/^\\[mysqld\\].*/\\[mysqld\\]\\nbind-address = 0.0.0.0/g' /etc/my.cnf");
                    }
                }

                mysql_init = "systemctl restart mysqld";
                _helpers.RunCmd(mysql_init, true);
                std::string temp_password_command = "grep \"A temporary password\" /var/log/mysqld.log";
                std::string output = _helpers.CmdOutput(temp_password_command);
                std::vector<std::string> passwd_array;
                _helpers.Split(passwd_array, output, " ");
                std::string temp_password = std::string(passwd_array[passwd_array.size() - 1]);
                _helpers.trim(temp_password);

                std::string change_mysql_query = "ALTER USER 'root'@'localhost' IDENTIFIED BY '" + temp_password + "';";
                std::string mysql_query = "SET GLOBAL validate_password.length = 6; SET GLOBAL validate_password.number_count = 0; SET GLOBAL validate_password.mixed_case_count = 0; SET GLOBAL validate_password.special_char_count = 0;ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '"+ _config->getValue("panel.sqlpass") +"'; FLUSH PRIVILEGES;";
                change_mysql_query = "mysql -u root -p\"" + temp_password + "\" --connect-expired-password -e \"" + change_mysql_query + "\" mysql";
                mysql_reset_query = "mysql -u root -p\"" + temp_password + "\" --connect-expired-password -e \"" + mysql_query + "\" mysql";
                _helpers.RunCmd(change_mysql_query, true);
                _helpers.RunCmd(mysql_reset_query, true);
                _helpers.RunCmd(mysql_init, true);


                std::string change_password_reqs = "SET GLOBAL validate_password.check_user_name = No;";
                std::string query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                change_password_reqs = "SET GLOBAL validate_password.dictionary_file = '';";
                query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                change_password_reqs = "SET GLOBAL validate_password.length = 0;";
                query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                change_password_reqs = "SET GLOBAL validate_password.mixed_case_count = 0;";
                query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                change_password_reqs = "SET GLOBAL validate_password.number_count = 0;";
                query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                change_password_reqs = "SET GLOBAL validate_password.policy = LOW;";
                query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                change_password_reqs = "SET GLOBAL validate_password.special_char_count = 0;";
                query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                change_password_reqs = "FLUSH PRIVILEGES;";
                query_command = "mysql -u root -p\"" + _config->getValue("panel.sqlpass") + "\" -e \"" + change_password_reqs + "\" mysql >/dev/null 2>&1";
                _helpers.RunCmd(query_command, true);

                _helpers.Println("MySQL root user altered", _helpers.YELLOW);
                _logger.Info("MySQL root user altered");
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                mysql_init = "systemctl restart mysql.service";
                if(mysql_version == "5") {
                    _helpers.RunCmd(mysql_init, _logger.debug);
                    std::string mysql_query = "SET PASSWORD FOR 'root'@'localhost' = PASSWORD('"+ _config->getValue("panel.sqlpass") +"');UPDATE mysql.user SET plugin = 'mysql_native_password' WHERE user = 'root' AND host = 'localhost'; FLUSH PRIVILEGES;";
                    _helpers.RunCmd("mysql -e \"" + mysql_query + "\" mysql;RETVAL=$?;echo $RETVAL", _logger.debug);
                    _helpers.Println("MySQL Temporary Password is changed", _helpers.YELLOW);
                    _logger.Info("MySQL Temporary Password is changed");
                    _helpers.RunCmd("mysql -e \"ALTER USER 'root'@'localhost' IDENTIFIED BY '"+ _config->getValue("panel.sqlpass") +"';\" mysql", _logger.debug);
                    _helpers.Println("MySQL root user altered", _helpers.YELLOW);
                    _logger.Info("MySQL root user altered");
                }
                else {
                    _helpers.RunCmd("echo \"default_authentication_plugin=mysql_native_password\" >> /etc/mysql/mysql.conf.d/mysqld.cnf", _logger.debug);
                    mysql_init = "systemctl start mysql.service";
                    _helpers.RunCmd(mysql_init, _logger.debug);
                    std::string mysql_query = "SET GLOBAL validate_password.length = 6; SET GLOBAL validate_password.number_count = 0; SET GLOBAL validate_password.mixed_case_count = 0; SET GLOBAL validate_password.special_char_count = 0;ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '"+ _config->getValue("panel.sqlpass") +"'; FLUSH PRIVILEGES;";
                    _helpers.RunCmd("mysql -u root -e \"" + mysql_query + "\" mysql", _logger.debug);
                    _helpers.Println("MySQL root user altered", _helpers.YELLOW);
                    _logger.Info("MySQL root user altered");
                }
//                bind-address    = 127.0.0.1
                if(_cloud_client) {
                    std::string retval = _helpers.CmdOutput("cat /etc/mysql/mysql.conf.d/mysqld.cnf | grep \"^bind-address\" > /dev/null;RETVAL=$?;echo $RETVAL");
                    _helpers.trim(retval);
                    if(retval.compare("0") == 0) {
                        _helpers.CmdOutput("sed -i 's/^bind-address.*/bind-address = 0.0.0.0/g' /etc/mysql/mysql.conf.d/mysqld.cnf");
                    }
                    else {
                        _helpers.CmdOutput("sed -i 's/^\\[mysqld\\].*/\\[mysqld\\]\\nbind-address = 0.0.0.0/g' /etc/mysql/mysql.conf.d/mysqld.cnf");
                    }
                }

            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd("echo \"default_authentication_plugin=mysql_native_password\" >> /etc/mysql/mysql.conf.d/mysqld.cnf", _logger.debug);
                mysql_init = "systemctl restart mysql.service";
                _helpers.RunCmd(mysql_init, _logger.debug);
                std::string mysql_query = "ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '"+ _config->getValue("panel.sqlpass") +"'; FLUSH PRIVILEGES;";
                std::string retval = _helpers.CmdOutput("mysql -u root -e \"" + mysql_query + "\" mysql");
                _helpers.Println("MySQL root user altered", _helpers.YELLOW);
                _logger.Info("MySQL root user altered");

                if(_cloud_client) {
                    retval = _helpers.CmdOutput("cat /etc/mysql/mysql.conf.d/mysqld.cnf | grep \"^bind-address\" > /dev/null;RETVAL=$?;echo $RETVAL");
                    _helpers.trim(retval);
                    if(retval.compare("0") == 0) {
                        _helpers.CmdOutput("sed -i 's/^bind-address.*/bind-address = 0.0.0.0/g' /etc/mysql/mysql.conf.d/mysqld.cnf");
                    }
                    else {
                        _helpers.CmdOutput("sed -i 's/^\\[mysqld\\].*/\\[mysqld\\]\\nbind-address = 0.0.0.0/g' /etc/mysql/mysql.conf.d/mysqld.cnf");
                    }
                }
            }

            std::string mysql_command = "mysql -u root -p"+_config->getValue("panel.sqlpass");
            std::string create_fpanel_user_query;
            std::string remote_create_fpanel_user_query;
            if(mysql_version == "5") {
                create_fpanel_user_query = "CREATE USER 'fpaneladmin'@'localhost' IDENTIFIED BY '"+ _config->getValue("panel.database.password") +"';";
                create_fpanel_user_query += "GRANT ALL PRIVILEGES ON *.* TO 'fpaneladmin'@'localhost' WITH GRANT OPTION;";
                remote_create_fpanel_user_query = "CREATE USER 'fpaneladmin'@'" + _main_ip + "' IDENTIFIED BY '"+ _config->getValue("panel.database.password") +"';";
                remote_create_fpanel_user_query += " alter user 'fpaneladmin'@'" + _main_ip + "' identified with mysql_native_password by '" + _config->getValue("panel.database.password") + "';";
                remote_create_fpanel_user_query += "GRANT ALL PRIVILEGES ON *.* TO 'fpaneladmin'@'" + _main_ip + "' WITH GRANT OPTION;";
            }
            else {
                create_fpanel_user_query = "CREATE USER 'fpaneladmin'@'localhost' IDENTIFIED WITH mysql_native_password BY '"+ _config->getValue("panel.database.password") +"';";
                create_fpanel_user_query += "GRANT ALL PRIVILEGES ON *.* TO 'fpaneladmin'@'localhost' WITH GRANT OPTION;";
                remote_create_fpanel_user_query = "CREATE USER 'fpaneladmin'@'" + _main_ip + "' IDENTIFIED WITH mysql_native_password BY '"+ _config->getValue("panel.database.password") +"';";
                remote_create_fpanel_user_query += "GRANT ALL PRIVILEGES ON *.* TO 'fpaneladmin'@'" + _main_ip + "' WITH GRANT OPTION;";
            }


            create_fpanel_user_query += "FLUSH PRIVILEGES;";
            _helpers.Println("MySQL fpanel user creating", _helpers.YELLOW);
            _logger.Info("MySQL fpanel user creating");
            _helpers.RunCmd(mysql_command + " -e \"" + create_fpanel_user_query + "\" mysql", _logger.debug);

            if(_cloud_client) {
                _helpers.RunCmd(mysql_command + " -e \"" + remote_create_fpanel_user_query + "\" mysql", _logger.debug);
            }

            _helpers.Println("MySQL fpanel user created", _helpers.YELLOW);
            _logger.Info("MySQL fpanel user created");
            _helpers.RunCmd(mysql_init, _logger.debug);
        }
    }

    void initMariaDbSettings()
    {
        if(_items->getMariaDbStatus() == "X") {
            _helpers.Println("MariaDB Settings Configuring...", _helpers.YELLOW);
            _logger.Info("MariaDB Settings Configuring...");
            std::string mysql_init = "/etc/init.d/mysql start";
            std::string mysql_cnf ="/etc/mysql/my.cnf";
            if(_config->getValue("system.os").compare("centos") == 0) {
                if((_config->getValue("system.version") == "8")) {
                    mysql_init = "systemctl start mysql.service";
                }
            }
            if(_config->getValue("system.os").compare("debian") == 0) {
                mysql_init = "systemctl start mysql.service";
            }
            if(_config->getValue("system.os").compare("ubuntu") == 0) {
                mysql_init = "systemctl start mysql.service";
            }

            if(_cloud_client) {
                std::string retval = _helpers.CmdOutput("cat " + mysql_cnf + " | grep \"^bind-address\" > /dev/null;RETVAL=$?;echo $RETVAL");
                _helpers.trim(retval);
                if(retval.compare("0") == 0) {
                    _helpers.CmdOutput("sed -i 's/^bind-address.*/bind-address = 0.0.0.0/g' " + mysql_cnf);
                }
                else {
                    _helpers.CmdOutput("sed -i 's/^\\[mysqld\\].*/\\[mysqld\\]\\nbind-address = 0.0.0.0/g' " + mysql_cnf);
                }
            }

            _helpers.RunCmd(mysql_init, _logger.debug);
            _helpers.RunCmd("mysql -e \"SET PASSWORD FOR 'root'@'localhost' = PASSWORD('"+ _config->getValue("panel.sqlpass") +"'); UPDATE mysql.user SET plugin = 'mysql_native_password' WHERE user = 'root' AND host = 'localhost'; FLUSH PRIVILEGES;\" mysql", _logger.debug);
            _helpers.RunCmd("mysql -e \"ALTER USER 'root'@'localhost' IDENTIFIED BY '"+ _config->getValue("panel.sqlpass") +"';\" mysql", _logger.debug);

            std::string mysql_command = "mysql -u root -p"+_config->getValue("panel.sqlpass");
            std::string create_fpanel_user_query = "CREATE USER 'fpaneladmin'@'localhost' IDENTIFIED BY '"+ _config->getValue("panel.database.password") +"';";
            create_fpanel_user_query += "GRANT ALL PRIVILEGES ON *.* TO 'fpaneladmin'@'localhost' WITH GRANT OPTION;";
            create_fpanel_user_query += "FLUSH PRIVILEGES;";
            _helpers.RunCmd(mysql_command + " -e \"" + create_fpanel_user_query + "\" mysql", _logger.debug);

            if(_cloud_client) {
                std::string remote_create_fpanel_user_query = "CREATE USER 'fpaneladmin'@'" + _main_ip + "' IDENTIFIED BY '"+ _config->getValue("panel.database.password") +"';";
                remote_create_fpanel_user_query += "GRANT ALL PRIVILEGES ON *.* TO 'fpaneladmin'@'" + _main_ip + "' WITH GRANT OPTION;";
                remote_create_fpanel_user_query += "FLUSH PRIVILEGES;";
                _helpers.RunCmd(mysql_command + " -e \"" + remote_create_fpanel_user_query + "\" mysql", _logger.debug);
            }
        }
    }

    void initPostgreSQLSettings()
    {
        if(_items->getPostgreStatus() == "X") {
            std::string output;
            _helpers.Println("PostgreSQL Settings Configuring...", _helpers.YELLOW);
            _logger.Info("PostgreSQL Settings Configuring...");
            std::string pg_init = "/etc/init.d/postgresql restart";
            std::string pg_config;
            if(_config->getValue("system.os").compare("centos") == 0) {
                pg_config = "/var/lib/pgsql/12/data/pg_hba.conf";
                pg_init = "systemctl restart postgresql-12";

                _helpers.RunCmd("/usr/pgsql-12/bin/postgresql-12-setup initdb", false);
                _helpers.RunCmd("systemctl enable --now postgresql-12", false);

                _helpers.CopyFile(pg_config, pg_config + ".sample");
                _helpers.SecSleep(1);
                _helpers.RunCmdWithOutput("echo 'local   all             all                                     trust' > " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             127.0.0.1/32            trust' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             ::1/128                 trust' >> " + pg_config);

                _helpers.RunCmd(pg_init, false);

                _helpers.MilliSleep(1500);

                std::string postgres_alter_command = "psql -U postgres -d template1 -h localhost -c \"ALTER USER postgres with password '"+ _config->getValue("panel.sqlpass") +"';\"";
                std::string fpanel_admin_create_command = "psql -U postgres -d template1 -h localhost -c \"CREATE USER fpaneladmin WITH PASSWORD '"+ _config->getValue("panel.database.password") +"';\"";
                std::string fpanel_admin_alter_command = "psql -U postgres -d template1 -h localhost -c \"ALTER USER fpaneladmin WITH SUPERUSER;\"";
                std::string fpanel_admin_alter_grant_command = "psql -U postgres -d template1 -h localhost -c \"GRANT ALL ON SCHEMA public TO fpaneladmin;\"";

                _helpers.CmdOutput(postgres_alter_command);
                _helpers.CmdOutput(fpanel_admin_create_command);
                _helpers.CmdOutput(fpanel_admin_alter_command);
                _helpers.CmdOutput(fpanel_admin_alter_grant_command);

                _helpers.MilliSleep(1500);
                _helpers.RunCmdWithOutput("echo '# TYPE  DATABASE        USER            ADDRESS                 METHOD' > " + pg_config);
                _helpers.RunCmdWithOutput("echo 'local   all             all                                     md5' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             127.0.0.1/32            md5' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             ::1/128                 md5' >> " + pg_config);
                if(_cloud_client) {
                    _helpers.RunCmdWithOutput("echo 'host    all             fpaneladmin             " + _main_ip + "/32                 md5' >> " + pg_config);
                    _helpers.RunCmdWithOutput("echo 'host    all             all             0.0.0.0/0                 md5' >> " + pg_config);
                }

                _helpers.RunCmdWithOutput("echo '# Replication' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    replication     all             127.0.0.1/32            md5' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    replication     all             ::1/128                 md5' >> " + pg_config);
                _helpers.SecSleep(2);

                _helpers.MilliSleep(1500);

                _helpers.CmdOutput("sed -i \"s/^listen_addresses.*=.*/listen_addresses = '*'/g\" /var/lib/pgsql/12/data/postgresql.conf");
                _helpers.CmdOutput("sed -i \"s/^#.*listen_addresses.*=.*/listen_addresses = '*'/g\" /var/lib/pgsql/12/data/postgresql.conf");

                _helpers.RunCmd(pg_init, false);
            }
            else {
                pg_config = "/etc/postgresql/12/main/pg_hba.conf";
                if(_config->getValue("system.os").compare("debian") == 0) {
                    if((_config->getValue("system.version").compare("10") == 0)) {
                        pg_init = "systemctl restart postgresql.service";
                    }
                }
                else {
                    //#listen_addresses = 'localhost'
                    pg_config = "/etc/postgresql/12/main/pg_hba.conf";
                    pg_init = "systemctl restart postgresql.service";
                }
                _helpers.CmdOutput("sed -i \"s/^listen_addresses.*=.*/listen_addresses = '*'/g\" /etc/postgresql/12/main/postgresql.conf");
                _helpers.CmdOutput("sed -i \"s/^#.*listen_addresses.*=.*/listen_addresses = '*'/g\" /etc/postgresql/12/main/postgresql.conf");

                _helpers.CopyFile(pg_config, pg_config + ".sample");
                //panel.sqlpass = "ENTER_SECRET_KEY";
                //panel.database.password = "ENTER_SECRET_KEY";
                _helpers.SecSleep(1);
                _helpers.RunCmdWithOutput("echo 'local   all             all                                     trust' > " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             127.0.0.1/32            trust' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             ::1/128                 trust' >> " + pg_config);


                _helpers.RunCmd(pg_init, false);

                _helpers.MilliSleep(1500);

                std::string postgres_alter_command = "psql -U postgres -d template1 -h localhost -c \"ALTER USER postgres with password '"+ _config->getValue("panel.sqlpass") +"';\"";
                std::string fpanel_admin_create_command = "psql -U postgres -d template1 -h localhost -c \"CREATE USER fpaneladmin WITH PASSWORD '"+ _config->getValue("panel.database.password") +"';\"";
                std::string fpanel_admin_alter_command = "psql -U postgres -d template1 -h localhost -c \"ALTER USER fpaneladmin WITH SUPERUSER;\"";
                std::string fpanel_admin_alter_grant_command = "psql -U postgres -d template1 -h localhost -c \"GRANT ALL ON SCHEMA public TO fpaneladmin;\"";

                _helpers.CmdOutput(postgres_alter_command);
                _helpers.CmdOutput(fpanel_admin_create_command);
                _helpers.CmdOutput(fpanel_admin_alter_command);
                _helpers.CmdOutput(fpanel_admin_alter_grant_command);

//            std::string hide_public_schema = "psql -U postgres -d template1 -h localhost -c \"REVOKE ALL ON DATABASE template1 FROM public; REVOKE ALL ON SCHEMA public FROM public; GRANT ALL ON SCHEMA public TO postgres;\"";
//            _helpers.RunCmdWithOutput(hide_public_schema);

//            std::string revoke_all_from_public = "psql -U postgres -d template1 -h localhost -c \"REVOKE ALL ON pg_user FROM public; REVOKE ALL ON pg_roles FROM public; REVOKE ALL ON pg_group FROM public; "
//                                             "REVOKE ALL ON pg_authid FROM public; REVOKE ALL ON pg_auth_members FROM public; REVOKE ALL ON pg_database FROM public; REVOKE ALL ON pg_tablespace FROM public; "
//                                             "REVOKE ALL ON pg_settings FROM public;\"";
//            _helpers.RunCmdWithOutput(revoke_all_from_public);

                _helpers.MilliSleep(1500);
                _helpers.RunCmdWithOutput("echo '# TYPE  DATABASE        USER            ADDRESS                 METHOD' > " + pg_config);
                _helpers.RunCmdWithOutput("echo 'local   all             all                                     md5' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             127.0.0.1/32            md5' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    all             all             ::1/128                 md5' >> " + pg_config);

                if(_cloud_client) {
                    _helpers.RunCmdWithOutput("echo 'host    all             fpaneladmin     " + _main_ip + "/32     md5' >> " + pg_config);
                    _helpers.RunCmdWithOutput("echo 'host    all             all             0.0.0.0/0               md5' >> " + pg_config);
                }
                _helpers.RunCmdWithOutput("echo '# Replication' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    replication     all             127.0.0.1/32            md5' >> " + pg_config);
                _helpers.RunCmdWithOutput("echo 'host    replication     all             ::1/128                 md5' >> " + pg_config);
                _helpers.SecSleep(2);

                _helpers.MilliSleep(1500);

                _helpers.RunCmd(pg_init, false);
            }
        }
    }

    void initBindSettings()
    {
        if(_items->getBindStatus() == "X") {
            _helpers.Println("Bind Settings Configuring...", _helpers.YELLOW);
            _logger.Info("Bind Settings Configuring...");
            std::string bind_init;
            std::string new_rndc_key;
            if(_config->getValue("system.os").compare("centos") == 0) {
                if(_helpers.IsDirExist("/etc/named")) {
                    _helpers.RunCmdWithOutput("rm -rf /etc/named");
                    _helpers.RunCmdWithOutput("rm -rf /etc/named.conf");
                }
                _helpers.RunCmdWithOutput("echo \"include \\\"/etc/bind/named.conf\\\";\" > /etc/named.conf");
                _helpers.CreateDirectory("/var/cache/bind");
                _helpers.RunCmd("chown named:named /var/cache/bind", _logger.debug);
                bind_init = "systemctl restart named.service";
            }
            else {
                if(_helpers.IsDirExist("/etc/bind")) {
                    _helpers.RunCmdWithOutput("rm -rf /etc/bind");
                }
                _helpers.CreateDirectory("/var/cache/bind");
                _helpers.RunCmd("chown bind:bind /var/cache/bind", _logger.debug);
                bind_init = "systemctl restart bind9.service";
            }

            _helpers.RunCmd("yes | cp -rPf "+ _config->getValue("panel.template.directory") +"bind /etc/", _logger.debug);
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd("rndc-confgen -a -b 128 -c /etc/bind/rndc.key -k rndc-key -r /dev/urandom", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                _helpers.RunCmd("rndc-confgen -a -b 128 -c /etc/bind/rndc.key -k rndc-key -r /dev/urandom", _logger.debug);

            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.RunCmd("rndc-confgen -a -b 128 -c /etc/bind/rndc.key -k rndc-key -r /dev/urandom", _logger.debug);
            }
            _helpers.RunCmd(bind_init, _logger.debug);
        }
    }

    void initProftpSettings()
    {
        if(_items->getProftpStatus() == "X") {
            _helpers.Println("ProFTPD Settings Configuring...", _helpers.YELLOW);
            _logger.Info("ProFTPD Settings Configuring...");
            std::string proftp_init = "/etc/init.d/proftpd restart";

            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd("setsebool -P allow_ftpd_full_access=1", _logger.debug);
                _helpers.RunCmdWithOutput("sed -i 's/SELINUX=.*/SELINUX=disabled/g' /etc/sysconfig/selinux");
                if((_config->getValue("system.version") == "8")) {
                    proftp_init = "systemctl restart proftpd.service";
                }

                // TODO: dogru hostname'i sisteme kaydet yoksa proftpd calismiyor /etc/hostname ve /etc/hosts dosyalarinda duzeltme yapilacak
            }
            else {
                if((_config->getValue("system.version").compare("10") == 0)) {
                    proftp_init = "systemctl restart proftpd.service";
                }
            }

            _helpers.RunCmdWithOutput("rm -rf /etc/proftpd.conf");
            _helpers.RunCmd("yes | cp -rPf "+ _config->getValue("panel.template.directory") +"proftpd/proftpd.conf /etc/", _logger.debug);
            _helpers.RunCmd("yes | cp -rPf "+ _config->getValue("panel.template.directory") +"proftpd /etc/", _logger.debug);
            _helpers.RunCmd("sed -i 's/#HOSTNAME#/" + _items->getHostname() + "/g' /etc/proftpd.conf", _logger.debug);
            _helpers.RunCmd(proftp_init, _logger.debug);
        }
    }

    void initApacheSettings()
    {
        if(_items->getApacheStatus() == "X") {
            _helpers.Println("Apache Settings Configuring...", _helpers.YELLOW);
            _logger.Info("Apache Settings Configuring...");
            std::string apache_dir;
            std::string apache_init;
            std::string apache_template_dir = _config->getValue("panel.template.directory") + "apache2";

            _helpers.CreateDirectory(_config->getValue("vhosts.dir"));
            _helpers.CreateDirectory(_config->getValue("vhosts.dir") + "/default");
            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
                apache_dir = "/etc/apache2/";
                apache_init = "/etc/init.d/apache2 restart";
                if((_config->getValue("system.version").compare("10") == 0) || (_config->getValue("system.version").compare("20") == 0)) {
                    apache_init = "systemctl restart apache2.service";
                }
                _helpers.RunCmdWithOutput("rm -rf " + apache_dir + "apache2.conf");
                _helpers.RunCmdWithOutput("rm -rf " + apache_dir + "conf*");
                _helpers.RunCmdWithOutput("rm -rf " + apache_dir + "sites-available");
                _helpers.RunCmdWithOutput("rm -rf " + apache_dir + "sites-enabled");
                _helpers.RunCmd("yes | cp -rPf "+ apache_template_dir +"/* " + apache_dir, _logger.debug);
                _helpers.RunCmdWithOutput("sed -i 's/IPADDRESS/"+ _default_ip_addr +"/g' "+ apache_dir +"conf.d/zzz*");
                _helpers.RunCmdWithOutput("sed -i 's/#HOSTNAME#/"+ _new_hostname +"/g' "+ apache_dir +"conf.d/zzz*");
                _helpers.RunCmdWithOutput("sed -i 's/"+ _old_hostname +"/"+ _new_hostname +"/g' "+ apache_dir +"conf.d/zzz*");
                if((_config->getValue("system.os").compare("debian") == 0)) {
                    _helpers.RunCmd("ln -s /etc/apache2/mods-available/ssl.conf /etc/apache2/mods-enabled/ssl.conf", _logger.debug);
                    _helpers.RunCmd("ln -s /etc/apache2/mods-available/ssl.load /etc/apache2/mods-enabled/ssl.load", _logger.debug);
                }
                else {
                    _helpers.RunCmd("rm -rf /etc/apache2/mods-enabled/ssl.*", _logger.debug);
                    _helpers.RunCmd("a2enmod ssl", _logger.debug);
                }

                _helpers.RunCmd("killall /usr/sbin/apache2", _logger.debug);
                if(_config->getValue("system.version").compare("10") == 0) {
                    if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "mod_ruid2" + _config->getValue("system.package.extension"))) {
                        _logger.Error("mod_ruid2" + _config->getValue("system.package.extension") + " is not exists");
                    }
                    _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "mod_ruid2" + _config->getValue("system.package.extension"), _logger.debug);
                    _helpers.RunCmd("a2enmod ruid2", _logger.debug);
                    _helpers.RemoveFile(_config->getValue("panel.tmp.directory") + "mod_ruid2" + _config->getValue("system.package.extension"));
                }
                _helpers.RunCmdWithOutput("ln -s /var/log/apache2 "+ apache_dir +"logs");
                _helpers.RunCmdWithOutput("ln -s /var/run/apache2 "+ apache_dir +"run");
                _helpers.RunCmdWithOutput("ln -s /etc/mime.types "+ apache_dir +"conf.d/mime.types");
                _helpers.RunCmdWithOutput("rm -rf /etc/apache2/mods-enabled/python.load");

            }
            else if(_config->getValue("system.os").compare("centos") == 0) {
                apache_dir = "/etc/httpd/";
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "mod_ruid2" + _config->getValue("system.package.extension"))) {
                    _logger.Error("mod_ruid2" + _config->getValue("system.package.extension") + " is not exists");
                }
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "mod_ruid2" + _config->getValue("system.package.extension"), _logger.debug);
                _helpers.RunCmdWithOutput("rm -rf " + apache_dir + "conf");
                _helpers.RunCmdWithOutput("rm -rf " + apache_dir + "conf.d");
                _helpers.RunCmdWithOutput("rm -rf " + apache_dir + "conf.modules.d");
                _helpers.RunCmd("yes | cp -rPf "+ apache_template_dir +"/* " + apache_dir, _logger.debug);
//                _helpers.RunCmdWithOutput("sed -i 's/IPADDRESS/"+ _default_ip_addr +"/g' "+ apache_dir +"conf.d/zzz001_httpd.conf");
                _helpers.RunCmdWithOutput("sed -i 's/IPADDRESS/"+ _default_ip_addr +"/g' "+ apache_dir +"conf.d/zzz002_httpd.conf");
                _helpers.RunCmdWithOutput("sed -i 's/"+ _old_hostname +"/"+ _new_hostname +"/g' "+ apache_dir +"conf.d/zzz002_httpd.conf");


                if(_items->getPythonStatus() == "X") {
                    if(_items->getApacheStatus() == "X") {
                        _helpers.writeNewFile("/usr/local/fpanel/core/templates/apache2/mods-available/python.load", "LoadModule python_module modules/mod_python.so");
                    }
                }

                apache_init = "/etc/init.d/httpd restart";
                if((_config->getValue("system.version") == "8")) {
                    apache_init = "systemctl restart httpd.service";
                }

                if(_helpers.IsDirExist("/var/lib/php/session/"))
                    _helpers.RunCmdWithOutput("chmod 0777 /var/lib/php/session/");
                _helpers.RunCmd(_config->getValue("system.install.command") + " mod_perl", _logger.debug);

                if(!_helpers.IsFileExists("/etc/httpd/mods-available/ssl.load")) {
                    _helpers.writeNewFile("/etc/httpd/mods-available/ssl.load", "LoadModule ssl_module /etc/httpd/modules/mod_ssl.so");
                }

                std::string ports_conf = "NameVirtualHost *:80\n";
                ports_conf += "\n";
                ports_conf += "<IfModule mod_ssl.c>\n";
                ports_conf += "    Listen 443\n";
                ports_conf += "</IfModule>\n";
                ports_conf += "\n";
                ports_conf += "<IfModule mod_gnutls.c>\n";
                ports_conf += "    Listen 443\n";
                ports_conf += "</IfModule>";

                _helpers.writeNewFile("/etc/httpd/conf.d/ports.conf", ports_conf);

                _helpers.CreateDirectorySymLink("/var/log/httpd", "/etc/httpd/logs");
                std::vector<std::string> files = {
                                              "alias.load",
                                              "access_compat.load",
                                              "actions.load",
                                              "auth_basic.load",
                                              "authn_core.load",
                                              "authn_file.load",
                                              "authz_core.load",
                                              "authz_host.load",
                                              "authz_user.load",
                                              "cgi.load",
                                              "deflate.load",
                                              "dir.load",
                                              "env.load",
                                              "filter.load",
                                              "headers.load",
                                              "include.load",
                                              "mime.load",
                                              "mpm_prefork.load",
                                              "negotiation.load",
                                              "reqtimeout.load",
                                              "rewrite.load",
                                              "status.load",
                                              "systemd.load",
                                              "unixd.load",
                                              "userdir.conf",
                                              "userdir.load",
                                              "ssl.load",
                                              "vhost_alias.load",
                                              "vhost_alias.load",
                                              "logio.load",
                                              "log_config.load",
                                              "ruid2.load",
                                              "wsgi.load",
                                              "perl.load"};
                for(std::string file : files) {
                    _helpers.CreateFileSymLink(apache_dir + "mods-available/" + file, apache_dir + "mods-enabled/" + file);
                }
                _helpers.RunCmd("rm -rf /etc/httpd/conf.modules.d", _logger.debug);
                if(_items->getPhp56Status() == "X") {
                    _helpers.CreateFileSymLink(apache_dir + "mods-available/php56.load", apache_dir + "mods-enabled/php56.load");
                    _helpers.CreateFileSymLink(apache_dir + "mods-available/php56.conf", apache_dir + "mods-enabled/php56.conf");
                }
                if(_items->getPhp72Status() == "X") {
                    _helpers.CreateFileSymLink(apache_dir + "mods-available/php72.load", apache_dir + "mods-enabled/php72.load");
                    _helpers.CreateFileSymLink(apache_dir + "mods-available/php72.conf", apache_dir + "mods-enabled/php72.conf");
                }
                if(_items->getPhp74Status() == "X") {
                    _helpers.CreateFileSymLink(apache_dir + "mods-available/php74.load", apache_dir + "mods-enabled/php74.load");
                    _helpers.CreateFileSymLink(apache_dir + "mods-available/php74.conf", apache_dir + "mods-enabled/php74.conf");
                }
            }

            if(_items->getdbMyAdminStatus() == "X") {
                _helpers.RunCmdWithOutput("sed -i 's/"+ _old_hostname +"/"+ _new_hostname +"/g' "+ apache_dir +"conf.d/zzz003_dbmyadmin.conf");
            }
            else {
                _helpers.RunCmdWithOutput("rm -rf "+ apache_dir +"conf.d/zzz003_dbmyadmin.conf");
            }

            if(_items->getRoundCubeStatus() == "X") {
                _helpers.RunCmdWithOutput("sed -i 's/"+ _old_hostname +"/"+ _new_hostname +"/g' "+ apache_dir +"conf.d/zzz004_webmail_rc.conf");
                std::string custom_des_key = _helpers.GenerateHex(12);
                _helpers.RunCmdWithOutput("sed -i 's/rcmail__24ByteDESkey_Str/" + custom_des_key + "/g' " + _config->getValue("vhosts.dir") + "/webmail_rc/config/config.inc.php");
            }
            else {
                _helpers.RunCmdWithOutput("rm -rf "+ apache_dir +"conf.d/zzz004_webmail_rc.conf");
            }

            _helpers.RunCmd(apache_init, _logger.debug);
        }
        else {
            if(_config->getValue("system.os").compare("centos") == 0) {
                if(_helpers.IsDirExist("/etc/httpd")) {
                    _helpers.CmdOutput("rm -rf /etc/httpd");
                }
            }
            else {
                if(_helpers.IsDirExist("/etc/apache2")) {
                    _helpers.CmdOutput("rm -rf /etc/apache2");
                }
            }
        }
    }

    void initNginxSettings()
    {
        if(_items->getApacheNginxStatus() == "X") {
            _helpers.Println("Nginx Settings Configuring...", _helpers.YELLOW);
            _logger.Info("Nginx Settings Configuring...");
            _helpers.CreateDirectory(_config->getValue("vhosts.dir"));
            _helpers.CreateDirectory(_config->getValue("vhosts.dir") + "/default");

            std::string nginx_dir = "/etc/nginx/";
            std::string nginx_init = "/etc/init.d/nginx restart";
            std::string nginx_user = "www-data";
            std::string nginx_group = "nogroup";

            if(_config->getValue("system.os").compare("centos") == 0) {
                nginx_user = "nginx";
                nginx_group = "nobody";
            }

            std::string default_php_fpm_dir = "/etc/php/fpm";
            std::string php_fpm_run_dir="/var/spool/php-fpm";
            std::string php_default_fpm_init="/etc/init.d/php-fpm";

            std::string php_fpm_init;
            std::string php_fpm_dir;

            if(_items->getPhp56Status() == "X") {
                php_fpm_dir = "/etc/php/5.6/fpm";
                php_fpm_init = "/etc/init.d/php5.6-fpm";
            }
            if(_items->getPhp72Status() == "X") {
                php_fpm_dir = "/etc/php/7.2/fpm";
                php_fpm_init = "/etc/init.d/php7.2-fpm";
            }
            if(_items->getPhp74Status() == "X") {
                php_fpm_dir = "/etc/php/7.4/fpm";
                php_fpm_init = "/etc/init.d/php7.4-fpm";
            }

            _helpers.CreateDirectory(php_fpm_run_dir);
            _helpers.CmdOutput("ln -s " + php_fpm_dir + " " + default_php_fpm_dir);
            _helpers.CmdOutput("ln -s " + php_fpm_init + " " + php_default_fpm_init);

            std::string json_log_format = "log_format combined_json escape=json '{\"msec\": \"$msec\",\"connection\": \"$connection\",\"connection_requests\": \"$connection_requests\",\"pid\": \"$pid\",\"request_id\": \"$request_id\",\"request_length\": \"$request_length\",\"remote_addr\": \"$remote_addr\",\"remote_user\": \"$remote_user\",\"remote_port\": \"$remote_port\",\"time_local\": \"$time_local\", \"time_iso8601\": \"$time_iso8601\",\"request\": \"$request\",\"request_uri\": \"$request_uri\",\"args\": \"$args\",\"status\": \"$status\",\"body_bytes_sent\": \"$body_bytes_sent\",\"bytes_sent\": \"$bytes_sent\",\"http_referer\": \"$http_referer\",\"http_user_agent\": \"$http_user_agent\",\"http_x_forwarded_for\": \"$http_x_forwarded_for\",\"http_host\": \"$http_host\",\"server_name\": \"$server_name\",\"request_time\": \"$request_time\",\"upstream\": \"$upstream_addr\",\"upstream_connect_time\": \"$upstream_connect_time\",\"upstream_header_time\": \"$upstream_header_time\",\"upstream_response_time\": \"$upstream_response_time\",\"upstream_response_length\": \"$upstream_response_length\",\"upstream_cache_status\": \"$upstream_cache_status\",\"ssl_protocol\": \"$ssl_protocol\",\"ssl_cipher\": \"$ssl_cipher\",\"scheme\": \"$scheme\",\"request_method\": \"$request_method\",\"server_protocol\": \"$server_protocol\",\"pipe\": \"$pipe\",\"gzip_ratio\": \"$gzip_ratio\", \"http_cf_ray\": \"$http_cf_ray\"}';";
            std::vector<std::string> nginx_conf_content = _helpers.readFile("/etc/nginx/nginx.conf");
            std::string new_content = "";
            for(std::size_t i = 0; i < nginx_conf_content.size(); ++i) {
                if(_helpers.IfStringContains(nginx_conf_content[i], "http {")) {
                    nginx_conf_content[i] = "http {\n\t" + json_log_format;
                }
                new_content += nginx_conf_content[i] + "\n";
            }

            _helpers.writeNewFile("/etc/nginx/nginx.conf", new_content);

            std::string zzz002_content = "# FPanel - Dont edit this file.\n";
            zzz002_content += "server {\n";
            zzz002_content += "\tinclude "+ nginx_dir +"conf.d/zzz001_httpd.include;\n";
            zzz002_content += "\tserver_name _;\n";
            zzz002_content += "\tindex index.html index.htm;\n";

            zzz002_content += "\taccess_log /var/log/nginx/access.log combined_json;\n";
            zzz002_content += "\terror_log /var/log/nginx/error.log;\n";

            if(_items->getdbMyAdminStatus() == "X") {
                zzz002_content += "\tinclude "+ nginx_dir +"conf.d/zzz003_httpd.include;\n";
            }
            if(_items->getRoundCubeStatus() == "X") {
                zzz002_content += "\tinclude "+ nginx_dir +"conf.d/zzz004_httpd.include;\n";
            }

            zzz002_content += "\tlocation ^~ / {\n";
            zzz002_content += "\t\troot "+ _config->getValue("vhosts.dir") +"/default/;\n";
            zzz002_content += "\t\tindex  index.php index.html index.htm;\n";
            zzz002_content += "\t}\n";

            zzz002_content += "\terror_page 500 502 503 504 /50x.html;\n";
            zzz002_content += "\tlocation ~ /\\.ht {\n";
            zzz002_content += "\t\tdeny all;\n";
            zzz002_content += "\t}\n";
            zzz002_content += "}";

            std::string zzz001_content = "# FPanel - Dont edit this file.\n";
            zzz001_content += "listen "+ _default_ip_addr +":80;\n";

            std::string zzz999_content = "# FPanel - Dont edit this file.\n";

            _helpers.writeNewFile(nginx_dir + "conf.d/zzz001_httpd.include", zzz001_content);
            _helpers.writeNewFile(nginx_dir + "conf.d/zzz002_httpd.conf", zzz002_content);
            _helpers.writeNewFile(nginx_dir + "conf.d/zzz999_httpd.conf", zzz999_content);

            if(_items->getRoundCubeStatus() == "X") {
                std::string zzz004_content = "# FPanel - Dont edit this file.\n";
                zzz004_content += "\tlocation ^~ /webmail {\n";
                zzz004_content += "\t\troot /var/www/;\n";
                zzz004_content += "\t\ttry_files $uri $uri/ index.php;\n";
                zzz004_content += "\t\tindex  index.php index.html index.htm;\n";
                zzz004_content += "\t\tlocation ~ \\.php$ {\n";
                zzz004_content += "\t\t\tfastcgi_split_path_info ^(.+?\\.php)(/.*)?$;\n";
                zzz004_content += "\t\t\tfastcgi_pass unix:"+ php_fpm_run_dir +"/webmail_rc.sock;\n";
                zzz004_content += "\t\t\tfastcgi_index index.php;\n";
                zzz004_content += "\t\t\tinclude fastcgi_params;\n";
                zzz004_content += "\t\t\tfastcgi_param  SCRIPT_FILENAME  $document_root$fastcgi_script_name;\n";
                zzz004_content += "\t\t\tfastcgi_param  QUERY_STRING     $query_string;\n";
                zzz004_content += "\t\t\tfastcgi_param  REQUEST_METHOD   $request_method;\n";
                zzz004_content += "\t\t\tfastcgi_param  CONTENT_TYPE     $content_type;\n";
                zzz004_content += "\t\t\tfastcgi_param  CONTENT_LENGTH   $content_length;\n";
                zzz004_content += "\t\t\tfastcgi_intercept_errors        on;\n";
                zzz004_content += "\t\t\tfastcgi_ignore_client_abort     off;\n";
                zzz004_content += "\t\t\tfastcgi_connect_timeout 60;\n";
                zzz004_content += "\t\t\tfastcgi_send_timeout 180;\n";
                zzz004_content += "\t\t\tfastcgi_read_timeout 180;\n";
                zzz004_content += "\t\t\tfastcgi_buffer_size 128k;\n";
                zzz004_content += "\t\t\tfastcgi_buffers 4 256k;\n";
                zzz004_content += "\t\t\tfastcgi_busy_buffers_size 256k;\n";
                zzz004_content += "\t\t\tfastcgi_temp_file_write_size 256k;\n";
                zzz004_content += "\t\t}\n";
                zzz004_content += "\t}";
                _helpers.writeNewFile(nginx_dir + "conf.d/zzz004_httpd.include", zzz004_content);

                std::string php_fpm_zzz004_content = "[webmail_rc]\n";
                php_fpm_zzz004_content += "listen = "+ php_fpm_run_dir +"/webmail_rc.sock\n";
                php_fpm_zzz004_content += "listen.owner = "+ nginx_user +"\n";
                php_fpm_zzz004_content += "listen.group = "+ nginx_group +"\n";
                php_fpm_zzz004_content += "listen.mode = 0660\n";
                php_fpm_zzz004_content += "user = "+ nginx_user +"\n";
                php_fpm_zzz004_content += "group = "+ nginx_group +"\n";
                php_fpm_zzz004_content += "pm = dynamic\n";
                php_fpm_zzz004_content += "pm.max_children = 50\n";
                php_fpm_zzz004_content += "pm.start_servers = 20\n";
                php_fpm_zzz004_content += "pm.min_spare_servers = 5\n";
                php_fpm_zzz004_content += "pm.max_spare_servers = 35\n";
                php_fpm_zzz004_content += "chdir = "+ _config->getValue("vhosts.dir") +"/webmail\n";
                php_fpm_zzz004_content += "php_admin_value[open_basedir] = "+ _config->getValue("vhosts.dir") +"/webmail:/tmp\n";
                php_fpm_zzz004_content += "php_admin_value[disable_functions] = dl,passthru,parse_ini_file,show_source";

                _helpers.writeNewFile(php_fpm_dir + "/pool.d/webmail_rc.conf", php_fpm_zzz004_content);
            }

            if(_items->getdbMyAdminStatus() == "X") {
                std::string zzz003_content = "# FPanel - Dont edit this file.\n";
                zzz003_content += "\tlocation ^~ /dbmyadmin {\n";
                zzz003_content += "\t\troot /var/www/;\n";
                zzz003_content += "\t\ttry_files $uri $uri/ index.php;\n";
                zzz003_content += "\t\tindex  index.php index.html index.htm;\n";
                zzz003_content += "\t\tlocation ~ \\.php$ {\n";
                zzz003_content += "\t\t\tfastcgi_split_path_info ^(.+?\\.php)(/.*)?$;\n";
                zzz003_content += "\t\t\tfastcgi_pass unix:"+ php_fpm_run_dir +"/dbmyadmin.sock;\n";
                zzz003_content += "\t\t\tfastcgi_index index.php;\n";
                zzz003_content += "\t\t\tinclude fastcgi_params;\n";
                zzz003_content += "\t\t\tfastcgi_param  SCRIPT_FILENAME  $document_root$fastcgi_script_name;\n";
                zzz003_content += "\t\t\tfastcgi_param  QUERY_STRING     $query_string;\n";
                zzz003_content += "\t\t\tfastcgi_param  REQUEST_METHOD   $request_method;\n";
                zzz003_content += "\t\t\tfastcgi_param  CONTENT_TYPE     $content_type;\n";
                zzz003_content += "\t\t\tfastcgi_param  CONTENT_LENGTH   $content_length;\n";
                zzz003_content += "\t\t\tfastcgi_intercept_errors        on;\n";
                zzz003_content += "\t\t\tfastcgi_ignore_client_abort     off;\n";
                zzz003_content += "\t\t\tfastcgi_connect_timeout 60;\n";
                zzz003_content += "\t\t\tfastcgi_send_timeout 180;\n";
                zzz003_content += "\t\t\tfastcgi_read_timeout 180;\n";
                zzz003_content += "\t\t\tfastcgi_buffer_size 128k;\n";
                zzz003_content += "\t\t\tfastcgi_buffers 4 256k;\n";
                zzz003_content += "\t\t\tfastcgi_busy_buffers_size 256k;\n";
                zzz003_content += "\t\t\tfastcgi_temp_file_write_size 256k;\n";
                zzz003_content += "\t\t}\n";
                zzz003_content += "\t}";
                _helpers.writeNewFile(nginx_dir + "conf.d/zzz003_httpd.include", zzz003_content);

                std::string php_fpm_zzz003_content = "[dbmyadmin]\n";
                php_fpm_zzz003_content += "listen = "+ php_fpm_run_dir +"/dbmyadmin.sock\n";
                php_fpm_zzz003_content += "listen.owner = "+ nginx_user +"\n";
                php_fpm_zzz003_content += "listen.group = "+ nginx_group +"\n";
                php_fpm_zzz003_content += "listen.mode = 0660\n";
                php_fpm_zzz003_content += "user = "+ nginx_user +"\n";
                php_fpm_zzz003_content += "group = "+ nginx_group +"\n";
                php_fpm_zzz003_content += "pm = dynamic\n";
                php_fpm_zzz003_content += "pm.max_children = 50\n";
                php_fpm_zzz003_content += "pm.start_servers = 20\n";
                php_fpm_zzz003_content += "pm.min_spare_servers = 5\n";
                php_fpm_zzz003_content += "pm.max_spare_servers = 35\n";
                php_fpm_zzz003_content += "chdir = "+ _config->getValue("vhosts.dir") +"/dbmyadmin\n";
                php_fpm_zzz003_content += "php_admin_value[open_basedir] = "+ _config->getValue("vhosts.dir") +"/dbmyadmin:/tmp\n";
                php_fpm_zzz003_content += "php_admin_value[disable_functions] = dl,passthru,parse_ini_file,show_source";

                _helpers.writeNewFile(php_fpm_dir + "/pool.d/dbmyadmin.conf", php_fpm_zzz003_content);
            }

            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
                if((_config->getValue("system.version").compare("10") == 0) || (_config->getValue("system.version").compare("20") == 0)) {
                    nginx_init = "systemctl restart nginx.service";
                }
            }
            else if(_config->getValue("system.os").compare("centos") == 0) {
                if((_config->getValue("system.version") == "8")) {
                    nginx_init = "systemctl restart nginx.service";
                }
            }

            _helpers.RunCmdWithOutput("sed -i 's/include \\/etc\\/nginx\\/sites-enabled.*//g' " + nginx_dir + "nginx.conf");
            _helpers.RunCmd("rm -rf "+ nginx_dir +"/sites-enabled", _logger.debug);
            _helpers.RunCmd("rm -rf "+ nginx_dir +"/sites-available", _logger.debug);
            _helpers.RunCmdWithOutput("mv "+ _config->getValue("vhosts.dir") +"/webmail_rc "+ _config->getValue("vhosts.dir") +"/webmail");
            _helpers.RunCmd("update-rc.d php-fpm defaults", _logger.debug);
            _helpers.RunCmd(php_fpm_init + " restart", _logger.debug);
            _helpers.RunCmd(nginx_init, _logger.debug);
        }
    }

    void initPhpSettings()
    {
        if(_items->getPhpStatus() == "X") {
            _helpers.Println("PHP Settings Configuring...", _helpers.YELLOW);
            _logger.Info("PHP Settings Configuring...");
            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {

            }
            else if(_config->getValue("system.os").compare("centos") == 0) {

            }
        }
    }

    void initPostfixDovecotSettings()
    {
        if(_items->getPostfixStatus() == "X") {
            _helpers.Println("Postfix Settings Configuring...", _helpers.YELLOW);
            _logger.Info("Postfix Settings Configuring...");
            _helpers.RemoveFile("/etc/init.d/dovecot");
            _helpers.CreateFileSymLink("/usr/local/fpanel/service-scripts/dovecot", "/etc/init.d/dovecot");
            _helpers.CreateFileSymLink("/usr/local/fpanel/service-scripts/postfix", "/etc/init.d/postfix");

            if((_config->getValue("system.os").compare("centos") == 0)) {
                _helpers.RunCmd("userdel postfix", _logger.debug);
                _helpers.RunCmd("groupdel postfix", _logger.debug);
                _helpers.RunCmd("groupdel postdrop", _logger.debug);
            }

            if(_helpers.IsFileExists("/lib/systemd/system/dovecot.service")) {
                _helpers.RemoveFile( "/lib/systemd/system/dovecot.service");
            }

            _helpers.CopyFile("/usr/local/fpanel/service-scripts/dovecot.service", "/lib/systemd/system/dovecot.service");

            std::string old_hostname = "#HOSTNAME#";
            _helpers.RunCmdWithOutput("sed -i 's/"+ old_hostname +"/"+ _new_hostname +"/g' "+ _config->getValue("panel.dir") +"mail/etc/main.cf");
            _helpers.RunCmd("yes | cp -rPf "+ _config->getValue("panel.template.directory") +"dovecot/* /etc/dovecot/", _logger.debug);
            _helpers.RunCmdWithOutput("sed -i 's/postmaster@HOSTNAME/postmaster@"+ _new_hostname +"/g' /etc/dovecot/dovecot.conf");
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/private", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/etc/postfix-files.d", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/active", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/bounce", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/corrupt", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/defer", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/deferred", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/flush", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/hold", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/incoming", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/maildrop", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/pid", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/private", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/public", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/saved", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/trace", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/policyd", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/var/spool/autoresponse", _logger.debug);
            _helpers.RunCmd("mkdir -p "+ _config->getValue("panel.dir") +"mail/domains", _logger.debug);
            _helpers.RunCmd("chmod 770 "+ _config->getValue("panel.dir") +"mail/domains", _logger.debug);
            _helpers.CreateDirectory(_config->getValue("panel.dir") +"mail/lib");


            _helpers.RunCmd("groupadd -g 189 postfwd", _logger.debug);
            _helpers.RunCmd("useradd -c \"Postfix Firewall User\" -d "+ _config->getValue("panel.dir") +"mail/etc -g postfwd -s /bin/false -u 189 postfwd", _logger.debug);

            _helpers.RunCmd("groupadd -g 90 postdrop", _logger.debug);
            _helpers.RunCmd("groupadd -g 89 postfix", _logger.debug);
            _helpers.RunCmd("useradd -c \"Postfix Daemon User\" -d "+ _config->getValue("panel.dir") +"mail/var/spool -g postfix -s /bin/false -u 89 postfix", _logger.debug);
            _helpers.RunCmd("groupadd vmail -g 5000", _logger.debug);
            _helpers.RunCmd("useradd -r -u 5000 -g vmail -d "+ _config->getValue("panel.dir") +"mail/domains -s /sbin/nologin -c \"virtual mailbox handler\" vmail", _logger.debug);
            _helpers.RunCmd("chown -R vmail:vmail "+ _config->getValue("panel.dir") +"mail/domains", _logger.debug);

            _helpers.Println("Dovecot Settings Configuring...", _helpers.YELLOW);
            _logger.Info("Dovecot Settings Configuring...");
            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmdWithOutput("ln -s /usr/libexec/dovecot/dovecot-lda /usr/lib64/dovecot/dovecot-lda");
                _helpers.RunCmd("setsebool -P httpd_can_network_connect 1", _logger.debug);
                _helpers.RunCmd("mkdir -p /var/run/dovecot", _logger.debug);
                _helpers.RunCmd("groupadd -g 187 dovecot", _logger.debug);
                _helpers.RunCmd("useradd -c \"Dovecot unprivileged user\" -d /dev/null -u 187 -g dovecot -s /bin/false dovecot", _logger.debug);
                _helpers.RunCmd("groupadd -g 188 dovenull", _logger.debug);
                _helpers.RunCmd("useradd -c \"Dovecot login user\" -d /dev/null -u 188 -g dovenull -s /bin/false dovenull", _logger.debug);
            }

            _helpers.RunCmd("groupadd -g 199 autoresponder", _logger.debug);
            _helpers.RunCmd("useradd -c \"Postfix Autoresponder User\" -d "+ _config->getValue("panel.dir") +"mail/var/spool/autoresponse -g autoresponder -s /bin/false -u 199 autoresponder", _logger.debug);


            std::string main_proc = _config->getValue("panel.dir") + "mail/libexec/master";
            std::string pid = _helpers.CmdOutput("pidof \"" + main_proc + "\"");
            _helpers.trim(pid);
            if(!pid.empty()) {
                _helpers.RunCmd("kill -9 " + pid, _logger.debug);
            }

            ////////////////// ALIASES ////////////////////////
            if(!_helpers.IsFileExists("/etc/aliases")) {
                std::string aliases_content = "#\n";
                aliases_content += "#  Aliases in this file will NOT be expanded in the header from\n";
                aliases_content += "#  Mail, but WILL be visible over networks or from /bin/mail.\n";
                aliases_content += "#\n";
                aliases_content += "#       >>>>>>>>>>      The program \"newaliases\" must be run after\n";
                aliases_content += "#       >> NOTE >>      this file is updated for any changes to\n";
                aliases_content += "#       >>>>>>>>>>      show through to sendmail.\n";
                aliases_content += "#\n";
                aliases_content += "\n";
                aliases_content += "# Basic system aliases -- these MUST be present.\n";
                aliases_content += "mailer-daemon:  postmaster\n";
                aliases_content += "postmaster:     root\n";
                aliases_content += "\n";
                aliases_content += "# General redirections for pseudo accounts.\n";
                aliases_content += "bin:            root\n";
                aliases_content += "daemon:         root\n";
                aliases_content += "adm:            root\n";
                aliases_content += "lp:             root\n";
                aliases_content += "sync:           root\n";
                aliases_content += "shutdown:       root\n";
                aliases_content += "halt:           root\n";
                aliases_content += "mail:           root\n";
                aliases_content += "news:           root\n";
                aliases_content += "uucp:           root\n";
                aliases_content += "operator:       root\n";
                aliases_content += "games:          root\n";
                aliases_content += "gopher:         root\n";
                aliases_content += "ftp:            root\n";
                aliases_content += "nobody:         root\n";
                aliases_content += "radiusd:        root\n";
                aliases_content += "nut:            root\n";
                aliases_content += "dbus:           root\n";
                aliases_content += "vcsa:           root\n";
                aliases_content += "canna:          root\n";
                aliases_content += "wnn:            root\n";
                aliases_content += "rpm:            root\n";
                aliases_content += "nscd:           root\n";
                aliases_content += "pcap:           root\n";
                aliases_content += "apache:         root\n";
                aliases_content += "webalizer:      root\n";
                aliases_content += "dovecot:        root\n";
                aliases_content += "fax:            root\n";
                aliases_content += "quagga:         root\n";
                aliases_content += "radvd:          root\n";
                aliases_content += "pvm:            root\n";
                aliases_content += "amandabackup:           root\n";
                aliases_content += "privoxy:        root\n";
                aliases_content += "ident:          root\n";
                aliases_content += "named:          root\n";
                aliases_content += "xfs:            root\n";
                aliases_content += "gdm:            root\n";
                aliases_content += "mailnull:       root\n";
                aliases_content += "postgres:       root\n";
                aliases_content += "sshd:           root\n";
                aliases_content += "smmsp:          root\n";
                aliases_content += "postfix:        root\n";
                aliases_content += "netdump:        root\n";
                aliases_content += "ldap:           root\n";
                aliases_content += "squid:          root\n";
                aliases_content += "ntp:            root\n";
                aliases_content += "mysql:          root\n";
                aliases_content += "desktop:        root\n";
                aliases_content += "rpcuser:        root\n";
                aliases_content += "rpc:            root\n";
                aliases_content += "nfsnobody:      root\n";
                aliases_content += "\n";
                aliases_content += "ingres:         root\n";
                aliases_content += "system:         root\n";
                aliases_content += "toor:           root\n";
                aliases_content += "manager:        root\n";
                aliases_content += "dumper:         root\n";
                aliases_content += "abuse:          root\n";
                aliases_content += "\n";
                aliases_content += "newsadm:        news\n";
                aliases_content += "newsadmin:      news\n";
                aliases_content += "usenet:         news\n";
                aliases_content += "ftpadm:         ftp\n";
                aliases_content += "ftpadmin:       ftp\n";
                aliases_content += "ftp-adm:        ftp\n";
                aliases_content += "ftp-admin:      ftp\n";
                aliases_content += "www:            webmaster\n";
                aliases_content += "webmaster:      root\n";
                aliases_content += "noc:            root\n";
                aliases_content += "security:       root\n";
                aliases_content += "hostmaster:     root\n";
                aliases_content += "info:           postmaster\n";
                aliases_content += "marketing:      postmaster\n";
                aliases_content += "sales:          postmaster\n";
                aliases_content += "support:        postmaster\n";
                aliases_content += "\n";
                aliases_content += "\n";
                aliases_content += "# trap decode to catch security attacks\n";
                aliases_content += "decode:         root\n";
                aliases_content += "\n";
                aliases_content += "# Person who should get root's mail\n";
                aliases_content += "#root:          marc";

                _helpers.writeNewFile("/etc/aliases", aliases_content);
            }

            _helpers.RunCmd("yes | cp -rPf /etc/aliases "+ _config->getValue("panel.dir") +"mail/etc/aliases", _logger.debug);
            _helpers.RunCmd(_config->getValue("panel.dir") + "mail/sbin/postalias -c " +_config->getValue("panel.dir") + "mail/etc " + _config->getValue("panel.dir") +"mail/etc/aliases", _logger.debug);
            ////////////////// ALIASES END ////////////////////

            _helpers.RunCmd("chown root:root "+ _config->getValue("panel.dir") +"mail/etc/mail.db", _logger.debug);
            _helpers.RunCmd("chmod 600 "+ _config->getValue("panel.dir") +"mail/etc/mail.db", _logger.debug);

            _helpers.RunCmd("chown -R postfix:root "+ _config->getValue("panel.dir") +"mail/lib", _logger.debug);
            _helpers.RunCmd("chown -R postfix:root "+ _config->getValue("panel.dir") +"mail/var/spool/*", _logger.debug);

            _helpers.RunCmd("chown -R postfix:postdrop "+ _config->getValue("panel.dir") +"mail/var/spool/maildrop", _logger.debug);
            _helpers.RunCmd("chown -R postfix:postdrop "+ _config->getValue("panel.dir") +"mail/var/spool/public", _logger.debug);

            _helpers.RunCmd("chgrp postdrop "+ _config->getValue("panel.dir") +"mail/var/spool/maildrop", _logger.debug);
            _helpers.RunCmd("chgrp postdrop "+ _config->getValue("panel.dir") +"mail/var/spool/public", _logger.debug);

            _helpers.RunCmd("chmod g+s "+ _config->getValue("panel.dir") +"mail/var/spool/maildrop", _logger.debug);
            _helpers.RunCmd("chmod g+s "+ _config->getValue("panel.dir") +"mail/var/spool/public", _logger.debug);

            _helpers.RunCmd("chmod 3730 "+ _config->getValue("panel.dir") +"mail/var/spool/maildrop", _logger.debug);
            _helpers.RunCmd("chmod 2730 "+ _config->getValue("panel.dir") +"mail/var/spool/public", _logger.debug);

            _helpers.RunCmd("chown -R root:root "+ _config->getValue("panel.dir") +"mail/var/spool/pid", _logger.debug);

            _helpers.RunCmd("chown -R root:postdrop "+ _config->getValue("panel.dir") +"mail/sbin/postqueue", _logger.debug);
            _helpers.RunCmd("chown -R root:postdrop "+ _config->getValue("panel.dir") +"mail/sbin/postdrop", _logger.debug);
            _helpers.RunCmd("chown -R autoresponder:autoresponder "+ _config->getValue("panel.dir") +"mail/sbin/autoresponse", _logger.debug);

            _helpers.RunCmd("chgrp postdrop "+ _config->getValue("panel.dir") +"mail/sbin/postqueue", _logger.debug);
            _helpers.RunCmd("chgrp postdrop "+ _config->getValue("panel.dir") +"mail/sbin/postdrop", _logger.debug);

            _helpers.RunCmd("chmod g+s "+ _config->getValue("panel.dir") +"mail/sbin/postqueue", _logger.debug);
            _helpers.RunCmd("chmod g+s "+ _config->getValue("panel.dir") +"mail/sbin/postdrop", _logger.debug);


            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
                _helpers.RunCmd("systemctl daemon-reload", _logger.debug);
                _helpers.RunCmd("systemctl enable postfix", _logger.debug);
                _helpers.RunCmd("systemctl enable dovecot", _logger.debug);
                _helpers.RunCmd("systemctl enable postfwd", _logger.debug);
            }
            else {
                _helpers.RunCmd("/usr/local/fpanel/mail/sbin/postfix set-permissions", _logger.debug);
                _helpers.RunCmd("systemctl daemon-reload", _logger.debug);
                _helpers.RunCmd("systemctl enable postfix", _logger.debug);
                _helpers.RunCmd("systemctl enable dovecot", _logger.debug);
                _helpers.RunCmd("systemctl enable postfwd", _logger.debug);
                _helpers.RunCmd("chkconfig dovecot on", _logger.debug);
                _helpers.RunCmd("chkconfig postfwd on", _logger.debug);
            }
        }
    }

    void initSpamassassinSettings()
    {
        if(_items->getSpamAssassinStatus() == "X") {
            _helpers.Println("SpamAssassin Settings Configuring...", _helpers.YELLOW);
            _logger.Info("SpamAssassin Settings Configuring...");
            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
                _helpers.RunCmd("systemctl stop spamassassin", _logger.debug);
                _helpers.RunCmd("groupadd -g 5555 spamd", _logger.debug);
                _helpers.RunCmd("useradd -c \"SpamAssassin User\" -d /var/lib/spamassassin -u 99 -g spamd -s /sbin/nologin spamd", _logger.debug);
                _helpers.RunCmd("systemctl enable spamassassin.service", _logger.debug);

                _helpers.RunCmdWithOutput("sed -i 's/CRON=.*/CRON=1/g' /etc/default/spamassassin");
                _helpers.RunCmdWithOutput("sed -i 's/^OPTIONS=.*//g' /etc/default/spamassassin");
                _helpers.RunCmdWithOutput("sed -i 's/^ENABLED=.*//g' /etc/default/spamassassin");
                _helpers.RunCmdWithOutput("sed -i 's/^PIDFILE=.*//g' /etc/default/spamassassin");
                _helpers.RunCmdWithOutput("echo \"ENABLED=1\" >> /etc/default/spamassassin");
                _helpers.RunCmdWithOutput("echo \"PIDFILE=\"/var/run/spamd.pid\"\" >> /etc/default/spamassassin");
                _helpers.RunCmdWithOutput("echo \"OPTIONS=\"--create-prefs --max-children 5 --helper-home-dir --username spamd -H /var/log/spamassassin/ -s /var/log/spamassassin/spamd.log\"\" >> /etc/default/spamassassin");

                _helpers.RunCmd("razor-admin -home=/etc/spamassassin/.razor -create", _logger.debug);
                _helpers.RunCmd("razor-admin -home=/etc/spamassassin/.razor -register", _logger.debug);
                _helpers.RunCmd("razor-admin -home=/etc/spamassassin/.razor -discover", _logger.debug);
            }
            else if(_config->getValue("system.os").compare("centos") == 0) {
                std::string spam_conf_default = "/etc/default/spamassassin";
                _helpers.RunCmd("ln -s /etc/sysconfig/spamassassin " + spam_conf_default, _logger.debug);

                _helpers.RunCmd("systemctl stop spamassassin", _logger.debug);

                _helpers.RunCmd("groupadd -g 5555 spamd", _logger.debug);

                _helpers.RunCmd("useradd -c \"SpamAssassin User\" -d /var/lib/spamassassin -u 99 -g spamd -s /bin/false spamd", _logger.debug);

                _helpers.RunCmd("systemctl enable spamassassin.service", _logger.debug);

                _helpers.RunCmdWithOutput("sed -i 's/CRON=.*/CRON=1/g' " + spam_conf_default);
                _helpers.RunCmdWithOutput("sed -i 's/^OPTIONS=.*//g' " + spam_conf_default);
                _helpers.RunCmdWithOutput("sed -i 's/^ENABLED=.*//g' " + spam_conf_default);
                _helpers.RunCmdWithOutput("sed -i 's/^PIDFILE=.*//g' " + spam_conf_default);

                _helpers.RunCmdWithOutput("echo \"ENABLED=1\" >> " + spam_conf_default);
                _helpers.RunCmdWithOutput("echo \"PIDFILE=\"/var/run/spamd.pid\"\" >> " + spam_conf_default);
                _helpers.RunCmdWithOutput("echo \"OPTIONS=\"--create-prefs --max-children 5 --helper-home-dir --username spamd -H /var/log/spamassassin/ -s /var/log/spamassassin/spamd.log\"\" >> " + spam_conf_default);

                _helpers.RunCmdWithOutput("echo \"razor_config /etc/spamassassin/.razor/razor-agent.conf\" >> /etc/spamassassin/local.cf");
                _helpers.RunCmdWithOutput("echo \"pyzor_options –homedir /etc/spamassassin\" >> /etc/spamassassin/local.cf");

                _helpers.RunCmdWithOutput("echo \"use_bayes 1\" >> /etc/spamassassin/local.cf");
                _helpers.RunCmdWithOutput("echo \"required_score 5\" >> /etc/spamassassin/local.cf");
                _helpers.RunCmdWithOutput("echo \"report_safe 1\" >> /etc/spamassassin/local.cf");
                _helpers.RunCmdWithOutput("echo \"use_pyzor 1\" >> /etc/spamassassin/local.cf");
                _helpers.RunCmdWithOutput("echo \"use_razor2 1\" >> /etc/spamassassin/local.cf");
                _helpers.RunCmdWithOutput("echo \"skip_rbl_checks 1\" >> /etc/spamassassin/local.cf");
                _helpers.RunCmdWithOutput("sed -i 's/^#spamd //g' " + _config->getValue("panel.dir") + "/mail/etc/master.cf");
                _helpers.RunCmd("razor-admin -home=/etc/spamassassin/.razor -create", _logger.debug);
                _helpers.RunCmd("razor-admin -home=/etc/spamassassin/.razor -register", _logger.debug);
                _helpers.RunCmd("razor-admin -home=/etc/spamassassin/.razor -discover", _logger.debug);
            }
        }
    }

    void initMailmanSettings()
    {
        if(_items->getMailManStatus() == "X") {
            _helpers.Println("Mailman Settings Configuring...", _helpers.YELLOW);
            _logger.Info("Mailman Settings Configuring...");
            if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {

            }
            else if(_config->getValue("system.os").compare("centos") == 0) {

            }
        }
    }

    void initPanelSettings()
    {
        _helpers.Println("FPanel settings configuring...", _helpers.CYAN);
        _logger.Info("FPanel settings configuring...");

        if(!_cloud_client) {
            if(_helpers.IsFileExists(_config->getValue("panel.dir") + "redis/redis.conf")) {
                _helpers.Println("FPanel Settings Configuring...", _helpers.YELLOW);
                _logger.Info("FPanel Settings Configuring...");
                _helpers.RunCmdWithOutput("sed -i '/requirepass/d' "+ _config->getValue("panel.dir") + "redis/redis.conf");
                _helpers.RunCmdWithOutput("echo \"requirepass "+ _config->getValue("panel.database.password") +"\" >> "+ _config->getValue("panel.dir") + "redis/redis.conf");
            }

            if (_config->getValue("system.os").compare("centos") == 0) {
                if(!_helpers.IsFileExists(_config->getValue("panel.tmp.directory") + "lsb-core" + _config->getValue("system.package.extension"))) {
                    _logger.Error("lsb-core" + _config->getValue("system.package.extension") + " is not exists");
                }
                _helpers.RunCmd(_config->getValue("system.force.install.command") + " " + _config->getValue("panel.tmp.directory") + "lsb-core" + _config->getValue("system.package.extension"), _logger.debug);
                _helpers.RunCmd("mv /usr/local/fpanel/httpd/php/bin/\"#INST@25862#\" /usr/local/fpanel/httpd/php/bin/php-config", _logger.debug);
                _helpers.RunCmd("mv /usr/local/fpanel/httpd/php/bin/\"#INST@25790#\" /usr/local/fpanel/httpd/php/bin/phpize", _logger.debug);
                _helpers.RunCmd("mv /usr/local/fpanel/httpd/php/bin/\"#INST@7253#\" /usr/local/fpanel/httpd/php/bin/phpdbg", _logger.debug);
                _helpers.RunCmd("mv /usr/local/fpanel/httpd/php/bin/\"#INST@7662#\" /usr/local/fpanel/httpd/php/bin/php", _logger.debug);
                _helpers.RunCmd("mv /usr/local/fpanel/httpd/php/bin/\"#INST@7932#\" /usr/local/fpanel/httpd/php/bin/php-cgi", _logger.debug);
            }

            _helpers.RunCmd("systemctl enable panel-httpd", _logger.debug);
            _helpers.RunCmd("systemctl enable panel-ws", _logger.debug);
            _helpers.RunCmd("systemctl enable panel-redis", _logger.debug);
        }

        _helpers.RunCmd("systemctl enable panel-agent", _logger.debug);
        _helpers.RunCmd("systemctl enable panel-core", _logger.debug);

        if(_items->getProftpStatus() == "X") {
            _helpers.RunCmd("systemctl enable proftpd", _logger.debug);
        }

        if (_config->getValue("system.os").compare("centos") == 0) {
            if(_items->getApacheStatus() == "X") {
                _helpers.RunCmd("systemctl enable httpd", _logger.debug);
            }
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl enable named", _logger.debug);
            }

            _helpers.RunCmd("firewall-cmd --permanent --add-port=8800-8899/tcp", _logger.debug);
            _helpers.RunCmd("systemctl reload firewalld", _logger.debug);
        }
        else if (_config->getValue("system.os").compare("ubuntu") == 0) {
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl enable named", _logger.debug);
            }
            if(_items->getApacheStatus() == "X") {
                _helpers.RunCmd("systemctl enable apache2", _logger.debug);
            }
        }
        else if (_config->getValue("system.os").compare("debian") == 0) {
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl enable bind9", _logger.debug);
            }
            if(_items->getApacheStatus() == "X") {
                _helpers.RunCmd("systemctl enable apache2", _logger.debug);
            }
        }

        // Panel User settings
        //For httpd
        if(!_cloud_client) {
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "httpd/var");
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "httpd/var/logs");
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "httpd/var/tmp");
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "httpd/var/run");
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "httpd/var/cache");

            //For Redis
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "redis/tmp");
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "redis/logs");
            _helpers.CreateDirectory(_config->getValue("panel.dir") + "redis/lib");

            _helpers.RunCmd("chown -R fpanel:fpanel "+ _config->getValue("panel.dir") +"httpd/htdocs", _logger.debug);
            _helpers.RunCmd("chown fpanel:fpanel "+ _config->getValue("panel.dir") +"httpd/var/logs", _logger.debug);
            _helpers.RunCmd("chown fpanel:fpanel "+ _config->getValue("panel.dir") +"httpd/var/tmp", _logger.debug);
            _helpers.RunCmd("chown fpanel:fpanel "+ _config->getValue("panel.dir") +"httpd/var/cache", _logger.debug);
            _helpers.RunCmd("chown -R fpanel:fpanel "+ _config->getValue("panel.dir") +"etc", _logger.debug);
        }
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "configs");
        _helpers.CreateDirectory(_config->getValue("panel.dir") + "configs/domains");

        // Socket'in timeout olmamasi icin gerekiyor Centos ve ubuntuda test edilmesi gerekiyor

        _helpers.RunCmd("sed -i 's/net\\.ipv4\\.tcp_tw_recycle.*//g' /etc/sysctl.conf", _logger.debug);
        _helpers.RunCmd("echo \"net.ipv4.tcp_tw_recycle = 1\" >> /etc/sysctl.conf", _logger.debug);

        _helpers.RunCmd("sed -i 's/net\\.ipv4\\.tcp_tw_reuse .*//g' /etc/sysctl.conf", _logger.debug);
        _helpers.RunCmd("echo \"net.ipv4.tcp_tw_reuse  = 1\" >> /etc/sysctl.conf", _logger.debug);

        _helpers.RunCmd("sed -i 's/net\\.ipv4\\.tcp_fin_timeout.*//g' /etc/sysctl.conf", _logger.debug);
        _helpers.RunCmd("echo \"net.ipv4.tcp_fin_timeout = 60\" >> /etc/sysctl.conf", _logger.debug);
        _helpers.RunCmd("sysctl -p", _logger.debug);

        std::string sys_packages;
        std::string httpd_server;
        std::string dns_server;
        std::string smtp_server;
        std::string imap_server;
        std::string ftp_server;
        std::string database_servers;
        std::string spam_service;
        std::string statistics;
        if(_items->getApacheNginxStatus() == "X") {
            httpd_server = "apache-nginx";
        }
        if(_items->getApacheStatus() == "X") {
            httpd_server = "apache";
        }
        if(_items->getBindStatus() == "X") {
            dns_server = "bind";
        }
        if(_items->getPostfixStatus() == "X") {
            smtp_server = "postfix";
        }
        if(_items->getDovecotStatus() == "X") {
            imap_server = "dovecot";
        }
        if(_items->getProftpStatus() == "X") {
            ftp_server = "proftpd";
        }

        if(_items->getWebalizerStatus() == "X") {
            statistics = "webalizer";
        }

        if(_items->getAwstatsStatus() == "X") {
            statistics = "awstats";
        }

        if(_items->getMySqlStatus() == "X") {
            database_servers += "mysql,";
        }
        if(_items->getPostgreStatus() == "X") {
            database_servers += "postgresql,";
        }
        if(_items->getMariaDbStatus() == "X") {
            database_servers += "mariadb,";
        }
        if(_items->getSpamAssassinStatus() == "X") {
            spam_service += "spamassassin";
        }
        _helpers.RemoveLastChar(database_servers);

        sys_packages =  "FTP_SERVER="       + ftp_server+"\n";
        sys_packages += "DNS_SERVER="       + dns_server+"\n";
        sys_packages += "HTTPD_SERVER="     + httpd_server+"\n";
        sys_packages += "SMTP_SERVER="      + smtp_server+"\n";
        sys_packages += "IMAP_SERVER="      + imap_server+"\n";
        sys_packages += "STATISTICS="       + statistics+"\n";
        sys_packages += "DATABASE_SERVERS=" + database_servers+"\n";
        sys_packages += "SPAM_SERVICE="     + spam_service+"\n";

        _helpers.writeNewFile(_config->getValue("panel.dir") + "etc/.sys.packages", sys_packages);

        if(!_cloud_client) {
            _helpers.RunCmd("systemctl stop panel-httpd", _logger.debug);
            _helpers.RunCmd("systemctl stop panel-ws", _logger.debug);
            _helpers.RunCmd("systemctl stop panel-redis", _logger.debug);
        }


        _helpers.RunCmd("systemctl stop panel-agent", _logger.debug);
        _helpers.RunCmd("systemctl stop panel-core", _logger.debug);


        if (_config->getValue("system.os").compare("centos") == 0) {
            if((_items->getApacheStatus() == "X") || (_items->getApacheNginxStatus() == "X")) {
                _helpers.RunCmd("systemctl stop httpd", _logger.debug);
            }
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl stop named", _logger.debug);
            }
        }
        else if (_config->getValue("system.os").compare("ubuntu") == 0) {
            if((_items->getApacheStatus() == "X") || (_items->getApacheNginxStatus() == "X")) {
                _helpers.RunCmd("systemctl stop apache2", _logger.debug);
            }
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl stop named", _logger.debug);
            }
        }
        else if (_config->getValue("system.os").compare("debian") == 0) {
            if((_items->getApacheStatus() == "X") || (_items->getApacheNginxStatus() == "X")) {
                _helpers.RunCmd("systemctl stop apache2", _logger.debug);
            }
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl stop bind9", _logger.debug);
            }
        }

        if(!_cloud_client) {
            _helpers.RunCmd("systemctl start panel-httpd", _logger.debug);
            _helpers.RunCmd("systemctl start panel-ws", _logger.debug);
            _helpers.RunCmd("systemctl start panel-redis", _logger.debug);
        }

        _helpers.RunCmd("systemctl start panel-agent", _logger.debug);
        _helpers.RunCmd("systemctl start panel-core", _logger.debug);

        if(_items->getProftpStatus() == "X") {
            _helpers.RunCmd("systemctl start proftpd", _logger.debug);
        }

        if (_config->getValue("system.os").compare("centos") == 0) {
            if((_items->getApacheStatus() == "X") || (_items->getApacheNginxStatus() == "X")) {
                _helpers.RunCmd("systemctl start httpd", _logger.debug);
            }
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl start named", _logger.debug);
            }
        }
        else if (_config->getValue("system.os").compare("ubuntu") == 0) {
            if((_items->getApacheStatus() == "X") || (_items->getApacheNginxStatus() == "X")) {
                _helpers.RunCmd("systemctl start apache2", _logger.debug);
            }
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl start named", _logger.debug);
            }
        }
        else if (_config->getValue("system.os").compare("debian") == 0) {
            if((_items->getApacheStatus() == "X") || (_items->getApacheNginxStatus() == "X")) {
                _helpers.RunCmd("systemctl start apache2", _logger.debug);
            }
            if(_items->getBindStatus() == "X") {
                _helpers.RunCmd("systemctl start bind9", _logger.debug);
            }
        }

        if(_items->getPostfixStatus() == "X") {
            _helpers.RunCmd(_config->getValue("panel.dir") + "core/bin/mailsrvmng --init " +
                            "--message-size-limit=10 " +
                            "--maximal-queue-lifetime=24 " +
                            "--smtpd-client-connection-count-limit=50 " +
                            "--smtpd-recipient-limit=100 "+
                            "--bounce-queue-lifetime=3600 "+
                            "--policyd-spy-timelimit=3600 "+
                            "--mf-check=off "+
                            "--soft-bounce=on "+
                            "--smtp-auth=off "+
                            "--pop-smtp-auth=on "+
                            "--sub-port=on "+
                            "--spf=on "+
                            "--enable-spam=off "+
                            "--rbl=off "+
                            "--hostname=" + _new_hostname, _logger.debug);
        }

        if(_items->getSpamAssassinStatus() == "X") {
            _helpers.RunCmd(_config->getValue("panel.dir") + "/core/bin/mailsrvmng --spam --serv=on --score=5.0 --bayes=on --rbl-check=on --razor=on --pyzor=on --report=on", _logger.debug);
            _helpers.RunCmd("systemctl start spamassassin", _logger.debug);
        }

        if(_items->getPostfixStatus() == "X") {
            _helpers.RunCmd("systemctl start postfix", _logger.debug);
        }

        if (_config->getValue("system.os").compare("centos") == 0) {
            if(_items->getDovecotStatus() == "X") {
                _helpers.RunCmd("/etc/init.d/dovecot start", _logger.debug);
                _helpers.RemoveFile("/etc/systemd/system/multi-user.target.wants/dovecot.service");
                _helpers.RemoveFile("/usr/lib/systemd/system/dovecot.service");
                _helpers.RunCmd("systemctl disable dovecot", _logger.debug);
                _helpers.RunCmd("chkconfig dovecot on", _logger.debug);
            }
//            _helpers.RunCmd("yum remove php72-php-fpm -y", _logger.debug);
        }
        else {
            if(_items->getDovecotStatus() == "X") {
                _helpers.RunCmd("systemctl start dovecot", _logger.debug);
            }
        }

        if(_helpers.IsDirExist(_config->getValue("vhosts.dir") + "/webmail_rc")) {
            if (_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd("chown -R apache:nobody " + _config->getValue("vhosts.dir") + "/webmail_rc", _logger.debug);
            }
            else {
                _helpers.RunCmd("chown -R www-data:nogroup " + _config->getValue("vhosts.dir") + "/webmail_rc", _logger.debug);
            }
        }

        if(_helpers.IsDirExist(_config->getValue("vhosts.dir") + "/dbmyadmin")) {
            if (_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd("chown -R apache:nobody " + _config->getValue("vhosts.dir") + "/dbmyadmin", _logger.debug);
            }
            else {
                _helpers.RunCmd("chown -R www-data:nogroup " + _config->getValue("vhosts.dir") + "/dbmyadmin", _logger.debug);
            }
        }

        if(_helpers.IsDirExist(_config->getValue("vhosts.dir") + "/awstats")) {
            if (_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmd("chown -R apache:nobody " + _config->getValue("vhosts.dir") + "/awstats/httpdoc", _logger.debug);
            }
            else {
                _helpers.RunCmd("chown -R www-data:nogroup " + _config->getValue("vhosts.dir") + "/awstats/httpdoc", _logger.debug);
            }
        }

        if(_cloud_main) {
            _helpers.RunCmd("sed -i 's/type = standalone/type = cloud/g' " + _config->getValue("panel.dir") + "httpd/htdocs/config/config.ini", _logger.debug);
        }
        _helpers.RunCmd("systemctl daemon-reload", _logger.debug);
    }

public:
    Settings &setConfig(Config *config)
    {
        _config = config;
        return *this;
    }

    Settings &setLogger(Logger logger)
    {
        _logger = logger;
        return *this;
    }

    Settings &setCloudMain(bool cloud_main)
    {
        _cloud_main = cloud_main;
        return *this;
    }

    Settings &setCloudClient(bool cloud_client)
    {
        _cloud_client = cloud_client;
        return *this;
    }

    Settings &setMainIp(std::string main_ip)
    {
        _main_ip = main_ip;
        return *this;
    }

    Settings &setItems(Items *items)
    {
        _items = items;
        return *this;
    }

    void InitSystemSettings()
    {
        _old_hostname = "\\#HOSTNAME\\#";
        _new_hostname = _items->getHostname();
        _helpers.trim(_new_hostname);
        _default_ip_addr = _helpers.GetLocalIpAddress();
        std::string current_hostname = _helpers.CmdOutput("hostname -f");
        _helpers.trim(current_hostname);

        initMysqlSettings();
        initMariaDbSettings();
        initPostgreSQLSettings();
        initBindSettings();
        initProftpSettings();
        initApacheSettings();
        initNginxSettings();
        initPhpSettings();
        initPostfixDovecotSettings();
        initSpamassassinSettings();
        initMailmanSettings();
        initPanelSettings();

        _helpers.Println("New Hostname: " + _new_hostname,_helpers.CYAN);
        _logger.Info("New Hostname: " + _new_hostname);
        _helpers.RunCmd("hostnamectl set-hostname " + _new_hostname, _logger.debug);
        _helpers.RunCmd("echo \"kernel.hostname = " + _new_hostname + "\" >> /etc/sysctl.conf", _logger.debug);
        _helpers.RunCmd("sysctl -p", _logger.debug);

        std::string hosts_content = "# Generated by FPanel\n"
                                    "127.0.0.1       " + _new_hostname + " localhost.localdomain localhost localdomain \n"
                                    "::1             localhost6.localdomain6 localhost6 localdomain6\n"
                                    + _default_ip_addr + "  " + _new_hostname;

        _helpers.RemoveFile("/etc/hosts");
        _helpers.writeNewFile("/etc/hosts", hosts_content);

        if (_config->getValue("system.os").compare("ubuntu") == 0) {
            _helpers.RunCmd("sed -i 's/preserve_hostname: false/preserve_hostname: true/g' /etc/cloud/cloud.cfg", _logger.debug);
        }
        if(_cloud_client) {
            _helpers.CmdOutput("rm -rf " + _config->getValue("panel.dir") + "httpd");
        }
    }
};
#endif //INSTALLER_SETTINGS_H
