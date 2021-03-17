#include "ozserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include "ozclient.h"
#include <QDebug>


OzServer::OzServer(int portNumber, QObject *parent) : QTcpServer(parent),
    port(portNumber)
{
    if(!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "Could not start server";
    }
    else
    {
        qDebug() << "Listening to port " << port << "...";
    }
}


void OzServer::incomingConnection(qintptr socketDescriptor)
{
    OzClient *client = new OzClient(socketDescriptor,ver);
    QThread *thr = new QThread; // це для запуску в паралельному потоці

    client->moveToThread(thr); // перенести
    connect(thr,&QThread::started,client,&OzClient::exec); // при запуску запустити
    connect(client,&OzClient::finish,thr,&QThread::quit); // по завершенню знищити
    connect(thr,&QThread::finished,thr,&QThread::deleteLater); // звільнити після себе пам'ять
    connect(client,&OzClient::infoMsg,this,&OzServer::slotMsg);

    thr->start();

}


void OzServer::slotMsg(QString msg)
{
    qDebug() << msg;
}


