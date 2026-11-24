#include "graph.h"
#include "node.h"
#include "edge.h"
#include <unordered_set>
#include <algorithm>

Graph::Graph(int id, const QString& name, const QString& description)
    : m_id(id)
    , m_name(name)
    , m_description(description)
{
}

bool Graph::assignId(int id)
{
    if (m_id != -1 || id <= 0) return false;
    m_id = id;
    return true;
}

std::optional<Graph> Graph::restore(int id, const QString& name, const QString& description,
                             const std::vector<Node>& nodes, const std::vector<Edge>& edges,
                             int nextNodeId, int nextEdgeId)
{
    // basic logic integrity check
    if (id <= 0 || nextNodeId < 0 || nextEdgeId < 0) return std::nullopt;
    std::unordered_set<int> nodeIds, edgeIds;
    std::unordered_set<QString> nodeLabels;
    int highestNodeId = -1, highestEdgeId = -1;

    // node integrity check
    for (const Node& node : nodes)
    {
        if (node.id() < 0) return std::nullopt;
        if (nodeIds.count(node.id())) return std::nullopt;
        if (nodeLabels.count(node.label())) return std::nullopt;
        nodeIds.insert(node.id());
        nodeLabels.insert(node.label());
        highestNodeId = std::max(highestNodeId, node.id());
    }

    //edge integrity check
    for (const Edge& edge : edges)
    {
        if (edge.id() < 0) return std::nullopt;
        if (edgeIds.count(edge.id())) return std::nullopt;
        if (!nodeIds.count(edge.src()) || !nodeIds.count(edge.dest())) return std::nullopt;
        edgeIds.insert(edge.id());
        highestEdgeId = std::max(highestEdgeId, edge.id());
    }

    if (nextNodeId <= highestNodeId || nextEdgeId <= highestEdgeId) return std::nullopt;

    //build the return graph

    Graph returnGraph(id, name, description);

    for (const Node& node : nodes) returnGraph.m_nodes.insert({node.id(), node});
    for (const Edge& edge : edges) returnGraph.m_edges.insert({edge.id(), edge});
    returnGraph.m_nextNodeId = nextNodeId;
    returnGraph.m_nextEdgeId = nextEdgeId;

    return returnGraph;
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

bool Graph::setNodeLabel(int id, const QString& label)
{
    if (m_nodes.find(id) == m_nodes.end()) return false;

    for (const auto& [otherId, node] : m_nodes)
        if (otherId != id && node.label() == label) return false;

    m_nodes.at(id).setLabel(label);
    return true;
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
