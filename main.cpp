#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <iostream>
#include "filehandler.h"
#include "actionstack.h"

#include <FL/fl_draw.H>
#include <cstring>
#include <string>
#include <cstdio>
#include <chrono>

// Globals used by editor and callbacks
static Fl_Text_Buffer* textbuf = nullptr;
static ActionStack undoStack;
static ActionStack redoStack;
// action grouping helpers
static std::chrono::steady_clock::time_point last_action_time = std::chrono::steady_clock::now();
static Action::Type last_action_type = Action::INS;
static int last_action_pos = -1;
static bool paste_in_progress = false;
static bool ignore_modify_callback = false;
static int paste_pos = 0;

// Forward declarations for undo/redo used inside MyTextEditor
void undo_action();
void redo_action();

class MyTextEditor : public Fl_Text_Editor {
public:
    bool caret_visible;
    double blink_interval;

    MyTextEditor(int X, int Y, int W, int H) : Fl_Text_Editor(X, Y, W, H) {
        caret_visible = true;
        blink_interval = 0.5; // seconds
    }

    // Toggle caret visibility and redraw
    static void blink_cb(void* v) {
        MyTextEditor* e = (MyTextEditor*)v;
        e->caret_visible = !e->caret_visible;
        e->redraw();
        Fl::repeat_timeout(e->blink_interval, blink_cb, v);
    }

