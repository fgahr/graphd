#include <gtest/gtest.h>

#include <graphd/input/parser/reduce.hpp>

#include <sstream>
#include <string>

using namespace graphd::input;

const Token eoi = Token{TokenType::EOI, ""};
const Token clb = Token{TokenType::CLOSING_BRACE, ""};
const Token smc = Token{TokenType::SEMICOLON, ""};

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

TEST(ReductionSuccess, statement) {
  reduce::ToStatement toStmt;
  ParseStack stack;
  add_tokens(stack, "strict graph {\n\ta -- b;");

  EXPECT_TRUE(toStmt.perform(clb, stack));
  EXPECT_EQ(stack.size(), 4);
  EXPECT_TRUE(expr::Statement::is_instance(stack.back()));

  add_tokens(stack, "b -- c;");

  EXPECT_TRUE(toStmt.perform(clb, stack));
  EXPECT_EQ(stack.size(), 5);
  EXPECT_TRUE(expr::Statement::is_instance(stack.back()));

  cleanup(stack);
}

TEST(ReductionFail, statement) {
  reduce::ToStatement toStmt;
  ParseStack stack;
  add_tokens(stack, "{ a -- b }");

  auto pre_size = stack.size();
  EXPECT_FALSE(toStmt.perform(eoi, stack));
  EXPECT_EQ(stack.size(), pre_size);

  cleanup(stack);
}

TEST(ReductionSuccess, stmtListNew) {
  reduce::ToStmtList toList;
  ParseStack stack;
  add_tokens(stack, "graph foo {");
  stack.push_back(new expr::EdgeStmt{"a", "b"});
  stack.push_back(new expr::EdgeStmt{"c", "d"});

  EXPECT_TRUE(toList.perform(clb, stack));
  EXPECT_EQ(stack.size(), 4);
  EXPECT_TRUE(expr::StmtList::is_instance(stack.back()));

  cleanup(stack);
}

TEST(ReductionSuccess, stmtListExists) {
  reduce::ToStmtList toList;
  ParseStack stack;
  expr::StmtList *list = new expr::StmtList;
  list->add_statement(new expr::EdgeStmt{"1", "2"});
  stack.push_back(list);
  stack.push_back(new expr::EdgeStmt{"1", "3"});

  EXPECT_TRUE(toList.perform(Token{TokenType::NUMERAL, "4"}, stack));
  EXPECT_EQ(stack.size(), 1);
  EXPECT_TRUE(expr::StmtList::is_instance(stack.back()));

  // Add another statement
  reduce::ToStatement toStmt;

  add_tokens(stack, "4 -- 3;");

  EXPECT_TRUE(toStmt.perform(clb, stack));
  EXPECT_EQ(stack.size(), 2);
  EXPECT_TRUE(expr::Statement::is_instance(stack.back()));

  // Fold it into the list

  EXPECT_TRUE(toList.perform(clb, stack));
  EXPECT_EQ(stack.size(), 1);
  EXPECT_TRUE(expr::StmtList::is_instance(stack.back()));

  cleanup(stack);
}

TEST(ReductionFail, stmtList) {
  reduce::ToStmtList toList;
  ParseStack stack;
  add_tokens(stack, "foo -- bar;");

  auto pre_size = stack.size();
  EXPECT_FALSE(toList.perform(eoi, stack));
  EXPECT_EQ(stack.size(), pre_size);

  cleanup(stack);
}

TEST(ReductionSuccess, graph) {
  reduce::ToGraph toGraph;
  ParseStack stack;
  add_tokens(stack, "strict graph foo {");
  stack.push_back(new expr::StmtList);
  add_tokens(stack, "}");

  EXPECT_TRUE(toGraph.perform(eoi, stack));
  EXPECT_EQ(stack.size(), 1);
  EXPECT_TRUE(expr::FullGraph::is_instance(stack.back()));

  cleanup(stack);
}

TEST(ReductionFail, graphNoEOI) {
  reduce::ToGraph toGraph;
  ParseStack stack;
  add_tokens(stack, "strict graph foo {");
  stack.push_back(new expr::StmtList);
  add_tokens(stack, "}");

  auto pre_size = stack.size();
  EXPECT_FALSE(toGraph.perform(smc, stack));
  EXPECT_EQ(stack.size(), pre_size);

  cleanup(stack);
}

TEST(ReductionFail, graphNoGraph) {
  reduce::ToGraph toGraph;
  ParseStack stack;
  add_tokens(stack, "strict graph foo {\nx -- y;");

  auto pre_size = stack.size();
  EXPECT_FALSE(toGraph.perform(eoi, stack));
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

TEST(ParseFail, illegalToken) {
  {
    std::istringstream in{"strict digraph mygraph {\n"
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
      ASSERT_NE(msg.find("digraph"), std::string::npos);
    }
    delete ex;
  }

  {
    std::istringstream in{"strict graph mygraph {\n"
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
