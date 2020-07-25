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

/**
 * Reduction for a group of individual statements into a statement list.
 */
class ToStmtList : public Reduction {
public:
  virtual bool perform(Token, ParseStack &s) override;
  virtual ~ToStmtList() = default;
};

/**
 * Reduction for a full well-formed input into a graph definition.
 */
class ToGraph : public Reduction {
public:
  virtual bool perform(Token lookahead, ParseStack &s) override;
  virtual ~ToGraph() = default;
};

} // namespace graphd::input::reduce

#endif // _GRAPHD_REDUCE_H_
