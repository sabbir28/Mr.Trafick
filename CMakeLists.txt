cmake_minimum_required(VERSION 3.10)

project(HTTPSniffer)

# Set the C++ compiler path
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_STANDARD 11)

# Find and link against zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# Add the path to the Firebase library
include_directories("lib/firebase")

# Explicitly list source files
set(SOURCES
    HTTPSniffer.cpp 
    lib/os/ElevatedPrivilegesChecker.cpp 
    #lib/firebase/firebase_storage.cpp
    #lib/firebase/firebase_realtime_database.cpp
)

add_executable(HTTPSniffer ${SOURCES})

# Link against the pcap library
find_library(PCAP_LIBRARY pcap)
target_link_libraries(HTTPSniffer PRIVATE ${PCAP_LIBRARY})

# Link against zlib
target_link_libraries(HTTPSniffer PRIVATE ${ZLIB_LIBRARIES})

# Link against the cURL library
find_package(CURL REQUIRED)
target_link_libraries(HTTPSniffer PRIVATE ${CURL_LIBRARIES})

# Link against OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(HTTPSniffer PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Find and link against cpprestsdk
find_package(cpprestsdk REQUIRED)
target_link_libraries(HTTPSniffer PRIVATE cpprestsdk::cpprest)
