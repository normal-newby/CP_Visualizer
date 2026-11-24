#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include "database.h"
#include <QSqlDatabase>

Database::Database(const QString& path, const QString& connectionName)
    : m_name(connectionName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_name);
    db.setDatabaseName(path);
    if (!db.open())
    {
        m_error = db.lastError().text();
        return;
    }

    QSqlQuery(db).exec("PRAGMA foreign_keys = ON");
    createSchema();
}

Database::~Database()
{
    {
        QSqlDatabase db = QSqlDatabase::database(m_name, false);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(m_name);
}

bool Database::isOpen() const { return QSqlDatabase::database(m_name, false).isOpen(); }

bool Database::createSchema()
{
    QFile file(":/schema.sql");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_error = "Cannot open schema.sql";
        return false;
    }

    const QString sql = QString::fromUtf8(file.readAll());
    QSqlDatabase db = QSqlDatabase::database(m_name, false);
    db.transaction();
    QSqlQuery q(db);
    for (const QString& statement : sql.split(';', Qt::SkipEmptyParts))
    {
        if (statement.trimmed().isEmpty()) continue;
        if (!q.exec(statement))
        {
            m_error = q.lastError().text();
            db.rollback();
            return false;
        }
    }

    return db.commit();
}