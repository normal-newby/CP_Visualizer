#include "graph-repository.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <vector>
#include <components/node.h>
#include <components/edge.h>

/* establish connection everytime before running query
 * use place holders + bind values using QSqlQuery::bindValue
 * prepare and exec functions
 * use while (q.next()) for multiple values
 * index at q.value(idx) --> matches column index
 * q.value(string) --> by column name
 * .value() returns QVariant --> must convert with to`{insert datatype T here}`()
*/

std::optional<int> GraphRepository::save(Graph& graph)
{
    // preliminary steps
    QSqlDatabase conn = db();
    int id = graph.id();
    bool newGraph = false;

    // start transaction because "save" must follow atomicity
    if (!conn.transaction()) return std::nullopt;
    QSqlQuery q(conn);

    // insert/update the graph
    if (id == -1)
    {
        newGraph = true;

        q.prepare("INSERT INTO graph (name, description, next_node_id, next_edge_id)"
                  "VALUES (:name, :description, :next_node_id, :next_edge_id)");
        q.bindValue(":name", graph.name());
        q.bindValue(":description", graph.description());
        q.bindValue(":next_node_id", graph.nextNodeId());
        q.bindValue(":next_edge_id", graph.nextEdgeId());

        if (!q.exec())
        {
            conn.rollback();
            return std::nullopt;
        }

        id = q.lastInsertId().toInt();
    } else
    {
        q.prepare("UPDATE GRAPH SET name=:name, description=:description, "
                  "next_node_id=:next_node_id, next_edge_id=:next_edge_id "
                  "WHERE graph_id=:id");
        q.bindValue(":name", graph.name());
        q.bindValue(":description", graph.description());
        q.bindValue(":next_node_id", graph.nextNodeId());
        q.bindValue(":next_edge_id", graph.nextEdgeId());
        q.bindValue(":id", id);

        if (!q.exec())
        {
            conn.rollback();
            return std::nullopt;
        }

        if (q.numRowsAffected() == 0) // nothing is changed, no graph with such id
        {
            conn.rollback();
            return std::nullopt;
        }

        // delete all nodes
        q.prepare("DELETE FROM node WHERE graph_id=:id");
        q.bindValue(":id", id);
        if (!q.exec())
        {
            conn.rollback();
            return std::nullopt;
        }

        // delete all edges
        q.prepare("DELETE FROM edge WHERE graph_id=:id");
        q.bindValue(":id", id);
        if (!q.exec())
        {
            conn.rollback();
            return std::nullopt;
        }
    }

    // insert all nodes
    q.prepare("INSERT INTO node (graph_id, node_id, node_label, x, y) "
              "VALUES (:graph_id, :node_id, :node_label, :x, :y)");
    q.bindValue(":graph_id", id);
    for (const auto& [node_id, node] : graph.nodes())
    {
        q.bindValue(":node_id", node_id);
        q.bindValue(":node_label", node.label());

        // for position
        QPointF point = node.position();
        q.bindValue(":x", point.x());
        q.bindValue(":y", point.y());

        // exec
        if (!q.exec())
        {
            conn.rollback();
            return std::nullopt;
        }
    }

    // insert all edges
    q.prepare("INSERT INTO edge (graph_id, edge_id, source_node, destination_node, weight, directed) "
              "VALUES (:graph_id, :edge_id, :source_node, :destination_node, :weight, :directed)");
    q.bindValue(":graph_id", id);
    for (const auto& [edge_id, edge] : graph.edges())
    {
        q.bindValue(":edge_id", edge_id);
        q.bindValue(":source_node", edge.src());
        q.bindValue(":destination_node", edge.dest());
        q.bindValue(":weight", edge.weight());
        q.bindValue(":directed", edge.directed());

        // exec
        if (!q.exec())
        {
            conn.rollback();
            return std::nullopt;
        }
    }

    if (!conn.commit())
    {
        conn.rollback();
        return std::nullopt;
    }

    if (newGraph) graph.assignId(id);
    return id;
}

std::optional<Graph> GraphRepository::load(int id) const
{
    QSqlDatabase conn = db();
    QSqlQuery q(conn);

    // query the graph
    q.prepare("SELECT * FROM graph WHERE graph_id=:id");
    q.bindValue(":id", id);
    if (!q.exec()) return std::nullopt;

    //check if there is a value
    if (!q.next()) return std::nullopt;

    // instantiate info
    QString name = q.value("name").toString();
    QString description = q.value("description").toString();
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    int nextNodeId = q.value("next_node_id").toInt();
    int nextEdgeId = q.value("next_edge_id").toInt();

    // read nodes
    q.prepare("SELECT * FROM node WHERE graph_id=:id");
    q.bindValue(":id", id);
    if (!q.exec()) return std::nullopt;
    while (q.next())
    {
        int node_id = q.value("node_id").toInt();
        double x = q.value("x").toDouble(),
            y = q.value("y").toDouble();
        QString label = q.value("node_label").toString();

        nodes.push_back(Node(node_id, label, QPointF(x, y)));
    }

    // read edges
    q.prepare("SELECT * FROM edge WHERE graph_id=:id");
    q.bindValue(":id", id);
    if (!q.exec()) return std::nullopt;
    while (q.next())
    {
        int edge_id = q.value("edge_id").toInt(),
            src = q.value("source_node").toInt(),
            dest = q.value("destination_node").toInt();
        double weight = q.value("weight").toDouble();
        bool directed = q.value("directed").toBool();

        edges.push_back(Edge(edge_id, src, dest, weight, directed));
    }

    return Graph::restore(id, name, description, nodes, edges, nextNodeId, nextEdgeId);
}

QList<GraphSummary> GraphRepository::listSummaries() const
{
    QSqlDatabase conn = db();
    QSqlQuery q(conn);

    QList<GraphSummary> summary;

    // prepare sql statement
    q.prepare("SELECT graph_id, name, description FROM graph");
    if (!q.exec()) return {};

    // go through stuff and add to list
    while (q.next())
    {
        int id = q.value("graph_id").toInt();
        QString name = q.value("name").toString(),
            description = q.value("description").toString();

        summary.append(GraphSummary{id, name, description});
    }

    return summary;
}

bool GraphRepository::remove(int id)
{
    QSqlDatabase conn = db();
    QSqlQuery q(conn);

    q.prepare("DELETE FROM graph WHERE graph_id=:id");
    q.bindValue(":id", id);
    if (!q.exec()) return false;

    return q.numRowsAffected() > 0;
}
