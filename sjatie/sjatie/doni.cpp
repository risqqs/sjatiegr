#include "CompressionAlgorithms.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <string>

std::vector<std::string> splitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string word;
    while (ss >> word) {
        words.push_back(word);
    }
    return words;
}

std::string doni_decompress(const std::string& compressed) {
    if (compressed.empty()) {
        return "";
    }

    std::stringstream ss(compressed);
    std::string token;
    std::string result = "";

    bool first = true;
    while (ss >> token) {
        size_t separator_pos = token.find('|');
        if (separator_pos != std::string::npos) {
            try {
                int count = std::stoi(token.substr(0, separator_pos));
                std::string word = token.substr(separator_pos + 1);

                for (int i = 0; i < count; i++) {
                    if (!first) result += " ";
                    result += word;
                    first = false;
                }
            }
            catch (...) {
                if (!first) result += " ";
                result += token;
                first = false;
            }
        }
        else {
            if (!first) result += " ";
            result += token;
            first = false;
        }
    }

    return result;
}

CompressionResult doni_compress(const std::string& input) {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::string compressed_data;

    if (input.empty()) {
        compressed_data = "";
    }
    else {
        auto words = splitIntoWords(input);

        if (!words.empty()) {
            std::string current_word = words[0];
            int count = 1;
            std::stringstream ss;

            for (size_t i = 1; i < words.size(); i++) {
                if (words[i] == current_word) {
                    count++;
                }
                else {
                    if (count > 1) {
                        ss << count << "|" << current_word << " ";
                    }
                    else {
                        ss << current_word << " ";
                    }
                    current_word = words[i];
                    count = 1;
                }
            }
            if (count > 1) {
                ss << count << "|" << current_word;
            }
            else {
                ss << current_word;
            }
            compressed_data = ss.str();
            if (!compressed_data.empty() && compressed_data.back() == ' ') {
                compressed_data.pop_back();
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto compression_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    auto decomp_start = std::chrono::high_resolution_clock::now();
    std::string decompressed = doni_decompress(compressed_data);
    auto decomp_end = std::chrono::high_resolution_clock::now();
    auto decompression_time = std::chrono::duration_cast<std::chrono::milliseconds>(decomp_end - decomp_start);

    CompressionResult result;
    result.algorithm_name = "RLE String";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    result.compression_ratio = compressed_data.empty() ? 1.0 : static_cast<double>(input.size()) / compressed_data.size();
    result.compression_time_ms = static_cast<double>(compression_time.count());
    result.decompression_time_ms = static_cast<double>(decompression_time.count());
    result.integrity_ok = (input == decompressed);

    return result;
}