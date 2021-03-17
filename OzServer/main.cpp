#include <QCoreApplication>
#include "ozserver.h"
#include <QSettings>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // тут зберігаються параметри сервера, особливо не парився з шляхами
    // якщо  програма його не знайде (поточний каталог) то будуть взяті значення за замовчанням. див. нижче
    QSettings set("OzServer.ini",QSettings::IniFormat);

    OzServer serv(set.value("port",45123).toInt());
    serv.setVersion(set.value("proto",1).toInt());

    return a.exec();
}
