#ifndef STACK_H
#define STACK_H

template<typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };
    Node* top;
    int count;

public:
    Stack() : top(nullptr), count(0) {}
    
    ~Stack() {
        while (!isEmpty()) {
            pop();
        }
    }
    
    void push(const T& item) {
        Node* newNode = new Node(item);
        newNode->next = top;
        top = newNode;
        count++;
    }
    
    T pop() {
        if (isEmpty()) return T();
        Node* temp = top;
        T data = temp->data;
        top = top->next;
        delete temp;
        count--;
        return data;
    }
    
    bool isEmpty() const { 
        return top == nullptr; 
    }
    
    int size() const { 
        return count; 
    }
    
    void clear() {
        while (!isEmpty()) {
            pop();
        }
    }
};

#endif