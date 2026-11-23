#include "node.h"

Node::Node(int id, const QString& label, const QPointF& position)
    : m_id(id)
    , m_label(label)
    , m_position(position)
{
}