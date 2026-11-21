#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QPointF>
#include <map>
#include "Node.h"
#include "Edge.h"

class Graph {
public:
    Graph() = default;
    explicit Graph(int id, const QString& name, const QString& description);

    //getters
    int id() const { return m_id; }
    const QString& name() const { return m_name; }
    const QString& description() const { return m_description; }
    const std::map<int, Node>& nodes() const { return m_nodes; }
    const std::map<int, Edge>& edges() const { return m_edges; }

    //setters
    void setName(const QString& name);
    void setDescription(const QString& description);

    // BEHAVIOUR

    //nodes
    int addNode(const QString& label, const QPointF pos); //returns the new id generated
    void moveNode(int id, const QPointF& pos);
    void setNodeLabel(int id, const QString& name);
    void removeNode(int id); // also removes edges

    //edges
    int addEdge(int src, int dest, double weight, bool directed); // returns -1 if src/dest invalid
    void removeEdge(int id);

private:
    int m_id = -1;
    QString m_name;
    QString m_description;

    std::map<int, Node> m_nodes;
    std::map<int, Edge> m_edges;
    int m_nextNodeId = 0, m_nextEdgeId = 0;
};

#endif // GRAPH_H