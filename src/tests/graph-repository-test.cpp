#include <QTest>

#include <algorithm>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "persistence/database.h"
#include "persistence/graph-repository.h"

/* Remember to use:
 * Arrange
 * Act
 * Assert
 */

class TestGraphRepository : public QObject
{
    Q_OBJECT

private slots:
    // get the database ready before and after each test, respectively
    void init();
    void cleanup();

    //save new
    void save_newGraph_returnsPositiveId();
    void save_emptyGraph_succeeds(); // does not reject graph with no nodes and edges
    void save_duplicateName_failsAndReturnsNullopt();
    //save existing
    void save_afterEdit_keepsSameId();
    void save_afterRemovingNode_nodeAndEdgesGoneAfterLoad();
    void save_afterNodeRelabel_correctLabelAfterLoad();
    void save_failedSave_previousVersionIntact();
    void save_existing_doesNotAffectOtherGraphs();
    void save_unknownId_returnsNullopt();

    //load
    void load_roundTrip_matchesOriginal();
    void load_nonexistentId_returnsNullopt(); //(Null optional)
    void load_edgesConnectSameLabels();
    void load_sameGraphTwice_identicalIds();

    //list summaries
    void listSummaries_empty_returnsEmptyList();
    void listSummaries_correctNameAndIds();

    // remove
    void remove_existingGraph_graphIsGone();
    void remove_cascadesNodesAndEdges();
    void remove_nonexistentId_returnsFalse();
    void remove_onlyRemovesSpecifiedGraph();

private:
    std::unique_ptr<Database> m_db;
    std::unique_ptr<GraphRepository> m_repo;
};

// helper functions

// compare nodes by label to make sure the remapping works
static std::map<QString, QPointF> nodeSignature(const Graph& g)
{
    std::map<QString, QPointF> out;
    for (const auto& [id, node] : g.nodes())
        out[node.label()] = node.position();
    return out;
}

// same thing as above, but with edges; sorted vector in case of duplicates which map cant pick up
static std::vector<std::tuple<QString, QString, double, bool>> edgeSignature(const Graph& g)
{
    std::vector<std::tuple<QString, QString, double, bool>> out;
    for (const auto& [id, edge] : g.edges())
        out.emplace_back(g.nodes().at(edge.src()).label(), g.nodes().at(edge.dest()).label(),
                         edge.weight(), edge.directed());
    std::sort(out.begin(), out.end());
    return out;
}

/* default graph, covers:
 * bidirectional directed
 * parallel
 * self loop
 * negative weight
 */
static Graph makeSampleGraph(const QString& name = "Sample")
{
    Graph g(-1, name, "sample description");
    int a = g.addNode("A", QPointF(1.5, -2.25));
    int b = g.addNode("B", QPointF(0, 0));
    int c = g.addNode("C", QPointF(10, 20));
    int d = g.addNode("D", QPointF(-3, 4.75));
    g.addEdge(a, b);
    g.addEdge(a, c, 2.5, true);
    g.addEdge(c, a, -0.1, true);
    g.addEdge(a, c, 7.0, false);
    g.addEdge(d, d);
    g.removeNode(b);
    return g;
}

// initalize + cleanup

void TestGraphRepository::init()
{
    m_db = std::make_unique<Database>(":memory:", "test"); // make own database for each test
    QVERIFY2(m_db->isOpen(), qPrintable(m_db->error()));
    QVERIFY2(m_db->error().isEmpty(), qPrintable(m_db->error()));

    m_repo = std::make_unique<GraphRepository>(m_db->connectionName());
}

void TestGraphRepository::cleanup()
{
    m_repo.reset();
    m_db.reset(); // closes and removes the connection
}

// save

//new

void TestGraphRepository::save_newGraph_returnsPositiveId()
{
    const Graph original = makeSampleGraph();

    std::optional<int> id = m_repo->save(original);

    QVERIFY(id.has_value());
    QVERIFY(*id > 0);
}

void TestGraphRepository::save_emptyGraph_succeeds()
{
    const Graph empty = new Graph(-1, "Empty", "");

    std::optional<int> id = m_repo->save(empty);

    QVERIFY(id.has_value());
    QVERIFY(*id > 0);
}
void TestGraphRepository::save_duplicateName_failsAndReturnsNullopt()
{
    const Graph original = makeSampleGraph();
    Graph duplicate = new Graph();
    duplicate.setName(original.name());

    std::optional<int> original_id = m_repo->save(original);
    std::optional<int> duplicate_id = m_repo->save(duplicate);

    QVERITY(original_id.has_value());
    QCOMPARE(!duplicate_id.has_value());
}

// load

void TestGraphRepository::load_roundTrip_matchesOriginal()
{
    const Graph original = makeSampleGraph();

    std::optional<int> id = m_repo->save(original);
    QVERIFY(id.has_value());
    std::optional<Graph> loaded = m_repo->load(*id);

    QVERIFY(loaded.has_value());
    QCOMPARE(loaded->id(), *id);
    QCOMPARE(loaded->name(), original.name());
    QCOMPARE(loaded->description(), original.description());
    QCOMPARE(loaded->nodes().size(), original.nodes().size());
    QVERIFY(nodeSignature(*loaded) == nodeSignature(original));
}

// todo
void TestGraphRepository::save_afterEdit_keepsSameId() { QSKIP("not written yet"); }
void TestGraphRepository::save_afterRemovingNode_nodeAndEdgesGoneAfterLoad() { QSKIP("not written yet"); }
void TestGraphRepository::save_afterNodeRelabel_correctLabelAfterLoad() { QSKIP("not written yet"); }
void TestGraphRepository::save_failedSave_previousVersionIntact() { QSKIP("not written yet"); }
void TestGraphRepository::save_existing_doesNotAffectOtherGraphs() { QSKIP("not written yet"); }
void TestGraphRepository::save_unknownId_returnsNullopt() { QSKIP("not written yet"); }
void TestGraphRepository::load_nonexistentId_returnsNullopt() { QSKIP("not written yet"); }
void TestGraphRepository::load_edgesConnectSameLabels() { QSKIP("not written yet"); } // use edgeSignature()
void TestGraphRepository::load_sameGraphTwice_identicalIds() { QSKIP("not written yet"); }
void TestGraphRepository::listSummaries_empty_returnsEmptyList() { QSKIP("not written yet"); }
void TestGraphRepository::listSummaries_correctNameAndIds() { QSKIP("not written yet"); }
void TestGraphRepository::remove_existingGraph_graphIsGone() { QSKIP("not written yet"); }
void TestGraphRepository::remove_cascadesNodesAndEdges() { QSKIP("not written yet"); }
void TestGraphRepository::remove_nonexistentId_returnsFalse() { QSKIP("not written yet"); }
void TestGraphRepository::remove_onlyRemovesSpecifiedGraph() { QSKIP("not written yet"); }

QTEST_GUILESS_MAIN(TestGraphRepository)
#include "graph-repository-test.moc"
