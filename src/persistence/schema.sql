CREATE TABLE IF NOT EXISTS graph (
    graph_id INTEGER PRIMARY KEY,
    name VARCHAR(255) NOT NULL UNIQUE,
    description TEXT
);

CREATE TABLE IF NOT EXISTS node (
    graph_id INTEGER NOT NULL,
    node_id INTEGER NOT NULL,
    node_label VARCHAR(50),
    x REAL NOT NULL,
    y REAL NOT NULL,
    PRIMARY KEY(graph_id, node_id),
    FOREIGN KEY (graph_id) REFERENCES graph(graph_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS edge (
    graph_id INTEGER NOT NULL,
    edge_id INTEGER NOT NULL,
    source_node INTEGER NOT NULL,
    destination_node INTEGER NOT NULL,
    weight REAL NOT NULL DEFAULT 0,
    directed BOOLEAN NOT NULL DEFAULT 0 CHECK (directed IN (0,1)),
    PRIMARY KEY(graph_id, edge_id),
    FOREIGN KEY (graph_id) REFERENCES graph(graph_id) ON DELETE CASCADE,
    FOREIGN KEY (graph_id, source_node) REFERENCES node(graph_id, node_id) ON DELETE CASCADE,
    FOREIGN KEY (graph_id, destination_node) REFERENCES node(graph_id, node_id) ON DELETE CASCADE
);
