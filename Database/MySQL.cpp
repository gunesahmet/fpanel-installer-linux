#include "MySQL.h"

MySQL& MySQL::setConfig(const std::string &host, const std::string &port, const std::string &dbname, const std::string &username, const std::string &password)
{
//    pconn = MySQLPool.GetConnection();
//    char buffer[100];
//    auto_ptr<Statement> pstmt(pconn->createStatement());
//    std::string use_db_query = "use " + dbname;
//    pstmt->execute(use_db_query.c_str());

    driver = get_driver_instance();
    con = driver->connect("tcp://"+ host+":"+ port, username, password);
    con->setSchema(dbname);
    driver->threadInit();
    stmt = con->createStatement();

    _and_where = std::map<int,std::string>();
    _or_where = std::map<int,std::string>();
    _inner_join = std::map<int,std::map<std::string,std::string>>();
    _left_join = std::map<int,std::map<std::string,std::string>>();
    _columns = std::map<int, std::string>();
    _generated_select_sql = "";
    return *this;
}

MySQL& MySQL::select(std::vector<std::string> selects)
{
    _result = new std::unordered_map<int, std::unordered_map<std::string,std::string>>();
    _result->clear();
    for(std::size_t i=0; i < selects.size(); ++i) {
        std::string col(selects[i].substr(selects[i].rfind(" as ") + 1));
        helpers.ReplaceWithString(col, "as ", "");
        _columns.insert({_columns.size(),col});
        _select.append(selects[i]);
        _select.append(",");
    }
    _select = _select.substr(0, _select.size()-1);
    return *this;
}

MySQL& MySQL::distinct(bool distinct)
{
    _distinct = distinct;
    return *this;
}

MySQL& MySQL::from(const std::string &table)
{
    
    _from = table;
    return *this;
}

MySQL& MySQL::where(std::string condition, std::map<std::string, std::string> bind)
{
    
    for (std::map<std::string, std::string>::iterator it = bind.begin(); it != bind.end(); it++) {
        std::string search = "?";
        search.append(it->first);
        std::string replace = it->second;
        helpers.ReplaceWithString(condition, search, replace);
    }

    _where = condition;
    return *this;
}

MySQL& MySQL::andWhere(std::string condition, std::map<std::string, std::string> bind)
{
    
    for (std::map<std::string, std::string>::iterator it = bind.begin(); it != bind.end(); it++) {
        std::string search = "?";
        search.append(it->first);
        std::string replace = it->second;
        helpers.ReplaceWithString(condition, search, replace);
    }
    _and_where.insert({_and_where.size(), condition});
    return *this;
}

MySQL& MySQL::orWhere(std::string condition, std::map<std::string, std::string> bind)
{
    
    for (std::map<std::string, std::string>::iterator it = bind.begin(); it != bind.end(); it++) {
        std::string search = "?";
        search.append(it->first);
        std::string replace = it->second;
        helpers.ReplaceWithString(condition, search, replace);
    }
    _or_where.insert({_or_where.size(), condition});
    return *this;
}

MySQL& MySQL::limit(int limit)
{
    
    _limit = " LIMIT " + std::to_string(limit);
    return *this;
}

MySQL& MySQL::limitOffSet(int limit_off_set)
{
    
    _limit.append(" OFFSET ");
    _limit.append(std::to_string(limit_off_set));
    return *this;
}

MySQL& MySQL::orderDesc(const std::string &order)
{
    
    _order_desc = order;
    return *this;
}

MySQL& MySQL::orderAsc(const std::string &order)
{
    
    _order_asc = order;
    return *this;
}

MySQL& MySQL::innerJoin(const std::string &table, std::string relation)
{
    
    std::map<std::string, std::string> join_data = std::map<std::string, std::string>();
    join_data.insert({"table", table});
    join_data.insert({"relation", relation});
    _inner_join.insert({_inner_join.size(), join_data});
    return *this;
}

MySQL& MySQL::leftJoin(const std::string &table, std::string relation)
{
    
    std::map<std::string, std::string> join_data = std::map<std::string, std::string>();
    join_data.insert({"table", table});
    join_data.insert({"relation", relation});
    _left_join.insert({_left_join.size(), join_data});
    return *this;
}

