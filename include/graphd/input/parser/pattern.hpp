#ifndef _GRAPHD_PATTERN_H_
#define _GRAPHD_PATTERN_H_

#include <graphd/input/parse.hpp>
#include <graphd/input/parser/expr.hpp>

#include <initializer_list>
#include <memory>

namespace graphd::input::pattern {
/**
 * StackWalker traverses a stack top to bottom.
 */
class StackWalker {
  public:
    StackWalker(ParseStack &s);
    /**
     * Whether the bottom of the stack has been reached.
     */
    bool exhausted();
    /**
     * Move down the stack by one position.
     */
    void shift();
    /**
     * Save the current stack position, i.e. push it to the save stack.
     */
    void save();
    /**
     * Restore the most recently saved stack position, i.e. pop it from the save
     * stack.
     */
    void restore();
    /**
     * Discard the most recently saved position from the save stack.
     */
    void forget();
    /**
     * The expression at the current stack position.
     */
    Expression *get();

  private:
    const ParseStack &stack;
    std::vector<int> saved;
    int idx;
};

class Slot {
  public:
    /**
     * Process an expression that has been matched.
     * Some Slot types may accept nullptr as argument.
     */
    virtual void put(Expression *e) = 0;
    virtual ~Slot() = default;
};

class Pattern {
  public:
    virtual bool match(StackWalker &walker) = 0;
    virtual ~Pattern() = default;
};

using pat = std::unique_ptr<Pattern>;
using slot = std::unique_ptr<Slot>;
using slot_list = std::vector<slot>;

slot flag(bool &b);
slot value(std::string &value);
slot add_to(std::vector<Expression *> &into);

pat exact(char token, slot_list into = {});
pat exact(std::string token, slot_list into = {});
pat optional(char token, slot_list into = {});
pat optional(std::string token, slot_list into = {});
pat optional(pat &&p, slot_list into = {});
pat identifier(slot_list into = {});

pat one_of(pat &&p1, pat &&p2, slot_list into = {});
pat sequence(std::initializer_list<pat &&> patterns, slot_list into = {});

} // namespace graphd::input::pattern

#endif // _GRAPHD_PATTERN_H_
