#include "CompressionAlgorithms.h"
#include <chrono>
#include <map>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

string simple_arithmetic_compress(const string& input) {
    if (input.empty()) return "";

    map<char, int> freq;
    for (char c : input) {
        freq[c]++;
    }

    vector<char> symbols;
    vector<int> frequencies;
    int total = 0;

    for (auto& [ch, count] : freq) {
        symbols.push_back(ch);
        frequencies.push_back(count);
        total += count;
    }

    vector<pair<int, int>> ranges;
    int current_low = 0;

    for (int count : frequencies) {
        int high = current_low + count;
        ranges.push_back({ current_low, high });
        current_low = high;
    }

    int low = 0;
    int high = total;

    for (char c : input) {
        for (size_t i = 0; i < symbols.size(); i++) {
            if (symbols[i] == c) {
                int range = high - low;
                high = low + (range * ranges[i].second) / total;
                low = low + (range * ranges[i].first) / total;
                break;
            }
        }
    }

    int code = (low + high) / 2;

    stringstream compressed;

    for (char ch : symbols) {
        compressed << (int)ch << ' ';
    }
    compressed << "| ";

    for (int f : frequencies) {
        compressed << f << ' ';
    }
    compressed << "| ";

    compressed << code << " " << total;

    return compressed.str();
}

string simple_arithmetic_decompress(const string& compressed) {
    if (compressed.empty()) return "";

    stringstream ss(compressed);
    vector<char> symbols;
    vector<int> frequencies;

    string token;
    while (ss >> token && token != "|") {
        try {
            symbols.push_back((char)stoi(token));
        }
        catch (...) {
            break;
        }
    }

    while (ss >> token && token != "|") {
        try {
            frequencies.push_back(stoi(token));
        }
        catch (...) {
            break;
        }
    }

    int code, total;
    if (!(ss >> code >> total)) {
        return "";
    }

    if (symbols.size() != frequencies.size() || symbols.empty()) {
        return "";
    }

    vector<pair<int, int>> ranges;
    int current_low = 0;

    for (int count : frequencies) {
        int high = current_low + count;
        ranges.push_back({ current_low, high });
        current_low = high;
    }

    string result;
    int current_code = code;

    for (int i = 0; i < total; i++) {
        for (size_t j = 0; j < symbols.size(); j++) {
            if (current_code >= ranges[j].first && current_code < ranges[j].second) {
                result += symbols[j];

                if (j < ranges.size()) {
                    int range = ranges[j].second - ranges[j].first;
                    if (range > 0) {
                        current_code = ((current_code - ranges[j].first) * total) / range;
                    }
                }
                break;
            }
        }
    }

    return result;
}

CompressionResult milyaeva_compress(const string& input) {
    auto start_time = chrono::high_resolution_clock::now();

    string compressed_data = simple_arithmetic_compress(input);

    auto end_time = chrono::high_resolution_clock::now();
    auto compression_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    auto decomp_start = chrono::high_resolution_clock::now();
    string decompressed = simple_arithmetic_decompress(compressed_data);
    auto decomp_end = chrono::high_resolution_clock::now();
    auto decompression_time = chrono::duration_cast<chrono::milliseconds>(decomp_end - decomp_start);

    CompressionResult result;
    result.algorithm_name = "Simple Arithmetic Coding";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    result.compression_ratio = (input.size() > 0 && compressed_data.size() > 0) ?
        (double)input.size() / compressed_data.size() : 1.0;
    result.compression_time_ms = compression_time.count();
    result.decompression_time_ms = decompression_time.count();
    result.integrity_ok = (input == decompressed);

    return result;
}

string milyaeva_decompress(const string& compressed) {
    return simple_arithmetic_decompress(compressed);
}