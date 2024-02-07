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
}

void ChatThread::thread_eventcb(struct bufferevent *bev, short what, void *ctx)
{

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

