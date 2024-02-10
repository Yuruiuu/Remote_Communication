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
	if(row[2])
	{
		frilist = std::string(row[2]);
	}
	if(row[3])
	{
		grolist = std::string(row[3]);
	}

	return true;

}

void DataBase::database_add_friend(Json::Value& v)
{
	std::string username = v["username"].asString();
	std::string friendname = v["friend"].asString();

	database_update_friendlist(username,friendname);
	database_update_friendlist(friendname,username);
}

void DataBase::database_update_friendlist(std::string &u, std::string &f)
{
	char sql[256] = {0};
	std::string friendlist;
	sprintf(sql, "select friendlist from chat_user where username = '%s';", u.c_str());

	std::unique_lock<std::mutex> lck(_mutex);

	if (mysql_query(mysql, sql) != 0)
	{
		std::cout << "select friendlist error" << std::endl;
		return;
	}

	MYSQL_RES *res = mysql_store_result(mysql);
	if (NULL == res)
	{
		std::cout << "store result error" << std::endl;
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row[0])//先前没有好友
			   //注意row==NULL是查询失败，row[0]==NULL是查询到的结果为空
	{
		friendlist.append(f);
	}
	else
	{
		friendlist.append(row[0]);//row[0]为原先好友列表
		friendlist.append("|");
		friendlist.append(f);
	}

	memset(sql, 0, sizeof(sql));

	sprintf(sql, "update chat_user set friendlist = '%s' where username = '%s';", friendlist.c_str(), u.c_str());

	if (mysql_query(mysql, sql) != 0)
	{
		std::cout << "update chat_user error" << std::endl;
	}
}

void DataBase::database_add_new_group(std::string g, std::string owner)
{
	char sql[256] = {0};
	std::string grouplist;

	//修改chat_group表
	sprintf(sql, "insert into chat_group values ('%s', '%s', '%s');",
			g.c_str(), owner.c_str(), owner.c_str());

	std::unique_lock<std::mutex> lck(_mutex);

	if (mysql_query(mysql, sql) != 0)
	{
		std::cout << "insert error" << std::endl;
		return;
	}

	//修改chat_user表
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select grouplist from chat_user where username = '%s';",
			owner.c_str());

	if (mysql_query(mysql, sql) != 0)
	{
		std::cout << "select friendlist error" << std::endl;
		return;
	}

	MYSQL_RES *res = mysql_store_result(mysql);
	if (NULL == res)
	{
		std::cout << "store result error" << std::endl;
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row[0])
	{
		grouplist.append(g);
	}
	else
	{
		grouplist.append(row[0]);
		grouplist.append("|");
		grouplist.append(g);
	}

	memset(sql, 0, sizeof(sql));

	sprintf(sql, "update chat_user set grouplist = '%s' where username = '%s';", grouplist.c_str(), owner.c_str());

	if (mysql_query(mysql, sql) != 0)
	{
		std::cout << "update chat_user error" << std::endl;
	}
}


void DataBase::database_update_group_member(std::string g, std::string u)
{
	//先修改chat_group内容
	database_update_info("chat_group", g, u);

	//再修改chat_user内容
	database_update_info("chat_user", g, u);

}

void DataBase::database_update_info(std::string table,
		 				std::string groupname, std::string username)
{
	//先把数据读出来
	char sql[256] = {0};
	std::string member;

	if (table == "chat_group")
	{
		sprintf(sql, "select groupmember from chat_group where groupname = '%s';", groupname.c_str());
	}
	else if (table == "chat_user")
	{
		sprintf(sql, "select grouplist from chat_user where username = '%s';", username.c_str());
	}

	std::unique_lock<std::mutex> lck(_mutex);

	if (mysql_query(mysql, sql) != 0)
	{
		std::cout << "select error" << std::endl;
		return;
	}

	MYSQL_RES *res = mysql_store_result(mysql);
	if (NULL == res)
	{
		std::cout << "store result error" << std::endl;
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if (row[0] == NULL)
	{
		if (table == "chat_group")
		{
			member.append(username);
		}
		else if (table == "chat_user")
		{
			member.append(groupname);
		}
	}
	else
	{
		if (table == "chat_group")
		{
			member.append(row[0]);
			member.append("|");
			member.append(username);
		}
		else if (table == "chat_user")
		{
			member.append(row[0]);
			member.append("|");
			member.append(groupname);
		}
	}

	mysql_free_result(res);

	//修改后再更新
	memset(sql, 0, sizeof(sql));

	if (table == "chat_group")
	{
		sprintf(sql, "update chat_group set groupmember = '%s' where groupname = '%s';", member.c_str(), groupname.c_str());
	}
	else if (table == "chat_user")
	{
		sprintf(sql, "update chat_user set grouplist = '%s' where username = '%s';", member.c_str(), username.c_str());
	}

	if (mysql_query(mysql, sql) != 0)
	{
		std::cout << "update chat_group error" << std::endl;
	}
}
