#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <FL/Fl_Widget.H>
#include "GapBuffer.h"
#include "Stack.h"
#include "EditorState.h"

class TextEditor : public Fl_Widget {
private:
    GapBuffer gapBuffer;
    Stack<EditorState> undoStack;
    Stack<EditorState> redoStack;
    
    int cursorPos;
    int selectionStart;
    int selectionEnd;
    bool selecting;
    
    int scrollOffset;
    int lineHeight;
    int charWidth;
    
    char mode;  // 'n' = normal, 'i' = insert
    char statusMsg[256];
    
    void saveState();
    void deleteSelection();
    
public:
    TextEditor(int X, int Y, int W, int H);
    
    void draw() override;
    int handle(int event) override;
    
    void undo();
    void redo();
    void startSelection();
    void updateSelection();
    void clearSelection();
    bool hasSelection() const;
    
    void saveToFile(const char* filename);
    void loadFromFile(const char* filename);
    void newFile();
};

#endif