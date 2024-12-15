#include <iostream>
#include <string>
#include <curl/curl.h>
#include <iomanip>
#include <fstream> 
#include <sstream>
#include <vector> 

size_t write_data(void* ptr, size_t size, size_t nmemb, std::vector<char>& data) {
    size_t total_size = size * nmemb;
    data.insert(data.end(), (char*)ptr, (char*)ptr + total_size);
    return total_size;
}

bool check_image(const std::string& url) {
    CURL* curl;
    CURLcode res;
    long http_code = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        }

        curl_easy_cleanup(curl);
    }

    return (http_code == 200);
}

bool download_image(const std::string& url, const std::string& output_file) {
    CURL* curl;
    CURLcode res;
    std::vector<char> image_data;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &image_data);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            std::ofstream outFile(output_file, std::ios::binary);
            if (!outFile) {
                std::cerr << "Failed to open file for writing: " << output_file << std::endl;
                curl_easy_cleanup(curl);
                return false;
            }
            outFile.write(image_data.data(), image_data.size());
            outFile.close();
            std::cout << "Image saved: " << output_file << std::endl;
        }
        else {
            std::cerr << "Failed to download image: " << url << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_cleanup(curl);
    }

    return true;
}

int main() {
    const std::string base_url = "https://gg.asuracomic.net/storage/media/{}/conversions/{}-Recovered-optimized.webp";
    const int start = 256000;
    const int end = 260000;

    std::ofstream outFile("working_links.txt");
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing!" << std::endl;
        return 1;
    }

    for (int i = start; i < end; ++i) {
        for (int month = 1; month <= 12; ++month) {
            std::ostringstream formatted_url;
            formatted_url << "https://gg.asuracomic.net/storage/media/" << i << "/conversions/"
                << std::setw(2) << std::setfill('0') << month << "-Recovered-optimized.webp";

            std::string url = formatted_url.str();
            if (check_image(url)) {
                std::cout << "Image found: " << url << std::endl;
                outFile << url << std::endl;

                std::string filename = "image_" + std::to_string(i) + "_" + std::to_string(month) + ".webp";
                download_image(url, filename);
            }
            else {
                std::cout << "No image at: " << url << std::endl;
            }
        }
    }

    outFile.close();

    return 0;
}
