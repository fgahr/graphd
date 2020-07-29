#include <graphd/input/parser/pattern.hpp>

#include <utility>

namespace graphd::input::pattern {
StackWalker::StackWalker(ParseStack &s) : stack{s} {
    idx = s.size() - 1;
}
bool StackWalker::exhausted() {
    return idx < 0;
}
void StackWalker::shift() {
    idx--;
}
void StackWalker::save() {
    saved.push_back(idx);
}
void StackWalker::restore() {
    if (saved.empty()) {
        throw std::logic_error{"attempting to restore stack position without "
                               "preceding call to save()"};
    }
    idx = saved.back();
    saved.pop_back();
}
void StackWalker::forget() {
    if (saved.empty()) {
        throw std::logic_error{"attempting to forget stack position without "
                               "preceding call to save()"};
    }
    saved.pop_back();
}
Expression *StackWalker::get() {
    return stack.at(idx);
}

class TokenMatch : public Pattern {
  public:
    TokenMatch(Token tok, slot_list into = {}) : t{tok}, ss{into} {}
    virtual bool match(StackWalker &walker) override {
        if (walker.exhausted()) {
            return false;
        }

        Expression *e = walker.get();

        if (!expr::TokenExpr::is_instance(e)) {
            return false;
        }

        Token tok = static_cast<expr::TokenExpr *>(e)->token;

        if (tok.type == t.type && tok.value == t.value) {
            for (auto &s : ss) {
                s->put(e);
            }
            walker.shift();
            return true;
        }
        return false;
    }

  private:
    Token t;
    std::vector<slot> ss;
};

class IdentifierMatch : public Pattern {
  public:
    IdentifierMatch(slot_list into = {}) : ss{into} {}
    virtual bool match(StackWalker &walker) override {
        if (walker.exhausted()) {
            return false;
        }

        Expression *e = walker.get();

        if (!expr::TokenExpr::is_instance(e)) {
            return false;
        }

        Token tok = static_cast<expr::TokenExpr *>(e)->token;

        if (tok.is_identifier()) {
            for (auto &s : ss) {
                s->put(e);
            }
            walker.shift();
            return true;
        }
        return false;
    }

  private:
    std::vector<slot> ss;
};

class OptionalMatch : public Pattern {
  public:
    OptionalMatch(pat &&pattern, slot_list into) : p{std::move(pattern)} {}
    virtual bool match(StackWalker &walker) override {
        walker.save();
        if (p->match(walker)) {
            walker.forget();
            for (auto &s : ss) {
                s->put(nullptr);
            }
        } else {
            walker.restore();
        }

        return true;
    }

  private:
    pat p;
    std::vector<slot> ss;
};

class OneOfTwoMatch : public Pattern {
  public:
    OneOfTwoMatch(pat &&p1, pat &&p2, slot_list into)
        : p1{std::move(p1)}, p2{std::move(p2)}, ss{into} {}
    virtual bool match(StackWalker &walker) override {
        walker.save();
        if (p1->match(walker)) {
            walker.forget();
            for (auto &s : ss) {
                s->put(nullptr);
            }
            return true;
        }

        walker.restore();
        if (p2->match(walker)) {
            for (auto &s : ss) {
                s->put(nullptr);
            }
            return true;
        }

        return false;
    }

  private:
    pat p1;
    pat p2;
    std::vector<slot> ss;
};

// TODO: Define Slot variants, slot-building functions.

pat exact(char token, slot_list into) {
    return std::make_unique<TokenMatch>(Token::from(token), into);
}

pat exact(std::string token, slot_list into) {
    return std::make_unique<TokenMatch>(Token::from(token), into);
}

pat optional(char token, slot_list into) {
    return std::make_unique<OptionalMatch>(exact(token), into);
}

pat optional(std::string token, slot_list into) {
    return std::make_unique<OptionalMatch>(exact(token), into);
}

pat identifier(slot_list into) {
    return std::make_unique<IdentifierMatch>(into);
}

pat one_of(pat &&p1, pat &&p2, slot_list into) {
    return std::make_unique<OneOfTwoMatch>(p1, p2, into);
}

} // namespace graphd::input::pattern
