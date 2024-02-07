#include"chat_list.h"

ChatInfo::ChatInfo()
{
	online_user = new std::list<User>;
	group_info = new std::map<std::string,std::list<std::string>>;
}

ChatInfo::~ChatInfo()
{
	if(online_user) delete online_user;
	if(group_info) delete group_info;
}


void ChatInfo::list_update_group(std::string* g, int size)
{
	int idx=0,start =0;
	std::string groupname,membername;
	std::list<std::string> l;

	for(int i=0;i<size;i++)
	{
		idx = g[i].find('|');
		groupname = g[i].substr(0,idx);
		//std::cout<<groupname<<std::endl;
		
		start = idx +1;
		while(1)//idx查找竖线，找不到是-1
		{
			idx = g[i].find('|',idx+1); //从idx开始查找
			if(idx==-1)break;
			membername = g[i].substr(start,idx-start);
			l.push_back(membername);
			start = idx +1;
		}

		membername = g[i].substr(start,idx - start);
		l.push_back(membername);

		this->group_info->insert(std::pair<std::string,std::list<std::string>>(groupname,l));
		
		l.clear();
	}

}

void ChatInfo::list_print_group()
{
	for(auto it=group_info->begin();it!=group_info->end();it++)
	{
		std::cout<<it->first<<" ";
		for(auto i = it->second.begin();i!=it->second.end();i++)
			std::cout<<*i<<" ";
		std::cout<<std::endl;
	}
}



bool ChatInfo::list_update_list(Json::Value v,struct bufferevent* bev)
{
	User u ={v["username"].asString(),bev};

	std::unique_lock<std::mutex> lck(list_mutex);//加锁
	
	online_user->push_back(u);

	return true;

}

//从在线用户里查找好友，若找到则在线，返回好友的bev
//若未找到，则不在线，返回NULL
struct bufferevent * ChatInfo::list_friend_online(std::string n)
{
	std::unique_lock<std::mutex> lck(list_mutex);

	for (auto it = online_user->begin(); it != online_user->end(); it++)
	{
		if(it->name == n)
		{
			return it->bev;
		}
	}

	return NULL;
}
