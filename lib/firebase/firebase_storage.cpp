// File: firebase_storage.cpp 
// Write by : sabbir 15/12/2023

#include "firebase_storage.h"

FirebaseStorage::FirebaseStorage(const char* firebaseStorageUrl) : firebaseStorageUrl(firebaseStorageUrl) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing libcurl." << std::endl;
        throw std::runtime_error("Error initializing libcurl.");
    }
}

FirebaseStorage::~FirebaseStorage() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

bool FirebaseStorage::uploadFile(const char* filePath) {
    // ... implementation for file upload
}

size_t FirebaseStorage::readCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    // ... implementation for read callback
}
