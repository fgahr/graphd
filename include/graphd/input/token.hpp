#ifndef _GRAPHD_TOKEN_H_
#define _GRAPHD_TOKEN_H_

#include <istream>
#include <string>

namespace graphd::input {
enum class TokenType {
  KEYWORD,                // strict|graph|digraph|...
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
  NUMBER_OF_TOKEN_TYPES,  // Needs to be declared last, no semantic value
  // NOTE: The DOT language knows several more types of tokens
  // that are not yet supported.
};

struct Token {
  TokenType type;
  std::string value;
  /**
   * Whether this token can represent an identifier.
   */
  bool is_identifier();
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
