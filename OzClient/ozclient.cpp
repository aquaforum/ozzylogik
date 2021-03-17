#include "ozclient.h"
#include <QByteArray>
#include <QDataStream>

#include <QFile>
#include <QTimer>

#include <QRandomGenerator>

OzClient::OzClient(QString hostName, int portN, QObject *parent) : QObject(parent),
  sock(nullptr),
  host(hostName),
  port(portN),
  ver(0),
  stage(0),
  databin(nullptr)
{

}


// ця функція виконується в паралельноу потоці
void OzClient::exec()
{
    sock=new QTcpSocket(this); // створити сокет
    // з'єдніти сигнали з слотами
    connect(sock,&QTcpSocket::connected,this,&OzClient::slotConnect);
    connect(sock,&QTcpSocket::readyRead,this,&OzClient::slotRead);
    connect(sock,&QTcpSocket::disconnected,this,&OzClient::slotDisconnect);
// це як в анектоді: "За Кучми такого не було...."
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    connect(sock,&QTcpSocket::errorOccurred,this,&OzClient::slotError);
#else
    connect(sock,&QTcpSocket::error,this,&OzClient::slotError);
#endif
    // спробувати з'єднатися
    sock->connectToHost(host,port);
}

void OzClient::slotConnect()
{
    // після того, як з'єднання встановлено
    QDataStream out(sock);
    //out.setVersion(QDataStream::Qt_5_15); // це бажано включити якщо клієнт і сервер збираються з різними версіями бібліотек
    out << ver; // відправляємо версію протокола
}

void OzClient::slotRead()
{
    QDataStream stream(sock);
    //stream.setVersion(QDataStream::Qt_5_15); // це бажано включити якщо клієнт і сервер збираються з різними версіями бібліотек
    for(;sock->bytesAvailable()>0;) // поки там щось є
    {
        switch (stage) {
        case 0: // читаємо відповідь сервера з узгодженням протоколу
            if(sock->bytesAvailable()<(int)sizeof (qint16)) // а якщо там замало даних
                return; // тоді на вихід
            qint16 verCheck; // знову не по феншую
            stream >> verCheck;
            if(verCheck!=0) // сервер прислав помилку
            {
                QString srvErrMsg; // прочитати текст помилки
                qint16 remVer;
                stream >> remVer >> srvErrMsg;
                QFile f("error.log"); // записати її в лог файл
                f.open(QIODevice::Append);
                f.write(QString("Err:%1\tRemVer:%2\tMsg:%3\n").arg(verCheck).arg(remVer).arg(srvErrMsg).toUtf8()); // ну подобається мені UTF-8
                f.close();
                sock->flush(); // щоб там ще не залишалося - до біса його.
                sock->close(); // закінчити
                return;
            }
            // відкрити файл для запису даних
            databin = new QFile(QString("data-%1.bin").arg(QRandomGenerator::global()->generate(),8,16,QChar('0') ));
            databin->open(QIODevice::WriteOnly );

            // через три секунди надіслати свої дані
            QTimer::singleShot(TIMEWAIT,this,&OzClient::slotWriteMyData);


            stage=1;
            return; // тут би перевірити чи в сокеті ще є дані, бо тоді засада, але з другої сторони протокол TCP з гарантованою доставкою, в даний момент там їх не повинно бути
            break;
        case 1: // а далі все що прийшло просто пишемов в файл
            if(databin->isOpen())
                databin->write(sock->readAll());
            break;
        }
    }

}

void OzClient::slotDisconnect()
{

   if(databin!=nullptr && databin->isOpen()) // якщо все спрацювало правильно то зараз
   {
       databin->close(); // потрібно закрити файл
       databin->deleteLater(); // і прибрати після себе
       databin=nullptr;
   }

    emit finish(); // завершити роботу
}


void OzClient::slotError(QTcpSocket::SocketError v)
{

    if(v!=QTcpSocket::RemoteHostClosedError) // це нормальна ситуація в даній програмі, сервер сам закриває з'єднання коли закінчив.
    {
        qDebug() << "Socket error: " << v;
        emit error(v); // ну і кому я його відправляю? тук-тук, в цій програмі нікого немає вдома......
    }
    slotDisconnect();
}

void OzClient::slotWriteMyData()
{
    QDataStream out(sock);
    //out.setVersion(QDataStream::Qt_5_15); // це бажано включити якщо клієнт і сервер збираються з різними версіями бібліотек
    out << _value; // відправити дані на сервер
}
