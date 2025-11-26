#include "CompressionAlgorithms.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <bitset>

using namespace std;

struct LZ77Triple {
    unsigned short offset;
    unsigned char length;
    char next_char;

    LZ77Triple(unsigned short o = 0, unsigned char l = 0, char c = 0) : offset(o), length(l), next_char(c) {}
};


string packTriple(const LZ77Triple& triple) {
    string packed = "";
    packed += static_cast<char>((triple.offset >> 8) & 0xFF);
    packed += static_cast<char>(triple.offset & 0xFF);
    packed += static_cast<char>(triple.length);
    packed += triple.next_char;
    return packed;
}

LZ77Triple unpackTriple(const string& data, size_t& pos) {
    if (pos + 4 > data.size()) return LZ77Triple();

    unsigned short offset = (static_cast<unsigned char>(data[pos]) << 8) |
        static_cast<unsigned char>(data[pos + 1]);
    pos += 2;
    unsigned char length = static_cast<unsigned char>(data[pos]);
    pos++;
    char next_char = data[pos];
    pos++;

    return LZ77Triple(offset, length, next_char);
}

LZ77Triple findLongestMatch(const string& input, size_t current_pos, size_t search_buffer_size, size_t look_ahead_buffer_size) {
    size_t start_search = (current_pos > search_buffer_size) ? current_pos - search_buffer_size : 0;
    size_t end_search = current_pos;
    size_t end_look_ahead = min(current_pos + look_ahead_buffer_size, input.length());

    size_t best_offset = 0;
    size_t best_length = 0;

    const size_t LIMITED_SEARCH_SIZE = 2048;
    size_t limited_start = (current_pos > LIMITED_SEARCH_SIZE) ? current_pos - LIMITED_SEARCH_SIZE : 0;
    start_search = max(start_search, limited_start);

    for (size_t i = start_search; i < end_search; ++i) {
        size_t len = 0;
        size_t j = i;
        size_t k = current_pos;
        while (j < end_search && k < end_look_ahead && input[j] == input[k]) {
            len++;
            j++;
            k++;
        }
        if (len > best_length) {
            best_length = len;
            best_offset = current_pos - i;
        }
    }

    char next_char = (current_pos + best_length < input.length()) ? input[current_pos + best_length] : 0;
    return LZ77Triple(static_cast<unsigned short>(best_offset), static_cast<unsigned char>(best_length), next_char);
}

CompressionResult milyaeva_compress(const string& input) {
    auto start_time = chrono::high_resolution_clock::now();

    string compressed_data = "";

    if (input.empty()) {
        compressed_data = "0|";
    }
    else {
        vector<LZ77Triple> triples;
        size_t i = 0;
        const size_t SEARCH_BUFFER_SIZE = 4096;
        const size_t LOOK_AHEAD_BUFFER_SIZE = 255;

        while (i < input.length()) {
            LZ77Triple match = findLongestMatch(input, i, SEARCH_BUFFER_SIZE, LOOK_AHEAD_BUFFER_SIZE);

            if (match.length == 0) {
                triples.push_back(LZ77Triple(0, 0, input[i]));
                i++;
            }
            else {
                triples.push_back(match);
                i += match.length + 1;
            }
        }

        stringstream ss;
        ss << triples.size() << "|";
        for (const auto& triple : triples) {
            ss << packTriple(triple);
        }
        compressed_data = ss.str();
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto compression_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time);

    auto decomp_start = chrono::high_resolution_clock::now();
    string decompressed = milyaeva_decompress(compressed_data);
    auto decomp_end = chrono::high_resolution_clock::now();
    auto decompression_time = chrono::duration_cast<chrono::microseconds>(decomp_end - decomp_start);

    CompressionResult result;
    result.algorithm_name = "Simple LZ77";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    result.compression_ratio = compressed_data.empty() ? 1.0 : static_cast<double>(input.size()) / compressed_data.size();
    result.compression_time_ms = static_cast<double>(compression_time.count());
    result.compression_time_ms = static_cast<double>(compression_time.count()) / 1000.0;
    result.decompression_time_ms = static_cast<double>(decompression_time.count()) / 1000.0;
    result.integrity_ok = (input == decompressed);

    return result;
}

string milyaeva_decompress(const string& compressed) {
    if (compressed.empty()) return "";

    size_t delim_pos = compressed.find('|');
    if (delim_pos == string::npos) return "";

    string count_str = compressed.substr(0, delim_pos);
    size_t num_triples;
    try {
        num_triples = stoul(count_str);
    }
    catch (...) {
        return "";
    }

    string data_part = compressed.substr(delim_pos + 1);

    string result = "";
    size_t pos = 0;

    for (size_t i = 0; i < num_triples && pos < data_part.size(); ++i) {
        LZ77Triple triple = unpackTriple(data_part, pos);

        if (triple.offset == 0 && triple.length == 0) {
            result += triple.next_char;
        }
        else if (triple.offset > 0 && triple.length > 0) {
            size_t start = result.length() - triple.offset;
            if (start + triple.length <= result.length()) {
                result.append(result.substr(start, triple.length));
            }
            else {
                return "";
            }
            if (triple.next_char != 0) {
                result += triple.next_char;
            }
        }
        else {
            return "";
        }
    }

    return result;
}