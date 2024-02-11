#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //初始化socket对象
    socket = new QTcpSocket();

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

}

