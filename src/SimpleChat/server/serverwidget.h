//  一直BUG，对于存在中文路径的文件，无法正常传送。

#ifndef SERVERWIDGET_H
#define SERVERWIDGET_H

#include <QWidget>
#include <QTcpServer>   //监听套接字
#include <QTcpSocket>   //通信套接字
#include <QUdpSocket>
#include <QFile>
#include <QTimer>

namespace Ui {
class ServerWidget;
}

class ServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServerWidget(QWidget *parent = nullptr);
    ~ServerWidget();

private slots:
    void on_buttonSend_clicked();
    void on_buttonClose_clicked();
    void on_buttonSelectFile_clicked();
    void on_buttonSendFile_clicked();

    void dealMsg();
    void sendFile();

private:
    Ui::ServerWidget *ui  = nullptr;

    QUdpSocket *udpSocket = nullptr;  //通信套接字 用来发送信息
    QTcpServer *tcpServer = nullptr;  //监听套接字
    QTcpSocket *tcpSocket = nullptr;  //通信套接字 用来发送文件

    QString localAddress;
    QString peerAddress;
    quint16 localTcpPort;
    quint16 peerTcpPort;
    quint16 localUdpPort;
    quint16 peerUdpPort;

    QFile file;
    QString fileName;
    qint64 fileSize;
    qint64 sendSize;
    QTimer timer;
};

#endif // SERVERWIDGET_H
