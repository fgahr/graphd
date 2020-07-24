#include <gtest/gtest.h>

#include <graphd/input/token.hpp>

#include <sstream>

using namespace graphd::input;

TEST(TokenizerSingleToken, numeral) {
  std::istringstream in{".23"};
  Tokenizer tok{in};

  auto token = tok.next_token();

  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token.value().type, TokenType::NUMERAL);
  EXPECT_EQ(token.value().value, ".23");
}
