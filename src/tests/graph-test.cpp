#include <QTest>

#include <optional>
#include <unordered_map>
#include <vector>

#include "components/graph.h"

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
    void setNodeLabel_rejectsDuplicateLabels();

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

    // assign id
    void assignId_newGraph_setsId();
    void assignId_alreadyHasId_rejectedAndIdUnchanged();
    void assignId_nonPositiveId_rejected();

    // restore
    void restore_valid_preservesEverything();
    void restore_emptyGraph_succeeds();
    void restore_thenAdd_usesStoredNextIds();
    void restore_nonPositiveGraphId_returnsNullopt();
    void restore_negativeNodeOrEdgeId_returnsNullopt();
    void restore_duplicateNodeId_returnsNullopt();
    void restore_duplicateNodeLabel_returnsNullopt();
    void restore_duplicateEdgeId_returnsNullopt();
    void restore_edgeToMissingNode_returnsNullopt();
    void restore_nextNodeIdNotAboveExisting_returnsNullopt();
    void restore_nextEdgeIdNotAboveExisting_returnsNullopt();
    void restore_negativeNextIds_returnsNullopt();
};

// helpers for restore

static std::vector<Node> sampleNodes()
{
    return {
        Node(0, "A", QPointF(1.5, -2.25)),
        Node(2, "C", QPointF(10, 20)),
        Node(3, "D", QPointF(-3, 4.75)),
    };
}

static std::vector<Edge> sampleEdges()
{
    return {
        Edge(1, 0, 2, 2.5, true),
        Edge(2, 2, 0, -0.1, true),
        Edge(3, 0, 2, 7.0, false),
        Edge(4, 3, 3, 0.0, false),
    };
}

// next ids deliberately larger than max id + 1
static constexpr int kNextNodeId = 7;
static constexpr int kNextEdgeId = 9;

template <typename T>
static std::unordered_map<int, T> byId(const std::vector<T>& items)
{
    std::unordered_map<int, T> out;
    for (const T& item : items)
        out.insert({item.id(), item});
    return out;
}

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
    bool ok = g.setNodeLabel(a, "B");

    QCOMPARE(a, 0);
    QCOMPARE(ok, true);
    QCOMPARE(g.nodes().size(), size_t(1));
    QCOMPARE(g.nodes().at(a).label(), QString("B"));
}

void TestGraph::setNodeLabel_idDoesNotExistDoesNothing()
{
    Graph g;

    int a = g.addNode("A", QPointF(0, 0));
    bool ok = g.setNodeLabel(1, "B");

    QCOMPARE(ok, false);
    QCOMPARE(g.nodes().size(), size_t(1));
    QCOMPARE(g.nodes().at(a).label(), "A");
}

