#ifndef _GRAPHD_PARSER_EXPR_H_
#define _GRAPHD_PARSER_EXPR_H_

#include <graphd/input/parse.hpp>

namespace graphd::input::expr {

typedef bool (*Predicate)(Expression *);

class TokenExpr : public Expression {
  public:
    virtual ExprType type() override;
    virtual void apply_to_graph(Graph &g) override;
    virtual ~TokenExpr() = default;

    static bool is_instance(Expression *e);
    template <TokenType tt> static bool is_instance(Expression *e) {
        if (e->type() != ExprType::TOKEN_EXPR) {
            return false;
        }
        TokenExpr *asTok = static_cast<TokenExpr *>(e);
        return asTok->token.type == tt;
    }
    TokenExpr(Token t);

    Token token;
};

class Attribute : public Expression {
  public:
    static bool is_instance(Expression *e);
    virtual ExprType type() override;
    virtual void apply_to_graph(Graph &g) override;
    virtual ~Attribute() = default;
    Attribute(std::string attr_name, std::string attr_value);

    std::string name;
    std::string value;
};

class AttributeList : public Expression {
  public:
    static bool is_instance(Expression *e);
    virtual ExprType type() override;
    virtual void apply_to_graph(Graph &g) override;
    virtual ~AttributeList() = default;
    AttributeList(std::vector<Attribute *> &&attrs);

  private:
    std::vector<Attribute *> attributes;
};

class AList : public Expression {
  public:
    static bool is_instance(Expression *e);
    virtual ExprType type() override;
    virtual void apply_to_graph(Graph &g) override;
    virtual ~AList() = default;
    void add_attribute(Attribute *attr);
    AttributeList *as_attr_list();

  private:
    std::vector<Attribute *> attributes;
};

class Statement : public Expression {
  public:
    static bool is_instance(Expression *e);
    virtual ~Statement() = default;
};

class EdgeStmt : public Statement {
  public:
    virtual ExprType type() override;
    virtual void apply_to_graph(Graph &g) override;
    virtual ~EdgeStmt() = default;
    EdgeStmt(std::string n1name, std::string n2name);

  private:
    std::string node1_name;
    std::string node2_name;
    double distance = 1.0;
};

class StmtList : public Expression {
  public:
    virtual ExprType type() override;
    virtual void apply_to_graph(Graph &g) override;
    virtual ~StmtList();
    StmtList();
    void add_statement(Statement *s);

    static bool is_instance(Expression *e);

  private:
    std::vector<Statement *> statements;
};

class FullGraph : public Expression {
  public:
    virtual ExprType type() override {
        return ExprType::GRAPH;
    }
    virtual void apply_to_graph(Graph &g) override;
    virtual ~FullGraph() {
        delete stmtList;
    }
    FullGraph(std::string name, StmtList *stmtList);

    static bool is_instance(Expression *e);

  private:
    std::string name;
    StmtList *stmtList;
};
} // namespace graphd::input::expr

#endif // _GRAPHD_PARSER_EXPR_H_
