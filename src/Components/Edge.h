#ifndef EDGE_H
#define EDGE_H

class Edge
{
public:
    explicit Edge(int id, int src, int dest, double weight, bool directed);

    //getters
    int id() const { return m_id; }
    int src() const { return m_src; }
    int dest() const { return m_dest; }
    double weight() const { return m_weight; }
    bool directed() const { return m_directed; }

    //setters
    void setWeight(double weight) { m_weight = weight; }
    void setDirected(bool directed) { m_directed = directed; }

private:
    int m_id, m_src, m_dest;
    double m_weight;
    bool m_directed;
};

inline bool operator==(const Edge& a, const Edge& b)
{
    return a.id() == b.id() && a.src() == b.src() && a.dest() == b.dest()
           && a.weight() == b.weight() && a.directed() == b.directed();
}

#endif // EDGE_H