    // Start blinking when focused and intercept key events for undo/redo tracking
    int handle(int event) override {
        if (event == FL_FOCUS) {
            caret_visible = true;
            Fl::add_timeout(blink_interval, blink_cb, this);
            redraw();
            return Fl_Text_Editor::handle(event);
        } else if (event == FL_UNFOCUS) {
            caret_visible = false;
            Fl::remove_timeout(blink_cb, this);
            redraw();
            return Fl_Text_Editor::handle(event);
        }

        if (event == FL_KEYDOWN) {
            int key = Fl::event_key();
            int state = Fl::event_state();

            // Intercept Ctrl+Z / Ctrl+Y at editor level
            if ((state & FL_CTRL) && (key == 'z' || key == 'Z')) {
                undo_action();
                return 1;
            }
            if ((state & FL_CTRL) && (key == 'y' || key == 'Y')) {
                redo_action();
                return 1;
            }

            // Handle Cut (Ctrl+X)
            if ((state & FL_CTRL) && (key == 'x' || key == 'X')) {
                if (textbuf && textbuf->selected()) {
                    int s = 0, e = 0;
                    textbuf->selection_position(&s, &e);
                    char* oldc = textbuf->selection_text();
                    std::string oldt = oldc ? oldc : std::string();
                    if (oldc) free(oldc);
                    // perform cut via default editor function but suppress modify callback
                    ignore_modify_callback = true;
                    Fl_Text_Editor::kf_cut(0, this);
                    ignore_modify_callback = false;
                    undoStack.push(Action(Action::DEL, s, oldt));
                    redoStack.clear();
                }
                return 1;
            }

            // Handle Paste (Ctrl+V) — use editor's paste and capture via modify callback
            if ((state & FL_CTRL) && (key == 'v' || key == 'V')) {
                // mark that a paste is happening so buf_mod_cb will record the inserted text
                paste_in_progress = true;
                paste_pos = insert_position();
                Fl_Text_Editor::kf_paste(0, this);
                // buf_mod_cb will push the action and clear paste_in_progress
                redoStack.clear();
                last_action_time = std::chrono::steady_clock::now();
                last_action_type = Action::INS;
                return 1;
            }

            // Printable text insertion: record position before insertion
            const char* txt = Fl::event_text();
            if (txt && txt[0] && !(state & FL_CTRL)) {
                // If there is an active selection, this should be a replace
                if (textbuf && textbuf->selected()) {
                    int s = 0, e = 0;
                    textbuf->selection_position(&s, &e);
                    char* oldc = textbuf->selection_text();
                    std::string oldt = oldc ? oldc : std::string();
                    if (oldc) free(oldc);
                    int ret = Fl_Text_Editor::handle(event); // performs replacement
                    // record REPL action
                    Action a(s, oldt, std::string(txt));
                    undoStack.push(a);
                    redoStack.clear();
                    last_action_time = std::chrono::steady_clock::now();
                    last_action_type = Action::REPL;
                    last_action_pos = s;
                    return ret;
                }

                int posBefore = insert_position();
                int ret = Fl_Text_Editor::handle(event); // perform insertion

                // group inserted text as a single action when possible
                auto now = std::chrono::steady_clock::now();
                std::string s = txt;
                if (!undoStack.isEmpty() && last_action_type == Action::INS && (std::chrono::duration<double>(now - last_action_time).count() < 0.8)) {
                    // try to pop previous action and merge if contiguous
                    Action prev = undoStack.pop();
                    if (prev.type == Action::INS && prev.pos + (int)prev.text.size() == posBefore) {
                        // merge
                        Action merged(Action::INS, prev.pos, prev.text + s);
                        undoStack.push(merged);
                    } else {
                        // not mergeable; push prev back and push new
                        undoStack.push(prev);
                        Action a(Action::INS, posBefore, s);
                        undoStack.push(a);
                    }
                } else {
                    Action a(Action::INS, posBefore, s);
                    undoStack.push(a);
                }
                        last_action_time = now;
                last_action_type = Action::INS;
                last_action_pos = posBefore;
                redoStack.clear();
                return ret;
            }

            // Handle Backspace/Delete: record character being removed
            if (key == FL_BackSpace || key == FL_Delete) {
                // If there is a selection, delete it as a single action
                if (textbuf && textbuf->selected()) {
                    int s = 0, e = 0;
                    textbuf->selection_position(&s, &e);
                    char* oldc = textbuf->selection_text();
                    std::string oldt = oldc ? oldc : std::string();
                    if (oldc) free(oldc);
                    int ret = Fl_Text_Editor::handle(event); // will remove selection
                    undoStack.push(Action(Action::DEL, s, oldt));
                    redoStack.clear();
                    last_action_time = std::chrono::steady_clock::now();
                    last_action_type = Action::DEL;
                    last_action_pos = s;
                    return ret;
                }

                int pos = insert_position();
                int remPos = (key == FL_BackSpace) ? (pos - 1) : pos;
                if (remPos >= 0) {
                    const char* full = textbuf ? textbuf->text() : nullptr;
                    char removed = '\0';
                    if (full) {
                        int len = strlen(full);
                        if (remPos < len && remPos >= 0) removed = full[remPos];
                    }
                    int ret = Fl_Text_Editor::handle(event);
                    if (removed != '\0') {
                        // push as (possibly grouped) delete action
                        auto now = std::chrono::steady_clock::now();
                        std::string s(1, removed);
                        if (!undoStack.isEmpty() && last_action_type == Action::DEL && (std::chrono::duration<double>(now - last_action_time).count() < 0.8)) {
                            Action prev = undoStack.pop();
                            // If consecutive delete (either backward or forward) and adjacent, merge appropriately
                            if (prev.type == Action::DEL) {
                                // Determine if this deletion is adjacent to prev
                                if (remPos + 1 == prev.pos) {
                                    // deleted char is before prev.pos -> prepend
                                    Action merged(Action::DEL, remPos, s + prev.text);
                                    undoStack.push(merged);
                                } else if (prev.pos + (int)prev.text.size() == remPos) {
                                    // deleted char after prev block -> append
                                    Action merged(Action::DEL, prev.pos, prev.text + s);
                                    undoStack.push(merged);
                                } else {
                                    // not contiguous
                                    undoStack.push(prev);
                                    undoStack.push(Action(Action::DEL, remPos, s));
                                }
                            } else {
                                undoStack.push(prev);
                                undoStack.push(Action(Action::DEL, remPos, s));
                            }
                        } else {
                            undoStack.push(Action(Action::DEL, remPos, s));
                        }
                        last_action_time = now;
                        last_action_type = Action::DEL;
                        last_action_pos = remPos;
                        redoStack.clear();
                    }
                    return ret;
                }
            }
        }

        return Fl_Text_Editor::handle(event);
    }

    // Draw the editor and then draw a custom caret
    void draw() override {
        Fl_Text_Editor::draw();

        if (Fl::focus() == this && caret_visible) {
            int ipos = insert_position();
            int x = 0, y = 0;
            // position_to_xy returns if successful in some FLTK versions; we'll try to call it
            position_to_xy(ipos, &x, &y);

            // draw a simple vertical bar as caret
            fl_color(FL_WHITE);
            fl_rectf(x, y, 2, textsize() + 2);
        }

        // If buffer empty, draw a centered placeholder hint (subtle)
        Fl_Text_Buffer* buf = buffer();
        if (buf && buf->length() == 0) {
            const char* hint = "Start typing...";
            fl_font(FL_HELVETICA, 14);
            fl_color(FL_GRAY);
            int hint_w = fl_width(hint);
            int cx = this->x() + this->w() / 2 - hint_w / 2;
            int cy = this->y() + this->h() / 2 - 7;
            fl_draw(hint, cx, cy + 10);
        }
    }
};

