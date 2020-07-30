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
    TokenMatch(Token tok, slot_list &&into) : t{tok}, ss{std::move(into)} {}
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
            for (size_t i = 0; i < ss.size(); i++) {
                ss.at(i)->put(nullptr);
            }
            walker.shift();
            return true;
        }
        return false;
    }
    virtual ~TokenMatch() = default;

  private:
    Token t;
    std::vector<slot> ss;
};

class IdentifierMatch : public Pattern {
  public:
    IdentifierMatch(slot_list &&into)
        : ss{std::forward<std::vector<slot>>(into)} {}
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
            for (size_t i = 0; i < ss.size(); i++) {
                ss.at(i)->put(e);
            }
            walker.shift();
            return true;
        }
        return false;
    }
    virtual ~IdentifierMatch() = default;

  private:
    std::vector<slot> ss;
};

class OptionalMatch : public Pattern {
  public:
    OptionalMatch(pat pattern, slot_list &&into)
        : p{std::move(pattern)}, ss{std::move(into)} {}
    virtual bool match(StackWalker &walker) override {
        walker.save();
        if (p->match(walker)) {
            walker.forget();
            for (size_t i = 0; i < ss.size(); i++) {
                ss.at(i)->put(nullptr);
            }
        } else {
            walker.restore();
        }

        return true;
    }
    virtual ~OptionalMatch() = default;

  private:
    pat p;
    std::vector<slot> ss;
};

class OneOfTwoMatch : public Pattern {
  public:
    OneOfTwoMatch(pat &&p1, pat &&p2, slot_list &&into)
        : p1{std::move(p1)}, p2{std::move(p2)}, ss{std::move(into)} {}
    virtual bool match(StackWalker &walker) override {
        walker.save();
        if (p1->match(walker)) {
            walker.forget();
            for (size_t i = 0; i < ss.size(); i++) {
                ss.at(i)->put(nullptr);
            }
            return true;
        }

        walker.restore();
        if (p2->match(walker)) {
            for (size_t i = 0; i < ss.size(); i++) {
                ss.at(i)->put(nullptr);
            }
            return true;
        }

        return false;
    }
    virtual ~OneOfTwoMatch() = default;

  private:
    pat p1;
    pat p2;
    std::vector<slot> ss;
};

class SequenceMatch : public Pattern {
  public:
    SequenceMatch(std::vector<pat> &&pats, slot_list &&into)
        : patterns{std::move(pats)}, ss{std::move(into)} {}
    virtual bool match(StackWalker &walker) override {
        walker.save();

        for (auto it = patterns.rbegin(); it != patterns.rend(); it++) {
            pat &p = *it;
            if (!p->match(walker)) {
                walker.restore();
                return false;
            }
        }

        walker.forget();
        for (size_t i = 0; i < ss.size(); i++) {
            ss.at(i)->put(nullptr);
        }

        return true;
    }
    virtual ~SequenceMatch() = default;

  private:
    std::vector<pat> patterns;
    std::vector<slot> ss;
};

class BoolSlot : public Slot {
  public:
    BoolSlot(bool &into) : into{into} {}
    virtual void put(Expression *) override {
        into = true;
    }
    virtual ~BoolSlot() = default;

  private:
    bool &into;
};

class ValueSlot : public Slot {
  public:
    ValueSlot(std::string &into) : into{into} {}
    virtual void put(Expression *e) override {
        if (!expr::TokenExpr::is_instance(e)) {
            throw std::logic_error{
                "attempting to gather value from non-token expression"};
        }

        Token tok = static_cast<expr::TokenExpr *>(e)->token;
        into = tok.value;
    }
    virtual ~ValueSlot() = default;

  private:
    std::string &into;
};

class ListSlot : public Slot {
  public:
    ListSlot(std::vector<Expression *> &into) : into{into} {}
    virtual void put(Expression *e) override {
        into.push_back(e);
    }
    virtual ~ListSlot() = default;

  private:
    std::vector<Expression *> &into;
};

slot flag(bool &b) {
    return std::make_unique<BoolSlot>(b);
}

slot value(std::string &value) {
    return std::make_unique<ValueSlot>(value);
}

slot add_to(std::vector<Expression *> &into) {
    return std::make_unique<ListSlot>(into);
}

pat exact(char token, slot_list into) {
    return std::make_unique<TokenMatch>(Token::from(token), std::move(into));
}

pat exact(std::string token, slot_list into) {
    return std::make_unique<TokenMatch>(Token::from(token), std::move(into));
}

pat optional(char token, slot_list into) {
    return std::make_unique<OptionalMatch>(exact(token), std::move(into));
}

pat optional(std::string token, slot_list into) {
    return std::make_unique<OptionalMatch>(exact(token), std::move(into));
}

pat optional(pat &&p, slot_list into) {
    return std::make_unique<OptionalMatch>(std::move(p), std::move(into));
}

pat identifier(slot_list into) {
    return std::make_unique<IdentifierMatch>(std::move(into));
}

pat one_of(pat &&p1, pat &&p2, slot_list into) {
    return std::make_unique<OneOfTwoMatch>(std::move(p1), std::move(p2),
                                           std::move(into));
}

pat sequence(std::vector<pat> patterns, slot_list into) {
    return std::make_unique<SequenceMatch>(std::move(patterns),
                                           std::move(into));
}

} // namespace graphd::input::pattern
