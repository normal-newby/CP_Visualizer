#include <QTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

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
    void save_newGraph_assignsIdToGraph();
    void save_emptyGraph_succeeds(); // does not reject graph with no nodes and edges
    void save_duplicateName_failsAndReturnsNullopt();
    //save existing
    void save_afterEdit_keepsSameId();
    void save_afterEdit_doesNotInsertNewRow();
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
    void load_afterRemovingHighestIds_doesNotReuseIds();

    //list summaries
    void listSummaries_empty_returnsEmptyList();
    void listSummaries_correctNameAndIds();

    // remove
    void remove_existingGraph_graphIsGone();
    void remove_cascadesNodesAndEdges();
    void remove_nonexistentId_returnsFalse();
    void remove_onlyRemovesSpecifiedGraph();
    void remove_freesNameForReuse();

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

static int nodeByLabel(const Graph& g, const QString& label)
{
    for (const auto& [id, node] : g.nodes())
    {
        if (node.label() == label) return id;
    }
    return -1;
}

// count num rows from DB
static int rowCount(const QString& connectionName, const QString& table, int graphId)
{
    QSqlQuery q(QSqlDatabase::database(connectionName));
    q.prepare(QString("SELECT COUNT(*) FROM %1 WHERE graph_id = ?").arg(table));
    q.addBindValue(graphId);
    if (!q.exec() || !q.next()) return -1;
    return q.value(0).toInt();
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
    Graph original = makeSampleGraph();

    std::optional<int> id = m_repo->save(original);

    QVERIFY(id.has_value());
    QVERIFY(*id > 0);
}

void TestGraphRepository::save_newGraph_assignsIdToGraph()
{
    Graph graph = makeSampleGraph();
    QCOMPARE(graph.id(), -1);

    std::optional<int> id = m_repo->save(graph);

    QVERIFY(id.has_value());
    QCOMPARE(graph.id(), *id);
}

void TestGraphRepository::save_emptyGraph_succeeds()
{
    Graph empty(-1, "Empty", "");

    std::optional<int> id = m_repo->save(empty);

    QVERIFY(id.has_value());
    QVERIFY(*id > 0);
}

void TestGraphRepository::save_duplicateName_failsAndReturnsNullopt()
{
    Graph original = makeSampleGraph();
    Graph duplicate(-1, original.name(), "");

    std::optional<int> original_id = m_repo->save(original);
    std::optional<int> duplicate_id = m_repo->save(duplicate);

    QVERIFY(original_id.has_value());
    QVERIFY(!duplicate_id.has_value());
    QCOMPARE(duplicate.id(), -1);
    QCOMPARE(m_repo->listSummaries().size(), 1);

    std::optional<Graph> stillThere = m_repo->load(*original_id);
    QVERIFY(stillThere.has_value());
    QVERIFY(stillThere->nodes() == original.nodes());
}

// existing

void TestGraphRepository::save_afterEdit_keepsSameId()
{
    Graph graph = makeSampleGraph();

    std::optional<int> first_save = m_repo->save(graph);
    QVERIFY(first_save.has_value());
    graph.setName("updated");
    std::optional<int> second_save = m_repo->save(graph);

    QVERIFY(second_save.has_value());
    QCOMPARE(*first_save, *second_save);
    QCOMPARE(graph.id(), *first_save);
}

void TestGraphRepository::save_afterEdit_doesNotInsertNewRow()
{
    Graph graph = makeSampleGraph();
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());

    graph.setName("updated");
    graph.setDescription("updated description");
    std::optional<int> second = m_repo->save(graph);

    QVERIFY(second.has_value());
    const QList<GraphSummary> summaries = m_repo->listSummaries();
    QCOMPARE(summaries.size(), 1);
    QCOMPARE(summaries.first().id, *id);
    QCOMPARE(summaries.first().name, QString("updated"));
    QCOMPARE(summaries.first().description, QString("updated description"));
}

void TestGraphRepository::save_afterRemovingNode_nodeAndEdgesGoneAfterLoad()
{
    Graph graph = makeSampleGraph();
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());
    std::optional<Graph> loaded = m_repo->load(*id);
    QVERIFY(loaded.has_value());
    Graph changed = *loaded;

    int cId = nodeByLabel(changed, "C");
    QVERIFY(cId != -1);
    changed.removeNode(cId);

    std::optional<int> changed_id = m_repo->save(changed);
    QVERIFY(changed_id.has_value());
    std::optional<Graph> reloaded = m_repo->load(*id);

    QVERIFY(reloaded.has_value());
    QVERIFY(nodeSignature(*reloaded) == nodeSignature(changed));
    QVERIFY(edgeSignature(*reloaded) == edgeSignature(changed));
}

