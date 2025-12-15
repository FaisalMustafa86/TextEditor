#include "filehandler.h"
#include <fstream>
#include <sstream>
#include <string>

void FileHandler::saveText(const char* filename, const char* data) {
    std::ofstream out(filename);
    if (!out) return;
    out << data;
    out.close();
}

std::string FileHandler::loadText(const char* filename) {
    std::ifstream in(filename);
    if (!in) return std::string();
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
