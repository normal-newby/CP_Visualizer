#ifndef NODE_H
#define NODE_H

#include <QString>
#include <QPointF>

class Node
{
public:
    explicit Node(int id, const QString& label, const QPointF& position);

    //getters
    int id() const { return m_id; }
    const QString& label() const { return m_label; }
    const QPointF& position() const { return m_position; }

    //setters
    void setLabel(const QString& label) { m_label = label; }
    void setPosition(const QPointF& position) { m_position = position; }

private:
    int m_id;
    QString m_label;
    QPointF m_position;
};

#endif // NODE_H