void TestGraphRepository::save_afterNodeRelabel_correctLabelAfterLoad()
{
    Graph graph = makeSampleGraph();
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());
    std::optional<Graph> loaded = m_repo->load(*id);
    QVERIFY(loaded.has_value());
    Graph changed = *loaded;

    int cId = nodeByLabel(changed, "C");
    QVERIFY(cId != -1);
    QVERIFY(changed.setNodeLabel(cId, "Z"));

    std::optional<int> changed_id = m_repo->save(changed);
    QVERIFY(changed_id.has_value());
    std::optional<Graph> reloaded = m_repo->load(*id);

    QVERIFY(reloaded.has_value());
    QVERIFY(nodeSignature(*reloaded) == nodeSignature(changed));
    QVERIFY(edgeSignature(*reloaded) == edgeSignature(changed));
}

void TestGraphRepository::save_failedSave_previousVersionIntact()
{
    Graph graph1 = makeSampleGraph("Sample");
    std::optional<int> id1 = m_repo->save(graph1);
    QVERIFY(id1.has_value());
    std::optional<Graph> before1 = m_repo->load(*id1);
    QVERIFY(before1.has_value());

    Graph graph2 = makeSampleGraph("Sample2");
    std::optional<int> id2 = m_repo->save(graph2);
    QVERIFY(id2.has_value());
    std::optional<Graph> before2 = m_repo->load(*id2);
    QVERIFY(before2.has_value());

    Graph changed = *before2;
    int cId = nodeByLabel(changed, "C");
    int aId = nodeByLabel(changed, "A");
    QVERIFY(cId != -1);
    QVERIFY(aId != -1);
    changed.removeNode(cId);
    changed.moveNode(aId, QPointF(100, 100));
    changed.setName("Sample");

    std::optional<int> changedId = m_repo->save(changed);

    QVERIFY(!changedId.has_value());

    std::optional<Graph> after2 = m_repo->load(*id2);
    QVERIFY(after2.has_value());
    QCOMPARE(after2->name(), "Sample2");
    QCOMPARE(after2->description(), before2->description());
    QVERIFY(nodeSignature(*after2) == nodeSignature(*before2));
    QVERIFY(edgeSignature(*after2) == edgeSignature(*before2));

    std::optional<Graph> after1 = m_repo->load(*id1);
    QVERIFY(after1.has_value());
    QCOMPARE(after1->name(), "Sample");
    QCOMPARE(after1->description(), before1->description());
    QVERIFY(nodeSignature(*after1) == nodeSignature(*before1));
    QVERIFY(edgeSignature(*after1) == edgeSignature(*before1));
}

void TestGraphRepository::save_existing_doesNotAffectOtherGraphs()
{
    Graph graph1 = makeSampleGraph("Sample");
    std::optional<int> id1 = m_repo->save(graph1);
    QVERIFY(id1.has_value());
    std::optional<Graph> before1 = m_repo->load(*id1);
    QVERIFY(before1.has_value());

    Graph graph2 = makeSampleGraph("Sample2");
    std::optional<int> id2 = m_repo->save(graph2);
    QVERIFY(id2.has_value());
    std::optional<Graph> before2 = m_repo->load(*id2);
    QVERIFY(before2.has_value());

    Graph changed = *before2;
    int cId = nodeByLabel(changed, "C");
    int aId = nodeByLabel(changed, "A");
    QVERIFY(cId != -1);
    QVERIFY(aId != -1);
    changed.removeNode(cId);
    changed.moveNode(aId, QPointF(100, 100));

    std::optional<int> changedId = m_repo->save(changed);

    QVERIFY(changedId.has_value());

    std::optional<Graph> after2 = m_repo->load(*id2);
    QVERIFY(after2.has_value());
    QCOMPARE(after2->name(), changed.name());
    QCOMPARE(after2->description(), changed.description());
    QVERIFY(nodeSignature(*after2) == nodeSignature(changed));
    QVERIFY(edgeSignature(*after2) == edgeSignature(changed));

    //verify does not affect other graph
    std::optional<Graph> after1 = m_repo->load(*id1);
    QVERIFY(after1.has_value());
    QCOMPARE(after1->name(), "Sample");
    QCOMPARE(after1->description(), before1->description());
    QVERIFY(nodeSignature(*after1) == nodeSignature(*before1));
    QVERIFY(edgeSignature(*after1) == edgeSignature(*before1));


}
void TestGraphRepository::save_unknownId_returnsNullopt()
{
    Graph graph(99, "Sample", "");

    std::optional<int> id = m_repo->save(graph);

    QVERIFY(!id.has_value());
    QCOMPARE(graph.id(), 99);
    QVERIFY(m_repo->listSummaries().isEmpty());
}

