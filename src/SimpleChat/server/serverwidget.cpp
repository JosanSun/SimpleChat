#include <QHostAddress>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>


#include "myheaders.h"
#include "server/serverwidget.h"
#include "ui_serverwidget.h"

ServerWidget::ServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerWidget)
{
    ui->setupUi(this);
    setWindowTitle(tr("SimpleChat Server"));
    setWindowIcon(QPixmap(":/images/server-icon.jpg"));

    udpSocket = new QUdpSocket(this);
    if(!udpSocket)
    {
        qcout << "Error04: cannot allocate the udpServer pointer";
    }
    udpSocket->bind(QHostAddress::Any, 8889);
    connect(udpSocket, &QUdpSocket::readyRead, this, &ServerWidget::dealMsg);
    connect(udpSocket, &QTcpSocket::disconnected,
            [=]()
    {
        udpSocket->close();
    });

    //监听套接字  指定父对象，让其自动回收空间
    tcpServer = new QTcpServer(this);
    if(!tcpServer)
    {
        qcout << "Error01: cannot allocate the tcpServer pointer";
    }

    //监听失败
    if(!tcpServer->listen(QHostAddress::Any, 8888))
    {
        QMessageBox::critical(this, tr("SimpleChat Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
    }

    // 建立连接
    connect(tcpServer, &QTcpServer::newConnection,
            [=]()
    {
        //取出建立好连接的套接字
        tcpSocket = tcpServer->nextPendingConnection();
        //获取对方的IP和端口 以及 本服务器用于Tcp连接的ip地址
        localAddress = tcpSocket->localAddress().toString();
        localTcpPort = tcpSocket->localPort();
        peerAddress = tcpSocket->peerAddress().toString();
        peerTcpPort = tcpSocket->peerPort();
        localUdpPort = udpSocket->localPort();
        //peerUdpPort = udpSocket->peerPort();
        peerUdpPort = 12345;


        QString temp = tr("有一个客户端已与服务器建立连接\n"
                          "服务器的IP, TCP端口号, UDP端口号[%1: %2, %3]\n"
                          "客户端的IP, TCP端口号, UDP端口号[%4: %5, %6]\n")
                .arg(localAddress).arg(localTcpPort).arg(localUdpPort)
                .arg(peerAddress).arg(peerTcpPort).arg(peerUdpPort);
        ui->textEditReceive->setText(temp);

//        //正确位置是这里。注意不要放到外边。
//        connect(tcpSocket, &QTcpSocket::readyRead,
//                [=]()
//        {
//            //从通信套接字中的取出内容
//            QByteArray array = tcpSocket->readAll();
//            ui->textEditReceive->append(tr("Client User %1")
//                                        .arg(QDateTime::currentDateTime().toString("yyyy/M/d hh:mm::ss")));
//            ui->textEditReceive->append(QString(array));
//        });

        connect(tcpSocket, &QTcpSocket::disconnected,
                [=]()
        {
            ui->textEditReceive->append(tr("已与客户端断开连接"));
            tcpSocket->close();
            tcpSocket = nullptr;
        });
    });
    connect(&timer, &QTimer::timeout,
            [=]()
            {
                timer.stop();
                sendFile();
            });
}

ServerWidget::~ServerWidget()
{
    delete ui;
}

//void ServerWidget::on_buttonSend_clicked()
//{
//    // possible errors
//    if(!tcpSocket)
//    {
//        qcout << "Error02: cannot send message, due to Server:tcpSocket is nullptr";
//        return;
//    }
//    if(!tcpSocket->isOpen())
//    {
//        qcout << "Error03: Server:tcpSocket is not nullptr, but is not open";
//        return ;
//    }
//    QString sendData = ui->textEditSend->toPlainText();
//    if(sendData.isEmpty())
//    {
//        // 对于空内容，直接返回。或者加一个警告
//        qcout << "Warning01: Server:tcpSocket is ok, but have nothing to send";
//        return ;
//    }

//    //给对方发送数据，使用套接字是tcpSocket
//    tcpSocket->write(sendData.toUtf8());
//    ui->textEditSend->clear();
//}

void ServerWidget::on_buttonSend_clicked()
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

void ServerWidget::dealMsg()
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

        ui->textEditReceive->append(tr("Client User %1")
                                    .arg(QDateTime::currentDateTime().toString("yyyy/M/d hh:mm::ss")));
        ui->textEditReceive->append(QString(buf));
    }
}

void ServerWidget::on_buttonSelectFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "open", "../");
    if(filePath.isEmpty())
    {
        qcout << tr("选择文件路径出错");
        return ;
    }

    QFileInfo fileInfo(filePath);
    fileName = fileInfo.fileName();
    fileSize = fileInfo.size();
    sendSize = 0;

    file.setFileName(filePath);
    bool isOk = file.open(QIODevice::ReadOnly);
    if(false == isOk)
    {
        qcout << tr("只读的方式打开文件失败");
    }

    ui->textEditReceive->append(filePath);

    ui->buttonSelectFile->setEnabled(false);
    ui->buttonSendFile->setEnabled(true);
}

void ServerWidget::on_buttonSendFile_clicked()
{
    //先发送文件头信息
    QString head = QString("%1##%2").arg(fileName).arg(fileSize);
    qint64 len = tcpSocket->write(head.toUtf8());
    if(len > 0)
    {
        timer.start(20);
    }
    else
    {
        qDebug() << "头部信息发送失败 110";
        file.close();
        //ui->buttonSelectFile->setEnabled(true);
        //ui->buttonSendFile->setEnabled(false);
    }

    //发送真正的文件信息
}

void ServerWidget::sendFile()
{
    qint64 len = 0;
    do
    {
        char buf[4 * 1024] = {0};
        len = 0;
        // 往文件中读数据
        len = file.read(buf, sizeof(buf));
        // 发送数据，读多少，发多少
        len = tcpSocket->write(buf, len);

        sendSize += len;
    }while(len > 0);

    if(sendSize == fileSize)
    {
        ui->textEditReceive->append("文件发送完毕");
        file.close();

        tcpSocket->disconnectFromHost();
        tcpSocket->close();
    }

    ui->buttonSelectFile->setEnabled(true);
    ui->buttonSendFile->setEnabled(false);
}
