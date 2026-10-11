#pragma once
#ifndef INSTALLER_MYSQL_H
#define INSTALLER_MYSQL_H

#include <iostream>
#include <map>
#include <Helpers/Helpers.h>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <Logger/Logger.h>
#include <unordered_map>

/**
 * Usage:
    MySQL mysql(config.configurations["main.db_host"],
    config.configurations["main.db_port"],
    config.configurations["main.db_name"],
    config.configurations["main.db_username"],
    config.configurations["main.db_password"]);

    mysql.select({
        "domains.id as domain_id",
        "domains.name as domain_name",
        "domain_whois.registered_date as registered_date"
    })
    .from("domains")
    .innerJoin("domain_whois","domain_whois.domain_id = domains.id")
    .orderAsc("domain_name")
    .limit(1)
    .find();
    std::map<int, std::map<std::string,std::string>> result = mysql.result();

    for (std::map<int, std::map<std::string,std::string>> ::iterator it = result.begin(); it != result.end(); it++) {
        helpers.Print("Row: ");
        helpers.Println(it->first);
        helpers.Print("Column Key: ");
        helpers.Println(it->second["domain_id"]);
        helpers.Print("Column Value: ");
        helpers.Println(it->second["domain_name"]);
        helpers.Print("Column Value: ");
        helpers.Println(it->second["registered_date"]);
    }
 */
//using namespace zdb;
using namespace std;
using namespace sql;
class MySQL {
private:
    Helpers helpers;
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
//    sql::PreparedStatement *pstmt;

    Logger logger;
//    Connection* pconn;
    std::map<int,std::string> _and_where;
    std::map<int,std::string> _or_where;



    std::map<int,std::map<std::string,std::string>> _inner_join;
    std::map<int,std::map<std::string,std::string>> _left_join;
    std::unordered_map<int, std::unordered_map<std::string,std::string>> *_result;
    bool _result_status = false;
    std::map<int, std::string> _columns;

    // For Select Queries
    std::string _order_desc;
    std::string _order_asc;
    std::string _where;
    std::string _select;
    std::string _group_by;
    std::string _from;
    std::string _limit;
    bool _distinct;
    std::string _generated_select_sql;

    // For Insert Update Remove Queries
    std::string _generated_insert_sql;
    std::string _generated_update_sql;
    std::string _generated_remove_sql;

public:
    MySQL() {};
    virtual ~MySQL() {};
    void clear();
    MySQL& generateSql();
    MySQL& setConfig(const std::string &host, const std::string &port, const std::string &dbname, const std::string &username, const std::string &password);
    MySQL& select(std::vector<std::string> selects);
    MySQL& from(const std::string &table);
    MySQL& distinct(bool distinct);
    MySQL& where(std::string condition, std::map<std::string, std::string> bind);
    MySQL& andWhere(std::string condition, std::map<std::string, std::string> bind);
    MySQL& orWhere(std::string condition, std::map<std::string, std::string> bind);
    MySQL& limit(int limit);
    MySQL& limitOffSet(int limit_off_set);
    MySQL& innerJoin(const std::string &table, std::string condition);
    MySQL& leftJoin(const std::string &table, std::string condition);
    MySQL& groupBy(std::vector<std::string> selects);
    MySQL& orderDesc(const std::string &order);
    MySQL& orderAsc(const std::string &order);
    MySQL& find();
    MySQL& findFirst();
    std::unordered_map<int, std::unordered_map<std::string,std::string>>* findRaw(std::string &raw_sql);
    std::unordered_map<int, std::unordered_map<std::string,std::string>>* findFirstRaw(std::string &raw_sql);
    std::unordered_map<int, std::unordered_map<std::string,std::string>>* result();
    std::string toSql();

    MySQL& insertColumns(std::vector<std::string> columns);
    MySQL& insertSetValue(const std::string &value);
    MySQL& insertSetValue(int value);
    MySQL& insertSetValue(double value);
    MySQL& insert(const std::string &table);
    MySQL& startValue();
    MySQL& endValue();
    std::string toInsertSql();
    bool executeInsert();
    bool InsertRaw(std::string query);

    MySQL& update(const std::string &table);
    MySQL& updateSetColumns(std::map<std::string,std::string> columns_and_values);
    MySQL& updateWhere(std::string condition, std::map<std::string, std::string> bind);
    bool executeUpdate();

    MySQL& remove(const std::string &table);
    MySQL& removeWhere(std::string condition, std::map<std::string, std::string> bind);
    bool executeRemove();
    bool resultStatus();
    void Close();
};


#endif //INSTALLER_MYSQL_H