// Undo/Redo implementation
void undo_action() {
    if (undoStack.isEmpty()) return;
    Action a = undoStack.pop();
    if (!textbuf) return;
    if (a.type == Action::INS) {
        // remove inserted text
        int end = a.pos + (int)a.text.size();
        textbuf->remove(a.pos, end);
        // push inverse (DEL) to redo
        Action inverse(Action::DEL, a.pos, a.text);
        redoStack.push(inverse);
    } else {
        // re-insert deleted text
        textbuf->insert(a.pos, a.text.c_str());
        // push inverse (INS) to redo
        Action inverse(Action::INS, a.pos, a.text);
        redoStack.push(inverse);
    }
}

void redo_action() {
    if (redoStack.isEmpty()) return;
    Action a = redoStack.pop();
    if (!textbuf) return;
    if (a.type == Action::INS) {
        textbuf->insert(a.pos, a.text.c_str());
        Action inverse(Action::DEL, a.pos, a.text);
        undoStack.push(inverse);
    } else {
        int end = a.pos + (int)a.text.size();
        textbuf->remove(a.pos, end);
        Action inverse(Action::INS, a.pos, a.text);
        undoStack.push(inverse);
    }
}

void undo_cb(Fl_Widget* w, void* v) { undo_action(); }
void redo_cb(Fl_Widget* w, void* v) { redo_action(); }

// Apply a uniform style (white text on black background) across the buffer
void apply_dark_style(MyTextEditor* editor) {
    if (!editor) return;
    Fl_Text_Buffer* buf = editor->buffer();
    if (!buf) return;
    int len = buf->length();
    char* styles = new char[(len > 0 ? len : 1) + 1];
    for (int i = 0; i < len; ++i) styles[i] = '0';
    styles[len] = '\0';

    static Fl_Text_Display::Style_Table_Entry st[] = {
        { FL_WHITE, FL_BLACK, 0, 0 }
    };

    static Fl_Text_Buffer* styleBuffer = nullptr;
    if (!styleBuffer) styleBuffer = new Fl_Text_Buffer();
    styleBuffer->text(styles);

    // signature: highlight_data(Fl_Text_Buffer* styleBuffer, const Style_Table_Entry* styleTable,
    //                             int styleTableSize, char defaultStyle, Unfinished_Style_Cb cb, void* cbArg)
    editor->highlight_data(styleBuffer, st, 1, '0', nullptr, nullptr);
    delete[] styles;
}

// Buffer modified callback to reapply dark style
void buf_mod_cb(int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText, void* cbp) {
    MyTextEditor* editor = (MyTextEditor*)cbp;
    // Ignore recording when flagged (e.g., we already handled cut)
    if (ignore_modify_callback) return;

    // Reapply dark style
    apply_dark_style(editor);

    // If this modification was caused by a paste, record the inserted text as one action
    if (paste_in_progress) {
        if (nInserted > 0) {
            Fl_Text_Buffer* buf = editor->buffer();
            if (buf) {
                char* inserted = buf->text_range(pos, pos + nInserted);
                if (inserted) {
                    // If there was deletedText (non-null) we treated it as replacement
                    if (nDeleted > 0 && deletedText) {
                        std::string oldt = std::string(deletedText);
                        std::string newt = std::string(inserted);
                        undoStack.push(Action(pos, oldt, newt));
                    } else {
                        undoStack.push(Action(Action::INS, pos, std::string(inserted)));
                    }
                    free(inserted);
                    redoStack.clear();
                }
            }
        }
        paste_in_progress = false;
    }
}

// Status bar
static Fl_Box* statusBox = nullptr;

void update_status(MyTextEditor* editor) {
    if (!editor || !statusBox) return;
    Fl_Text_Buffer* buf = editor->buffer();
    if (!buf) return;
    int pos = editor->insert_position();
    const char* s = buf->text();
    int line = 1, col = 1;
    for (int i = 0; i < pos; ++i) if (s[i] == '\n') { line++; col = 1; } else col++;
    char label[128];
    snprintf(label, sizeof(label), "Ln %d, Col %d", line, col);
    statusBox->label(label);
}

void status_timer_cb(void* v) {
    MyTextEditor* editor = (MyTextEditor*)v;
    update_status(editor);
    Fl::repeat_timeout(0.25, status_timer_cb, v);
}

void open_cb(Fl_Widget* w, void* v) {
    Fl_Native_File_Chooser fc;
    fc.title("Open File");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if (fc.show() == 0 && fc.filename()) {
        std::string content = FileHandler::loadText(fc.filename());
        if (textbuf) textbuf->text(content.c_str());
    }
}

