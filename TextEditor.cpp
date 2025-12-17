#include "TextEditor.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cstring>
#include <fstream>
#include <cstdio>

TextEditor::TextEditor(int X, int Y, int W, int H) 
    : Fl_Widget(X, Y, W, H), 
      cursorPos(0), selectionStart(-1), selectionEnd(-1), selecting(false),
      scrollOffset(0), lineHeight(16), charWidth(8), mode('n') {
    strcpy(statusMsg, "-- NORMAL --");
}

void TextEditor::saveState() {
    char* text = new char[gapBuffer.getLength() + 1];
    gapBuffer.getText(text, gapBuffer.getLength() + 1);
    
    EditorState state(text, cursorPos, selectionStart, selectionEnd);
    undoStack.push(state);
    redoStack.clear();
    
    delete[] text;
}

void TextEditor::deleteSelection() {
    if (!hasSelection()) return;
    
    int start = selectionStart < selectionEnd ? selectionStart : selectionEnd;
    int end = selectionStart > selectionEnd ? selectionStart : selectionEnd;
    
    gapBuffer.moveCursorTo(start);
    for (int i = start; i < end; i++) {
        gapBuffer.deleteRight();
    }
    
    cursorPos = start;
    clearSelection();
}

void TextEditor::draw() {
    // Background
    fl_color(FL_BLACK);
    fl_rectf(x(), y(), w(), h());
    
    // Set up text rendering
    fl_font(FL_COURIER, 14);
    lineHeight = 16;
    charWidth = 8;
    
    int textLen = gapBuffer.getLength();
    int cx = x() + 10;
    int cy = y() + 20;
    
    // Draw text with selection highlighting
    for (int i = 0; i < textLen; i++) {
        char c = gapBuffer.getCharAt(i);
        
        bool isSelected = hasSelection() && 
                        ((selectionStart <= i && i < selectionEnd) ||
                         (selectionEnd <= i && i < selectionStart));
        
        if (isSelected) {
            fl_color(FL_BLUE);
            fl_rectf(cx, cy - lineHeight + 4, charWidth, lineHeight);
        }
        
        if (c == '\n') {
            cy += lineHeight;
            cx = x() + 10;
        } else {
            fl_color(isSelected ? FL_BLACK : FL_WHITE);
            char str[2] = {c, '\0'};
            fl_draw(str, cx, cy);
            cx += charWidth;
        }
        
        if (cy > y() + h()) break;
    }
    
    // Draw cursor
    gapBuffer.moveCursorTo(cursorPos);
    int cursorX = x() + 10;
    int cursorY = y() + 20;
    
    for (int i = 0; i < cursorPos; i++) {
        char c = gapBuffer.getCharAt(i);
        if (c == '\n') {
            cursorY += lineHeight;
            cursorX = x() + 10;
        } else {
            cursorX += charWidth;
        }
    }
    
    fl_color(mode == 'i' ? FL_GREEN : FL_CYAN);
    fl_rectf(cursorX, cursorY - lineHeight + 4, mode == 'i' ? 2 : charWidth, lineHeight);
    
    // Status bar
    fl_color(45, 45, 45);
    fl_rectf(x(), y() + h() - 25, w(), 25);
    
    fl_color(FL_WHITE);
    fl_font(FL_COURIER_BOLD, 12);
    fl_draw(statusMsg, x() + 10, y() + h() - 8);
    
    char posInfo[100];
    sprintf(posInfo, "Pos: %d | Len: %d", cursorPos, textLen);
    fl_draw(posInfo, x() + w() - 200, y() + h() - 8);
}

