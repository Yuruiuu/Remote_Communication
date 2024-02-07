#include "chat_database.h"


DataBase::DataBase()
{
}

DataBase::~DataBase()
{
}


bool DataBase::database_connect()
{
	//初始化数据库句柄
	mysql = mysql_init(NULL);//分配堆内存

	//连接数据库
	mysql = mysql_real_connect(mysql, "localhost","root","root",
			"chat_database",0,NULL,0);
	if(mysql==NULL)
	{
		std::cout<<"mysql_real_connect error"<<std::endl;
		return false;
	}

	//设置编码格式   (防止中文乱码）
	if(mysql_query(mysql, "set names utf8;")!=0)
	{
		std::cout<<"set names utf8 error"<<std::endl;
		return false;
	}

	return true;
}

void DataBase::database_disconnect()
{
	mysql_close(mysql);
}

bool DataBase::database_init_table()
{
	database_connect();

	const char* g="create table if not exists chat_group(groupname varchar(128),groupowner varchar(128),groupmember varchar(4096))charset utf8;";
	if(mysql_query(mysql,g)!=0)
	{
		return false;
	}

	const char* u="create table if not exists chat_user(username varchar(128),password varchar(128),friendlist varchar(4096),grouplist varchar(4096))charset utf8;";

	if(mysql_query(mysql,u)!=0)
        {
                return false;
        }

	database_disconnect();

	return true;
}

int DataBase::database_get_group_info(std::string *g)
{
	if(mysql_query(mysql,"select * from chat_group;")!=0)
	{
		std::cout<<"select error"<<std::endl;
		return -1;
	}
	MYSQL_RES *res = mysql_store_result(mysql);
	if(res==NULL)
	{
		std::cout<<"store result error"<<std::endl;
		return -1;
	}
	MYSQL_ROW r;
	int idx=0;
	while(r = mysql_fetch_row(res))
	{
		g[idx] += r[0];
		g[idx] +='|';
		g[idx] += r[2];
		//std::cout<<g[idx]<<std::endl;
		idx++;
	}

	mysql_free_result(res);
	return idx;
}


bool DataBase::database_user_is_exist(std::string u)
{
	char sql[256]={0};

	sprintf(sql,"select * from chat_user where username = '%s';",u.c_str());

	std::unique_lock<std::mutex> lck(_mutex);//加锁

	if(mysql_query(mysql,sql)!=0)
	{
		std::cout<<"select error"<<std::endl;
		return true;//用户已存在
	}

	MYSQL_RES *res = mysql_store_result(mysql);
	if(res==NULL)
	{
		std::cout<<"store result error"<<std::endl;
		return true;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if(row == NULL)
	{
		return false;
	}
	else 
	{
		return true;
	}
}


void DataBase::database_insert_user_info(Json::Value& v)
{
	std::string username = v["username"].asString();
	std::string password = v["password"].asString();

	char sql[256] = {0};

	sprintf(sql,"insert into chat_user (username, password) values ('%s', '%s');",
			username.c_str(),password.c_str());

	std::unique_lock<std::mutex> lck(_mutex);//加锁

	if(mysql_query(mysql,sql)!=0)
	{
		std::cout<<"insert into error"<<std::endl;
		return;
	}
}


bool DataBase::database_password_correct(Json::Value& v)
{
	std::string username = v["username"].asString();
	std::string password = v["password"].asString();

	char sql[256] = {0};
	sprintf(sql,"select password from chat_user where username = '%s';",username.c_str());

	std::unique_lock<std::mutex> lck(_mutex);//加锁

	if(mysql_query(mysql,sql)!=0)
	{
		std::cout<<"select password error"<<std::endl;
		return false;
	}

	MYSQL_RES *res = mysql_store_result(mysql);
	if(res==NULL)
	{
		std::cout<<"mysql store result error"<<std::endl;
		return false;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if(row==NULL)
	{
		std::cout<<"fetch row error"<<std::endl;
		return false;
	}

	if(!strcmp(row[0],password.c_str()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DataBase::database_get_friend_group(Json::Value& v,std::string& frilist, std::string& grolist)
{
	char sql[1024] = {0};
	sprintf(sql,"select * from chat_user where username = '%s';",
			v["username"].asString().c_str());

	std::unique_lock<std::mutex> lck(_mutex);//加锁

	if(mysql_query(mysql,sql)!=0)
	{
		std::cout<<"select * error"<<std::endl;
		return false;
	}

	MYSQL_RES* res = mysql_store_result(mysql);
	if(res==NULL)
	{
		std::cout<<"store result error"<<std::endl;
		return false;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if(row==NULL)
	{
		return false;
	}

	frilist = std::string(row[2]);
	grolist = std::string(row[3]);

	return true;

}
