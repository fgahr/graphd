#include <graphd/input/token.hpp>

#include <cctype>
#include <stdexcept>

namespace graphd::input {
Tokenizer::Tokenizer(std::istream &in) : in{in} {}

std::string Tokenizer::read_string() {
  // TODO
  return "";
}

std::string Tokenizer::read_name() {
  // TODO
  return "";
}

std::string Tokenizer::read_numeral() {
  // TODO
  return "";
}

std::optional<Token> Tokenizer::next_token() {
  // NOTE: Error handling not bulletproof. In particular, badbit needs to be
  // checked after putback.
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
    case '=':
      return Token{TokenType::EQUAL_SIGN, ""};
    case '"':
      return Token{TokenType::NAME, read_string()};
    case '-':
      c = in.get();
      if (c == EOF) {
        throw std::runtime_error{"unexpected end of input"};
      } else if (c == '-') {
        return Token{TokenType::UNDIRECTED_EDGE, ""};
      } else if (c == '>') {
        return Token{TokenType::DIRECTED_EDGE, ""};
      } else {
        in.putback(c);
        return Token{TokenType::NUMERAL, "-" + read_numeral()};
      }
    default:
      if (std::isalpha(c) || c == '_') {
        in.putback(c);
        return Token{TokenType::NAME, read_name()};
      } else if ('0' <= c && c <= '9') {
        in.putback(c);
        return Token{TokenType::NUMERAL, read_numeral()};
      } else {
        throw std::runtime_error{"unexpected input byte: " +
                                 std::to_string((char)c)};
      }
    }
  }

  return std::nullopt;
}
} // namespace graphd::input
