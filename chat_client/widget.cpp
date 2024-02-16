#include "widget.h"
#include "ui_widget.h"
#include"chatlist.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //初始化socket对象
    socket = new QTcpSocket();

    //初始化文本输入框
    ui->passWordEdit->setEchoMode(QLineEdit::Password);

    //向服务器发起连接
    socket->connectToHost(QHostAddress(IP),PORT);

    //连接信号与槽
    connect(socket,&QTcpSocket::connected,this,&Widget::client_connect_success);
    connect(socket,&QTcpSocket::disconnected,this,&Widget::client_disconnect);
    connect(socket,&QTcpSocket::readyRead,this,&Widget::client_reply_info);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::client_connect_success()
{
    QMessageBox::information(this,u8"连接提示",u8"连接服务器成功");
}

void Widget::client_disconnect()
{
    QMessageBox::information(this,u8"连接提示",u8"服务器断开连接");
}


void Widget::client_reply_info()
{
    QByteArray ba;
    client_recv_data(ba);

    QJsonObject obj = QJsonDocument::fromJson(ba).object();

    QString cmd = obj.value("cmd").toString();

    if(cmd=="register_reply")
    {
        client_register_handler(obj);
    }
    else if (cmd == "login_reply")
    {
        client_login_handler(obj);
    }
}


void Widget::on_registerButton_clicked()
{
    QString username = ui->userEdit->text();
    QString password = ui->passWordEdit->text();

    QJsonObject obj;
    obj.insert("cmd","register");
    obj.insert("username",username);
    obj.insert("password",password);

    client_send_data(obj);
}

void Widget::client_send_data(QJsonObject &obj)
{
    QByteArray ba = QJsonDocument(obj).toJson();

    int size = ba.size();
    QByteArray sendData;
    sendData.insert(0,(char*)&size,4);
    sendData.append(ba);

    socket->write(sendData);
}

void Widget::client_recv_data(QByteArray &ba)
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

void Widget::client_register_handler(QJsonObject &obj)
{
    QString result = obj.value("result").toString();

    if(result=="user_exist")
    {
        QMessageBox::warning(this,u8"注册提示",u8"用户已存在");
    }
    else if(result == "success")
    {
        QMessageBox::warning(this,u8"注册提示",u8"注册成功");
    }
}

void Widget::client_login_handler(QJsonObject &obj)
{
    QString result = obj.value("result").toString();

    if(result == "not_exist")
    {
        QMessageBox::warning(this,u8"登录提示",u8"用户不存在");
    }
    else if(result == "password_error")
    {
        QMessageBox::warning(this,u8"登录提示",u8"密码错误");
    }
    else if(result == "success")
    {
        //跳转新界面
        QString friendlist = obj.value("friendlist").toString();
        QString grouplist = obj.value("grouplist").toString();
        QString username = ui->userEdit->text();

        this->hide();
        socket->disconnect(SIGNAL(readyRead()));
        socket->disconnect(SIGNAL(disconnected()));

        Chatlist* c = new Chatlist(socket,friendlist,grouplist,username);//在堆空间申请
        c->setWindowTitle(username);
        c->show();
    }
}


void Widget::on_loginButton_clicked()
{
    QString username = ui->userEdit->text();
    QString password = ui->passWordEdit->text();

    QJsonObject obj;
    obj.insert("cmd","login");
    obj.insert("username",username);
    obj.insert("password",password);

    client_send_data(obj);
}

