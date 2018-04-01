#include <QApplication>

#include "server/serverwidget.h"
#include "client/clientwidget.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ServerWidget server;
    server.show();

    ClientWidget client;
    client.show();

    return app.exec();
}