MySQL& MySQL::groupBy(std::vector<std::string> group_by)
{
    
    std::string _group_bys;
    for(std::size_t i=0; i<group_by.size(); ++i) {
        if(_group_by.empty()) {
            _group_bys = group_by[i];
        }
        else {
            _group_bys.append(",");
            _group_bys.append(group_by[i]);
        }
    }

    _group_by = " GROUP BY " + _group_bys;
    return *this;
}

MySQL& MySQL::find()
{
    _result_status = false;
    generateSql();
    try {
        sql::ResultSet *res = stmt->executeQuery(_generated_select_sql);
        while (res->next()) {
            std::unordered_map<std::string,std::string> result = std::unordered_map<std::string,std::string>();
            for (std::map<int, std::string>::iterator it = _columns.begin(); it != _columns.end(); it++) {
                std::string column_value = res->getString(it->second.c_str());
                result.insert({it->second,column_value});
            }
            _result->insert({ _result->size(), result});
        }
        _result_status = true;
        delete res;
    }
    catch (sql::SQLException &e) {
        logger.Error("SQL: " + _generated_select_sql);
        logger.Error("ERR: " + std::string(e.what()));
        logger.Error("Code: " + std::to_string(e.getErrorCode()));
    }
    _generated_select_sql = "";
    return *this;
}

MySQL& MySQL::findFirst()
{
    _result_status = false;
    generateSql();
    try {
        sql::ResultSet *res = stmt->executeQuery(_generated_select_sql);
        while (res->next()) {
            std::unordered_map<std::string,std::string> result = std::unordered_map<std::string,std::string>();
            for (std::map<int, std::string>::iterator it = _columns.begin(); it != _columns.end(); it++) {
                std::string column_value = res->getString(it->second.c_str());
                result.insert({it->second,column_value});
            }
            _result->insert({ _result->size(), result});
        }
        _result_status = true;
        delete res;
    }
    catch (sql::SQLException &e) {
        logger.Error("SQL: " + _generated_select_sql);
        logger.Error("ERR: " + std::string(e.what()));
        logger.Error("Code: " + std::to_string(e.getErrorCode()));
    }
    _generated_select_sql = "";
    return *this;
}

bool MySQL::resultStatus()
{
    
    return _result_status;
}

void MySQL::clear()
{
    _order_desc.clear();
    _order_asc.clear();
    _columns.clear();
    _where.clear();
    _and_where.clear();
    _or_where.clear();
    _group_by.clear();
    _from.clear();
    _inner_join.clear();
    _left_join.clear();
    _limit.clear();
    _select.clear();
    _generated_select_sql = "";
}

std::unordered_map<int, std::unordered_map<std::string,std::string>>* MySQL::result()
{
    clear();
    return _result;
}

std::string MySQL::toSql()
{
    return _generated_select_sql;
}

