#include "chatlist.h"
#include "ui_chatlist.h"



Chatlist::Chatlist(QTcpSocket *s, QString &f, QString &g, QString &u) :
    ui(new Ui::Chatlist)
{
    ui->setupUi(this);

    this->socket = s;
    this->friendlist = f;
    this->grouplist = g;
    this->username = u;

    //qDebug()<<friendlist<<"  "<<grouplist;

    //初始化好友和群
    QStringList friList = friendlist.split('|');
    for(int i=0;i<friList.size();i++)
    {
        if(friList.at(i) != "")
        {
            ui->friendList->addItem(friList.at(i));
        }
    }

    QStringList grpList = grouplist.split('|');
    for(int i=0;i<grpList.size();i++)
    {
        if(grpList.at(i) != "")
        {
            ui->groupList->addItem(grpList.at(i));
        }
    }

    //连接信号与槽
    connect(socket,&QTcpSocket::disconnected,this,&Chatlist::chat_server_disconnect);
    connect(socket,&QTcpSocket::readyRead,this,&Chatlist::chat_server_reply);
}

Chatlist::~Chatlist()
{
    delete ui;
}

void Chatlist::chat_server_reply()
{
    QByteArray ba;
    chat_read_data(ba);

    QJsonObject obj = QJsonDocument::fromJson(ba).object();

    QString cmd = obj.value("cmd").toString();

    if(cmd=="addfriend_reply")
    {

    }
    else if (cmd == "_reply")
    {

    }
}

void Chatlist::chat_server_disconnect()
{
    QMessageBox::warning(this,"连接提示","服务器断开");
}

void Chatlist::chat_read_data(QByteArray &ba)
{
    char buf[1024] = {0};
    int size,sum=0;
    bool flag = true;

    socket->read(buf,4);
    memcpy(&size,buf,4);

    qDebug()<<"get data len : "<<size;

    while(flag)
    {
        memset(buf,0,1024);
        sum+=socket->read(buf,size-sum);
        if(sum>=size)
        {
            flag = false;
        }
        ba.append(buf);
    }
    qDebug()<<"data : "<<ba;

}