// load

void TestGraphRepository::load_roundTrip_matchesOriginal()
{
    Graph original = makeSampleGraph();

    std::optional<int> id = m_repo->save(original);
    QVERIFY(id.has_value());
    std::optional<Graph> loaded = m_repo->load(*id);

    QVERIFY(loaded.has_value());
    QCOMPARE(loaded->id(), *id);
    QCOMPARE(loaded->name(), original.name());
    QCOMPARE(loaded->description(), original.description());
    QCOMPARE(loaded->nodes().size(), original.nodes().size());
    QVERIFY(nodeSignature(*loaded) == nodeSignature(original));
    QVERIFY(edgeSignature(*loaded) == edgeSignature(original));

    // ids are stable: same ids, same counters
    QVERIFY(loaded->nodes() == original.nodes());
    QVERIFY(loaded->edges() == original.edges());
    QCOMPARE(loaded->nextNodeId(), original.nextNodeId());
    QCOMPARE(loaded->nextEdgeId(), original.nextEdgeId());
}

void TestGraphRepository::load_nonexistentId_returnsNullopt()
{
    std::optional<Graph> fromEmptyDb = m_repo->load(1);

    Graph graph = makeSampleGraph();
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());
    std::optional<Graph> wrongId = m_repo->load(*id + 1);
    std::optional<Graph> unsavedId = m_repo->load(-1);

    QVERIFY(!fromEmptyDb.has_value());
    QVERIFY(!wrongId.has_value());
    QVERIFY(!unsavedId.has_value());
}

// every edge point at the nodes with the same labels as original
void TestGraphRepository::load_edgesConnectSameLabels()
{
    Graph original = makeSampleGraph();
    std::optional<int> id = m_repo->save(original);
    QVERIFY(id.has_value());

    std::optional<Graph> loaded = m_repo->load(*id);

    QVERIFY(loaded.has_value());
    QCOMPARE(loaded->edges().size(), original.edges().size());
    for (const auto& [edgeId, edge] : loaded->edges())
    {
        QVERIFY(original.edges().count(edgeId));
        const Edge& before = original.edges().at(edgeId);
        QCOMPARE(loaded->nodes().at(edge.src()).label(), original.nodes().at(before.src()).label());
        QCOMPARE(loaded->nodes().at(edge.dest()).label(), original.nodes().at(before.dest()).label());
        QCOMPARE(edge.weight(), before.weight());
        QCOMPARE(edge.directed(), before.directed());
    }
    QVERIFY(edgeSignature(*loaded) == edgeSignature(original));
}

void TestGraphRepository::load_sameGraphTwice_identicalIds()
{
    Graph original = makeSampleGraph();
    std::optional<int> id = m_repo->save(original);
    QVERIFY(id.has_value());

    std::optional<Graph> first = m_repo->load(*id);
    std::optional<Graph> second = m_repo->load(*id);

    QVERIFY(first.has_value());
    QVERIFY(second.has_value());
    QCOMPARE(first->id(), second->id());
    QVERIFY(first->nodes() == second->nodes());
    QVERIFY(first->edges() == second->edges());
    QCOMPARE(first->nextNodeId(), second->nextNodeId());
    QCOMPARE(first->nextEdgeId(), second->nextEdgeId());
}

// the counters must be persisted, not recomputed as max id + 1
void TestGraphRepository::load_afterRemovingHighestIds_doesNotReuseIds()
{
    Graph graph = makeSampleGraph(); // nodes A=0, C=2, D=3; D->D = 4
    int dId = nodeByLabel(graph, "D");
    QCOMPARE(dId, 3);
    graph.removeNode(dId); // removes 4
    const int expectedNodeId = graph.nextNodeId(); // should be 4
    const int expectedEdgeId = graph.nextEdgeId(); // should be 5
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());

    std::optional<Graph> loaded = m_repo->load(*id);
    QVERIFY(loaded.has_value());
    int newNode = loaded->addNode("E", QPointF(0, 0));
    int newEdge = loaded->addEdge(newNode, newNode);

    QCOMPARE(newNode, expectedNodeId); // not 3
    QCOMPARE(newEdge, expectedEdgeId); // not 4
}

// list summaries

void TestGraphRepository::listSummaries_empty_returnsEmptyList()
{
    const QList<GraphSummary> summaries = m_repo->listSummaries();

    QVERIFY(summaries.isEmpty());
}

