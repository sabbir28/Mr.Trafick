// File: firebase_storage.h
// Write by : sabbir 15/12/2023


#ifndef FIREBASE_STORAGE_H
#define FIREBASE_STORAGE_H

#include <iostream>
#include <curl/curl.h>

class FirebaseStorage {
public:
    FirebaseStorage(const char* firebaseStorageUrl);

    ~FirebaseStorage();

    bool uploadFile(const char* filePath);

private:
    static size_t readCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);

    CURL* curl;
    const char* firebaseStorageUrl;
};

#endif // FIREBASE_STORAGE_H
