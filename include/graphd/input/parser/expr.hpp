#ifndef _GRAPHD_PARSER_EXPR_H_
#define _GRAPHD_PARSER_EXPR_H_

#include <graphd/input/parse.hpp>

namespace graphd::input::expr {
class TokenExpr : public Expression {
public:
  virtual ExprType type() override { return ExprType::TOKEN_EXPR; }
  virtual void apply_to_graph(Graph &g) override {
    throw std::logic_error{"attempting to apply token expression to graph"};
  }
  virtual ~TokenExpr() = default;

  template <TokenType tt> static bool is_instance(Expression *e) {
    if (e->type() != ExprType::TOKEN_EXPR) {
      return false;
    }
    TokenExpr *asTok = static_cast<TokenExpr *>(e);
    return asTok->token.type == tt;
  }

  Token token;
  TokenExpr(Token t) : token{t} {}
};

class Statement : public Expression {
public:
  static bool is_instance(Expression *e) {
    switch (e->type()) {
    case ExprType::NODE_STATEMENT:
    case ExprType::EDGE_STATEMENT:
      return true;
    default:
      return false;
    }
  }
  virtual ~Statement() = default;
};

class StmtList : public Expression {
public:
  virtual ExprType type() override { return ExprType::STATEMENT_LIST; }
  virtual void apply_to_graph(Graph &g) override {
    for (auto s : statements) {
      s->apply_to_graph(g);
    }
  }
  virtual ~StmtList() {
    for (auto s : statements) {
      delete s;
    }
  }

  static bool is_instance(Expression *e) {
    return e->type() == ExprType::STATEMENT_LIST;
  }
  StmtList() : statements{} {}
  void add_statement(Statement *s) { statements.push_back(s); }

private:
  std::vector<Statement *> statements;
};

class FullGraph : public Expression {
public:
  virtual ExprType type() override { return ExprType::GRAPH; }
  virtual void apply_to_graph(Graph &g) override {
    // TODO
  }
  virtual ~FullGraph() = default;

  static bool is_instance(Expression *e) {
    return e->type() == ExprType::GRAPH;
  }

  std::string name;
  StmtList *stmtList;
  FullGraph(std::string name, StmtList *stmtList)
      : name{name}, stmtList{stmtList} {}
};
} // namespace graphd::input::expr

#endif // _GRAPHD_PARSER_EXPR_H_
