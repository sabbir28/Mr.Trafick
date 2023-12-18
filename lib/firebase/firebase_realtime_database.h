#ifndef FIREBASE_REALTIME_DATABASE_H
#define FIREBASE_REALTIME_DATABASE_H

#include <iostream>
#include <curl/curl.h>

class FirebaseRealtimeDatabase {
public:
    FirebaseRealtimeDatabase(const std::string& firebaseUrl);

    ~FirebaseRealtimeDatabase();

    void sendData(const std::string& jsonData);

private:
    std::string firebaseUrl;
};

#endif // FIREBASE_REALTIME_DATABASE_H
