#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>

class FileHandler {
public:
    // Helpers for GUI/editor usage (plain text)
    static void saveText(const char* filename, const char* data);
    static std::string loadText(const char* filename);
};

#endif
