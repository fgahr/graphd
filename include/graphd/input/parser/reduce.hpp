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
  virtual bool perform(ParseStack &s) override {
    if (s.size() < 2) {
      return false;
    }

    if (!expr::Statement::is_instance(s.back())) {
      return false;
    } else {
      Expression *next_to_last = s.at(s.size() - 2);
      if (expr::StmtList::is_instance(next_to_last)) {
        expr::StmtList *sl = static_cast<expr::StmtList *>(next_to_last);
        sl->add_statement(AS_STMT(s.back()));
        s.pop_back();
        return true;
      } else {
        // Turn the last statement into a list by itself.
        expr::Statement *stmt = AS_STMT(s.back());
        s.pop_back();
        expr::StmtList *list = new expr::StmtList;
        list->add_statement(stmt);
        s.push_back(list);
      }
    }
    return false;
  }
  virtual ~ToStmtList() = default;
};

class ToGraph : public Reduction {

public:
  virtual bool perform(ParseStack &s) override {
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
