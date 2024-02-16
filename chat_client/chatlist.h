#ifndef CHATLIST_H
#define CHATLIST_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>a

namespace Ui {
class Chatlist;
}

class Chatlist : public QWidget
{
    Q_OBJECT

public:
    explicit Chatlist(QTcpSocket *, QString &, QString &, QString &);
    ~Chatlist();

private slots:
    void chat_server_reply();
    void chat_server_disconnect();

private:
    void chat_read_data(QByteArray &);

private:
    Ui::Chatlist *ui;
    QTcpSocket *socket;
    QString friendlist;
    QString grouplist;
    QString username;
};

#endif // CHATLIST_H