MySQL& MySQL::generateSql()
{
    std::string inner_join;
    for (std::map<int,std::map<std::string,std::string>>::iterator it = _inner_join.begin(); it != _inner_join.end(); it++) {
        std::map<std::string,std::string> joins = it->second;
        inner_join.append(std::string(" INNER JOIN "))
                .append(joins["table"])
                .append(" ON ")
                .append(joins["relation"]);
    }
    std::string left_join;
    for (std::map<int,std::map<std::string,std::string>>::iterator it = _left_join.begin(); it != _left_join.end(); it++) {
        std::map<std::string,std::string> joins = it->second;
        left_join.append(std::string(" LEFT JOIN "))
                .append(joins["table"])
                .append(" ON ")
                .append(joins["relation"]);
    }
    std::string where;
    std::string and_where;
    std::string or_where;
    if (!_where.empty()) {
        where = std::string(" WHERE ");
        where.append(_where);
        for (std::map<int,std::string>::iterator it = _and_where.begin(); it != _and_where.end(); it++) {
            std::string wd = it->second;
            and_where.append(std::string(" AND ") + wd);
        }

        for (std::map<int,std::string>::iterator it = _or_where.begin(); it != _or_where.end(); it++) {
            std::string owd = it->second;
            or_where.append(std::string(" OR ") + owd);
        }
    }

    std::string order;
    if(!_order_desc.empty()) {
        order = " ORDER BY " + _order_desc + " DESC";
    }
    else if(!_order_asc.empty()) {
        order = " ORDER BY " + _order_asc + " ASC";
    }

    if(_distinct) {
        _generated_select_sql = "SELECT DISTINCT ";
    }
    else {
        _generated_select_sql = "SELECT ";
    }

    _generated_select_sql += _select;
    _generated_select_sql += " FROM ";
    _generated_select_sql += _from;
    _generated_select_sql += inner_join;
    _generated_select_sql += left_join;
    _generated_select_sql += " ";
    _generated_select_sql += where;
    _generated_select_sql += and_where;
    _generated_select_sql += or_where;
    _generated_select_sql += _group_by;
    _generated_select_sql += order;
    _generated_select_sql += _limit;
    _generated_select_sql += ";";
    return *this;
}

// For Inserts
MySQL& MySQL::insertColumns(std::vector<std::string> columns)
{
    std::string insert_columns;
    for(std::size_t i=0; i < columns.size(); ++i) {
        insert_columns.append(columns[i]);
        insert_columns.append(",");
    }
    insert_columns = insert_columns.substr(0, insert_columns.size()-1);

    _generated_insert_sql += " (";
    _generated_insert_sql +=  insert_columns;
    _generated_insert_sql +=  ") VALUES ";

    return *this;
}

MySQL& MySQL::startValue()
{
    _generated_insert_sql += "(";
    return *this;
}

MySQL& MySQL::insertSetValue(const std::string &value)
{
    _generated_insert_sql += "'";
    _generated_insert_sql += value;
    _generated_insert_sql += "',";

    return *this;
}

MySQL& MySQL::insertSetValue(int value)
{
    _generated_insert_sql += "'";
    _generated_insert_sql += std::to_string(value);
    _generated_insert_sql += "',";

    return *this;
}

MySQL& MySQL::insertSetValue(double value)
{
    _generated_insert_sql += "'";
    _generated_insert_sql += std::to_string(value);
    _generated_insert_sql += "',";
    return *this;
}

MySQL& MySQL::endValue()
{
    helpers.RemoveLastChar(_generated_insert_sql);
    _generated_insert_sql += "),";
    return *this;
}

MySQL& MySQL::insert(const std::string &table)
{
    // INSERT INTO table_name (column_name_1, column_name_2, column_name_3, ..., column_name_n) VALUES (value_1, value_2, value_3, ..., value_n)
    _generated_insert_sql = "INSERT INTO ";
    _generated_insert_sql += table;
    return *this;
}

std::string MySQL::toInsertSql()
{
    return _generated_insert_sql;
}

bool MySQL::executeInsert()
{
    _result_status = false;
    helpers.RemoveLastChar(_generated_insert_sql);
    _generated_insert_sql += ";";
    try {
        stmt->executeQuery(_generated_insert_sql);
        _generated_insert_sql = "";
        _result_status = true;
        clear();
        return true;
    }
    catch (sql::SQLException &e) {
        if ((e.getErrorCode() != 0) && (e.getErrorCode() != 1062)) {
            logger.Error("SQL: " + _generated_insert_sql);
            logger.Error("ERR: " + std::string(e.what()));
            logger.Error("Code: " + std::to_string(e.getErrorCode()));
        }
    }
    clear();
    _generated_insert_sql = "";
    return false;
}

bool MySQL::InsertRaw(std::string query)
{
    _result_status = false;
    try {
        stmt->executeQuery(query);
        _result_status = true;
        clear();
        return true;
    }
    catch (sql::SQLException &e) {
        if ((e.getErrorCode() != 0) && (e.getErrorCode() != 1062)) {
            logger.Error("SQL: " + _generated_insert_sql);
            logger.Error("ERR: " + std::string(e.what()));
            logger.Error("Code: " + std::to_string(e.getErrorCode()));
        }
    }
    clear();
    return false;
}

