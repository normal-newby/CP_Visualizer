#include <QTest>

#include "Components/Graph.h"

/* Remember to use:
 * Arrange
 * Act
 * Assert
 */

class TestGraph : public QObject
{
    Q_OBJECT

private slots:

    // add node
    void addNode_returnsIncreasingIds();
    void addNode_idsNeverReused();
    void addNode_duplicateLabelReturnsMinusOne();

    // move node
    void moveNode_newPositionIsCorrect();
    void moveNode_idDoesNotExistDoesNothing();

    // set node label
    void setNodeLabel_newLabelIsCorrect();
    void setNodeLabel_idDoesNotExistDoesNothing();

    // remove node
    void removeNode_removesOnlyConnectedEdges();
    void removeNode_removesDirectedEdgesInBothDirections();
    void removeNode_hasNoEdgesWorks();
    void removeNode_idDoesNotExistDoesNothing();

    // add edge
    void addEdge_returnsIncreasingIds();
    void addEdge_idsNeverReused();
    void addEdge_srcOrDestInvalidDoesNothing();
    void addEdge_valuesStoredExactlyAsPassed();
    void addEdge_selfLoopsAllowed();
    void addEdge_parallelEdgesAllowed();

    // remove edge
    void removeEdge_keepsNodesIntact();
    void removeEdge_parallelEdgesOnlyRemovesOneSpecified();
    void removeEdge_idDoesNotExistDoesNothing();

    // graph
    void defaultGraph_isEmptyWithIdMinusOne();
};

void TestGraph::addNode_returnsIncreasingIds()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(10, 10));
    int c = g.addNode("C", QPointF(20, 20));

    QCOMPARE(a, 0);
    QCOMPARE(b, 1);
    QCOMPARE(c, 2);

    QCOMPARE(g.nodes().size(), size_t(3));
    QCOMPARE(g.nodes().at(b).label(), QString("B"));
}

void TestGraph::addNode_idsNeverReused()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    g.removeNode(a);
    int b = g.addNode("B", QPointF(10, 10));

    QCOMPARE(a, 0);
    QCOMPARE(b, 1);

    QCOMPARE(g.nodes().size(), size_t(1));
    QVERIFY(g.nodes().find(a) == g.nodes().end());
    QCOMPARE(g.nodes().at(b).label(), QString("B"));
}

void TestGraph::addNode_duplicateLabelReturnsMinusOne()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int a2 = g.addNode("A", QPointF(10, 10));

    QCOMPARE(a, 0);
    QCOMPARE(a2, -1);

    QCOMPARE(g.nodes().size(), size_t(1));
    QVERIFY(g.nodes().find(a2) == g.nodes().end());
    QCOMPARE(g.nodes().at(a).label(), QString("A"));
}

void TestGraph::moveNode_newPositionIsCorrect()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    QPointF newPoint = QPointF(10, 10);
    g.moveNode(a, newPoint);

    QCOMPARE(g.nodes().size(), size_t(1));
    QCOMPARE(g.nodes().at(a).position(), newPoint);
}

void TestGraph::moveNode_idDoesNotExistDoesNothing()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    auto before = g.nodes();

    g.moveNode(99, QPointF(10, 10));

    QVERIFY(g.nodes() == before);
    QCOMPARE(g.nodes().at(a).position(), QPointF(0, 0));
}

void TestGraph::setNodeLabel_newLabelIsCorrect()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    g.setNodeLabel(a, "B");

    QCOMPARE(a, 0);
    QCOMPARE(g.nodes().size(), size_t(1));
    QCOMPARE(g.nodes().at(a).label(), QString("B"));
}

void TestGraph::setNodeLabel_idDoesNotExistDoesNothing()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    g.setNodeLabel(1, "B");

    QCOMPARE(g.nodes().size(), size_t(1));
    QCOMPARE(g.nodes().at(a).label(), "A");
}

void TestGraph::removeNode_removesOnlyConnectedEdges()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(10, 10));
    int c = g.addNode("C", QPointF(20, 20));
    int ab = g.addEdge(a, b, 0.0, false);
    int bc = g.addEdge(b, c, 0.0, true);
    const Edge bcBefore = g.edges().at(bc);

    g.removeNode(a);

    QVERIFY(g.nodes().find(a) == g.nodes().end());
    QVERIFY(g.edges().find(ab) == g.edges().end());
    QCOMPARE(g.nodes().size(), size_t(2));
    QCOMPARE(g.edges().size(), size_t(1));

    QVERIFY(g.edges().find(bc) != g.edges().end());
    QCOMPARE(g.edges().at(bc), bcBefore);
}

void TestGraph::removeNode_removesDirectedEdgesInBothDirections()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));
    int forward = g.addEdge(a, b, 0.0, true);
    int backward = g.addEdge(b, a, 0.0, true);

    g.removeNode(a);

    QVERIFY(g.nodes().find(a) == g.nodes().end());
    QVERIFY(g.edges().find(forward) == g.edges().end());
    QVERIFY(g.edges().find(backward) == g.edges().end());
    QCOMPARE(g.nodes().size(), size_t(1));
    QCOMPARE(g.edges().size(), size_t(0));
}

void TestGraph::removeNode_hasNoEdgesWorks()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));
    const Node bNode = g.nodes().at(b);

    g.removeNode(a);

    QVERIFY(g.nodes().find(a) == g.nodes().end());
    QCOMPARE(g.nodes().at(b), bNode);
    QCOMPARE(g.nodes().size(), size_t(1));
}

void TestGraph::removeNode_idDoesNotExistDoesNothing()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    const Node aBefore = g.nodes().at(a);

    g.removeNode(2);

    QCOMPARE(g.nodes().at(a), aBefore);
    QCOMPARE(g.nodes().size(), size_t(1));
}

QTEST_APPLESS_MAIN(TestGraph)
#include "graph_test.moc"