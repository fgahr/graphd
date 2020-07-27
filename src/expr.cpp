#include <graphd/input/parser/expr.hpp>

namespace graphd::input::expr {

TokenExpr::TokenExpr(Token t) : token{t} {}

ExprType TokenExpr::type() { return ExprType::TOKEN_EXPR; }

void TokenExpr::apply_to_graph(Graph &) {
  throw std::logic_error{"attempting to apply token expression to graph"};
}

bool TokenExpr::is_instance(Expression *e) {
  return e->type() == ExprType::TOKEN_EXPR;
}

bool Statement::is_instance(Expression *e) {
  switch (e->type()) {
  // NOTE: other statement types currently unsupported
  case ExprType::EDGE_STATEMENT:
    return true;
  default:
    return false;
  }
}

EdgeStmt::EdgeStmt(std::string n1name, std::string n2name)
    : node1_name{n1name}, node2_name{n2name} {}

ExprType EdgeStmt::type() { return ExprType::EDGE_STATEMENT; }

void EdgeStmt::apply_to_graph(Graph &g) {
  g.add_edge(node1_name, node2_name, distance);
}

StmtList::StmtList() : statements{} {}

ExprType StmtList::type() { return ExprType::STATEMENT_LIST; }

void StmtList::apply_to_graph(Graph &g) {
  for (auto s : statements) {
    s->apply_to_graph(g);
  }
}

StmtList::~StmtList() {
  for (auto s : statements) {
    delete s;
  }
}

void StmtList::add_statement(Statement *s) { statements.push_back(s); }

bool StmtList::is_instance(Expression *e) {
  return e->type() == ExprType::STATEMENT_LIST;
}

FullGraph::FullGraph(std::string name, StmtList *stmtList)
    : name{name}, stmtList{stmtList} {}

void FullGraph::apply_to_graph(Graph &g) {
  g.set_name(name);
  stmtList->apply_to_graph(g);
}

bool FullGraph::is_instance(Expression *e) {
  return e->type() == ExprType::GRAPH;
}

} // namespace graphd::input::expr
