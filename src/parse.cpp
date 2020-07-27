#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>
#include <graphd/input/parser/reduce.hpp>

#include <utility>

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

  Expression *ret = stack.front();
  stack.clear();
  return ret;
}

bool Parser::shift() {
  if (lookahead.type == TokenType::EOI) {
    return false;
  }

  stack.push_back(new expr::TokenExpr{lookahead});
  lookahead = tok.next_token();
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
  auto reductions = std::vector<Reduction *>{
      new reduce::ToStatement, new reduce::ToStmtList, new reduce::ToGraph};
  return Parser{tok, next, reductions};
}

Parser::Parser(Tokenizer tokenizer, Token first_token,
               std::vector<Reduction *> reductions)
    : stack{}, lookahead{first_token}, tok{tokenizer}, reductions{reductions} {}

Parser::~Parser() {
  for (auto red : reductions) {
    delete red;
  }

  for (auto ex : stack) {
    delete ex;
  }
}

} // namespace graphd::input
