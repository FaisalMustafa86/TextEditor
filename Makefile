CXX = g++
CXXFLAGS = `fltk-config --cxxflags` -std=c++11
LDFLAGS = `fltk-config --ldflags`

TARGET = texteditor
OBJS = main.o GapBuffer.o EditorState.o TextEditor.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: main.cpp TextEditor.h GapBuffer.h Stack.h EditorState.h
	$(CXX) $(CXXFLAGS) -c main.cpp

GapBuffer.o: GapBuffer.cpp GapBuffer.h
	$(CXX) $(CXXFLAGS) -c GapBuffer.cpp

EditorState.o: EditorState.cpp EditorState.h
	$(CXX) $(CXXFLAGS) -c EditorState.cpp

TextEditor.o: TextEditor.cpp TextEditor.h GapBuffer.h Stack.h EditorState.h
	$(CXX) $(CXXFLAGS) -c TextEditor.cpp

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run