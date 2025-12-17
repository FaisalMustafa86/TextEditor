#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <cstdlib>
#include "TextEditor.h"

TextEditor* editor = nullptr;

// Menu callbacks
void new_cb(Fl_Widget* w, void* data) {
    editor->newFile();
}

void open_cb(Fl_Widget* w, void* data) {
    Fl_Native_File_Chooser chooser;
    chooser.title("Open File");
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    
    if (chooser.show() == 0) {
        editor->loadFromFile(chooser.filename());
    }
}

void save_cb(Fl_Widget* w, void* data) {
    Fl_Native_File_Chooser chooser;
    chooser.title("Save File");
    chooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    
    if (chooser.show() == 0) {
        editor->saveToFile(chooser.filename());
    }
}

void exit_cb(Fl_Widget* w, void* data) {
    exit(0);
}

void undo_cb(Fl_Widget* w, void* data) {
    editor->undo();
}

void redo_cb(Fl_Widget* w, void* data) {
    editor->redo();
}

int main(int argc, char** argv) {
    Fl_Window* window = new Fl_Window(900, 600, "NeoVim-Style Text Editor");
    window->color(FL_BLACK);
    
    // Menu bar
    Fl_Menu_Bar* menubar = new Fl_Menu_Bar(0, 0, 900, 25);
    menubar->box(FL_FLAT_BOX);
    menubar->color(fl_rgb_color(30, 30, 30));
    menubar->textcolor(FL_WHITE);
    
    menubar->add("File/New", "^n", new_cb);
    menubar->add("File/Open", "^o", open_cb);
    menubar->add("File/Save", "^s", save_cb);
    menubar->add("File/Exit", "^q", exit_cb);
    menubar->add("Edit/Undo", "^z", undo_cb);
    menubar->add("Edit/Redo", "^y", redo_cb);
    
    // Text editor
    editor = new TextEditor(0, 25, 900, 575);
    
    window->end();
    window->resizable(editor);
    window->show(argc, argv);
    
    return Fl::run();
}