#ifndef GAPBUFFER_H
#define GAPBUFFER_H

class GapBuffer {
private:
    char* buffer;
    int capacity;
    int gapStart;
    int gapEnd;
    
    void resize(int newCapacity);
    
public:
    GapBuffer(int initialCapacity = 1024);
    ~GapBuffer();
    
    void moveCursorTo(int pos);
    void insert(char c);
    void deleteLeft();
    void deleteRight();
    int getCursorPosition() const;
    int getLength() const;
    char getCharAt(int pos) const;
    void getText(char* dest, int maxLen) const;
    void clear();
    void loadFromString(const char* str);
};

#endif