#include<iostream>
#include"chat_server.h"

using namespace std;

int main()
{
	//创建服务器对象
	ChatServer s;
	s.listen(IP,PORT);
	return 0;
}
