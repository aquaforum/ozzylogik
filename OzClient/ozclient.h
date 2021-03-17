/*
 *  Розмова клієнта з сервером відбувається в такому порядку
 *  1. крієнт встановлює з'єнання з сервером, якщо вдалося тоді
 *  2. клієнт надсилає серверу свою версію протокола - два байти тип qint16
 *  3.1 сервер відповідає клієнту двома байтами (qint16) якщо 0 - то все ок версії  протокола співпали - чекаємо дані від сервер
 *  3.2 якщо відповідь !=0  це код помилки, далі йде QString із текстом помилки, після цього сервер закриває з'єднання.
 *  4. клієнт надсилає серверу свій double.
 *  5. сервер надсилає відповідь і закриває з'єднання.
 *  6. клієнт все прийняте зберігає в файл
 *
*/
#ifndef OZCLIENT_H
#define OZCLIENT_H

#include <QObject>
#include <QTcpSocket>

// чекати три секунди
#define TIMEWAIT 3000
class QFile;

class OzClient : public QObject
{
    Q_OBJECT
public:
    explicit OzClient(QString hostName=QStringLiteral("localhost"),int portN=45123,QObject *parent = nullptr);

    // версія
    qint16 version() {return ver;}
    void setVersion(qint16 version) {ver=version;}

    double value() {return _value;}
    void setValue(double value) {_value=value;}

public slots:
    void exec();    

signals:
    void finish();
    void error(QTcpSocket::SocketError);

private:
    QTcpSocket *sock; // через це будемо спілкуватися з сервером

    QString host; // параметри з'єднання
    int port;

    QByteArray inBuff; // буфер приймача
    qint16 ver; // це версія
    double _value; // це значення що надсилається на сервер

    qint32 stage; // це те ж що й в сервері, див. туди....

    QFile *databin;


private slots:
    void slotConnect();
    void slotRead();
    void slotDisconnect();
    void slotError(QTcpSocket::SocketError);
    void slotWriteMyData();


};

#endif // OZCLIENT_H
