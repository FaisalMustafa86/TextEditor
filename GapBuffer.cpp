#include "GapBuffer.h"
#include <cstring>

GapBuffer::GapBuffer(int initialCapacity) {
    capacity = initialCapacity;
    buffer = new char[capacity];
    gapStart = 0;
    gapEnd = capacity;
}

GapBuffer::~GapBuffer() {
    delete[] buffer;
}

void GapBuffer::resize(int newCapacity) {
    char* newBuffer = new char[newCapacity];
    int gapSize = gapEnd - gapStart;
    
    // Copy pre-gap content
    for (int i = 0; i < gapStart; i++) {
        newBuffer[i] = buffer[i];
    }
    
    // New gap position
    int newGapEnd = newCapacity - (capacity - gapEnd);
    
    // Copy post-gap content
    for (int i = gapEnd; i < capacity; i++) {
        newBuffer[newGapEnd + (i - gapEnd)] = buffer[i];
    }
    
    delete[] buffer;
    buffer = newBuffer;
    capacity = newCapacity;
    gapEnd = newGapEnd;
}

void GapBuffer::moveCursorTo(int pos) {
    if (pos < gapStart) {
        // Move gap left
        int distance = gapStart - pos;
        gapEnd -= distance;
        gapStart = pos;
        for (int i = 0; i < distance; i++) {
            buffer[gapEnd + i] = buffer[gapStart + i];
        }
    } else if (pos > gapStart) {
        // Move gap right
        int distance = pos - gapStart;
        for (int i = 0; i < distance; i++) {
            buffer[gapStart + i] = buffer[gapEnd + i];
        }
        gapStart = pos;
        gapEnd += distance;
    }
}

void GapBuffer::insert(char c) {
    if (gapStart == gapEnd) {
        resize(capacity * 2);
    }
    buffer[gapStart++] = c;
}

void GapBuffer::deleteLeft() {
    if (gapStart > 0) {
        gapStart--;
    }
}

void GapBuffer::deleteRight() {
    if (gapEnd < capacity) {
        gapEnd++;
    }
}

int GapBuffer::getCursorPosition() const {
    return gapStart;
}

int GapBuffer::getLength() const {
    return capacity - (gapEnd - gapStart);
}

char GapBuffer::getCharAt(int pos) const {
    if (pos < 0 || pos >= getLength()) return '\0';
    if (pos < gapStart) {
        return buffer[pos];
    } else {
        return buffer[gapEnd + (pos - gapStart)];
    }
}

void GapBuffer::getText(char* dest, int maxLen) const {
    int len = getLength();
    if (len > maxLen - 1) len = maxLen - 1;
    
    int i = 0;
    for (int pos = 0; pos < len; pos++) {
        dest[i++] = getCharAt(pos);
    }
    dest[i] = '\0';
}

void GapBuffer::clear() {
    gapStart = 0;
    gapEnd = capacity;
}

void GapBuffer::loadFromString(const char* str) {
    clear();
    int len = strlen(str);
    while (capacity < len + 100) {
        resize(capacity * 2);
    }
    for (int i = 0; i < len; i++) {
        insert(str[i]);
    }
}