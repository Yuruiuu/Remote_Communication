#include"chat_thread.h"

ChatThread::ChatThread()
{
	_thread = new std::thread(worker,this);
	_id = _thread->get_id();//get_id()时线程标准库里的

	base = event_base_new();
}

ChatThread::~ChatThread()
{
	if(_thread)
		delete _thread;
}

void ChatThread::start(ChatInfo *i, DataBase *d)
{
	this->info = i;
	this->db = d;
}

void ChatThread::worker(ChatThread *t)
{
	//因为静态成员函数worker只能访问静态成员变量，所以再写一个run
	//静态成员函数可以调用普通成员函数
	t->run();
}

void ChatThread::run()
{
	//因为集合中若是无事件，便退出，所以随便放一个事件
	//集合中放入一个定时器事件
	struct event timeout;
	struct timeval tv;
	
	//将事件与集合绑定
	//base是构造函数初始化的base
	//EV_PERSIST表示定时器永远都有用
	event_assign(&timeout, base, -1, EV_PERSIST, timeout_cb,this);

	evutil_timerclear(&tv);
	tv.tv_sec=3;
	event_add(&timeout,&tv);

	std::cout<<"--- thread "<<_id<<" start working ---"<<std::endl;

	event_base_dispatch(base);//死循环，当集合中没有事件的时候退出
	
	event_base_free(base);

}
	
void ChatThread::timeout_cb(evutil_socket_t fd, short event, void *arg)
{
	ChatThread *t=(ChatThread *)arg;
	//std::cout<<"-- thread "<<t->thread_get_id()<<" is listening --"<<std::endl;
}

std::thread::id ChatThread::thread_get_id()
{
	return _id;
}

struct event_base *ChatThread::thread_get_base()
{
	return base;
}

void ChatThread::thread_readcb(struct bufferevent *bev, void *ctx)
{
	ChatThread *t = (ChatThread *)ctx;

	char buf[1024] = {0};
	if(!t->thread_read_data(bev,buf))
	{
		std::cout<<"thread read data error"<<std::endl;
		return;
	}

	std::cout<<"--- thread "<<t->thread_get_id()<<" receive data ";
	std::cout<<buf<<std::endl;

	Json::Reader reader;   //Json解析对象
	Json::Value val;
	//判断buf是否是json格式
	if(!reader.parse(buf,val))
	{
		std::cout<<"ERROR : data is not json"<<std::endl;
		return;
	}
	if(val["cmd"]=="register")
	{
		t->thread_register(bev,val);
	}
	else if(val["cmd"]=="login")
	{
		t->thread_login(bev,val);	
	}
	else if(val["cmd"]=="addfriend")
	{
		t->thread_add_friend(bev, val);
	}
	else if (val["cmd"] == "private")
	{
		t->thread_private_chat(bev, val);
	}
	else if (val["cmd"] == "creategroup")
	{
		t->thread_create_group(bev, val);
	}
	else if (val["cmd"] == "joingroup")
	{
		t->thread_join_group(bev, val);
	}
	else if (val["cmd"] == "groupchat")
	{
		t->thread_group_chat(bev, val);
	}
	else if (val["cmd"] == "file")
	{
		t->thread_transfer_file(bev, val);
	}
	else if (val["cmd"] == "offline")
	{
		t->thread_client_offline(bev, val);
	}
	else if (val["cmd"] == "groupmember")
	{
		t->thread_get_group_member(bev, val);
	}
}

