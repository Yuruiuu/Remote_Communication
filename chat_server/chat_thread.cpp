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
		
	}
	else if(val["cmd"]=="login")
	{
		
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
