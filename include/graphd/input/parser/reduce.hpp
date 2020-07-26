#ifndef _GRAPHD_REDUCE_H_
#define _GRAPHD_REDUCE_H_

#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>

namespace graphd::input::reduce {
/**
 * A reduction to be performed on a parse stack.
 */
class Reduction {
public:
  /**
   * Returns true if it succeeded, false otherwise.
   * When false is returned, the stack must not be altered.
   */
  virtual bool perform(Token lookahead, ParseStack &s) = 0;
  virtual ~Reduction() = default;
};

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
  std::string attr_name;
  std::string attr_val;
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
