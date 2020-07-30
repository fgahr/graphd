#include <graphd/input/parser/pattern.hpp>
#include <graphd/input/parser/reduce.hpp>

#include <utility>

#define AS_TKN(e) static_cast<expr::TokenExpr *>(e)

namespace graphd::input::reduce {

using namespace pattern;

static std::string getval(Expression *e) {
    if (!expr::TokenExpr::is_instance(e)) {
        throw std::logic_error{"mistakenly encountered non-token expression"};
    }
    return AS_TKN(e)->token.value;
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
        return vp(AS_TKN(e)->token.value);
    }
    return false;
}

bool identifier_token(Expression *e) {
    if (expr::TokenExpr::is_instance(e)) {
        return AS_TKN(e)->token.is_identifier();
    }
    return false;
}

using pattern::StackWalker;

struct ExprPattern {
    /**
     * How many expressions to expect.
     */
    enum { MANDATORY, OPTIONAL, ONE_OR_MORE, ANY } qualifier;
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
        case ANY:
            return any_match(walker);
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
    bool any_match(StackWalker &walker) {
        if (walker.exhausted()) {
            return true;
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
    Self any(epred pred, evec *exp_into = nullptr) {
        epats.push_back(ExprPattern{
            ExprPattern::ANY,
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
    StackWalker walker{s};
    if (pattern->match(walker)) {
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
    deletable.clear();
}

ToAttribute::ToAttribute() : attr_name{""}, attr_value{""} {
    pattern.reset(sequence({
        optional(',', {add_to(deletable)}),
        identifier({value(attr_name), add_to(deletable)}),
        exact('=', {add_to(deletable)}),
        identifier({value(attr_value), add_to(deletable)}),
    }));
}

bool ToAList::perform(Token, ParseStack &s) {
    reset();
    StackWalker walker{s};

    if (!pattern->match(walker)) {
        return false;
    }

    expr::AList *alist;
    if (list.empty()) {
        alist = new expr::AList;
    } else {
        alist = static_cast<expr::AList *>(list.front());
    }

    if (attributes.empty()) {
        throw std::logic_error{"collecting attributes failed"};
    }

    for (auto attr : attributes) {
        alist->add_attribute(static_cast<expr::Attribute *>(attr));
        s.pop_back();
    }

    if (list.empty()) {
        s.push_back(alist);
    }

    return true;
}

void ToAList::reset() {
    attributes.clear();
}

ToAList::ToAList() {
    pattern.reset(sequence({
        exact('['), // Leave this token in place
        optional(has_type(ExprType::A_LIST, {add_to(list)})),
        repeated(has_type(ExprType::ATTRIBUTE, {add_to(attributes)})),
    }));
}

bool ToStatement::perform(Token, ParseStack &s) {
    reset();
    StackWalker walker{s};
    if (pattern->match(walker)) {
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
    pattern.reset(sequence({
        identifier({value(n1name), add_to(deletable)}),
        exact("--", {add_to(deletable)}),
        identifier({value(n2name), add_to(deletable)}),
        exact(';', {add_to(deletable)}),
    }));
}

bool ToStmtList::perform(Token, ParseStack &s) {
    reset();
    expr::StmtList *slist = nullptr;

    StackWalker walker{s};
    if (!pattern->match(walker)) {
        return false;
    }

    if (list.empty()) {
        slist = new expr::StmtList;
    } else {
        slist = static_cast<expr::StmtList *>(list.front());
    }

    for (auto stmt : statements) {
        slist->add_statement(static_cast<expr::Statement *>(stmt));
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
    pattern.reset(sequence({
        optional(has_type(ExprType::STMT_LIST, {add_to(list)})),
        repeated(has_type(ExprType::STATEMENT, {add_to(statements)})),
    }));
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

    StackWalker walker{s};

    if (pattern->match(walker)) {
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
    pattern.reset(sequence({
        optional(exact("strict", {add_to(deletable)})),
        exact("graph", {add_to(deletable)}),
        optional(identifier({value(name), add_to(deletable)})),
        exact('{', {add_to(deletable)}),
        has_type(ExprType::STMT_LIST, {add_to(stmtList)}),
        exact('}', {add_to(deletable)}),
    }));
}

} // namespace graphd::input::reduce