int TextEditor::handle(int event) {
    switch(event) {
        case FL_FOCUS:
            return 1;
            
        case FL_KEYDOWN: {
            int key = Fl::event_key();
            
            // Mode switching
            if (mode == 'n' && key == 'i') {
                mode = 'i';
                strcpy(statusMsg, "-- INSERT --");
                redraw();
                return 1;
            }
            
            if (mode == 'i' && key == FL_Escape) {
                mode = 'n';
                strcpy(statusMsg, "-- NORMAL --");
                redraw();
                return 1;
            }
            
            // Insert mode
            if (mode == 'i') {
                if (key == FL_BackSpace) {
                    saveState();
                    if (hasSelection()) {
                        deleteSelection();
                    } else if (cursorPos > 0) {
                        gapBuffer.moveCursorTo(cursorPos);
                        gapBuffer.deleteLeft();
                        cursorPos--;
                    }
                    redraw();
                    return 1;
                }
                
                if (key == FL_Delete) {
                    saveState();
                    if (hasSelection()) {
                        deleteSelection();
                    } else {
                        gapBuffer.moveCursorTo(cursorPos);
                        gapBuffer.deleteRight();
                    }
                    redraw();
                    return 1;
                }
                
                if (key == FL_Enter) {
                    saveState();
                    if (hasSelection()) deleteSelection();
                    gapBuffer.moveCursorTo(cursorPos);
                    gapBuffer.insert('\n');
                    cursorPos++;
                    redraw();
                    return 1;
                }
                
                const char* text = Fl::event_text();
                if (text && text[0] >= 32 && text[0] <= 126) {
                    saveState();
                    if (hasSelection()) deleteSelection();
                    gapBuffer.moveCursorTo(cursorPos);
                    gapBuffer.insert(text[0]);
                    cursorPos++;
                    redraw();
                    return 1;
                }
            }
            
            // Navigation (both modes)
            if (key == FL_Left) {
                if (Fl::event_state(FL_SHIFT)) {
                    startSelection();
                } else {
                    clearSelection();
                }
                if (cursorPos > 0) cursorPos--;
                if (Fl::event_state(FL_SHIFT)) {
                    updateSelection();
                }
                redraw();
                return 1;
            }
            
            if (key == FL_Right) {
                if (Fl::event_state(FL_SHIFT)) {
                    startSelection();
                } else {
                    clearSelection();
                }
                if (cursorPos < gapBuffer.getLength()) cursorPos++;
                if (Fl::event_state(FL_SHIFT)) {
                    updateSelection();
                }
                redraw();
                return 1;
            }
            
            // Vim-style navigation in normal mode
            if (mode == 'n') {
                if (key == 'h' && cursorPos > 0) {
                    cursorPos--;
                    redraw();
                    return 1;
                }
                if (key == 'l' && cursorPos < gapBuffer.getLength()) {
                    cursorPos++;
                    redraw();
                    return 1;
                }
                if (key == 'x') {
                    saveState();
                    gapBuffer.moveCursorTo(cursorPos);
                    gapBuffer.deleteRight();
                    redraw();
                    return 1;
                }
            }
            
            // Undo/Redo
            if (Fl::event_state(FL_CTRL) && key == 'z') {
                undo();
                return 1;
            }
            if (Fl::event_state(FL_CTRL) && key == 'y') {
                redo();
                return 1;
            }
            
            break;
        }
    }
    return Fl_Widget::handle(event);
}

void TextEditor::undo() {
    if (undoStack.isEmpty()) return;
    
    char* currentText = new char[gapBuffer.getLength() + 1];
    gapBuffer.getText(currentText, gapBuffer.getLength() + 1);
    EditorState currentState(currentText, cursorPos, selectionStart, selectionEnd);
    redoStack.push(currentState);
    delete[] currentText;
    
    EditorState prevState = undoStack.pop();
    gapBuffer.loadFromString(prevState.text);
    cursorPos = prevState.cursorPos;
    selectionStart = prevState.selStart;
    selectionEnd = prevState.selEnd;
    
    redraw();
}

void TextEditor::redo() {
    if (redoStack.isEmpty()) return;
    
    char* currentText = new char[gapBuffer.getLength() + 1];
    gapBuffer.getText(currentText, gapBuffer.getLength() + 1);
    EditorState currentState(currentText, cursorPos, selectionStart, selectionEnd);
    undoStack.push(currentState);
    delete[] currentText;
    
    EditorState nextState = redoStack.pop();
    gapBuffer.loadFromString(nextState.text);
    cursorPos = nextState.cursorPos;
    selectionStart = nextState.selStart;
    selectionEnd = nextState.selEnd;
    
    redraw();
}

void TextEditor::startSelection() {
    if (!selecting) {
        selectionStart = cursorPos;
        selecting = true;
    }
}

void TextEditor::updateSelection() {
    selectionEnd = cursorPos;
}

void TextEditor::clearSelection() {
    selectionStart = -1;
    selectionEnd = -1;
    selecting = false;
}

bool TextEditor::hasSelection() const {
    return selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd;
}

void TextEditor::saveToFile(const char* filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        int len = gapBuffer.getLength();
        for (int i = 0; i < len; i++) {
            file << gapBuffer.getCharAt(i);
        }
        file.close();
        sprintf(statusMsg, "Saved to %s", filename);
        redraw();
    }
}

void TextEditor::loadFromFile(const char* filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        char* content = new char[1000000];
        file.read(content, 999999);
        content[file.gcount()] = '\0';
        gapBuffer.loadFromString(content);
        cursorPos = 0;
        clearSelection();
        delete[] content;
        file.close();
        sprintf(statusMsg, "Loaded %s", filename);
        redraw();
    }
}

void TextEditor::newFile() {
    gapBuffer.clear();
    cursorPos = 0;
    clearSelection();
    undoStack.clear();
    redoStack.clear();
    strcpy(statusMsg, "New file");
    redraw();
}