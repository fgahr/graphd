#ifndef _GRAPHD_GRAPH_H_
#define _GRAPHD_GRAPH_H_

#include <string>
#include <unordered_set>

namespace graphd {
class Node {
  // TODO
};

class Edge {
  // TODO
};

class Graph {
  using Nname = std::string;

public:
  void add_edge(Nname n1, Nname n2, double weight = 1.0);

private:
};
}; // namespace graphd

#endif // _GRAPHD_GRAPH_H_
