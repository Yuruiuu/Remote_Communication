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
