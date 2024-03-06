# Remote_Communication
## 项目介绍
项目基于 C/S 架构，实现了用户注册、登录、添加好友、创建群、聊天、群发、文件 传输以及上下线提醒功能。

服务端部署在阿里云服务器上，使用 libevent + 线程池来解决 并发的问题，可以同时处理多个客户端的数据信息。

客户端基于Windows Qt，整个交互过程 采用json格式，其中，发送文件采用了多线程，避免了UI界面阻塞。

## 功能演示
![image](https://github.com/Yuruiuu/Remote_Communication/blob/main/asserts/example.png)

## 克隆仓库
```
git clone https://github.com/Yuruiuu/Remote_Communication.git
```

## 使用指南
### 服务器
#### 安装依赖
下载libevent和jsoncpp库。
```
cd yourprojectname
sudo apt install libevent-dev
sudo apt install libjsoncpp-dev
```

#### 设置 
在`chatserver.h`中填写服务器私网IP和端口。（使用`ifconfig`命令查看私网IP。)
```
#define IP "your server IP"
#define PORT xxxx
```

#### 编译
编译：运行`make`命令以使用MakeFile编译。
```
make
```

### 客户端
环境： Qt5 、 C++11

使用`Qt Creator`打开`.pro`文件。

填写您的服务器公网ip以及端口号（需与服务器端口号相同）
```
#define IP "your server IP"
#define PORT xxxx
```
