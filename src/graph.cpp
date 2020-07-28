#include <graphd/graph.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace graphd {

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

Path Graph::dijkstra(NodeName start, NodeName end) {
  DistMap distances = {{start, 0.0}};
  std::unordered_map<NodeName, NodeName> coming_from;

  while (!contains(distances, end)) {
    double shortest_next_distance = std::numeric_limits<double>::max();
    // Which node to add in this step, connecting from prev
    NodeName next = "";
    NodeName prev = "";

    // Explore options for the next step
    for (auto [node, node_dist] : distances) {
      for (auto [neighbor, dist_from_node] : nodes[node].neighbors) {
        if (contains(distances, neighbor)) {
          // already mapped
          continue;
        }

        double total_dist = node_dist + dist_from_node;
        if (total_dist < shortest_next_distance) {
          shortest_next_distance = total_dist;
          next = neighbor;
          prev = node;
        }
      }
    }

    if (next == "") {
      throw std::runtime_error{"nodes not connected: " + start + ", " + end};
    }

    distances[next] = shortest_next_distance;
    coming_from[next] = prev;
  }

  // Trace the path backwards from end to start, building the path in reverse.
  std::vector<NodeName> hops;
  for (NodeName n = end; n != start; n = coming_from[n]) {
    hops.push_back(n);
  }
  hops.push_back(start);

  std::reverse(hops.begin(), hops.end());

  return Path{distances[end], hops};
}

Path Graph::shortest_path(NodeName from, NodeName to) {
  if (!contains(nodes, from)) {
    throw std::runtime_error{"no such node: " + from};
  }
  if (!contains(nodes, to)) {
    throw std::runtime_error{"no such node: " + to};
  }

  return dijkstra(from, to);
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

  nodes.try_emplace(n1);
  nodes.try_emplace(n2);

  nodes[n1].add_neighbor(n2, weight);
  nodes[n2].add_neighbor(n1, weight);
}
} // namespace graphd
