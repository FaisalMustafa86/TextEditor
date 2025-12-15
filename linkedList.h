#ifndef LINKEDLIST_H
#define LINKEDLIST_H

struct CharNode {
    char data;
    CharNode* prev;
    CharNode* next;

    CharNode(char d);
};

class LinkedList {
private:
    CharNode* head;
    CharNode* tail;

public:
    LinkedList();
    ~LinkedList();

    void insert(char c);
    void removeLast();
    void clear();
    void display();
    char* toCharArray();
};

#endif
