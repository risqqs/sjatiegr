#include "CompressionAlgorithms.h"
#include <chrono>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

vector<string> splitIntoWords(const string& text) {
    vector<string> words;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        words.push_back(word);
    }
    return words;
}

CompressionResult doni_compress(const string& input) {
    auto start_time = chrono::high_resolution_clock::now();

    string compressed_data;

    if (input.empty()) {
        compressed_data = "";
    }
    else {
        auto words = splitIntoWords(input);

        if (!words.empty()) {
            string current_word = words[0];
            int count = 1;

            for (size_t i = 1; i < words.size(); i++) {
                if (words[i] == current_word) {
                    count++;
                }
                else {
                    compressed_data += to_string(count) + "|" + current_word + " ";
                    current_word = words[i];
                    count = 1;
                }
            }
            compressed_data += to_string(count) + "|" + current_word;
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto compression_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    CompressionResult result;
    result.algorithm_name = "RLE String Compression";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    result.compression_ratio = compressed_data.empty() ? 1.0 : (double)input.size() / compressed_data.size();
    result.compression_time_ms = compression_time.count();
    result.decompression_time_ms = 0;
    result.integrity_ok = false;

    return result;
}