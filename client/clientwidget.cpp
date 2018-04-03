#include <QHostAddress>
#include <QDateTime>
#include <QTextEdit>
#include <QMessageBox>

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

    udpSocket = new QUdpSocket(this);
    if(!udpSocket)
    {
        qcout << "Error01: cannot allocate the udpServer pointer";
    }
    udpSocket->bind(QHostAddress::AnyIPv4, 12345);


    //udpSocket->bind(QHostAddress::AnyIPv4, 8889);
    connect(udpSocket, &QUdpSocket::readyRead, this, &ClientWidget::dealMsg);
    connect(udpSocket, &QTcpSocket::disconnected,
            [=]()
    {
        udpSocket->close();
    });

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
                localAddress = tcpSocket->localAddress().toString();
                localTcpPort = tcpSocket->localPort();
                peerAddress = tcpSocket->peerAddress().toString();
                peerTcpPort = tcpSocket->peerPort();
                localUdpPort = udpSocket->localPort();
                //peerUdpPort = udpSocket->peerPort();
                peerUdpPort = peerTcpPort + 1;


                QString temp = tr("已与服务器建立连接\n"
                                  "服务器的IP, TCP端口号, UDP端口号[%1: %2, %3]\n"
                                  "客户端的IP, TCP端口号, UDP端口号[%4: %5, %6]\n")
                        .arg(peerAddress).arg(peerTcpPort).arg(peerUdpPort)
                        .arg(localAddress).arg(localTcpPort).arg(localUdpPort);
                ui->textEditReceive->setText(temp);
            });

    // 读文件
    connect(tcpSocket, &QTcpSocket::readyRead,
            [=]()
            {
                // 取出接收的内容
                QByteArray buf = tcpSocket->readAll();
                if(true == isStart)
                {
                    isStart = false;

                    fileName = QString(buf).section("##", 0, 0);
                    fileSize = QString(buf).section("##", 1, 1).toInt();
                    recvSize = 0;

                    file.setFileName(fileName);
                    bool isOk = file.open(QIODevice::WriteOnly);
                    if(false == isOk)
                    {
                        qDebug() << "WriteOnly error 40";
                    }
                }
                else
                {
                    qint64 len = file.write(buf);

                    recvSize += len;
                    if(recvSize == fileSize)
                    {
                        file.close();
                        QMessageBox::information(this, tr("传输完成"),
                                                 tr("文件接收完成"));
                        tcpSocket->disconnectFromHost();
                        tcpSocket->close();
                    }
                    int progress = static_cast<int>(static_cast<double>(recvSize) / fileSize * 100);
                    ui->progressBar->setValue(progress);
                }
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
    peerAddress = ui->lineEditIP->text();
    peerTcpPort = ui->lineEditPort->text().toInt();

    //主动和服务器建立连接
    tcpSocket->connectToHost(QHostAddress(peerAddress), peerTcpPort);
}

void ClientWidget::on_buttonSend_clicked()
{
//    if(!udpSocket->isOpen())
//    {
//        qcout << "Error05: Server:udpSocket is not nullptr, but is not open";
//        return ;
//    }
    QString sendData = ui->textEditSend->toPlainText();
    if(sendData.isEmpty())
    {
        // 对于空内容，直接返回。或者加一个警告
        qcout << "Warning01: Server:udpSocket is ok, but have nothing to send";
        return ;
    }

    //给对方发送数据，使用套接字是tcpSocket
    udpSocket->writeDatagram(sendData.toUtf8(), QHostAddress(peerAddress), peerUdpPort);
    ui->textEditSend->clear();
}

void ClientWidget::on_buttonClose_clicked()
{
//    if(!tcpSocket->isOpen())
//    {
//        qcout << "Error02_01: Client:tcpSocket is already closed, no need to close again";
//        return;
//    }
    //主动和客户端端口连接
    tcpSocket->disconnectFromHost();

    if(!udpSocket->isOpen())
    {
        qcout << "Error02_02: Client:udpSocket is already closed, no need to close again";
        return;
    }
    udpSocket->disconnectFromHost();
}

void ClientWidget::dealMsg()
{
    char buf[1024] = {0};
    QHostAddress address;
    quint16 port;
    qint64 len = udpSocket->readDatagram(buf, sizeof(buf), &address, &port);
    if(len > 0)
    {
        // 格式化
        QString str = QString("[%1:%2] %3")
                .arg(address.toString())
                .arg(port)
                .arg(buf);
        qcout << str;

        ui->textEditReceive->append(tr("Server User %1")
                                    .arg(QDateTime::currentDateTime().toString("yyyy/M/d hh:mm::ss")));
        ui->textEditReceive->append(QString(buf));
    }
}
