#include "linkedList.h"
#include <iostream>

CharNode::CharNode(char d) {
	data = d;
	prev = next = nullptr;
}

LinkedList::LinkedList() {
	head = tail = nullptr;
}

LinkedList::~LinkedList() {
	clear();
}

void LinkedList::insert(char c) {
	CharNode* node = new CharNode(c);
	if (!head) {
		head = tail = node;
	} else {
		tail->next = node;
		node->prev = tail;
		tail = node;
	}
}

void LinkedList::removeLast() {
	if (!tail) return;

	CharNode* temp = tail;
	tail = tail->prev;

	if (tail) tail->next = nullptr;
	else head = nullptr;

	delete temp;
}

void LinkedList::clear() {
	while (tail)
		removeLast();
}

void LinkedList::display() {
	CharNode* cur = head;
	while (cur) {
		std::cout << cur->data;
		cur = cur->next;
	}
}

char* LinkedList::toCharArray() {
	int size = 0;
	CharNode* cur = head;
	while (cur) {
		size++;
		cur = cur->next;
	}

	char* arr = new char[size + 1];
	cur = head;
	for (int i = 0; i < size; i++) {
		arr[i] = cur->data;
		cur = cur->next;
	}
	arr[size] = '\0';
	return arr;
}