void ChatThread::thread_eventcb(struct bufferevent *b, short s, void *a)
{
	if (s & BEV_EVENT_EOF)
	{
		std::cout << "[disconnect] client offline" << std::endl;
		bufferevent_free(b);
	}
	else
	{
		std::cout << "UNKOWN ERROR" << std::endl;
	}
}
bool ChatThread::thread_read_data(struct bufferevent* bev, char* s)
{
	int size;
	size_t count=0;

	//bufferevent_read()返回值时读取的字符数
	//从bev里读，读取的内容放入size
	//根据我们设计的包的结构，size的内容就是数据的长度
	if(bufferevent_read(bev,&size,4)!=4)
	{
		return false;
	}

	char buf[1024]={0};
	while(1)
	{
		//从bev里读，读取的内容放入buf
		count+=bufferevent_read(bev,buf,1024);
		strcat(s,buf);
		memset(buf,0,1024);
		
		if(count>=size)
			break;
	}

	return true;
}


void ChatThread::thread_register(struct bufferevent* bev,Json::Value& v)
{
	db->database_connect();
	
	//判断用户是否存在
	if(db->database_user_is_exist(v["username"].asString()))
	{
		Json::Value val;
		val["cmd"] = "register_reply";
		val["result"] = "user_exist";

		thread_write_data(bev,val);
	}
	else//用户不存在
	{
		db->database_insert_user_info(v);

		Json::Value val;
                val["cmd"] = "register_reply";
                val["result"] = "success";

                thread_write_data(bev,val);

	}

	db->database_disconnect();
}


void ChatThread::thread_write_data(struct bufferevent* bev,Json::Value& v)
{
	std::string s = Json::FastWriter().write(v);
	int len = s.size();
	char buf[1024]={0};
	memcpy(buf,&len,4);
	memcpy(buf+4,s.c_str(),len);
	if(bufferevent_write(bev,buf,len+4)==-1)
	{
		std::cout<<"bufferevent_write error"<<std::endl;
	}



}


void ChatThread::thread_login(struct bufferevent* bev,Json::Value& v)
{
	db->database_connect();

	if(!db->database_user_is_exist(v["username"].asString()))
	{
		//用户不存在
		Json::Value val;
		val["cmd"] = "login_reply";
		val["result"] = "not exist";

		thread_write_data(bev,val);

		db->database_disconnect();
		return;
	}

	//用户存在，判断密码是否正确
	if(!db->database_password_correct(v))
	{
		Json::Value val;
                val["cmd"] = "login_reply";
                val["result"] = "password error";

                thread_write_data(bev,val);

                db->database_disconnect();
                return;

	}

	//获取好友列表以及群列表
	std::string friendlist,grouplist;
	if(!db->database_get_friend_group(v,friendlist,grouplist))
	{
		std::cout<<"get friendlist error"<<std::endl;
		return;
	}

	db->database_disconnect();

	//回复客户端登录成功
	Json::Value val;
	val["cmd"]="login_reply";
	val["result"] = "success";
	val["friendlist"] = friendlist;
	val["grouplist"] = grouplist;

	thread_write_data(bev,val);

	//加入在线用户链表
	info->list_update_list(v,bev);
	
	//通知所有好友
	if(friendlist.empty())
	{
		return;
	}
	int idx = friendlist.find('|');
	int start = 0;
	while(idx!=-1)
	{
		//从好友列表里面拉取好友名字
		std::string name = friendlist.substr(start,idx - start);
		//如果好友在线，给好友发送数据
		struct bufferevent* b = info->list_friend_online(name);
		if(b!=NULL)
		{
			val.clear();
                	val["cmd"]="online";
                	val["uername"]=v["username"];

                	thread_write_data(b,val);
		}

		start = idx+1;
		idx = friendlist.find('|',idx+1);
	}

	std::string name = friendlist.substr(start,idx - start);
	info->list_friend_online(name);
	struct bufferevent* b = info->list_friend_online(name);
        if(b!=NULL)
        {
        	val.clear();
                val["cmd"]="online";
                val["uername"]=v["username"];

                thread_write_data(b,val);
        }

}

