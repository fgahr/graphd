#include <graphd/input/parser/reduce.hpp>

#include <utility>

#define AS_TOK(e) static_cast<expr::TokenExpr *>(e)
#define AS_STMT(e) static_cast<expr::Statement *>(e)

namespace graphd::input::reduce {
template <TokenType tt> bool is_tkn(Expression *e) {
    return expr::TokenExpr::is_instance<tt>(e);
}

static std::string getval(Expression *e) {
    if (!expr::TokenExpr::is_instance(e)) {
        throw std::logic_error{"mistakenly encountered non-token expression"};
    }
    return AS_TOK(e)->token.value;
}

typedef bool (*ValuePredicate)(const std::string);

static bool any_value(const std::string) {
    return true;
}
static bool kw_strict(const std::string s) {
    return s == "strict";
}
static bool kw_graph(const std::string s) {
    return s == "graph";
}

template <TokenType tt, ValuePredicate vp = any_value>
bool token_p(Expression *e) {
    if (expr::TokenExpr::is_instance<tt>(e)) {
        return vp(AS_TOK(e)->token.value);
    }
    return false;
}

bool identifier_token(Expression *e) {
    if (expr::TokenExpr::is_instance(e)) {
        return AS_TOK(e)->token.is_identifier();
    }
    return false;
}

class StackWalker {
  public:
    StackWalker(ParseStack &s) : stack{s} {
        idx = s.size() - 1;
    }
    bool exhausted() {
        return idx < 0;
    }
    bool has_next() {
        return idx >= 0;
    }
    void shift() {
        idx--;
    }
    Expression *get() {
        return stack.at(idx);
    }

