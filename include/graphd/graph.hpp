#ifndef _GRAPHD_GRAPH_H_
#define _GRAPHD_GRAPH_H_

#include <string>
#include <unordered_map>
#include <vector>

namespace graphd {
using NodeName = std::string;
using DistMap = std::unordered_map<NodeName, double>;

struct Node {
  NodeName name;
  DistMap neighbors;
  Node(NodeName name);
  void add_neighbor(NodeName n, double edge_weight);
};

struct Path {
  double total_distance;
  std::vector<NodeName> nodes;
};

class Graph {
public:
  Path shortest_path(NodeName from, NodeName to);
  void set_name(std::string name);
  void add_edge(NodeName n1, NodeName n2, double distance = 1.0);

private:
  void dijkstra(NodeName from, NodeName to);
  Path follow_shortest_path(NodeName from, NodeName to);
  // NOTE: Might well be useless for now.
  std::string name;
  std::unordered_map<NodeName, Node> nodes;
  DistMap distances;
};
}; // namespace graphd

#endif // _GRAPHD_GRAPH_H_