void TestGraph::setNodeLabel_rejectsDuplicateLabels()
{
    Graph g;
    int a = g.addNode("A", QPointF(0, 0));
    int b = g.addNode("B", QPointF(10, 10));

    bool ok = g.setNodeLabel(a, "B");

    QCOMPARE(ok, false);
    QCOMPARE(g.nodes().size(), size_t(2));
    QCOMPARE(g.nodes().at(a).label(), "A");
    QCOMPARE(g.nodes().at(b).label(), "B");
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

// assign id

void TestGraph::assignId_newGraph_setsId()
{
    Graph g(-1, "G", "");

    bool ok = g.assignId(5);

    QCOMPARE(ok, true);
    QCOMPARE(g.id(), 5);
}

void TestGraph::assignId_alreadyHasId_rejectedAndIdUnchanged()
{
    Graph exists(3, "G", "");
    Graph assigned(-1, "H", "");
    QVERIFY(assigned.assignId(5));

    bool ok = exists.assignId(7);
    bool okTwice = assigned.assignId(7);

    QCOMPARE(ok, false);
    QCOMPARE(exists.id(), 3);
    QCOMPARE(okTwice, false);
    QCOMPARE(assigned.id(), 5);
}

void TestGraph::assignId_nonPositiveId_rejected()
{
    Graph g(-1, "G", "");

    bool okZero = g.assignId(0);
    bool okNegative = g.assignId(-3);

    QCOMPARE(okZero, false);
    QCOMPARE(okNegative, false);
    QCOMPARE(g.id(), -1);
}

// restore

void TestGraph::restore_valid_preservesEverything()
{
    const std::vector<Node> nodes = sampleNodes();
    const std::vector<Edge> edges = sampleEdges();

    std::optional<Graph> g = Graph::restore(4, "Restored", "a description", nodes, edges,
                                            kNextNodeId, kNextEdgeId);

    QVERIFY(g.has_value());
    QCOMPARE(g->id(), 4);
    QCOMPARE(g->name(), QString("Restored"));
    QCOMPARE(g->description(), QString("a description"));
    QVERIFY(g->nodes() == byId(nodes));
    QVERIFY(g->edges() == byId(edges));
    QCOMPARE(g->nextNodeId(), kNextNodeId);
    QCOMPARE(g->nextEdgeId(), kNextEdgeId);
}

void TestGraph::restore_emptyGraph_succeeds()
{
    std::optional<Graph> g = Graph::restore(1, "Empty", "", {}, {}, 0, 0);

    QVERIFY(g.has_value());
    QCOMPARE(g->id(), 1);
    QVERIFY(g->nodes().empty());
    QVERIFY(g->edges().empty());
    QCOMPARE(g->nextNodeId(), 0);
    QCOMPARE(g->nextEdgeId(), 0);
}

void TestGraph::restore_thenAdd_usesStoredNextIds()
{
    std::optional<Graph> g = Graph::restore(1, "G", "", sampleNodes(), sampleEdges(),
                                            kNextNodeId, kNextEdgeId);
    QVERIFY(g.has_value());

    int e = g->addNode("E", QPointF(0, 0));
    int f = g->addNode("F", QPointF(0, 0));
    int ef = g->addEdge(e, f);

    // stored counters are used, not max id + 1 (which would reuse deleted ids 4 and 5)
    QCOMPARE(e, kNextNodeId);
    QCOMPARE(f, kNextNodeId + 1);
    QCOMPARE(ef, kNextEdgeId);
    QCOMPARE(g->nodes().size(), size_t(5));
    QCOMPARE(g->edges().size(), size_t(5));
}

// ones that should fail

void TestGraph::restore_nonPositiveGraphId_returnsNullopt()
{
    std::optional<Graph> zero = Graph::restore(0, "G", "", sampleNodes(), sampleEdges(),
                                               kNextNodeId, kNextEdgeId);
    std::optional<Graph> unsaved = Graph::restore(-1, "G", "", sampleNodes(), sampleEdges(),
                                                  kNextNodeId, kNextEdgeId);

    QVERIFY(!zero.has_value());
    QVERIFY(!unsaved.has_value());
}

void TestGraph::restore_negativeNodeOrEdgeId_returnsNullopt()
{
    std::vector<Node> badNodes = sampleNodes();
    badNodes.push_back(Node(-1, "X", QPointF(0, 0)));
    std::vector<Edge> badEdges = sampleEdges();
    badEdges.push_back(Edge(-1, 0, 2, 0.0, false));

    std::optional<Graph> nodeCase = Graph::restore(1, "G", "", badNodes, sampleEdges(),
                                                   kNextNodeId, kNextEdgeId);
    std::optional<Graph> edgeCase = Graph::restore(1, "G", "", sampleNodes(), badEdges,
                                                   kNextNodeId, kNextEdgeId);

    QVERIFY(!nodeCase.has_value());
    QVERIFY(!edgeCase.has_value());
}

void TestGraph::restore_duplicateNodeId_returnsNullopt()
{
    std::vector<Node> nodes = sampleNodes();
    nodes.push_back(Node(2, "Z", QPointF(0, 0))); // id 2 already used

    std::optional<Graph> g = Graph::restore(1, "G", "", nodes, sampleEdges(),
                                            kNextNodeId, kNextEdgeId);

    QVERIFY(!g.has_value());
}

void TestGraph::restore_duplicateNodeLabel_returnsNullopt()
{
    std::vector<Node> nodes = sampleNodes();
    nodes.push_back(Node(5, "A", QPointF(0, 0))); // label "A" already used by node 0

    std::optional<Graph> g = Graph::restore(1, "G", "", nodes, sampleEdges(),
                                            kNextNodeId, kNextEdgeId);

    QVERIFY(!g.has_value());
}

void TestGraph::restore_duplicateEdgeId_returnsNullopt()
{
    std::vector<Edge> edges = sampleEdges();
    edges.push_back(Edge(3, 2, 3, 1.0, true)); // id 3 already used

    std::optional<Graph> g = Graph::restore(1, "G", "", sampleNodes(), edges,
                                            kNextNodeId, kNextEdgeId);

    QVERIFY(!g.has_value());
}

void TestGraph::restore_edgeToMissingNode_returnsNullopt()
{
    // node 1 is not in sampleNodes()
    std::vector<Edge> badSrc = sampleEdges();
    badSrc.push_back(Edge(5, 1, 0, 0.0, true));
    std::vector<Edge> badDest = sampleEdges();
    badDest.push_back(Edge(5, 0, 1, 0.0, true));

    std::optional<Graph> srcCase = Graph::restore(1, "G", "", sampleNodes(), badSrc,
                                                  kNextNodeId, kNextEdgeId);
    std::optional<Graph> destCase = Graph::restore(1, "G", "", sampleNodes(), badDest,
                                                   kNextNodeId, kNextEdgeId);

    QVERIFY(!srcCase.has_value());
    QVERIFY(!destCase.has_value());
}

void TestGraph::restore_nextNodeIdNotAboveExisting_returnsNullopt()
{
    // highest node id in sampleNodes() is 3
    std::optional<Graph> equal = Graph::restore(1, "G", "", sampleNodes(), sampleEdges(),
                                                3, kNextEdgeId);
    std::optional<Graph> below = Graph::restore(1, "G", "", sampleNodes(), sampleEdges(),
                                                2, kNextEdgeId);
    std::optional<Graph> justAbove = Graph::restore(1, "G", "", sampleNodes(), sampleEdges(),
                                                    4, kNextEdgeId);

    QVERIFY(!equal.has_value());
    QVERIFY(!below.has_value());
    QVERIFY(justAbove.has_value()); // boundary: max id + 1 is fine
}

void TestGraph::restore_nextEdgeIdNotAboveExisting_returnsNullopt()
{
    // highest edge id in sampleEdges() is 4
    std::optional<Graph> equal = Graph::restore(1, "G", "", sampleNodes(), sampleEdges(),
                                                kNextNodeId, 4);
    std::optional<Graph> below = Graph::restore(1, "G", "", sampleNodes(), sampleEdges(),
                                                kNextNodeId, 3);
    std::optional<Graph> justAbove = Graph::restore(1, "G", "", sampleNodes(), sampleEdges(),
                                                    kNextNodeId, 5);

    QVERIFY(!equal.has_value());
    QVERIFY(!below.has_value());
    QVERIFY(justAbove.has_value()); // boundary: max id + 1 is fine
}

void TestGraph::restore_negativeNextIds_returnsNullopt()
{
    // empty graph, so "above every existing id" is trivially true; counters must still be >= 0
    std::optional<Graph> nodeCase = Graph::restore(1, "G", "", {}, {}, -1, 0);
    std::optional<Graph> edgeCase = Graph::restore(1, "G", "", {}, {}, 0, -1);

    QVERIFY(!nodeCase.has_value());
    QVERIFY(!edgeCase.has_value());
}

QTEST_APPLESS_MAIN(TestGraph)
#include "graph-test.moc"
