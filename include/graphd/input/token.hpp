#ifndef _GRAPHD_TOKEN_H_
#define _GRAPHD_TOKEN_H_

#include <istream>
#include <string>

namespace graphd::input {
enum class TokenType {
  KEYWORD,                // strict|graph|digraph
  NAME,                   // [_a-zA-Z][_0-9a-zA-Z]*
  NUMERAL,                // [-]?(.[0-9]+|[0-9]+(.[0-9]*)?)
  SEMICOLON,              // ;
  COMMA,                  // ,
  UNDIRECTED_EDGE,        // --
  DIRECTED_EDGE,          // ->
  OPENING_BRACE,          // {
  CLOSING_BRACE,          // }
  OPENING_SQUARE_BRACKET, // [
  CLOSING_SQUARE_BRACKET, // ]
  EQUAL_SIGN,             // =
  EOI,                    // End-of-input marker
  // NOTE: The DOT language knows several more types of tokens
  // that are as of now unsupported.
};

struct Token {
  TokenType type;
  std::string value;
};

class Tokenizer {
public:
  /**
   * @param in Input source from which to fetch tokens.
   */
  Tokenizer(std::istream &in);
  /**
   * The next token from the input stream.
   */
  Token next_token();

private:
  std::string read_string();
  std::string read_name();
  std::string read_numeral();
  std::istream &in;
};
} // namespace graphd::input

#endif // _GRAPHD_TOKEN_H_
