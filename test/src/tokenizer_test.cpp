#include <graphd/input/token.hpp>

#include <gtest/gtest.h>

#include <sstream>
#include <vector>

using namespace graphd::input;

TEST(TokenizerGeneral, eof) {
  std::istringstream in{""};
  Tokenizer tok{in};

  auto token = tok.next_token();

  ASSERT_EQ(token.type, TokenType::EOI);
}

TEST(TokenizerSingleToken, keyword) {
  std::istringstream in{"Strict"};
  Tokenizer tok{in};

  auto token = tok.next_token();

  EXPECT_EQ(token.type, TokenType::KEYWORD);
  EXPECT_EQ(token.value, "strict");
}

TEST(TokenizerSingleToken, numeral) {
  std::istringstream in{".23"};
  Tokenizer tok{in};

  auto token = tok.next_token();

  EXPECT_EQ(token.type, TokenType::NUMERAL);
  EXPECT_EQ(token.value, ".23");
}

TEST(TokenizerSingleToken, name) {
  std::istringstream in{"abc"};
  Tokenizer tok{in};

  auto token = tok.next_token();

  EXPECT_EQ(token.type, TokenType::NAME);
  EXPECT_EQ(token.value, "abc");
}

TEST(TokenizerSingleToken, string) {
  std::istringstream in{"\"_foo_bar_\""};
  Tokenizer tok{in};

  auto token = tok.next_token();

  EXPECT_EQ(token.type, TokenType::NAME);
  EXPECT_EQ(token.value, "_foo_bar_");
}

TEST(TokenizerSingleTokenFail, numeral) {
  std::istringstream in{"1.23."};
  Tokenizer tok{in};

  ASSERT_ANY_THROW(tok.next_token());
}

TEST(TokenizerSingleTokenFail, string) {
  std::istringstream in{"\"unterminated"};
  Tokenizer tok{in};

  ASSERT_ANY_THROW(tok.next_token());
}

TEST(TokenizerMultipleTokens, simpleGraph) {
  std::stringstream in;
  in << "GRAPH gname {\n"
     << "    a -- b [weight=2.3];\n"
     << "}";
  Tokenizer tok{in};

  // clang-format off
  std::vector<Token> expected{
      {TokenType::KEYWORD, "graph"},
      {TokenType::NAME, "gname"},
      {TokenType::OPENING_BRACE, ""},
      {TokenType::NAME, "a"},
      {TokenType::UNDIRECTED_EDGE, ""},
      {TokenType::NAME, "b"},
      {TokenType::SEMICOLON, ""},
      {TokenType::OPENING_SQUARE_BRACKET, ""},
      {TokenType::NAME, "weight"},
      {TokenType::EQUAL_SIGN, ""},
      {TokenType::NUMERAL, "2.3"},
      {TokenType::CLOSING_SQUARE_BRACKET, ""},
      {TokenType::CLOSING_BRACE, ""},
      {TokenType::EOI, ""},
  };
  // clang-format on

  for (auto ex : expected) {
    Token token = tok.next_token();
    EXPECT_EQ(token.type, ex.type);
    EXPECT_EQ(token.value, ex.value);
  }
}
