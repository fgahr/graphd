#ifndef _GRAPHD_REDUCE_H_
#define _GRAPHD_REDUCE_H_

#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>

#define AS_TOK(e) static_cast<expr::TokenExpr *>(e)
#define AS_STMT(e) static_cast<expr::Statement *>(e)

namespace graphd::input::reduce {

template <TokenType tt> bool is_tkn(Expression *e) {
  return expr::TokenExpr::is_instance<tt>(e);
}

class ToStmtList : public Reduction {
public:
  virtual bool perform(Token, ParseStack &s) override {
    int lastidx = s.size() - 1;
    int i = lastidx;
    /*
     * We can reduce if the top of the stack is made up of statements
     * <-- bottom                     top-->
     * non-stmts | stmt | ..stmts.. | stmt
     *                                ^^i^^
     */
    while (i > 0 && expr::Statement::is_instance(s.at(i))) {
      // Jump across statements
      --i;
    }

    if (i == lastidx) {
      // No statements at the top of the stack
      return false;
    }

    expr::StmtList *list = nullptr;
    /*
     * We hope to have the following on the top of the stack:
     * <--bottom                         top-->
     * non-stmts | slist | stmt | stmt | stmts..
     *             ^^i^^
     * Then we can proceed to merge all the statements into the list.
     */
    if (expr::StmtList::is_instance(s.at(i))) {
      list = static_cast<expr::StmtList *>(s.at(i));
    } else {
      /*
       * Stack layout:
       * <--bottom                     top-->
       * non-stmts | non-stmt | stmt | stmts...
       *              ^^i^^ ->->-^
       * Thus, transform the first statement into a statement list.
       */
      i++;
      expr::Statement *first = AS_STMT(s.at(i));
      expr::StmtList *list = new expr::StmtList;
      list->add_statement(first);
      s.at(i) = list;
    }

    /*
     * Now, gather all statements on top of the stack into the list.
     */
    for (int j = i + 1; j <= lastidx; j++) {
      list->add_statement(AS_STMT(s.at(j)));
    }

    /*
     * Finally, remove the statements from the stack.
     */
    for (int j = lastidx; j > i; j--) {
      s.pop_back();
    }

    return true;
  }
  virtual ~ToStmtList() = default;
};

class ToGraph : public Reduction {

public:
  virtual bool perform(Token lookahead, ParseStack &s) override {
    if (lookahead.type != TokenType::EOI) {
      // Input must not contain more than just a graph definition.
      return false;
    }

    /*
     * Structure:
     * [strict] (graph|digraph) [ID] '{' stmt_list '}'
     */
    std::string name = "";
    expr::StmtList *stmtList = nullptr;

    Expression *e;
    int idx = s.size() - 1;
    if (idx < 3) {
      return false;
    }

    e = s.at(idx);
    if (!is_tkn<TokenType::CLOSING_BRACE>(e)) {
      return false;
    }

    idx--;
    e = s.at(idx);
    if (!expr::StmtList::is_instance(e)) {
      return false;
    }

    idx--;
    e = s.at(idx);
    if (!is_tkn<TokenType::OPENING_BRACE>(e)) {
      return false;
    }

    idx--;
    e = s.at(idx);
    if (is_tkn<TokenType::NAME>(e)) {
      name = getval(AS_TOK(e));
      if (idx == 0) {
        return false;
      }
      idx--;
      e = s.at(idx);
    }

    if (is_tkn<TokenType::KEYWORD>(e)) {
      if (getval(AS_TOK(e)) != "graph") {
        return false;
      }
    } else {
      return false;
    }

    if (idx > 1) {
      return false;
    } else if (idx == 1) {
      if (!is_tkn<TokenType::KEYWORD>(s[0]) || getval(AS_TOK(e)) != "strict") {
        return false;
      }
    }

    // We can finally reduce

    // Some expressions will be inaccessible afterwards, so delete them
    for (auto ex : s) {
      // We still need the statements!
      if (ex != stmtList) {
        delete ex;
      }
    }

    s.clear();
    s.push_back(new expr::FullGraph(name, stmtList));

    return true;
  }
  virtual ~ToGraph() = default;

private:
  std::string getval(expr::TokenExpr *e) {
    if (e->token.type != TokenType::KEYWORD) {
      throw std::logic_error{"mistakenly encountered non-keyword token"};
    }
    return e->token.value;
  }
};

} // namespace graphd::input::reduce

#endif // _GRAPHD_REDUCE_H_
