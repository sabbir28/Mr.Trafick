// custom_printer.h
#ifndef CUSTOM_PRINTER_H
#define CUSTOM_PRINTER_H

#include <iostream>
#include <sstream>

void print() {
    std::cout << std::endl;
}

template <typename T, typename... Args>
void print(const T& value, const Args&... args) {
    std::cout << value << " ";
    print(args...);
}


// HexConverter class as defined before
class HexConverter {
public:
    static int hexToDec(uint16_t hexValue) {
        std::stringstream ss;
        ss << std::hex << hexValue;
        int decimalValue;
        ss >> decimalValue;
        return decimalValue;
    }
};




#endif  // CUSTOM_PRINTER_H
