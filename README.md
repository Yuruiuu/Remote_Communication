# Remote_Communication
## 项目介绍
项目基于 C/S 架构，实现了用户注册、登录、添加好友、创建群、聊天、群发、文件 传输以及上下线提醒功能。

服务端部署在阿里云服务器上，使用 libevent + 线程池来解决 并发的问题，可以同时处理多个客户端的数据信息。

客户端基于Windows Qt，整个交互过程 采用json格式，其中，发送文件采用了多线程，避免了UI界面阻塞。

## 功能演示
![image](https://github.com/Yuruiuu/Remote_Communication/blob/main/asserts/example.png)