void save_cb(Fl_Widget* w, void* v) {
    Fl_Native_File_Chooser fc;
    fc.title("Save File");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    if (fc.show() == 0 && fc.filename()) {
        const char* txt = textbuf ? textbuf->text() : "";
        FileHandler::saveText(fc.filename(), txt);
    }
}

void exit_cb(Fl_Widget* w, void* v) {
    exit(0);
}

int main(int argc, char** argv) {
    Fl_Window* win = new Fl_Window(800, 600, "TextEditor - FLTK");
    win->begin();

    static Fl_Menu_Item menuitems[] = {
        {"&File", 0, 0, 0, FL_SUBMENU},
        {"Open", FL_CTRL + 'o', (Fl_Callback*)open_cb, 0},
        {"Save", FL_CTRL + 's', (Fl_Callback*)save_cb, 0},
        {"Exit", FL_CTRL + 'q', (Fl_Callback*)exit_cb, 0},
        {0},

        {"&Edit", 0, 0, 0, FL_SUBMENU},
        {"Undo", FL_CTRL + 'z', (Fl_Callback*)undo_cb, 0},
        {"Redo", FL_CTRL + 'y', (Fl_Callback*)redo_cb, 0},
        {0},

        {0},
    };

    Fl_Menu_Bar* menubar = new Fl_Menu_Bar(0, 0, win->w(), 28);
    menubar->menu(menuitems);
    // Minimal dark menu styling
    menubar->box(FL_FLAT_BOX);
    menubar->color(FL_BLACK);
    menubar->labelcolor(FL_WHITE);
    menubar->textsize(12);

    // Toolbar (minimal, flat, icon-like buttons)
    Fl_Group* toolbar = new Fl_Group(0, menubar->h(), win->w(), 36);
    toolbar->box(FL_FLAT_BOX);
    toolbar->color(FL_BLACK);
    int bx = 6;
    int bw = 36;
    Fl_Button* bopen = new Fl_Button(bx, menubar->h() + 6, bw, 24, "📂"); bx += bw + 6;
    Fl_Button* bsave = new Fl_Button(bx, menubar->h() + 6, bw, 24, "💾"); bx += bw + 6;
    Fl_Button* bundo = new Fl_Button(bx, menubar->h() + 6, bw, 24, "↶"); bx += bw + 6;
    Fl_Button* bredo = new Fl_Button(bx, menubar->h() + 6, bw, 24, "↷"); bx += bw + 6;
    bopen->box(FL_FLAT_BOX); bsave->box(FL_FLAT_BOX); bundo->box(FL_FLAT_BOX); bredo->box(FL_FLAT_BOX);
    bopen->color(FL_BLACK); bsave->color(FL_BLACK); bundo->color(FL_BLACK); bredo->color(FL_BLACK);
    bopen->labelcolor(FL_WHITE); bsave->labelcolor(FL_WHITE); bundo->labelcolor(FL_WHITE); bredo->labelcolor(FL_WHITE);
    bopen->labelsize(14); bsave->labelsize(14); bundo->labelsize(14); bredo->labelsize(14);
    bopen->callback(open_cb); bsave->callback(save_cb); bundo->callback(undo_cb); bredo->callback(redo_cb);
    toolbar->end();

    textbuf = new Fl_Text_Buffer();
    MyTextEditor* editor = new MyTextEditor(0, menubar->h() + 36, win->w(), win->h() - menubar->h() - 36 - 22);
    editor->buffer(textbuf);
    // Editor modern minimal styling
    editor->box(FL_FLAT_BOX);
    editor->color(FL_BLACK);
    editor->textfont(FL_COURIER);
    editor->textsize(14);

    // Status bar
    statusBox = new Fl_Box(0, win->h() - 22, win->w(), 22, "Ln 1, Col 1");
    statusBox->box(FL_FLAT_BOX);
    statusBox->color(FL_BLACK);
    statusBox->labelcolor(FL_WHITE);
    statusBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    // Apply initial dark style and keep it in sync
    apply_dark_style(editor);
    textbuf->add_modify_callback(buf_mod_cb, (void*)editor);
    // start status update timer
    Fl::add_timeout(0.25, status_timer_cb, editor);
    // Editor dark mode colors
    editor->color(FL_BLACK);
    editor->textfont(FL_COURIER);
    editor->textsize(14);

    // Apply initial dark style and keep it in sync
    apply_dark_style(editor);
    textbuf->add_modify_callback(buf_mod_cb, (void*)editor);

    // Make the editor expand with the window and prevent the window
    // from shrinking below the initial 800x600 size. Also set window color
    // for dark mode.
    win->color(FL_BLACK);
    win->resizable(editor);
    win->size_range(800, 600);

    win->end();
    win->show(argc, argv);
    return Fl::run();
}
