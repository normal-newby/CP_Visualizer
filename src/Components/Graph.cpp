#include "Graph.h"
#include "Node.h"
#include "Edge.h"

Graph::Graph(int id, const QString& name, const QString& description)
    : m_id(id)
    , m_name(name)
    , m_description(description)
{
}

int Graph::addNode(const QString& label, const QPointF pos)
{
    for (const auto& [id, node] : m_nodes)
        if (node.label() == label) return -1;

    m_nodes.insert({m_nextNodeId, Node(m_nextNodeId, label, pos)});
    return m_nextNodeId++;
}

void Graph::moveNode(int id, const QPointF& pos)
{
    if (m_nodes.find(id) == m_nodes.end()) return;

    m_nodes.at(id).setPosition(pos);
}

void Graph::setNodeLabel(int id, const QString& label)
{
    if (m_nodes.find(id) == m_nodes.end()) return;

    m_nodes.at(id).setLabel(label);
}

void Graph::removeNode(int id)
{
    if (m_nodes.find(id) == m_nodes.end()) return;

    for (auto it = m_edges.begin(); it != m_edges.end();)
    {
        if (it->second.dest() == id || it->second.src() == id)
            it = m_edges.erase(it);
        else
            ++it;
    }

    m_nodes.erase(id);
}

int Graph::addEdge(int src, int dest, double weight, bool directed)
{
    if (m_nodes.find(src) == m_nodes.end()
        || m_nodes.find(dest) == m_nodes.end()) return -1;

    m_edges.insert({m_nextEdgeId, Edge(m_nextEdgeId, src, dest, weight, directed)});
    return m_nextEdgeId++;
}

int Graph::addEdge(int src, int dest)
{
    return addEdge(src, dest, 0.0, false);
}

void Graph::removeEdge(int id)
{
    if (m_edges.find(id) == m_edges.end()) return;

    m_edges.erase(id);
}
