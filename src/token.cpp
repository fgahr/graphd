#include <graphd/input/token.hpp>

namespace graphd::input {
Tokenizer::Tokenizer(std::istream &in) : in{in} {}

std::optional<Token> Tokenizer::next_token() {
  // TODO
  return std::nullopt;
}
} // namespace graphd::input
