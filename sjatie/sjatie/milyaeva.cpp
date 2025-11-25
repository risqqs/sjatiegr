#include "CompressionAlgorithms.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <bitset>
#include <unordered_map>

using namespace std;

struct LZ77Triple {
    unsigned short offset;
    unsigned char length;
    char next_char;

    LZ77Triple(unsigned short o = 0, unsigned char l = 0, char c = 0) : offset(o), length(l), next_char(c) {}
};

string packTriple(const LZ77Triple& triple) {
    string packed;
    packed.reserve(4);
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
    if (current_pos >= input.length()) return LZ77Triple(0, 0, 0);

    size_t start_search = (current_pos > search_buffer_size) ? current_pos - search_buffer_size : 0;
    size_t end_search = current_pos;
    size_t end_look_ahead = min(current_pos + look_ahead_buffer_size, input.length());

    if (end_look_ahead <= current_pos) return LZ77Triple(0, 0, input[current_pos]);

    size_t best_offset = 0;
    size_t best_length = 0;

    const size_t MIN_MATCH_LENGTH = 3;
    const size_t LIMITED_SEARCH_SIZE = 4096;
    size_t limited_start = (current_pos > LIMITED_SEARCH_SIZE) ? current_pos - LIMITED_SEARCH_SIZE : 0;
    start_search = max(start_search, limited_start);

    size_t max_possible_length = end_look_ahead - current_pos;
    if (max_possible_length < MIN_MATCH_LENGTH) {
        return LZ77Triple(0, 0, input[current_pos]);
    }

    for (size_t i = start_search; i < end_search; ++i) {
        if (input[i] != input[current_pos]) continue;

        size_t len = 1;
        while (i + len < end_search &&
            current_pos + len < end_look_ahead &&
            input[i + len] == input[current_pos + len] &&
            len < 255) {
            len++;
        }

        if (len >= MIN_MATCH_LENGTH && len > best_length) {
            best_length = len;
            best_offset = current_pos - i;
            if (best_length == max_possible_length) break;
        }
    }

    if (best_length < MIN_MATCH_LENGTH) {
        return LZ77Triple(0, 0, input[current_pos]);
    }

    char next_char = (current_pos + best_length < input.length()) ? input[current_pos + best_length] : 0;
    return LZ77Triple(static_cast<unsigned short>(best_offset), static_cast<unsigned char>(best_length), next_char);
}

CompressionResult milyaeva_compress(const string& input) {
    auto start_time = chrono::high_resolution_clock::now();

    string compressed_data;

    if (input.empty()) {
        compressed_data = "0|";
    }
    else if (input.length() == 1) {
        compressed_data = "1|";
        compressed_data += packTriple(LZ77Triple(0, 0, input[0]));
    }
    else {
        vector<LZ77Triple> triples;
        triples.reserve(input.length() / 2);
        size_t i = 0;
        const size_t SEARCH_BUFFER_SIZE = 8192;
        const size_t LOOK_AHEAD_BUFFER_SIZE = 255;

        while (i < input.length()) {
            LZ77Triple match = findLongestMatch(input, i, SEARCH_BUFFER_SIZE, LOOK_AHEAD_BUFFER_SIZE);

            if (match.length == 0) {
                triples.push_back(LZ77Triple(0, 0, input[i]));
                i++;
            }
            else {
                triples.push_back(match);
                i += match.length + (match.next_char != 0 ? 1 : 0);
            }
        }

        if (triples.size() * 4 > input.length()) {
            triples.clear();
            triples.reserve(input.length());
            for (size_t j = 0; j < input.length(); ++j) {
                triples.push_back(LZ77Triple(0, 0, input[j]));
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
    auto compression_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    CompressionResult result;
    result.algorithm_name = "Advanced LZ77";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    result.compression_ratio = compressed_data.empty() ? 1.0 : static_cast<double>(input.size()) / compressed_data.size();
    result.compression_time_ms = static_cast<double>(compression_time.count());

    auto decomp_start = chrono::high_resolution_clock::now();
    string decompressed = milyaeva_decompress(compressed_data);
    auto decomp_end = chrono::high_resolution_clock::now();
    auto decompression_time = chrono::duration_cast<chrono::milliseconds>(decomp_end - decomp_start);

    result.decompression_time_ms = static_cast<double>(decompression_time.count());
    result.integrity_ok = (input == decompressed);

    if (!result.integrity_ok) {
        result.compression_ratio = 1.0;
    }

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
    if (data_part.size() < num_triples * 4) return "";

    string result;
    result.reserve(num_triples * 3);
    size_t pos = 0;

    for (size_t i = 0; i < num_triples && pos < data_part.size(); ++i) {
        LZ77Triple triple = unpackTriple(data_part, pos);

        if (triple.offset == 0 && triple.length == 0) {
            if (triple.next_char != 0) {
                result += triple.next_char