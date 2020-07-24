#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>
#include <graphd/input/parser/reduce.hpp>

namespace graphd::input {

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
