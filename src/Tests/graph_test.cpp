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
    void addEdge_defaultIsCorrect();
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

void TestGraph::addEdge_returnsIncreasingIds()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));
    int c = g.addNode("C", QPointF(0, 0));

    int ab = g.addEdge(a, b);
    int bc = g.addEdge(b, c);
    int ac = g.addEdge(a, c);

    QCOMPARE(ab, 0);
    QCOMPARE(bc, 1);
    QCOMPARE(ac, 2);

    QCOMPARE(g.edges().size(), size_t(3));
    QCOMPARE(g.edges().at(bc).dest(), c);
    QCOMPARE(g.edges().at(bc).src(), b);
}

void TestGraph::addEdge_idsNeverReused()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));

    int ab = g.addEdge(a, b);
    g.removeEdge(ab);
    int ab2 = g.addEdge(a, b);
    int ba = g.addEdge(b, a);

    QCOMPARE(ab, 0);
    QCOMPARE(ab2, 1);
    QCOMPARE(ba, 2);
    QCOMPARE(g.edges().size(), size_t(2));
    QCOMPARE(g.edges().at(ba).dest(), a);
    QCOMPARE(g.edges().at(ba).src(), b);
}

void TestGraph::addEdge_defaultIsCorrect()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));

    int ab = g.addEdge(a, b);

    QCOMPARE(g.edges().size(), size_t(1));
    const Edge& e = g.edges().at(ab);
    QCOMPARE(e.src(), a);
    QCOMPARE(e.dest(), b);
    QCOMPARE(e.weight(), 0.0);
    QCOMPARE(e.directed(), false);
}

void TestGraph::addEdge_srcOrDestInvalidDoesNothing()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));
    int c = 98, d = 99; // ids that don't exist

    int ac = g.addEdge(a, c, 0.0, true); // nothing happens
    int db = g.addEdge(d, b, 0.0, true); // nothing happens
    int cd = g.addEdge(c, d, 0.0, true); // nothing happens

    QCOMPARE(ac, -1);
    QCOMPARE(db, -1);
    QCOMPARE(cd, -1);
    QCOMPARE(g.edges().size(), size_t(0));
    QCOMPARE(g.nodes().size(), size_t(2));
}

void TestGraph::addEdge_valuesStoredExactlyAsPassed()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));

    int ab = g.addEdge(a, b, 2.5, true);
    int ba = g.addEdge(b, a, -3.1, false);


    QCOMPARE(g.edges().size(), size_t(2));

    const Edge& ab_edge = g.edges().at(ab);
    const Edge& ba_edge = g.edges().at(ba);

    QCOMPARE(ab_edge.id(), ab);
    QCOMPARE(ab_edge.src(), a);
    QCOMPARE(ab_edge.dest(), b);
    QCOMPARE(ab_edge.weight(), 2.5);
    QCOMPARE(ab_edge.directed(), true);

    QCOMPARE(ba_edge.id(), ba);
    QCOMPARE(ba_edge.src(), b);
    QCOMPARE(ba_edge.dest(), a);
    QCOMPARE(ba_edge.weight(), -3.1);
    QCOMPARE(ba_edge.directed(), false);
}

void TestGraph::addEdge_selfLoopsAllowed()
{
    Graph g;
    int a = g.addNode("A", QPointF(0, 0));

    int aa = g.addEdge(a, a);

    QVERIFY(aa != -1);
    QCOMPARE(g.edges().size(), size_t(1));
    QCOMPARE(g.edges().at(aa).id(), aa);
    QCOMPARE(g.edges().at(aa).src(), a);
    QCOMPARE(g.edges().at(aa).dest(), a);
}

void TestGraph::addEdge_parallelEdgesAllowed()
{
    Graph g;
    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));

    int ab = g.addEdge(a, b, 1.0, true);
    int ba = g.addEdge(b, a, 2.0, true);
    int undirected1 = g.addEdge(a, b);
    int undirected2 = g.addEdge(a, b);

    QCOMPARE(g.edges().size(), size_t(4));
    QCOMPARE(ab, 0);
    QCOMPARE(ba, 1);
    QCOMPARE(undirected1, 2);
    QCOMPARE(undirected2, 3);
    QCOMPARE(g.edges().at(undirected1).src(), a);
    QCOMPARE(g.edges().at(undirected2).src(), a);
}

void TestGraph::removeEdge_keepsNodesIntact()
{
    Graph g;
    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));
    const Node aBefore = g.nodes().at(a);
    const Node bBefore = g.nodes().at(b);
    int ab = g.addEdge(a, b);

    g.removeEdge(ab);

    QCOMPARE(g.edges().size(), size_t(0));
    QCOMPARE(g.nodes().size(), size_t(2));
    QCOMPARE(g.nodes().at(a), aBefore);
    QCOMPARE(g.nodes().at(b), bBefore);
}

void TestGraph::removeEdge_parallelEdgesOnlyRemovesOneSpecified()
{
    Graph g;
    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));
    int ab1 = g.addEdge(a, b);
    int ab2 = g.addEdge(a, b);
    const Edge ab2Before = g.edges().at(ab2);

    g.removeEdge(ab1);

    QCOMPARE(g.edges().size(), size_t(1));
    QVERIFY(g.edges().find(ab1) == g.edges().end());
    QCOMPARE(g.edges().at(ab2), ab2Before);
}

void TestGraph::removeEdge_idDoesNotExistDoesNothing()
{
    Graph g;
    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(0, 0));
    int ab = g.addEdge(a, b);
    const Edge abBefore = g.edges().at(ab); // copy, not reference

    g.removeEdge(99);

    QCOMPARE(ab, 0);
    QCOMPARE(g.edges().size(), size_t(1));
    QCOMPARE(g.edges().at(ab), abBefore);
}

void TestGraph::defaultGraph_isEmptyWithIdMinusOne()
{
    Graph g;

    QCOMPARE(g.id(), -1);
    QVERIFY(g.name().isEmpty());
    QVERIFY(g.description().isEmpty());
    QVERIFY(g.nodes().empty());
    QVERIFY(g.edges().empty());
}

QTEST_APPLESS_MAIN(TestGraph)
#include "graph_test.moc"