void ChatThread::thread_add_friend(struct bufferevent *bev, Json::Value &v)
{
	//判断是不是自己
	if (v["friend"] == v["username"])
	{
		return;
	}

	db->database_connect();

	//判断好友是否存在
	if (!db->database_user_is_exist(v["friend"].asString()))
	{
		//好友不存在
		Json::Value val;
		val["cmd"] = "addfriend_reply";
		val["result"] = "not_exist";

		thread_write_data(bev, val);

		db->database_disconnect();

		return;
	}

	//判断是不是好友关系
	std::string friendlist, grouplist;
	if (db->database_get_friend_group(v, friendlist, grouplist))
	{
		std::string str[1024];
		int num = thread_parse_string(friendlist, str);

		for (int i = 0; i < num; i++)
		{
			if (str[i] == v["friend"].asString())
			{
				Json::Value val;
				val["cmd"] = "addfriend_reply";
				val["result"] = "already_friend";

				thread_write_data(bev, val);

				db->database_disconnect();

				return;
			}
		}
	}

	//修改数据库
	db->database_add_friend(v);

	db->database_disconnect();

	//回复好友
	Json::Value val;
	val["cmd"] = "be_addfriend";
	val["friend"] = v["username"];

	struct bufferevent * b;
	b = info->list_friend_online(v["friend"].asString());
	if (NULL != bev)
	{
		thread_write_data(b, val);
	}

	//回复自己
	val.clear();
	val["cmd"] = "addfriend_reply";
	val["result"] = "success";
	val["friend"] = v["friend"];

	thread_write_data(bev, val);
}


int ChatThread::thread_parse_string(std::string& f, std::string* s)
{
	int count = 0,start = 0;
	int idx = f.find('|');

	while(idx!=-1)
	{
		s[count++] = f.substr(start,idx - start);
		start = idx +1;
		idx = f.find('|',start);
	}
	s[count++] = f.substr(start);
	
	return count;
}

void ChatThread::thread_private_chat(struct bufferevent *bev, Json::Value &v)
{
	//判断对方在不在线
	std::string name = v["tofriend"].asString();
	struct bufferevent *b = info->list_friend_online(name);
	if (NULL == b)
	{
		//对方不在线
		Json::Value val;
		val["cmd"] = "private_reply";
		val["result"] = "offline";

		thread_write_data(bev, val);

		return;
	}

	//转发数据
	Json::Value val;
	val["cmd"] = "private";
	val["fromfriend"] = v["username"];
	val["text"] = v["text"];

	thread_write_data(b, val);
}


void ChatThread::thread_create_group(struct bufferevent *bev, Json::Value &v)
{
	//判断群是否存在
	std::string groupname = v["groupname"].asString();
	if (info->list_group_is_exist(groupname))
	{
		Json::Value val;
		val["cmd"] = "creategroup_reply";
		val["result"] = "exist";

		thread_write_data(bev, val);

		return;
	}

	//修改数据库
	db->database_connect();

	db->database_add_new_group(groupname, v["owner"].asString());

	db->database_disconnect();

	//修改map
	info->list_add_new_group(groupname, v["owner"].asString());

	//返回客户端
	Json::Value val;
	val["cmd"] = "creategroup_reply";
	val["result"] = "success";
	val["groupname"] = v["groupname"];

	thread_write_data(bev, val);
}

