#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QTcpSocket>
#include<QHostAddress>
#include<QMessageBox>

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

private:
    Ui::Widget *ui;
    QTcpSocket* socket;
};
#endif // WIDGET_H
