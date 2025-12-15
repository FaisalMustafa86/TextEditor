#include "stack.h"

Stack::Stack() {
	top = nullptr;
}

Stack::~Stack() {
	while (!isEmpty())
		pop();
}

void Stack::push(char c) {
	StackNode* node = new StackNode(c, top);
	top = node;
}

char Stack::pop() {
	if (isEmpty()) return '\0';

	StackNode* temp = top;
	char value = temp->data;
	top = temp->next;
	delete temp;
	return value;
}

bool Stack::isEmpty() {
	return top == nullptr;
}
