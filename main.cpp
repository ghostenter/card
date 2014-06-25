#include <QCoreApplication>
#include <smartcardmanage.h>
#include <QDebug>
#include <QString>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SmartCardManage scardmage;

    //test
    qDebug() << "reader1 name length:" <<wcslen(scardmage.szReaders[0]);
    if(scardmage.CardOpen(0))
    {
        qDebug("success open the card");
    }

    if(scardmage.CardOpen(0))
    {
        qDebug("success reopen the card");
    }


    return a.exec();
}
