#ifndef _GRAPHD_REDUCE_H_
#define _GRAPHD_REDUCE_H_

#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>

namespace graphd::input::reduce {

class StackPattern;

class ToStatement : public Reduction {
public:
  virtual bool perform(Token, ParseStack &s) override;
  virtual ~ToStatement();
  ToStatement();

private:
  void reset();
  std::string n1name;
  std::string n2name;
  // Gather all obsolete expressions here
  std::vector<Expression *> deletable;
  StackPattern *pattern;
};

/**
 * Reduction for a group of individual statements into a statement list.
 */
class ToStmtList : public Reduction {
public:
  virtual bool perform(Token, ParseStack &s) override;
  virtual ~ToStmtList();
  ToStmtList();

private:
  void reset();
  std::vector<Expression *> list;
  std::vector<Expression *> statements;
  StackPattern *pattern;
};

/**
 * Reduction for a full well-formed input into a graph definition.
 */
class ToGraph : public Reduction {
public:
  virtual bool perform(Token lookahead, ParseStack &s) override;
  virtual ~ToGraph();
  ToGraph();

private:
  void reset();
  std::string name;
  std::vector<Expression *> stmtList;
  StackPattern *pattern;
};

} // namespace graphd::input::reduce

#endif // _GRAPHD_REDUCE_H_
