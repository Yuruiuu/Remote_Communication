#ifndef CHAT_THREAD_H
#define CHAT_THREAD_H

#include<thread>
#include<event.h>
#include"chat_list.h"
#include"chat_database.h"
#include<string.h>
#include<json/json.h>


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
	void thread_write_data(struct bufferevent* bev,Json::Value& v);
	void thread_register(struct bufferevent* bev,Json::Value& val);
	void thread_login(struct bufferevent* bev,Json::Value& val);
	void thread_add_friend(struct bufferevent* bev,Json::Value& val);
	int thread_parse_string(std::string&, std::string*);
	void thread_private_chat(struct bufferevent *bev, Json::Value &v);
	void thread_create_group(struct bufferevent *bev, Json::Value &v);
	void thread_join_group(struct bufferevent *bev, Json::Value &v);
	void thread_group_chat(struct bufferevent *bev, Json::Value &v);
	void thread_transfer_file(struct bufferevent *bev, Json::Value &v);
	void thread_client_offline(struct bufferevent *, Json::Value &);
	void thread_get_group_member(struct bufferevent *, Json::Value &);
	static void worker(ChatThread*);
	static void timeout_cb(evutil_socket_t fd, short event, void *arg);
	static void thread_readcb(struct bufferevent *bev, void *ctx);
	static void thread_eventcb(struct bufferevent *bev, short what, void *ctx);

};


#endif
