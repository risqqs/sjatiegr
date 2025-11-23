#include "CompressionAlgorithms.h"
#include <chrono>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;

string arithmetic_compress(const string& input) {
    if (input.empty()) return "";

    // Подсчет частот символов
    map<char, int> freq;
    for (char c : input) {
        freq[c]++;
    }

    // Создание таблицы диапазонов
    vector<pair<char, pair<double, double>>> ranges;
    double current_low = 0.0;

    for (auto& [ch, count] : freq) {
        double prob = (double)count / input.size();
        ranges.push_back({ ch, {current_low, current_low + prob} });
        current_low += prob;
    }

    double low = 0.0;
    double high = 1.0;

    for (char c : input) {
        for (auto& [ch, range] : ranges) {
            if (ch == c) {
                double range_width = high - low;
                high = low + range_width * range.second;
                low = low + range_width * range.first;
                break;
            }
        }
    }

    double encoded_value = (low + high) / 2.0;

    stringstream compressed;

    for (auto& [ch, count] : freq) {
        compressed << (int)ch << ",";
    }
    compressed << "|";
    for (auto& [ch, count] : freq) {
        compressed << count << ",";
    }
    compressed << "|";
    compressed << fixed << setprecision(20) << encoded_value;

    return compressed.str();
}

string arithmetic_decompress(const string& compressed) {
    if (compressed.empty()) return "";

    stringstream ss(compressed);
    string chars_str, freqs_str;
    double encoded_value;

    getline(ss, chars_str, '|');
    getline(ss, freqs_str, '|');
    ss >> encoded_value;

    vector<char> characters;
    vector<int> frequencies;

    stringstream chars_ss(chars_str);
    string char_token;
    while (getline(chars_ss, char_token, ',')) {
        if (!char_token.empty()) {
            characters.push_back((char)stoi(char_token));
        }
    }

    stringstream freqs_ss(freqs_str);
    string freq_token;
    while (getline(freqs_ss, freq_token, ',')) {
        if (!freq_token.empty()) {
            frequencies.push_back(stoi(freq_token));
        }
    }

    int total_chars = 0;
    for (int freq : frequencies) {
        total_chars += freq;
    }

    vector<pair<char, pair<double, double>>> ranges;
    double current_low = 0.0;

    for (size_t i = 0; i < characters.size(); i++) {
        double prob = (double)frequencies[i] / total_chars;
        ranges.push_back({ characters[i], {current_low, current_low + prob} });
        current_low += prob;
    }

    string result;
    double value = encoded_value;

    for (int i = 0; i < total_chars; i++) {

        for (auto& [ch, range] : ranges) {
            if (value >= range.first && value < range.second) {
                result += ch;
                value = (value - range.first) / (range.second - range.first);
                break;
            }
        }
    }

    return result;
}

CompressionResult milyaeva_compress(const string& input) {
    auto start_time = chrono::high_resolution_clock::now();

    string compressed_data = arithmetic_compress(input);

    auto end_time = chrono::high_resolution_clock::now();
    auto compression_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    auto decomp_start = chrono::high_resolution_clock::now();
    string decompressed = arithmetic_decompress(compressed_data);
    auto decomp_end = chrono::high_resolution_clock::now();
    auto decompression_time = chrono::duration_cast<chrono::milliseconds>(decomp_end - decomp_start);

    CompressionResult result;
    result.algorithm_name = "Arithmetic Coding";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    result.compression_ratio = compressed_data.empty() ? 1.0 : (double)input.size() / compressed_data.size();
    result.compression_time_ms = compression_time.count();
    result.decompression_time_ms = decompression_time.count();
    result.integrity_ok = (input == decompressed);

    return result;
}

string milyaeva_decompress(const string& compressed) {
    return arithmetic_decompress(compressed);
}