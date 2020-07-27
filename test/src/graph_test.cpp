#include <gtest/gtest.h>

#include <graphd/graph.hpp>

using namespace graphd;

TEST(Graph, shortest_same) {
  Graph g;

  g.add_edge("a", "b");

  Path p = g.shortest_path("a", "a");

  EXPECT_EQ(p.total_distance, 0.0);
  EXPECT_EQ(p.nodes.size(), 1);
}

TEST(Graph, shortest_1) {
  Graph g;

  // a->b->c->d is shorter than a->d directly
  g.add_edge("a", "b", 1.0);
  g.add_edge("b", "c", 2.0);
  g.add_edge("c", "d", 0.5);
  g.add_edge("a", "d", 4.0);

  Path p = g.shortest_path("a", "d");

  EXPECT_NEAR(p.total_distance, 3.5, 1E-8);
  EXPECT_EQ(p.nodes.size(), 4);
}
