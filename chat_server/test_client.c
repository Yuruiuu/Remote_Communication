#include<stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<jsoncpp/json/json.h>


int main()
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in server_info;
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(8888);
	server_info.sin_addr.s_addr = inet_addr("172.30.153.250");
	int size = sizeof(server_info);

	if(connect(sockfd,(struct sockaddr*)&server_info,size)==-1)
	{
		perror("connect");
		exit(1);
	}

	Json::Value val;
	val["cmd"]="register";
	val["name"]="tom";
	val["password"]="1111";

	std::string s = Json::FastWriter().write(val);
	char buf[128]={0};
	int len = s.size();
	memcpy(buf,&len,4);
	memcpy(buf+4,s.c_str(),len);

	send(sockfd,buf,len+4,0);

	while(1);



	return 0;
}