  private:
    const ParseStack &stack;
    int idx;
};

struct ExprPattern {
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
    bool matches(StackWalker &walker) {
        switch (qualifier) {
        case MANDATORY:
            return matches_one(walker);
        case OPTIONAL:
            return matches_optional(walker);
        case ONE_OR_MORE:
            return matches_one_or_more(walker);
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
    bool matches_one(StackWalker &walker) {
        if (walker.exhausted()) {
            return false;
        }

        if (predicate(walker.get())) {
            store_expr(walker.get());
            walker.shift();
            return true;
        } else {
            return false;
        }
    }
    bool matches_optional(StackWalker &walker) {
        if (walker.exhausted()) {
            return true;
        }

        if (predicate(walker.get())) {
            store_expr(walker.get());
            walker.shift();
        }
        return true;
    }
    bool matches_one_or_more(StackWalker &walker) {
        if (walker.exhausted()) {
            return false;
        }

        if (!predicate(walker.get())) {
            return false;
        }

        while (predicate(walker.get())) {
            store_expr(walker.get());
            walker.shift();
        }
        return true;
    }
};

class StackPatternBuilder;

class StackPattern {
  public:
    bool matches(ParseStack &s);

  private:
    friend class StackPatternBuilder;
    StackPattern(std::vector<ExprPattern> &&patterns) : epats{patterns} {}
    std::vector<ExprPattern> epats;
};

bool StackPattern::matches(ParseStack &s) {
    StackWalker walker{s};
    for (auto el_it = epats.rbegin(); el_it != epats.rend(); el_it++) {
        if (el_it->matches(walker)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

class StackPatternBuilder {
    using Self = StackPatternBuilder &;
    using epred = expr::Predicate;
    // NOTE: same declaration as ParseStack but used differently
    using evec = std::vector<Expression *>;
    using str = std::string;

  public:
    static StackPatternBuilder get() {
        return StackPatternBuilder{};
    }
    StackPattern *build();
    Self one(epred pred, evec *exp_into = nullptr, str *val_into = nullptr) {
        epats.push_back(ExprPattern{
            ExprPattern::MANDATORY,
            pred,
            exp_into,
            val_into,
        });
        return *this;
    }
    Self optional(epred pred, evec *exp_into = nullptr,
                  str *val_into = nullptr) {
        epats.push_back(ExprPattern{
            ExprPattern::OPTIONAL,
            pred,
            exp_into,
            val_into,
        });
        return *this;
    }
    Self at_least_one(epred pred, evec *exp_into = nullptr) {
        epats.push_back(ExprPattern{
            ExprPattern::ONE_OR_MORE,
            pred,
            exp_into,
            // Retrieving values does not apply here
            nullptr,
        });
        return *this;
    }

  private:
    std::vector<ExprPattern> epats;
};

StackPattern *StackPatternBuilder::build() {
    return new StackPattern{std::move(epats)};
}

bool ToAttribute::perform(Token, ParseStack &s) {
    reset();
    if (pattern->matches(s)) {
        for (auto ex : deletable) {
            delete ex;
            s.pop_back();
        }

        s.push_back(new expr::Attribute{attr_name, attr_value});
        return true;
    }
    return false;
}

void ToAttribute::reset() {
    attr_name.clear();
    attr_value.clear();
}

ToAttribute::ToAttribute() {
    pattern = StackPatternBuilder::get()
                  .one(identifier_token, &deletable, &attr_name)
                  .one(token_p<TokenType::EQUAL_SIGN>, &deletable)
                  .one(identifier_token, &deletable, &attr_value)
                  .build();
}

ToAttribute::~ToAttribute() {
    delete pattern;
}

bool ToStatement::perform(Token, ParseStack &s) {
    reset();
    if (pattern->matches(s)) {
        for (auto ex : deletable) {
            delete ex;
            s.pop_back();
        }

        s.push_back(new expr::EdgeStmt{n1name, n2name});
        return true;
    }
    return false;
}

void ToStatement::reset() {
    n1name.clear();
    n2name.clear();
    deletable.clear();
}

ToStatement::ToStatement() {
    pattern = StackPatternBuilder::get()
                  .one(identifier_token, &deletable, &n1name)
                  .one(token_p<TokenType::UNDIRECTED_EDGE>, &deletable)
                  .one(identifier_token, &deletable, &n2name)
                  .one(token_p<TokenType::SEMICOLON>, &deletable)
                  // TODO: Add attribute (list)
                  .build();
}
ToStatement::~ToStatement() {
    delete pattern;
}

bool ToStmtList::perform(Token, ParseStack &s) {
    reset();
    expr::StmtList *slist = nullptr;
    if (!pattern->matches(s)) {
        return false;
    }

    if (list.empty()) {
        slist = new expr::StmtList;
    } else {
        slist = static_cast<expr::StmtList *>(list.front());
    }

    for (auto stmt : statements) {
        slist->add_statement(AS_STMT(stmt));
        s.pop_back();
    }

    // slist might alredy be on the stack. Else we have to add it.
    if (list.empty()) {
        s.push_back(slist);
    }

    return true;
}

void ToStmtList::reset() {
    list.clear();
    statements.clear();
}

ToStmtList::ToStmtList() {
    pattern = StackPatternBuilder::get()
                  .optional(expr::StmtList::is_instance, &list)
                  .at_least_one(expr::Statement::is_instance, &statements)
                  .build();
}

ToStmtList::~ToStmtList() {
    delete pattern;
}

bool ToGraph::perform(Token lookahead, ParseStack &s) {
    deletable.clear();
    if (lookahead.type != TokenType::EOI) {
        /*
         * Either we're not yet at the end of the graph or the input was
         * malformed. In both cases, do nothing.
         */
        return false;
    }

    if (pattern->matches(s)) {
        // Some expressions will be inaccessible, delete what we don't need.
        for (auto ex : deletable) {
            delete ex;
        }

        /*
         * FIXME: We just assume that the graph occupies the entire stack.
         * Otherwise we have malformed input. We ignore that possibility for
         * now.
         */
        s.clear();
        s.push_back(new expr::FullGraph(
            name, static_cast<expr::StmtList *>(stmtList.front())));
        return true;
    } else {
        return false;
    }
}

void ToGraph::reset() {
    name.clear();
    stmtList.clear();
}

ToGraph::ToGraph() {
    pattern =
        StackPatternBuilder::get()
            .optional(token_p<TokenType::KEYWORD, kw_strict>, &deletable)
            .one(token_p<TokenType::KEYWORD, kw_graph>, &deletable)
            .optional(token_p<TokenType::NAME, any_value>, &deletable, &name)
            .one(token_p<TokenType::OPENING_BRACE>, &deletable)
            .one(expr::StmtList::is_instance, &stmtList)
            .one(token_p<TokenType::CLOSING_BRACE>, &deletable)
            .build();
}

ToGraph::~ToGraph() {
    delete pattern;
}

} // namespace graphd::input::reduce
