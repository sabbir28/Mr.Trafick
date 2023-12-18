#include "text_file_writer.h"

TextFileWriter::TextFileWriter(const std::string& filePath) {
    fileStream.open(filePath);
    if (!fileStream.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath);
    }
}

TextFileWriter::~TextFileWriter() {
    if (fileStream.is_open()) {
        fileStream.close();
    }
}

bool TextFileWriter::writeToFile(const std::string& content) {
    if (!fileStream.is_open()) {
        std::cerr << "File is not open for writing." << std::endl;
        return false;
    }

    fileStream << content;
    return true;
}
