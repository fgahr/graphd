#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>
#include <graphd/input/parser/reduce.hpp>

namespace graphd::input {

Expression *Parser::parse() {
  while (shift()) {
    while (reduce()) {
      // keep reducing
    }
  }

  if (stack.size() != 1) {
    throw std::runtime_error{
        "input must contain exactly one full graph definition"};
  }

  return stack.at(0);
}

bool Parser::shift() {
  if (lookahead.type == TokenType::EOI) {
    return false;
  }

  stack.push_back(new expr::TokenExpr(lookahead));
  Token lookahead = tok.next_token();
  return true;
}

bool Parser::reduce() {
  bool performed = false;
  for (auto red : reductions) {
    if (red->perform(lookahead, stack)) {
      performed = true;
    }
  }
  return performed;
}

Parser Parser::of(std::istream &in) {
  Tokenizer tok{in};
  Token next = tok.next_token();
  return Parser{tok, next};
}

Parser::Parser(Tokenizer tokenizer, Token lookahead)
    : tok{tokenizer}, lookahead{lookahead}, stack{} {
  this->reductions =
      std::vector<Reduction *>{new reduce::ToGraph, new reduce::ToStmtList};
}

Parser::~Parser() {
  for (auto red : reductions) {
    delete red;
  }
}

} // namespace graphd::input
