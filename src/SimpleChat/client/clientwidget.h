#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QTcpSocket>   //通信套接字
#include <QUdpSocket>
#include <QFile>

namespace Ui {
class ClientWidget;
}

class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(QWidget *parent = nullptr);
    ~ClientWidget();

private slots:
    void on_buttonConnect_clicked();
    void on_buttonSend_clicked();
    void on_buttonClose_clicked();
    void dealMsg();

private:
    Ui::ClientWidget *ui  = nullptr;
    QTcpSocket* tcpSocket = nullptr;
    QUdpSocket* udpSocket = nullptr;

    QString localAddress;
    QString peerAddress;
    quint16 localTcpPort;
    quint16 peerTcpPort;
    quint16 localUdpPort;
    quint16 peerUdpPort;

    QFile file;
    QString fileName;
    qint64 fileSize;
    qint64 recvSize;
    bool isStart = true;
};

#endif // CLIENTWIDGET_H
