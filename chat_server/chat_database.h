#ifndef CHAT_DATABASE_H
#define CHAT_DATABASE_H


#include<mysql/mysql.h>
#include<mutex>
#include<iostream>
#include<stdio.h>  //sprintf
#include <json/json.h>
#include<string.h>


class DataBase
{
private:
	MYSQL *mysql;
	std::mutex _mutex;
public:
	DataBase();
	~DataBase();
	bool database_connect();
	void database_disconnect();
	bool database_init_table();
	bool database_user_is_exist(std::string);
	void database_insert_user_info(Json::Value& v);
	int database_get_group_info(std::string *);//返回群的数量
	bool database_password_correct(Json::Value& val);
	bool database_get_friend_group(Json::Value& ,std::string&, std::string&);
	void database_add_friend(Json::Value&);
	void database_update_friendlist(std::string &u, std::string &f);
	void database_add_new_group(std::string, std::string);
	void database_update_group_member(std::string g, std::string u);
	void database_update_info(std::string table,std::string groupname, std::string username);
};



#endif
