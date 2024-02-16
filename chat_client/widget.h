#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QTcpSocket>
#include<QHostAddress>
#include<QMessageBox>
#include<QJsonObject>
#include<QJsonDocument>
#include<string.h>
#include<QDebug>

#define IP "121.41.28.165"
#define PORT 8888

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void client_connect_success();
    void client_disconnect();
    void client_reply_info();

    void on_registerButton_clicked();


    void on_loginButton_clicked();

private:
    void client_send_data(QJsonObject &);
    void client_recv_data(QByteArray &);
    void client_register_handler(QJsonObject &);
    void client_login_handler(QJsonObject &);

private:
    Ui::Widget *ui;
    QTcpSocket* socket;
};
#endif // WIDGET_H
