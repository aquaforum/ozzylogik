/* цей клас буде займатися роботую з клієнтами, завантажується в паралельному потоці
 *
*/
#ifndef OZCLIENT_H
#define OZCLIENT_H

#include <QObject>
#include <QTcpSocket>

// таймаут роботи з сокетом, за 10 секунд його точно буде закрито
#define SOCKTIMEOUT 10000
// коефіцієнт розмноження
#define REPCOEF 1000000

class OzClient : public QObject
{
    Q_OBJECT
public:
    explicit OzClient(qintptr socketDescriptor,qint16 protoVer = 1 , QObject *parent = nullptr);

signals:
    void finish(); // сигнал висилається коли закінчено передачу даних
    void error(QTcpSocket::SocketError);
    void infoMsg(QString);

private:
    qintptr sockID; // це буде сокет клієнта
    QTcpSocket *sock;

    QByteArray buff; // буфер приймачаґ
    qint16 ver; // версія протоколу

    quint32 stage; // це щось типу кінцевого автомата.
    // якщо 0 то тільки-що зєдналися і чакаємо версію протокола від клієнта
    // якщо 1 то версія узгоджена і чекаємо дані від клієнта
    // ну і так далі.....
    // по красоті його б зробити як enum

private slots:
    void slotRead(); // тут читаємо дані
    void slotTimeout(); // а раптом немає зв'язку....
    void slotDisconnect();

public slots:
    void exec();

};

#endif // OZCLIENT_H
