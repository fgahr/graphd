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
  Token token = tok.next_token();
  if (token.type == TokenType::EOI) {
    return false;
  }
  stack.push_back(new expr::TokenExpr(token));
  return true;
}

bool Parser::reduce() {
  bool performed = false;
  for (auto red : reductions) {
    if (red->perform(stack)) {
      performed = true;
    }
  }
  return performed;
}

Parser Parser::of(std::istream &in) {
  Tokenizer tok{in};
  return Parser{tok};
}

Parser::Parser(Tokenizer tokenizer) : tok{tokenizer}, stack{} {
  this->reductions =
      std::vector<Reduction *>{new reduce::ToGraph, new reduce::ToStmtList};
}

Parser::~Parser() {
  for (auto red : reductions) {
    delete red;
  }
}

} // namespace graphd::input
