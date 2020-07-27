#include <graphd/graph.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace graphd {

Node::Node(NodeName n) : name{n} {}

// NOTE: map.contains() not available in C++17
template <typename K, typename V>
static bool contains(std::unordered_map<K, V> &map, K key) {
  return map.find(key) != map.end();
}

static double keep_shortest(DistMap &map, NodeName n, double dist) {
  if (contains(map, n)) {
    double current_dist = map[n];
    if (dist < current_dist) {
      map[n] = dist;
      return dist;
    } else {
      return current_dist;
    }
  } else {
    map[n] = dist;
    return dist;
  }
}

void Node::add_neighbor(NodeName n, double distance) {
  keep_shortest(neighbors, n, distance);
}

void Graph::dijkstra(NodeName from, NodeName to) {
  DistMap next_step;

  while (!contains(distances, to)) {
    double shortest_next = std::numeric_limits<double>::max();
    NodeName next = "";
    next_step.clear();
    for (auto [n, dist] : distances) {
      for (auto [neigh, ndist] : nodes[n].neighbors) {
        if (contains(distances, neigh)) {
          continue;
        }

        double d = keep_shortest(next_step, neigh, dist + ndist);
        if (d < shortest_next) {
          shortest_next = d;
          next = neigh;
        }
      }
    }

    if (next == "") {
      throw std::runtime_error{"nodes not connected: " + a + ", " + b};
    }
    distances[next] = shortest_next;
  }
}

Path Graph::follow_shortest_path(NodeName from, NodeName to) {
  // We start from the end of the path and work our way back.
  // This means  we are gathering hops back to front and need to reverse the
  // list later.
  NodeName current_hop = to;
  std::vector<NodeName> hops;

  while (current_hop != from) {
    hops.push_back(current_hop);

    NodeName next_hop = "";
    double dist = std::numeric_limits<double>::max();

    for (auto [neigh, _] : nodes[current_hop].neighbors) {
      if (contains(distances, neigh) && distances[neigh] < dist) {
        next_hop = neigh;
        dist = distances[neigh];
      }
    }

    if (next_hop == "") {
      throw std::logic_error{"unable to reconstruct shortest path"};
    }
    current_hop = next_hop;
  }
  hops.push_back(from);

  std::reverse(hops.begin(), hops.end());

  return Path{distances[to], hops};
}

Path Graph::shortest_path(NodeName from, NodeName to) {
  distances.clear();
  if (!contains(nodes, from)) {
    throw std::runtime_error{"no such node: " + from};
  }
  if (!contains(nodes, to)) {
    throw std::runtime_error{"no such node: " + to};
  }

  if (from == to) {
    return Path{0.0, {from}};
  }

  DistMap distances = {{from, 0.0}};
  dijkstra(from, to);
  return follow_shortest_path(from, to);
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
