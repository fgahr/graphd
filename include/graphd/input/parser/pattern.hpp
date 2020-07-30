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

using slot_list = std::initializer_list<Slot *>;

Slot *flag(bool &b);
Slot *value(std::string &value);
Slot *add_to(std::vector<Expression *> &into);

Pattern *exact(char token, slot_list into = {});
Pattern *exact(std::string token, slot_list into = {});
Pattern *optional(char token, slot_list into = {});
Pattern *optional(std::string token, slot_list into = {});
Pattern *optional(Pattern *p, slot_list into = {});

Pattern *has_type(ExprType type, slot_list into = {});
Pattern *identifier(slot_list into = {});

Pattern *one_of(Pattern *p1, Pattern *p2, slot_list into = {});
Pattern *repeated(Pattern *pattern, slot_list into = {});
Pattern *sequence(std::initializer_list<Pattern *> patterns,
                  slot_list into = {});

} // namespace graphd::input::pattern

#endif // _GRAPHD_PATTERN_H_
