#ifndef GRAPHREPOSITORY_H
#define GRAPHREPOSITORY_H

#include <utility>
#include <optional>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include "components/graph.h"

struct GraphSummary
{
    int id;
    QString name, description;
};

class GraphRepository
{
public:
    explicit GraphRepository(const QString& connectionName) : m_conn(connectionName) {}
    std::optional<int> save(Graph& graph); // insert if id is -1; update otherwise
    std::optional<Graph> load(int id) const;
    QList<GraphSummary> listSummaries() const;
    bool remove(int id);


private:
    QSqlDatabase db() const { return QSqlDatabase::database(m_conn); }
    QString m_conn;
};

#endif // GRAPHREPOSITORY_H
