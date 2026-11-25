#include "graph-repository.h"

std::optional<int> GraphRepository::save(Graph& graph)
{
    Q_UNUSED(graph);
    return std::nullopt;
}

std::optional<Graph> GraphRepository::load(int id) const
{
    Q_UNUSED(id);
    return std::nullopt;
}

QList<GraphSummary> GraphRepository::listSummaries() const
{
    return {};
}

bool GraphRepository::remove(int id)
{
    Q_UNUSED(id);
    return false;
}
