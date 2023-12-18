#ifndef TEXT_FILE_WRITER_H
#define TEXT_FILE_WRITER_H

#include <string>
#include <fstream>

class TextFileWriter {
public:
    TextFileWriter(const std::string& filePath);

    ~TextFileWriter();

    bool writeToFile(const std::string& content);

private:
    std::ofstream fileStream;
};

#endif // TEXT_FILE_WRITER_H
