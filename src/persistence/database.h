#ifndef DATABASE_H
#define DATABASE_H

#include <QString>

class Database
{
public:
    Database(const QString& path, const QString& connectionName = "db");
    ~Database();
    bool isOpen() const;
    const QString& connectionName() const { return m_name; }
    const QString& error() const { return m_error; }

    // Maintain one owner per connection
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

private:
    bool createSchema(); //returns true if creates something new/edits; false otherwise
    QString m_name, m_error;
};

#endif // DATABASE_H
