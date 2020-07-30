#include <graphd/input/parser/expr.hpp>

#include <utility>

namespace graphd::input::expr {

TokenExpr::TokenExpr(Token t) : token{t} {}

ExprType TokenExpr::type() {
    return ExprType::TOKEN_EXPR;
}

void TokenExpr::apply_to_graph(Graph &) {
    throw std::logic_error{"attempting to apply token expression to graph"};
}

bool TokenExpr::is_instance(Expression *e) {
    return e->type() == ExprType::TOKEN_EXPR;
}

bool Attribute::is_instance(Expression *e) {
    return e->type() == ExprType::ATTRIBUTE;
}

ExprType Attribute::type() {
    return ExprType::ATTRIBUTE;
}

void Attribute::apply_to_graph(Graph &) {
    throw std::logic_error{
        "'Attribute' object cannot be applied to graph directly"};
}

Attribute::Attribute(std::string attr_name, std::string attr_value)
    : name{attr_name}, value{attr_value} {}

bool AttributeList::is_instance(Expression *e) {
    return e->type() == ExprType::ATTRIBUTE_LIST;
}

ExprType AttributeList::type() {
    return ExprType::ATTRIBUTE_LIST;
}

void AttributeList::apply_to_graph(Graph &) {
    throw std::logic_error{
        "'AttributeList' object cannot be applied to graph directly"};
}

AttributeList::AttributeList(std::vector<Attribute *> &&attrs)
    : attributes{attrs} {}

bool AList::is_instance(Expression *e) {
    return e->type() == ExprType::A_LIST;
}

ExprType AList::type() {
    return ExprType::A_LIST;
}

void AList::apply_to_graph(Graph &) {
    throw std::logic_error{
        "'AList' object cannot be applied to graph directly"};
}

AList::~AList() {
    for (auto attr : attributes) {
        delete attr;
    }
}

void AList::add_attribute(Attribute *attr) {
    attributes.push_back(attr);
}

AttributeList *AList::as_attr_list() {
    auto attr_list = new AttributeList{std::move(attributes)};
    attributes.clear();
    return attr_list;
}

bool Statement::is_instance(Expression *e) {
    switch (e->type()) {
    // NOTE: other statement types currently unsupported
    case ExprType::STATEMENT:
        return true;
    default:
        return false;
    }
}

EdgeStmt::EdgeStmt(std::string n1name, std::string n2name)
    : node1_name{n1name}, node2_name{n2name} {}

ExprType EdgeStmt::type() {
    return ExprType::STATEMENT;
}

void EdgeStmt::apply_to_graph(Graph &g) {
    g.add_edge(node1_name, node2_name, distance);
}

StmtList::StmtList() : statements{} {}

ExprType StmtList::type() {
    return ExprType::STMT_LIST;
}

void StmtList::apply_to_graph(Graph &g) {
    for (auto s : statements) {
        s->apply_to_graph(g);
    }
}

StmtList::~StmtList() {
    for (auto s : statements) {
        delete s;
    }
}

void StmtList::add_statement(Statement *s) {
    statements.push_back(s);
}

bool StmtList::is_instance(Expression *e) {
    return e->type() == ExprType::STMT_LIST;
}

FullGraph::FullGraph(std::string name, StmtList *stmtList)
    : name{name}, stmtList{stmtList} {}

void FullGraph::apply_to_graph(Graph &g) {
    g.set_name(name);
    stmtList->apply_to_graph(g);
}

bool FullGraph::is_instance(Expression *e) {
    return e->type() == ExprType::GRAPH;
}

} // namespace graphd::input::expr
