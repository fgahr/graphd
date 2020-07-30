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
    TokenMatch(Token token, std::vector<Slot *> slots)
        : expected{token}, slots{slots} {}
    virtual bool match(StackWalker &walker) override {
        if (walker.exhausted()) {
            return false;
        }

        Expression *e = walker.get();
        if (!expr::TokenExpr::is_instance(e)) {
            return false;
        }

        Token tok = static_cast<expr::TokenExpr *>(e)->token;
        if (tok.type == expected.type && tok.value == expected.value) {
            for (size_t i = 0; i < slots.size(); i++) {
                slots.at(i)->put(nullptr);
            }
            walker.shift();
            return true;
        }
        return false;
    }
    virtual ~TokenMatch() {
        for (auto s : slots) {
            delete s;
        }
    }

  private:
    Token expected;
    std::vector<Slot *> slots;
};

class IdentifierMatch : public Pattern {
  public:
    IdentifierMatch(std::vector<Slot *> __slots) : slots{} {
        for (auto s : __slots) {
            slots.push_back(std::unique_ptr<Slot>{s});
        }
    }
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
            for (size_t i = 0; i < slots.size(); i++) {
                slots.at(i)->put(e);
            }
            walker.shift();
            return true;
        }
        return false;
    }
    virtual ~IdentifierMatch() = default;

  private:
    std::vector<std::unique_ptr<Slot>> slots;
};

class OptionalMatch : public Pattern {
  public:
    OptionalMatch(Pattern *pattern, std::vector<Slot *> slots)
        : pattern{pattern}, slots{slots} {}
    virtual bool match(StackWalker &walker) override {
        walker.save();
        if (pattern->match(walker)) {
            walker.forget();
            for (size_t i = 0; i < slots.size(); i++) {
                slots.at(i)->put(nullptr);
            }
        } else {
            walker.restore();
        }

        return true;
    }
    virtual ~OptionalMatch() {
        for (auto s : slots) {
            delete s;
        }
    }

  private:
    std::unique_ptr<Pattern> pattern;
    std::vector<Slot *> slots;
};

class OneOfTwoMatch : public Pattern {
  public:
    OneOfTwoMatch(Pattern *p1, Pattern *p2, std::vector<Slot *> slots)
        : p1{p1}, p2{p2}, slots{slots} {}
    virtual bool match(StackWalker &walker) override {
        walker.save();
        if (p1->match(walker)) {
            walker.forget();
            for (size_t i = 0; i < slots.size(); i++) {
                slots.at(i)->put(nullptr);
            }
            return true;
        }

        walker.restore();
        if (p2->match(walker)) {
            for (size_t i = 0; i < slots.size(); i++) {
                slots.at(i)->put(nullptr);
            }
            return true;
        }

        return false;
    }
    virtual ~OneOfTwoMatch() {
        for (auto s : slots) {
            delete s;
        }
    }

  private:
    std::unique_ptr<Pattern> p1;
    std::unique_ptr<Pattern> p2;
    std::vector<Slot *> slots;
};

class RepeatedMatch : public Pattern {
  public:
    RepeatedMatch(Pattern *pattern, std::vector<Slot *> slots)
        : pattern{pattern}, slots{slots} {}
    virtual bool match(StackWalker &walker) override {
        bool matched = false;

        while (pattern->match(walker)) {
            matched = true;
        }

        if (matched) {
            for (auto s : slots) {
                s->put(nullptr);
            }

            for (auto ex : deletable) {
                delete ex;
            }

            return true;
        }
        return false;
    }

  private:
    std::unique_ptr<Pattern> pattern;
    std::vector<Expression *> deletable;
    std::vector<Slot *> slots;
};

class TypeMatch : public Pattern {
  public:
    TypeMatch(ExprType type, std::vector<Slot *> slots)
        : type{type}, slots{slots} {}
    virtual bool match(StackWalker &walker) override {
        if (walker.exhausted()) {
            return false;
        }

        Expression *e = walker.get();
        if (e->type() == type) {
            for (auto s : slots) {
                s->put(e);
            }
            walker.shift();
            return true;
        }
        return false;
    }

  private:
    ExprType type;
    std::vector<Slot *> slots;
};

class SequenceMatch : public Pattern {
  public:
    SequenceMatch(std::vector<Pattern *> patterns, std::vector<Slot *> slots)
        : patterns{patterns}, slots{slots} {}
    virtual bool match(StackWalker &walker) override {
        if (walker.exhausted()) {
            return false;
        }
        walker.save();

        for (auto it = patterns.rbegin(); it != patterns.rend(); it++) {
            auto &p = *it;
            if (!p->match(walker)) {
                walker.restore();
                return false;
            }
        }

        walker.forget();
        for (size_t i = 0; i < slots.size(); i++) {
            slots.at(i)->put(nullptr);
        }

        return true;
    }
    virtual ~SequenceMatch() {
        for (auto p : patterns) {
            delete p;
        }

        for (auto s : slots) {
            delete s;
        }
    }

  private:
    std::vector<Pattern *> patterns;
    std::vector<Slot *> slots;
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

Slot *flag(bool &b) {
    return new BoolSlot{b};
}

Slot *value(std::string &value) {
    return new ValueSlot{value};
}

Slot *add_to(std::vector<Expression *> &into) {
    return new ListSlot{into};
}

Pattern *exact(char token, slot_list into) {
    return new TokenMatch(Token::from(token), into);
}

Pattern *exact(std::string token, slot_list into) {
    return new TokenMatch(Token::from(token), into);
}

Pattern *optional(char token, slot_list into) {
    return new OptionalMatch{exact(token), into};
}

Pattern *optional(std::string token, slot_list into) {
    return new OptionalMatch{exact(token), into};
}

Pattern *optional(Pattern *p, slot_list into) {
    return new OptionalMatch{p, into};
}

Pattern *has_type(ExprType type, slot_list into) {
    return new TypeMatch{type, into};
}

Pattern *identifier(slot_list into) {
    return new IdentifierMatch{into};
}

Pattern *one_of(Pattern *p1, Pattern *p2, slot_list into) {
    return new OneOfTwoMatch{p1, p2, into};
}

Pattern *repeat(Pattern *p, slot_list into) {
    return new RepeatedMatch{p, into};
}

Pattern *sequence(std::initializer_list<Pattern *> patterns, slot_list into) {
    return new SequenceMatch(patterns, into);
}

} // namespace graphd::input::pattern
