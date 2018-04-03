#include <QHostAddress>
#include <QMessageBox>
#include <QDateTime>

#include "myheaders.h"
#include "server/serverwidget.h"
#include "ui_serverwidget.h"

ServerWidget::ServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerWidget)
{
    ui->setupUi(this);
    setFixedSize(350, 350);
    move(500, 100);
    initButtons();
    // setWindowTitle(tr("SimpleChat Server"));
    // setWindowIcon(QPixmap(":/images/server-icon.jpg"));

    //监听套接字  指定父对象，让其自动回收空间
    tcpServer = new QTcpServer(this);
    if(!tcpServer)
    {
        qcout << "Error01: cannot allocate the tcpServer pointer";
    }

    //监听失败
    if(!tcpServer->listen(QHostAddress::AnyIPv4, 8888))
    {
        QMessageBox::critical(this, tr("SimpleChat Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
    }

    connect(tcpServer, &QTcpServer::newConnection,
            [=]()
    {
        changeButtons();

        //取出建立好连接的套接字
        tcpSocket = tcpServer->nextPendingConnection();
        //获取对方的IP和端口 以及 本服务器用于Tcp连接的ip地址
        QString ipServer = tcpSocket->localAddress().toString();
        quint16 portServer = tcpSocket->localPort();
        QString ipClient = tcpSocket->peerAddress().toString();
        quint16 portClient = tcpSocket->peerPort();
        QString temp = tr("有一个客户端已与服务器建立连接\n"
                          "服务器的IP和端口号[%1: %2]\n"
                          "客户端的IP和端口号[%3: %4]\n")
                .arg(ipServer).arg(portServer)
                .arg(ipClient).arg(portClient);
        ui->textEditReceive->setText(temp);

        //正确位置是这里。注意不要放到外边。
        connect(tcpSocket, &QTcpSocket::readyRead,
                [=]()
        {
            //从通信套接字中的取出内容
            QByteArray array = tcpSocket->readAll();
            ui->textEditReceive->append(tr("Client User %1")
                                        .arg(QDateTime::currentDateTime().toString("yyyy/M/d hh:mm::ss")));
            ui->textEditReceive->append(QString(array));
        });
        connect(tcpSocket, &QTcpSocket::disconnected,
                [=]()
        {
            initButtons();

            ui->textEditReceive->append(tr("已与客户端断开连接"));
            tcpSocket->close();
            tcpSocket = nullptr;
        });
    });

}

ServerWidget::~ServerWidget()
{
    delete ui;
}

void ServerWidget::on_buttonSend_clicked()
{
    // possible errors
    if(!tcpSocket)
    {
        qcout << "Error02: cannot send message, due to Server:tcpSocket is nullptr";
        return;
    }
    if(!tcpSocket->isOpen())
    {
        qcout << "Error03: Server:tcpSocket is not nullptr, but is not open";
        return ;
    }
    QString sendData = ui->textEditSend->toPlainText();
    if(sendData.isEmpty())
    {
        // 对于空内容，直接返回。或者加一个警告
        qcout << "Warning01: Server:tcpSocket is ok, but have nothing to send";
        return ;
    }

    //给对方发送数据，使用套接字是tcpSocket
    tcpSocket->write(sendData.toUtf8());
    ui->textEditSend->clear();
}

void ServerWidget::on_buttonClose_clicked()
{
    if(nullptr == tcpSocket)
    {
        qcout << "Error02_01: Server:tcpSocket is already nullptr, no need to close again";
        return;
    }
    //主动和客户端端口连接
    tcpSocket->disconnectFromHost();
}

void ServerWidget::initButtons()
{
    ui->buttonSend->setEnabled(false);
    ui->buttonClose->setEnabled(false);
}

void ServerWidget::changeButtons()
{
    ui->buttonSend->setEnabled(true);
    ui->buttonClose->setEnabled(true);
}
