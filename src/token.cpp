#include <graphd/input/token.hpp>

#include <cctype>
#include <stdexcept>

namespace graphd::input {
Tokenizer::Tokenizer(std::istream &in) : in{in} {}

static Token read_string(std::istream &in) {
  // TODO
  return Token{};
}

// Read a token starting with an
static Token read_edge_or_negative_numeral(std::istream &in) {
  // TODO
  return Token{};
}

static Token read_name(char initial, std::istream &in) {
  // TODO
  return Token{};
}

static Token read_numeral(char initial, std::istream &in) {
  // TODO
  return Token{};
}

std::optional<Token> Tokenizer::next_token() {
  while (!in.eof()) {
    int c = in.get();
    switch (c) {
    case EOF:
      return std::nullopt;
    case ' ':
    case '\t':
    case '\n':
      continue;
    case ';':
      return Token{TokenType::SEMICOLON, ""};
    case ',':
      return Token{TokenType::COMMA, ""};
    case '{':
      return Token{TokenType::OPENING_BRACE, ""};
    case '}':
      return Token{TokenType::CLOSING_BRACE, ""};
    case '[':
      return Token{TokenType::OPENING_SQUARE_BRACKET, ""};
    case ']':
      return Token{TokenType::CLOSING_SQUARE_BRACKET, ""};
    case '"':
      return read_string(this->in);
    case '-':
      return read_edge_or_negative_numeral(this->in);
    default:
      if (std::isalpha(c) || c == '_') {
        return read_name(c, this->in);
      } else if ('0' <= c && c <= '9') {
        return read_numeral(c, this->in);
      } else {
        throw std::runtime_error{"unexpected input byte: " +
                                 std::to_string((char)c)};
      }
    }
  }

  return std::nullopt;
}
} // namespace graphd::input
