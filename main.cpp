#include<FL/Fl.H> 
#include<FL/Fl_Window.H>
#include<FL/Fl_Widget.H>
#include<FL/fl_draw.H>

int main(){
Fl_Window window(1280,720, "Text Editor");
window.color(0x14141400);
window.end();
window.show();
return Fl::run();
}