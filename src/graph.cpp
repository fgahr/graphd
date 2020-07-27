#include <graphd/graph.hpp>

#include <stdexcept>

namespace graphd {

Node::Node(NodeName n) : name{n} {}

void Node::add_neighbor(NodeName n, double edge_weight) {
  // FIXME: If the same neighbor is added several times, the connection with
  // lowest weight should be kept.
  neighbors[n] = edge_weight;
}

void Graph::set_name(std::string name) { this->name = name; }

void Graph::add_edge(NodeName n1, NodeName n2, double weight) {
  if (weight < 0) {
    throw std::runtime_error{"negative edge weight not permitted: " +
                             std::to_string(weight)};
  }

  if (n1 == n2) {
    return;
  }

  nodes.try_emplace(n1, n1);
  nodes.try_emplace(n2, n2);

  nodes[n1].add_neighbor(n2, weight);
  nodes[n2].add_neighbor(n1, weight);
}
} // namespace graphd
