#include "firebase_realtime_database.h"

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

FirebaseRealtimeDatabase::FirebaseRealtimeDatabase(const std::string& firebaseUrl) : firebaseUrl(firebaseUrl) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

FirebaseRealtimeDatabase::~FirebaseRealtimeDatabase() {
    curl_global_cleanup();
}

void FirebaseRealtimeDatabase::sendData(const std::string& jsonData) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = firebaseUrl + "/.json";  // Your Firebase Realtime Database URL

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        
        // Response data will be stored here
        std::string responseData;

        // Set the response data pointer
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "Failed to send data to Firebase: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Response from Firebase: " << responseData << std::endl;
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }
}
