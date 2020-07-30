#ifndef _GRAPHD_REDUCE_H_
#define _GRAPHD_REDUCE_H_

#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>
#include <graphd/input/parser/pattern.hpp>

namespace graphd::input::reduce {

class StackPattern;

using Pat = std::unique_ptr<pattern::Pattern>;

/**
 * Reduction to a single attribute in an attribute list.
 */
class ToAttribute : public Reduction {
  public:
    virtual bool perform(Token, ParseStack &s) override;
    virtual ~ToAttribute() = default;
    ToAttribute();

  private:
    void reset();
    std::string attr_name;
    std::string attr_value;
    std::vector<Expression *> deletable;
    Pat pattern;
};

/**
 * Reduction to an AList, i.e. the inner part of an attribute list.
 */
class ToAList : public Reduction {
  public:
    virtual bool perform(Token, ParseStack &s) override;
    virtual ~ToAList() = default;
    ToAList();

  private:
    void reset();
    /*
     * If a list exists, we can use it.
     */
    std::vector<Expression *> list;
    std::vector<Expression *> attributes;
    Pat pattern;
};

class ToAttrList : public Reduction {
  public:
    virtual bool perform(Token, ParseStack &s) override;
    virtual ~ToAttrList() = default;
    ToAttrList();

  private:
    void reset();
    std::vector<Expression *> alist;
    std::vector<Expression *> deletable;
    Pat pattern;
};

/**
 * Reduction to a single statement.
 */
class ToStatement : public Reduction {
  public:
    virtual bool perform(Token, ParseStack &s) override;
    virtual ~ToStatement() = default;
    ToStatement();

  private:
    void reset();
    std::string n1name;
    std::string n2name;
    std::vector<Expression *> deletable;
    std::vector<Expression *> attr_list;
    Pat pattern;
};

/**
 * Reduction for a group of individual statements into a statement list.
 */
class ToStmtList : public Reduction {
  public:
    virtual bool perform(Token, ParseStack &s) override;
    virtual ~ToStmtList() = default;
    ToStmtList();

  private:
    void reset();
    std::vector<Expression *> list;
    std::vector<Expression *> statements;
    Pat pattern;
};

/**
 * Reduction for a full well-formed input into a graph definition.
 */
class ToGraph : public Reduction {
  public:
    virtual bool perform(Token lookahead, ParseStack &s) override;
    virtual ~ToGraph() = default;
    ToGraph();

  private:
    void reset();
    std::string name;
    std::vector<Expression *> stmtList;
    std::vector<Expression *> deletable;
    Pat pattern;
};

} // namespace graphd::input::reduce

#endif // _GRAPHD_REDUCE_H_
