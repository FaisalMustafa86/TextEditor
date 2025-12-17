#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include <cstring>

struct EditorState {
    char* text;
    int cursorPos;
    int selStart;
    int selEnd;
    
    EditorState();
    EditorState(const char* t, int cp, int ss, int se);
    ~EditorState();
    EditorState(const EditorState& other);
};

#endif