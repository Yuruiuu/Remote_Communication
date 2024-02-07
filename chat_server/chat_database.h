#ifndef CHAT_DATABASE_H
#define CHAT_DATABASE_H


#include<mysql/mysql.h>
#include<mutex>
#include<iostream>
#include<stdio.h>  //sprintf
#include<jsoncpp/json/json.h>



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
};



#endif
