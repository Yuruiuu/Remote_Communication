#ifndef CHAT_DATABASE_H
#define CHAT_DATABASE_H


#include<mysql/mysql.h>
#include<mutex>
#include<iostream>

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
	int database_get_group_info(std::string *);//返回群的数量
};



#endif
