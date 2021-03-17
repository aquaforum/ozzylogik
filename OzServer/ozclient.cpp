#include "ozclient.h"
#include <QTimer>
#include <QHostAddress>
#include <QDataStream>


OzClient::OzClient(qintptr socketDescriptor, qint16 protoVer, QObject *parent) : QObject(parent),
    sockID(socketDescriptor),
    ver(protoVer),
    stage(0)
{

}

void OzClient::slotRead()
{
    buff.append(sock->readAll()); // прочитати все, що там прийшло
    for(;;) // цей цикл потрібен для того щоб перебрати всі байти в буфері вихід з циклу шукати в середині
    {
        QDataStream out(sock);
        //out.setVersion(QDataStream::Qt_5_15); // це бажано включити якщо клієнт і сервер збираються з різними версіями бібліотек

        QDataStream in(&buff,QIODevice::ReadOnly);
        //in.setVersion(QDataStream::Qt_5_15); // це бажано включити якщо клієнт і сервер збираються з різними версіями бібліотек

        switch (stage) {
        case 0: // прийом версії протоколу
            if(buff.size()< (int)sizeof(qint16)) // чекаємо поки буде 2 байта версії протокола
                return; // виходимо, поки не прийдуть дані тут нічого робити

            // ок, є два байти, розбираємо
            // оголошення змінної не по феншую, всередині свіча
            qint16 remVer; // номер версії, що прислав клієнт
            in >> remVer; // читаємо
            if(remVer!=ver) // перевіряємо
            { // якщо не співпало
                out << qint16(1) << ver << QStringLiteral("Невірна версія протоколу");
                sock->close(); // фініта ля комедія.
                return;
            }

            // якщо  ж добралися до цього місця
            stage=1; // перейти на наступний крок

            buff.remove(0,sizeof(qint16)); // два байти вже обробити, видалити їх

            out << qint16(0); // передаємо на клієнт 0 - все Ок
            break;
        case 1: // отримуємо double із клієнта
            if(buff.size()< (int)sizeof (double)) // перевірити чи є там дагі
                return; // чекаємо далі

            // ok, отримано наш дабл
            double val; // ще одне оголошення не по феншую
            in >> val;

            for(int i=0;i<REPCOEF;++i) // це буде великим, дуже великим....
            {
                out << val++;
            }
            sock->close();
            buff.remove(0,sizeof (double)); // почистити буфер
            stage=2; // це на всяк випадок
            return;  //
            break; // це для краси.
        case 2: // ну раптом все-таки сюди добралися
            sock->close();
            break;
        default: // щось пішло не так
            out << qint16(2) << ver << QStringLiteral("На сервері щось пішло нетак.....");
            sock->close(); // фініта ля комедія.
            return;
            break; // це теж для краси
        }
    }

}

void OzClient::slotTimeout()
{
    sock->close(); // закрити по таймауту
}


void OzClient::exec()
{
    sock= new QTcpSocket(); // створити новий об'єкт сокет
    if(!sock->setSocketDescriptor(this->sockID)) // та причепитися до сирого сокета
    {
        emit error(sock->error());
        deleteLater(); // і себе також знищити
        sock->deleteLater();
    }
    else
    {
        connect(sock,&QTcpSocket::readyRead,this,&OzClient::slotRead); // приєднатися до сигналу про приймання даних
        connect(sock,&QTcpSocket::disconnected,this,&OzClient::slotDisconnect); // при завершенні з'єднання треба щось зробити....
        buff.clear(); // про всяк випадок очистити
        buff.append(sock->readAll()); // раптом поки ми його туди-сюди переміщували по потоках прийшли якісь дані
        if(sock->bytesAvailable()>0) // якщо вже все прийнято
            slotRead(); // розібрати що там є....

        // таймер аварійного переривання роботи
        QTimer *tmrOut = new QTimer(this);
        tmrOut->setInterval(SOCKTIMEOUT);
        connect(tmrOut,&QTimer::timeout,this,&OzClient::slotTimeout);
        tmrOut->start(); // не забути запустити

        emit infoMsg(QString("Client connection from %1").arg(sock->peerAddress().toString()));
    }
}

void OzClient::slotDisconnect()
{
    emit finish(); // просигналізувати про фініш
    sock->deleteLater(); // знищити після виходу
    deleteLater(); // і себе також знищити

}