// For Updates
MySQL& MySQL::update(const std::string &table)
{
    // UPDATE table_name SET where_column_name = 'where_column_value' WHERE where_column_name = 'value'
    _generated_update_sql = "UPDATE ";
    _generated_update_sql += table;
    _generated_update_sql += " SET ";
    return *this;
}

MySQL& MySQL::updateSetColumns(std::map<std::string,std::string> columns_and_values)
{
    for (std::map<std::string, std::string>::iterator it = columns_and_values.begin(); it != columns_and_values.end(); it++) {
        _generated_update_sql.append(it->first)
        .append(" = ")
        .append("'")
        .append(it->second)
        .append("',");
    }
    _generated_update_sql = _generated_update_sql.substr(0, _generated_update_sql.size()-1);
    _generated_update_sql.append(" ");
    return *this;
}

MySQL& MySQL::updateWhere(std::string condition, std::map<std::string, std::string> bind)
{
    
    _generated_update_sql.append(" WHERE ");

    for (std::map<std::string, std::string>::iterator it = bind.begin(); it != bind.end(); it++) {
        std::string search = "?"+it->first;
        std::string replace = it->second;
        helpers.ReplaceWithString(condition, search, replace);
    }

    _generated_update_sql.append(condition);
    return *this;
}

bool MySQL::executeUpdate()
{
    
    _result_status = false;
    _generated_update_sql.append(";");
    const sql::SQLString& generated_update_sql = _generated_update_sql.c_str();
    try {
//        pstmt = con->prepareStatement(generated_update_sql);
//        pstmt->executeUpdate();
        stmt->executeQuery(generated_update_sql);
        _generated_update_sql = "";
        _result_status = true;
//        pstmt = NULL;
        clear();
        return true;
    }
    catch (sql::SQLException &e) {
        if (e.getErrorCode() != 0) {
            logger.Error("SQL: " + _generated_update_sql);
            logger.Error("ERR: " + std::string(e.what()));
            logger.Error("Code: " + std::to_string(e.getErrorCode()));
        }
    }
//    pstmt = NULL;
    _generated_update_sql = "";
    clear();
    return false;
}

// For Removes
MySQL& MySQL::remove(const std::string &table)
{
    
    //DELETE FROM table_name WHERE column_name = column_value;
    _generated_remove_sql.append("DELETE FROM ")
            .append(table);
    return *this;
}

MySQL& MySQL::removeWhere(std::string condition, std::map<std::string, std::string> bind)
{
    
    _generated_remove_sql.append(" WHERE ");
    for (std::map<std::string, std::string>::iterator it = bind.begin(); it != bind.end(); it++) {
        std::string search = "?"+it->first;
        std::string replace = it->second;
        helpers.ReplaceWithString(condition, search.c_str(), replace.c_str());
    }
    _generated_remove_sql.append(condition);
    return *this;
}

bool MySQL::executeRemove()
{
    
    _result_status = false;
    _generated_remove_sql.append(";");
    const sql::SQLString& generated_remove_sql = _generated_remove_sql.c_str();
    try {
//        pstmt = con->prepareStatement(generated_remove_sql);
//        pstmt->executeUpdate();
        stmt->executeQuery(generated_remove_sql);
        _generated_remove_sql = "";
        _result_status = true;
        clear();
//        pstmt = NULL;
        return true;
    }
    catch (sql::SQLException &e) {
        logger.Error("SQL: " + _generated_remove_sql);
        logger.Error("ERR: " + std::string(e.what()));
        logger.Error("Code: " + std::to_string(e.getErrorCode()));
    }
    _generated_remove_sql = "";
    clear();
    return false;
}

void MySQL::Close()
{
    try
    {
//        MySQLPool.ReleaseConnection(pconn);
        stmt->close();
        delete stmt;
        con->close();
        delete con;
        driver->threadEnd();
    }
    catch(sql::SQLException &e)
    {
        logger.Error("Failed To Close Connection to DataBase Server");
    }
}