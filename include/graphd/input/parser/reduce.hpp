#ifndef _GRAPHD_REDUCE_H_
#define _GRAPHD_REDUCE_H_

#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>
#include <graphd/input/parser/pattern.hpp>

namespace graphd::input::reduce {

class StackPattern;

using _StackPattern = pattern::Pattern;

/**
 * Reduction to a single attribute in an attribute list.
 */
class ToAttribute : public Reduction {
  public:
    virtual bool perform(Token, ParseStack &s) override;
    virtual ~ToAttribute();
    ToAttribute();

  private:
    void reset();
    std::string attr_name;
    std::string attr_value;
    std::vector<Expression *> deletable;
    _StackPattern *pattern;
};

/**
 * Reduction to an AList, i.e. the inner part of an attribute list.
 */
class ToAList : public Reduction {
  public:
    virtual bool perform(Token, ParseStack &s) override;
    virtual ~ToAList();
    ToAList();

  private:
    void reset();
    // name and value of the first attribute in the list
    std::string name;
    std::string value;
    std::vector<Expression *> deletable;
    StackPattern *pattern;
    std::vector<Expression *> attributes;
};

/**
 * Reduction to a single statement.
 */
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
    std::vector<Expression *> deletable;
    StackPattern *pattern;
};

} // namespace graphd::input::reduce

#endif // _GRAPHD_REDUCE_H_
