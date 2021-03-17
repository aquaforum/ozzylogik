#ifndef OZSERVER_H
#define OZSERVER_H

#include <QTcpServer>



class OzServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit OzServer(int portNumber=45123,QObject *parent = nullptr);

    // версія
    qint16 version() {return ver;}
    void setVersion(qint16 version) {ver=version;}


signals:

protected:
    void incomingConnection(qintptr socketDescriptor);
private slots:
    void slotMsg(QString);
private:
    int port;
    qint16 ver;
};

#endif // OZSERVER_H
