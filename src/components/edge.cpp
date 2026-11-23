#include "edge.h"

Edge::Edge(int id, int src, int dest, double weight, bool directed)
    : m_id(id)
    , m_src(src)
    , m_dest(dest)
    , m_weight(weight)
    , m_directed(directed)
{
}