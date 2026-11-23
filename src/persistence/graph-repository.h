#ifndef GRAPHREPOSITORY_H
#define GRAPHREPOSITORY_H

#include <utility>
#include <QString>
#include <QList>

struct GraphSummary
{
    int id;
    QString name, description;
};

class GraphRepository
{
public:
    explicit GraphRepository(QString& connectionName) : m_conn(connectionName) {}
    bool save(const Graph& graph); // insert if id is -1; update otherwise
    std::optional<Graph> load(int id) const;
    QList<GraphSummary> listSummaries();
    bool remove(int id);


private:
    QSqlDatabase db() const { return QSqlDatabase::database(m_conn); }
    QString m_conn;
};

#endif // GRAPHREPOSITORY_H
