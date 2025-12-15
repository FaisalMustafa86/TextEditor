#ifndef ACTIONSTACK_H
#define ACTIONSTACK_H

// Custom action stack used for Undo/Redo (no STL for stack nodes)
#include <string>

class Action {
public:
    enum Type { INS = 0, DEL = 1, REPL = 2 };
    Type type;
    int pos;
    std::string text; // For INS: inserted text; DEL: deleted text; REPL: old text
    std::string text2; // For REPL: new text

    Action() : type(INS), pos(0), text(), text2() {}
    // INS or DEL
    Action(Type t, int p, const std::string& s) : type(t), pos(p), text(s), text2() {}
    // REPL
    Action(int p, const std::string& oldt, const std::string& newt) : type(REPL), pos(p), text(oldt), text2(newt) {}
};

struct ActionNode {
    Action data;
    ActionNode* next;
    ActionNode(const Action& a, ActionNode* n = nullptr) : data(a), next(n) {}
};

class ActionStack {
private:
    ActionNode* top;
public:
    ActionStack();
    ~ActionStack();

    void push(const Action& a);
    Action pop();
    bool isEmpty() const;
    void clear();
};

#endif
