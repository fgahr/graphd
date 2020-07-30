#include <gtest/gtest.h>

#include <graphd/input/parser/reduce.hpp>

#include <sstream>
#include <string>

using namespace graphd::input;

Token t(char c) {
    switch (c) {
    case '\0':
        return Token{TokenType::EOI, ""};
    case '}':
        return Token{TokenType::CLOSING_BRACE, ""};
    case ']':
        return Token{TokenType::CLOSING_SQUARE_BRACKET, ""};
    case ';':
        return Token{TokenType::SEMICOLON, ""};
    case ',':
        return Token{TokenType::COMMA, ""};
    default:
        throw std::logic_error{"no token shorthand for " + std::to_string(c)};
    }
}

void add_tokens(ParseStack &s, std::string code) {
    std::istringstream in{code};
    Tokenizer tok{in};

    ParseStack tokens;

    for (Token t = tok.next_token(); t.type != TokenType::EOI;
         t = tok.next_token()) {
        s.push_back(new expr::TokenExpr{t});
    }
}

void cleanup(ParseStack &s) {
    for (auto e : s) {
        delete e;
    }
}

TEST(ReductionSuccess, attribute) {
    reduce::ToAttribute to_attr;
    ParseStack stack;
    add_tokens(stack, "key=value");

    EXPECT_TRUE(to_attr.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), 1);
    EXPECT_TRUE(expr::Attribute::is_instance(stack.back()));

    add_tokens(stack, ",weight=2.0");

    EXPECT_TRUE(to_attr.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), 2);
    EXPECT_TRUE(expr::Attribute::is_instance(stack.back()));

    cleanup(stack);
}

TEST(ReductionFail, attributeNoValue) {
    reduce::ToAttribute to_attr;
    ParseStack stack;
    add_tokens(stack, "weight=,");

    auto pre_size = stack.size();
    EXPECT_FALSE(to_attr.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), pre_size);

    cleanup(stack);
}

TEST(ReductionSuccess, attributeNoComma) {
    reduce::ToAttribute to_attr;
    ParseStack stack;
    add_tokens(stack, "[weight=1.0");

    EXPECT_TRUE(to_attr.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), 2);
    EXPECT_TRUE(expr::Attribute::is_instance(stack.back()));

    cleanup(stack);
}

TEST(ReductionSuccess, alist) {
    reduce::ToAList to_alist;
    ParseStack stack;
    add_tokens(stack, "[");
    stack.push_back(new expr::Attribute("key", "value"));

    EXPECT_TRUE(to_alist.perform(t(','), stack));
    EXPECT_EQ(stack.size(), 2);
    EXPECT_TRUE(expr::AList::is_instance(stack.back()));

    cleanup(stack);
}

TEST(ReductionFail, alist) {
    reduce::ToAList to_alist;
    ParseStack stack;
    add_tokens(stack, ",zero=0");

    auto pre_size = stack.size();
    EXPECT_FALSE(to_alist.perform(t(','), stack));
    EXPECT_EQ(stack.size(), pre_size);

    cleanup(stack);
}

TEST(ReductionSuccess, statement) {
    reduce::ToStatement to_stmt;
    ParseStack stack;
    add_tokens(stack, "strict graph {\n\ta -- b;");

    EXPECT_TRUE(to_stmt.perform(t('}'), stack));
    EXPECT_EQ(stack.size(), 4);
    EXPECT_TRUE(expr::Statement::is_instance(stack.back()));

    add_tokens(stack, "b -- c;");

    EXPECT_TRUE(to_stmt.perform(t('}'), stack));
    EXPECT_EQ(stack.size(), 5);
    EXPECT_TRUE(expr::Statement::is_instance(stack.back()));

    cleanup(stack);
}

TEST(ReductionFail, statement) {
    reduce::ToStatement to_stmt;
    ParseStack stack;
    add_tokens(stack, "{ a -- b }");

    auto pre_size = stack.size();
    EXPECT_FALSE(to_stmt.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), pre_size);

    cleanup(stack);
}

TEST(ReductionSuccess, stmtListNew) {
    reduce::ToStmtList to_list;
    ParseStack stack;
    add_tokens(stack, "graph foo {");
    stack.push_back(new expr::EdgeStmt{"a", "b"});
    stack.push_back(new expr::EdgeStmt{"c", "d"});

    EXPECT_TRUE(to_list.perform(t('}'), stack));
    EXPECT_EQ(stack.size(), 4);
    EXPECT_TRUE(expr::StmtList::is_instance(stack.back()));

    cleanup(stack);
}