void TestGraphRepository::listSummaries_correctNameAndIds()
{
    Graph g1 = makeSampleGraph("First");
    Graph g2(-1, "Second", "");
    Graph g3(-1, "Third", "third description");
    std::optional<int> id1 = m_repo->save(g1);
    std::optional<int> id2 = m_repo->save(g2);
    std::optional<int> id3 = m_repo->save(g3);
    QVERIFY(id1.has_value());
    QVERIFY(id2.has_value());
    QVERIFY(id3.has_value());

    const QList<GraphSummary> summaries = m_repo->listSummaries();

    QCOMPARE(summaries.size(), 3);
    std::map<int, std::pair<QString, QString>> byId;
    for (const GraphSummary& s : summaries)
        byId[s.id] = {s.name, s.description};
    QCOMPARE(byId.size(), size_t(3));
    QVERIFY(byId.count(*id1));
    QVERIFY(byId.count(*id2));
    QVERIFY(byId.count(*id3));
    QCOMPARE(byId[*id1].first, QString("First"));
    QCOMPARE(byId[*id1].second, QString("sample description"));
    QCOMPARE(byId[*id2].first, QString("Second"));
    QVERIFY(byId[*id2].second.isEmpty());
    QCOMPARE(byId[*id3].first, QString("Third"));
    QCOMPARE(byId[*id3].second, QString("third description"));
}

// remove

void TestGraphRepository::remove_existingGraph_graphIsGone()
{
    Graph graph = makeSampleGraph();
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());

    bool ok = m_repo->remove(*id);

    QCOMPARE(ok, true);
    QVERIFY(!m_repo->load(*id).has_value());
    QVERIFY(m_repo->listSummaries().isEmpty());
}

void TestGraphRepository::remove_cascadesNodesAndEdges()
{
    Graph graph = makeSampleGraph();
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());
    QCOMPARE(rowCount(m_db->connectionName(), "node", *id), int(graph.nodes().size()));
    QCOMPARE(rowCount(m_db->connectionName(), "edge", *id), int(graph.edges().size()));

    bool ok = m_repo->remove(*id);

    QCOMPARE(ok, true);
    QCOMPARE(rowCount(m_db->connectionName(), "node", *id), 0);
    QCOMPARE(rowCount(m_db->connectionName(), "edge", *id), 0);
}

void TestGraphRepository::remove_nonexistentId_returnsFalse()
{
    bool okEmptyDb = m_repo->remove(1);

    Graph graph = makeSampleGraph();
    std::optional<int> id = m_repo->save(graph);
    QVERIFY(id.has_value());
    bool okWrongId = m_repo->remove(*id + 1);

    QCOMPARE(okEmptyDb, false);
    QCOMPARE(okWrongId, false);
    QVERIFY(m_repo->load(*id).has_value()); // existing graph untouched
}

void TestGraphRepository::remove_onlyRemovesSpecifiedGraph()
{
    Graph g1 = makeSampleGraph("Sample");
    Graph g2 = makeSampleGraph("Sample2");
    std::optional<int> id1 = m_repo->save(g1);
    std::optional<int> id2 = m_repo->save(g2);
    QVERIFY(id1.has_value());
    QVERIFY(id2.has_value());

    bool ok = m_repo->remove(*id1);

    QCOMPARE(ok, true);
    QVERIFY(!m_repo->load(*id1).has_value());

    std::optional<Graph> survivor = m_repo->load(*id2);
    QVERIFY(survivor.has_value());
    QCOMPARE(survivor->name(), QString("Sample2"));
    QVERIFY(survivor->nodes() == g2.nodes());
    QVERIFY(survivor->edges() == g2.edges());
    QCOMPARE(rowCount(m_db->connectionName(), "node", *id2), int(g2.nodes().size()));
    QCOMPARE(rowCount(m_db->connectionName(), "edge", *id2), int(g2.edges().size()));

    const QList<GraphSummary> summaries = m_repo->listSummaries();
    QCOMPARE(summaries.size(), 1);
    QCOMPARE(summaries.first().id, *id2);
}

void TestGraphRepository::remove_freesNameForReuse()
{
    Graph original = makeSampleGraph("Reused");
    std::optional<int> id = m_repo->save(original);
    QVERIFY(id.has_value());
    QVERIFY(m_repo->remove(*id));

    Graph replacement(-1, "Reused", "");
    std::optional<int> newId = m_repo->save(replacement);

    QVERIFY(newId.has_value());
}

QTEST_GUILESS_MAIN(TestGraphRepository)
#include "graph-repository-test.moc"
