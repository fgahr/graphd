#include <graphd/input/parser/pattern.hpp>
#include <graphd/input/parser/reduce.hpp>

#include <utility>

namespace graphd::input::reduce {

using namespace pattern;

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
