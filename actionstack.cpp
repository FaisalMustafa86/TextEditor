#include "actionstack.h"

ActionStack::ActionStack() {
    top = nullptr;
}

ActionStack::~ActionStack() {
    clear();
}

void ActionStack::push(const Action& a) {
    ActionNode* n = new ActionNode(a, top);
    top = n;
}

Action ActionStack::pop() {
    if (!top) return Action();
    ActionNode* n = top;
    Action a = n->data;
    top = top->next;
    delete n;
    return a;
}

bool ActionStack::isEmpty() const {
    return top == nullptr;
}

void ActionStack::clear() {
    while (top) {
        ActionNode* n = top;
        top = top->next;
        delete n;
    }
}
