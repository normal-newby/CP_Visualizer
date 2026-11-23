#include "main-window.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("D:/Projects/sql_practice/database.db");

    if (!db.open()){
        qDebug() << db.lastError().text();
    }

    w.show();
    return QApplication::exec();
}
