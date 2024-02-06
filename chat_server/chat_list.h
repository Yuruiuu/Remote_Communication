#ifndef CHAT_LIST_H
#define CHAT_LIST_H

#include<iostream>
#include<string>
#include<map>
#include<list>
#include<mutex>

struct User
{
	std::string name;//账号（用户名
	struct bufferevent* bev;//客户端对应的事件
};

class ChatInfo
{
private:
	//保存在线用户信息
	std::list<User> *online_user;
	//保存所有群的信息
	std::map<std::string,std::list<std::string>> *group_info;
	//访问在线用户的锁
	std::mutex list_mutex;
	//访问群信息的锁
	std::mutex map_mutex;
public:
	ChatInfo();
	~ChatInfo();
	void list_update_group(std::string* ,int);
	void list_print_group();
};

#endif
