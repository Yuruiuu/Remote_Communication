#ifndef CHAT_THREAD_H
#define CHAT_THREAD_H

#include<thread>
#include<event.h>
#include"chat_list.h"
#include"chat_database.h"
#include<string.h>
#include<jsoncpp/json/json.h>


class ChatThread
{
private:
	std::thread *_thread;
	std::thread::id _id;
	struct event_base *base;
	ChatInfo *info;
	DataBase *db;
public:
	ChatThread();
	~ChatThread();
	void start(ChatInfo *,DataBase *);
	void run();
	std::thread::id thread_get_id();//线程标准库里有个get_id()
	struct event_base *thread_get_base();
	bool thread_read_data(struct bufferevent *, char *);
	static void worker(ChatThread*);
	static void timeout_cb(evutil_socket_t fd, short event, void *arg);
	static void thread_readcb(struct bufferevent *bev, void *ctx);
	static void thread_eventcb(struct bufferevent *bev, short what, void *ctx);

};


#endif