void ChatThread::thread_join_group(struct bufferevent *bev, Json::Value &v)
{
	//判断群是否存在
	std::string groupname = v["groupname"].asString();
	std::string username = v["username"].asString();
	if (!info->list_group_is_exist(groupname))
	{
		Json::Value val;
		val["cmd"] = "joingroup_reply";
		val["result"] = "not_exist";

		thread_write_data(bev, val);

		return;
	}

	//判断是不是已经在群里
	if (info->list_member_is_group(groupname, username))
	{
		Json::Value val;
		val["cmd"] = "joingroup_reply";
		val["result"] = "already";

		thread_write_data(bev, val);

		return;
	}

	//修改数据库
	db->database_connect();

	db->database_update_group_member(groupname, username);

	db->database_disconnect();

	//修改map
	info->list_update_group_member(groupname, username);

	//通知所有成员
	struct bufferevent *b;
	std::string member;
	std::list<std::string> l = info->list_get_list(groupname);
	for (auto it = l.begin(); it != l.end(); it++)
	{
		if (*it == username)
		{
			continue;
		}

		member += *it;
		member += "|";

		b = info->list_friend_online(*it);
		if (b != NULL)
		{
			Json::Value val;
			val["cmd"] = "new_member_join";
			val["groupname"] = groupname;
			val["username"] = username;

			thread_write_data(b, val);
		}
	}

	std::cout << member << std::endl;
	member.erase(member.size() - 1);//删除多的竖线

	//通知本人
	Json::Value val;
	val["cmd"] = "joingroup_reply";
	val["result"] = "success";
	val["member"] = member;
	val["groupname"] = v["groupname"];

	thread_write_data(bev, val);
}

void ChatThread::thread_group_chat(struct bufferevent *bev, Json::Value &v)
{
	//获取群成员
	std::string groupname = v["groupname"].asString();
	std::list<std::string> l = info->list_get_list(groupname);

	//转发数据
	struct bufferevent *b;

	for (auto it = l.begin(); it != l.end(); it++)
	{
		if (*it == v["username"].asString())
		{
			continue;
		}

		b = info->list_friend_online(*it);
		if (NULL == b)
		{
			continue;
		}

		Json::Value val;
		val["cmd"] = "groupchat_reply";
		val["from"] = v["username"];
		val["groupname"] = groupname;
		val["text"] = v["text"];

		thread_write_data(b, val);
	}
}

void ChatThread::thread_transfer_file(struct bufferevent *bev, Json::Value &v)
{
	Json::Value val;
	std::string friendname = v["friendname"].asString();

	struct bufferevent *b = info->list_friend_online(friendname);
	if (!b)
	{
		val["cmd"] = "file_reply";
		val["result"] = "offline";

		thread_write_data(bev, val);

		return;
	}

	val["cmd"] = "file_reply";
	val["result"] = "online";

	thread_write_data(bev, val);

	if (v["step"] == "1")
	{
		//转发文件属性
		val["cmd"] = "file_name";
		val["filename"] = v["filename"];
		val["filelength"] = v["filelength"];
		val["fromuser"] = v["username"];
	}
	else if (v["step"] == "2")
	{
		//转发文件内容
		val["cmd"] = "file_transfer";
		val["text"] = v["text"];
	}
	else if (v["step"] == "3")
	{
		//文件传输结束
		val["cmd"] = "file_end";

		bufferevent_free(bev);
	}

	thread_write_data(b, val);
}

void ChatThread::thread_client_offline(struct bufferevent *bev, Json::Value &v)
{
	std::string username = v["username"].asString();

	//删除在线用户链表结点
	info->list_delete_user(username);

	//释放bufferevent
	bufferevent_free(bev);

	//通知好友
	std::string friendlist, grouplist;

	db->database_connect();

	db->database_get_friend_group(v, friendlist, grouplist);

	db->database_disconnect();

	if (friendlist.empty())
	{
		return;
	}

	std::string str[1024];
	int num = thread_parse_string(friendlist, str);

	for (int i = 0; i < num; i++)
	{
		Json::Value val;
		val["cmd"] = "friend_offline";
		val["username"] = v["username"];

		struct bufferevent *b = info->list_friend_online(str[i]);
		if (b)
		{
			thread_write_data(b, val);
		}

	}

	std::cout << "[disconnet] client offline" << std::endl;
}

void ChatThread::thread_get_group_member(struct bufferevent *bev, Json::Value &v)
{
	std::string groupname = v["groupname"].asString();
	std::string member;

	info->list_get_group_member(groupname, member);

	Json::Value val;
	val["cmd"] = "groupmember_reply";
	val["member"] = member;

	thread_write_data(bev, val);
}
