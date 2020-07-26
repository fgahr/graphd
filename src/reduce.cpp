#include <graphd/input/parser/reduce.hpp>

#define AS_TOK(e) static_cast<expr::TokenExpr *>(e)
#define AS_STMT(e) static_cast<expr::Statement *>(e)

namespace graphd::input::reduce {
template <TokenType tt> bool is_tkn(Expression *e) {
  return expr::TokenExpr::is_instance<tt>(e);
}

static std::string getval(Expression *e) {
  if (expr::TokenExpr::is_instance(e)) {
    throw std::logic_error{"mistakenly encountered non-token expression"};
  }
  return AS_TOK(e)->token.value;
}

typedef bool (*ValuePredicate)(const std::string);

static bool any_value(const std::string) { return true; }
static bool kw_strict(const std::string s) { return s == "strict"; }
static bool kw_graph(const std::string s) { return s == "graph"; }

template <TokenType tt, ValuePredicate vp = any_value>
bool TokenP(Expression *e) {
  if (expr::TokenExpr::is_instance<tt>(e)) {
    return vp(AS_TOK(e)->token.value);
  }
  return false;
}

struct StackElement {
  /**
   * How many expressions to expect.
   */
  enum { MANDATORY, OPTIONAL, ONE_OR_MORE } qualifier;
  /**
   * Which conditions the expressions need to satisfy.
   */
  expr::Predicate predicate;
  /**
   * Where to anchor matching expressions.
   */
  std::vector<Expression *> *expr_into = nullptr;
  /**
   * Where to store the token value, if any. Use only with token expressions.
   */
  std::string *value_into = nullptr;
  bool matches(ParseStack::reverse_iterator &it) {
    switch (qualifier) {
    case MANDATORY:
      return matches_one(it);
    case OPTIONAL:
      return matches_optional(it);
    default:
      throw std::logic_error{"illegal 'qualifier' enum value: " +
                             std::to_string(qualifier)};
    }
  }

private:
  void store_expr(Expression *e) {
    if (expr_into != nullptr) {
      expr_into->push_back(e);
    }
    if (value_into != nullptr) {
      *value_into = getval(e);
    }
  }
  bool matches_one(ParseStack::reverse_iterator &it) {
    if (predicate(*it)) {
      store_expr(*it);
      ++it;
      return true;
    } else {
      return false;
    }
  }
  bool matches_optional(ParseStack::reverse_iterator &it) {
    if (predicate(*it)) {
      if (predicate(*it)) {
        store_expr(*it);
      }
      ++it;
    }
    return true;
  }
  bool matches_one_or_more(ParseStack::reverse_iterator &it) {
    if (!predicate(*it)) {
      return false;
    }

    while (predicate(*it)) {
      store_expr(*it);
      ++it;
    }
    return true;
  }
};

class StackState {
  using Self = StackState &;

public:
  bool matches(ParseStack &s);
  StackState(std::vector<StackElement> &&elems) : elements{elems} {}

private:
  std::vector<StackElement> elements;
};

bool StackState::matches(ParseStack &s) {
  // Need to traverse both stack and elements back to front.
  auto stack_it = s.rbegin();
  for (auto el_it = elements.rbegin(); el_it != elements.rend(); el_it++) {
    if (stack_it == s.rend()) {
      return false;
    }
    if (el_it->matches(stack_it)) {
      continue;
    } else {
      return false;
    }
  }
  return true;
}

bool ToStmtList::perform(Token, ParseStack &s) {
  int lastidx = s.size() - 1;
  int i = lastidx;
  /*
   * We can reduce if the top of the stack is made up of statements
   * <-- bottom                     top-->
   * non-stmts | stmt | ..stmts.. | stmt
   *                                ^^i^^
   * In most cases there will be at most one statement on top of the stack but
   * we cannot asssume that.
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
    ++i;
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

ToStmtList::~ToStmtList() { delete state; }

bool ToGraph::perform(Token lookahead, ParseStack &s) {
  if (lookahead.type != TokenType::EOI) {
    /*
     * Either we're not yet at the end of the graph or the input was malformed.
     * In both cases, do nothing.
     */
    return false;
  }

  if (state->matches(s)) {
    // Some expressions will be inaccessible afterwards, so delete them
    for (auto ex : s) {
      // We still need the statements!
      if (ex != stmtList.front()) {
        delete ex;
      }
    }

    /*
     * FIXME: We just assume that the graph occupies the entire stack. Otherwise
     * we have malformed input. We ignore that possibility for now.
     */
    s.clear();
    s.push_back(new expr::FullGraph(
        name, static_cast<expr::StmtList *>(stmtList.front())));
    return true;
  } else {
    return false;
  }
}

ToGraph::ToGraph() {
  state = new StackState{{
      StackElement{
          StackElement::OPTIONAL,
          TokenP<TokenType::KEYWORD, kw_strict>,
      },
      StackElement{
          StackElement::MANDATORY,
          TokenP<TokenType::KEYWORD, kw_graph>,
      },
      StackElement{
          StackElement::OPTIONAL,
          TokenP<TokenType::NAME, any_value>,
          nullptr,
          &name,
      },
      StackElement{
          StackElement::MANDATORY,
          TokenP<TokenType::OPENING_BRACE>,
      },
      StackElement{
          StackElement::MANDATORY,
          expr::StmtList::is_instance,
          &stmtList,
      },
      StackElement{
          StackElement::MANDATORY,
          TokenP<TokenType::CLOSING_BRACE>,
      },
  }};
}

void ToGraph::reset() {
  name = "";
  stmtList.clear();
}

ToGraph::~ToGraph() { delete state; }

} // namespace graphd::input::reduce
