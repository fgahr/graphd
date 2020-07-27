#ifndef _GRAPHD_GRAPH_H_
#define _GRAPHD_GRAPH_H_

#include <string>
#include <unordered_map>
#include <vector>

namespace graphd {
using NodeName = std::string;

struct Node {
  NodeName name;
  std::unordered_map<NodeName, double> neighbors;
  Node(NodeName name);
  void add_neighbor(NodeName n, double edge_weight);
};

struct Path {
  double total_weight;
  std::vector<NodeName> nodes;
};

class Graph {
public:
  Path shortest_path_between(NodeName a, NodeName b);
  void set_name(std::string name);
  void add_edge(NodeName n1, NodeName n2, double weight = 1.0);

private:
  // NOTE: Might well be useless for now.
  std::string name;
  std::unordered_map<const NodeName, Node> nodes;
};
}; // namespace graphd

#endif // _GRAPHD_GRAPH_H_
