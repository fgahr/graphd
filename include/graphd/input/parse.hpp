#ifndef _GRAPHD_PARSE_H_
#define _GRAPHD_PARSE_H_

#include <graphd/graph.hpp>
#include <graphd/input/token.hpp>

#include <istream>
#include <vector>

namespace graphd::input {

/**
 * The possible types of expressions. This is a small subset of the DOT
 * language. See http://www.graphviz.org/doc/info/lang.html for the full
 * grammar.
 */
enum class ExprType {
    GRAPH,
    STATEMENT_LIST,
    EDGE_STATEMENT,
    ATTRIBUTE,
    A_LIST,
    ATTRIBUTE_LIST,
    TOKEN_EXPR,
};

/**
 * Common interface for DOT-language expressions.
 */
class Expression {
  public:
    /**
     * Type of the expression.
     */
    virtual ExprType type() = 0;
    /**
     * Modify the graph g according to this expression's semantic value.
     */
    virtual void apply_to_graph(Graph &g) = 0;
    virtual ~Expression() = default;
};

typedef std::vector<Expression *> ParseStack;

/**
 * A reduction to be performed on a parse stack.
 */
class Reduction {
  public:
    /**
     * Returns true if it succeeded, false otherwise.
     * When false is returned, the stack was not altered.
     */
    virtual bool perform(Token lookahead, ParseStack &s) = 0;
    virtual ~Reduction() = default;
};

/**
 * A simple shift-reduce-type parser without a parsing table, completely
 * adequate for the subset of the DOT language we aim to support.
 */
class Parser {
  public:
    Expression *parse();
    static Parser of(std::istream &in);
    ~Parser();

  private:
    Parser(Tokenizer tokenizer, Token first_token,
           std::vector<Reduction *> reductions);
    bool shift();
    bool reduce();
    ParseStack stack;
    Token lookahead;
    Tokenizer tok;
    std::vector<Reduction *> reductions;
};
} // namespace graphd::input

#endif // _GRAPHD_PARSE_H_
