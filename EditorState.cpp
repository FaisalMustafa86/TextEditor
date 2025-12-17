#include "EditorState.h"

EditorState::EditorState() : text(nullptr), cursorPos(0), selStart(-1), selEnd(-1) {}

EditorState::EditorState(const char* t, int cp, int ss, int se) 
    : cursorPos(cp), selStart(ss), selEnd(se) {
    int len = strlen(t);
    text = new char[len + 1];
    strcpy(text, t);
}

EditorState::~EditorState() {
    if (text) delete[] text;
}

EditorState::EditorState(const EditorState& other) 
    : cursorPos(other.cursorPos), selStart(other.selStart), selEnd(other.selEnd) {
    if (other.text) {
        int len = strlen(other.text);
        text = new char[len + 1];
        strcpy(text, other.text);
    } else {
        text = nullptr;
    }
}