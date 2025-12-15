#ifndef STACK_H
#define STACK_H

struct StackNode {
    char data;
    StackNode* next;
    StackNode(char d, StackNode* n=nullptr) : data(d), next(n) {}
};

class Stack {
private:
    StackNode* top;

public:
    Stack();
    ~Stack();

    void push(char);
    char pop();
    bool isEmpty();
};

#endif