TEST(ReductionSuccess, stmtListExists) {
    reduce::ToStmtList to_list;
    ParseStack stack;
    expr::StmtList *list = new expr::StmtList;
    list->add_statement(new expr::EdgeStmt{"1", "2"});
    stack.push_back(list);
    stack.push_back(new expr::EdgeStmt{"1", "3"});

    EXPECT_TRUE(to_list.perform(Token{TokenType::NUMERAL, "4"}, stack));
    EXPECT_EQ(stack.size(), 1);
    EXPECT_TRUE(expr::StmtList::is_instance(stack.back()));

    // Add another statement
    reduce::ToStatement to_stmt;

    add_tokens(stack, "4 -- 3;");

    EXPECT_TRUE(to_stmt.perform(t('}'), stack));
    EXPECT_EQ(stack.size(), 2);
    EXPECT_TRUE(expr::Statement::is_instance(stack.back()));

    // Fold it into the list

    EXPECT_TRUE(to_list.perform(t('}'), stack));
    EXPECT_EQ(stack.size(), 1);
    EXPECT_TRUE(expr::StmtList::is_instance(stack.back()));

    cleanup(stack);
}

TEST(ReductionFail, stmtList) {
    reduce::ToStmtList to_list;
    ParseStack stack;
    add_tokens(stack, "foo -- bar;");

    auto pre_size = stack.size();
    EXPECT_FALSE(to_list.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), pre_size);

    cleanup(stack);
}

TEST(ReductionSuccess, graph) {
    reduce::ToGraph to_graph;
    ParseStack stack;
    add_tokens(stack, "graph foo {");
    stack.push_back(new expr::StmtList);
    add_tokens(stack, "}");

    EXPECT_TRUE(to_graph.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), 1);
    EXPECT_TRUE(expr::FullGraph::is_instance(stack.back()));

    cleanup(stack);
}

TEST(ReductionFail, graphNoEOI) {
    reduce::ToGraph to_graph;
    ParseStack stack;
    add_tokens(stack, "graph foo {");
    stack.push_back(new expr::StmtList);
    add_tokens(stack, "}");

    auto pre_size = stack.size();
    EXPECT_FALSE(to_graph.perform(t(';'), stack));
    EXPECT_EQ(stack.size(), pre_size);

    cleanup(stack);
}

TEST(ReductionFail, graphNoGraph) {
    reduce::ToGraph to_graph;
    ParseStack stack;
    add_tokens(stack, "graph foo {\nx -- y;");

    auto pre_size = stack.size();
    EXPECT_FALSE(to_graph.perform(t('\0'), stack));
    EXPECT_EQ(stack.size(), pre_size);

    cleanup(stack);
}

TEST(ParseSuccess, fullGraph) {
    std::istringstream in{"strict graph mygraph {\n"
                          "    1 -- 2;\n"
                          "    3 -- 1;\n"
                          "    2 -- 3;\n"
                          "}"};
    auto p = Parser::of(in);
    Expression *ex = p.parse();

    ASSERT_NE(ex, nullptr);

    delete ex;
}

TEST(ParseFail, incomplete) {
    std::istringstream in{"name { a -- b; }"};

    Parser p = Parser::of(in);

    ASSERT_ANY_THROW(p.parse());
}

TEST(ParseFail, illegalToken) {
    {
        std::istringstream in{"digraph mygraph {\n"
                              "    1 -> 2;\n"
                              "    3 -> 1;\n"
                              "    2 -> 3;\n"
                              "}\n\t"};

        Expression *ex = nullptr;
        try {
            auto p = Parser::of(in);
            ex = p.parse();
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::string msg = e.what();
            // Message contains the first illegal token?
            ASSERT_NE(msg.find("digraph"), std::string::npos);
        }
        delete ex;
    }

    {
        std::istringstream in{"graph mygraph {\n"
                              "    1 -> 2;\n"
                              "    3 -> 1;\n"
                              "    2 -> 3;\n"
                              "}\n\t"};
        auto p = Parser::of(in);

        Expression *ex = nullptr;
        try {
            ex = p.parse();
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::string msg = e.what();
            // Message contains the first illegal token?
            ASSERT_NE(msg.find("->"), std::string::npos);
        }
        delete ex;
    }
}
