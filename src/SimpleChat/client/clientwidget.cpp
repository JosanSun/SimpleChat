#include <QHostAddress>
#include <QDateTime>
#include <QTextEdit>

#include "client/clientwidget.h"
#include "ui_clientwidget.h"
#include "myheaders.h"


ClientWidget::ClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientWidget)
{
    ui->setupUi(this);

    setWindowTitle(tr("SimpleChat Client"));
    setWindowIcon(QPixmap(":/images/client-icon.png"));


    //分配空间，指定父对象
    tcpSocket = new QTcpSocket(this);

    if(!tcpSocket)
    {
        qcout << "Error04: cannot allocate the tcpSocket pointer";
    }

    connect(tcpSocket, &QTcpSocket::connected,
            [=]()
            {
                //获取对方的IP和端口 以及 本服务器用于Tcp连接的ip地址
                QString ipServer = tcpSocket->peerAddress().toString();
                quint16 portServer = tcpSocket->peerPort();
                QString ipClient = tcpSocket->localAddress().toString();
                quint16 portClient = tcpSocket->localPort();
                QString temp = tr("成功和服务器建立连接\n"
                                  "服务器的IP和端口号[%1: %2]\n"
                                  "本主机的IP和端口号[%3: %4]\n")
                        .arg(ipServer).arg(portServer)
                        .arg(ipClient).arg(portClient);
                ui->textEditReceive->setText(temp);
            });

    connect(tcpSocket, &QTcpSocket::readyRead,
            [=]()
            {
                //从通信套接字中的取出内容
                QByteArray array = tcpSocket->readAll();
                ui->textEditReceive->append(tr("I'm server %1")
                                            .arg(QDateTime::currentDateTime().toString("yyyy/M/d hh:mm::ss")));
                ui->textEditReceive->append(QString(array));
            });

    connect(tcpSocket, &QTcpSocket::disconnected,
            [=]()
            {
                ui->textEditReceive->append(tr("已与服务器断开连接"));
                tcpSocket->close();
                // tcpSocket = nullptr;
            });
}

ClientWidget::~ClientWidget()
{
    delete ui;
}

void ClientWidget::on_buttonConnect_clicked()
{
    //获取服务器ip和端口
    QString ip = ui->lineEditIP->text();
    qint16 port = ui->lineEditPort->text().toInt();

    //主动和服务器建立连接
    tcpSocket->connectToHost(QHostAddress(ip), port);
}

void ClientWidget::on_buttonSend_clicked()
{
    // possible errors
    if(!tcpSocket)
    {
        qcout << "Error02: cannot send message, due to Client:tcpSocket is nullptr";
        return;
    }
    if(!tcpSocket->isOpen())
    {
        qcout << "Error03: Client:tcpSocket is not nullptr, but is not open";
        return ;
    }
    QString sendData = ui->textEditSend->toPlainText();
    if(sendData.isEmpty())
    {
        // 对于空内容，直接返回。或者加一个警告
        qcout << "Warning01: Client:tcpSocket is ok, but have nothing to send";
        return ;
    }

    //给对方发送数据，使用套接字是tcpSocket
    tcpSocket->write(sendData.toUtf8());
    ui->textEditSend->clear();
}

void ClientWidget::on_buttonClose_clicked()
{
    if(nullptr == tcpSocket)
    {
        qcout << "Error02_01: Client:tcpSocket is already nullptr, no need to close again";
        return;
    }
    //主动和客户端端口连接
    tcpSocket->disconnectFromHost();
}
