//chat_server.h
#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <iostream>
#include <event.h>
#include <event2/listener.h>
#include <string.h>    //memset
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>   //exit()
#include"chat_database.h"//自己写的数据库类
#include"chat_list.h"
#include"chat_thread.h"


#define IP "172.30.153.250"
#define PORT 8888

class ChatServer
{
private:
	struct event_base *base;//监听事件的集合
	DataBase *db;//数据库对象
	ChatInfo *info;//数据结构对象
	ChatThread *pool;//线程池对象
	int thread_num;//线程的数量
	int cur_thread;//当前的线程下标
public:
	ChatServer();
	~ChatServer();
	void listen(const char* ip, int port);
	void server_update_group_info();
	void server_alloc_enevt(int fd);
	static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, 
                struct sockaddr *client_info, int socklen, void *arg);

};





#endif
