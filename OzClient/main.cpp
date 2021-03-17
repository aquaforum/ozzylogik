#include <QCoreApplication>
#include "ozclient.h"
#include <QThread>
#include <QSettings>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // тут зберігаються параметри сервера, особливо не парився з шляхами
    // якщо  програма його не знайде (поточний каталог) то будуть взяті значення за замовчанням. див. нижче
    QSettings set("OzClient.ini",QSettings::IniFormat);

    // за умовами задачі основний цикл програми не можна блокувати....
    QThread *thr = new QThread; // значить ствоюємо новий потік
    OzClient *client = new OzClient(set.value("host","localhost").toString(),set.value("port",45123).toInt());

    client->setVersion(set.value("proto",1).toInt()); // встановити версію протокола
    client->setValue(set.value("value",0.0).toDouble()); // завантажити значення клієнта

    client->moveToThread(thr); // нафіг з пляжу....тобто з цього потока Ж;-)

    QObject::connect(thr,&QThread::started,client,&OzClient::exec); // робимо корисну справу
    QObject::connect(client,&OzClient::finish,thr,&QThread::quit);  // це команда СТОП
    QObject::connect(thr,&QThread::finished,thr,&QThread::deleteLater); // звільняємо після себе пам'ять, чисто для краси, рельно програма завершується
    QObject::connect(thr,&QThread::finished,client,&OzClient::deleteLater); // і тут також

    QObject::connect(thr,&QThread::finished,&a,QCoreApplication::quit); // але ж треба якось завершити програму після отримання даних..... хоч вона більше й нічого не робить...
    thr->start(); // запустити на виконання потік.

    return a.exec();
}
