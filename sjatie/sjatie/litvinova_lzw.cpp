#include "CompressionAlgorithms.h"

#include <unordered_map>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;

string lzw_compress_str(const string& s) {
    unordered_map<string, int> dict;
    int code = 256;

    for (int i = 0; i < 256; i++) {
        dict[string(1, (char)i)] = i;
    }

    string w = "";
    stringstream out;

    for (char c : s) {
        string wc = w + c;
        if (dict.count(wc)) {
            w = wc;
        }
        else {
            out << dict[w] << " ";
            dict[wc] = code++;
            w = string(1, c);
        }
    }

    if (!w.empty()) {
        out << dict[w];
    }

    return out.str();
}

string lzw_decompress_str(const string& compressed) {
    stringstream ss(compressed);
    vector<int> codes;
    int num;
    while (ss >> num) {
        codes.push_back(num);
    }

    vector<string> dict(256);
    for (int i = 0; i < 256; i++) {
        dict[i] = string(1, (char)i);
    }

    if (codes.empty()) return "";

    string res = dict[codes[0]];
    string prev = dict[codes[0]];

    for (size_t i = 1; i < codes.size(); i++) {
        int curr_code = codes[i];

        string curr;
        if (curr_code < (int)dict.size()) {
            curr = dict[curr_code];
        }
        else {
            curr = prev + prev[0];
        }

        res += curr;
        dict.push_back(prev + curr[0]);
        prev = curr;
    }

    return res;
}

CompressionResult litvinova_compress(const string& input) {
    auto start = chrono::high_resolution_clock::now();
    string compressed = lzw_compress_str(input);
    auto end = chrono::high_resolution_clock::now();
    auto comp_time = chrono::duration_cast<chrono::milliseconds>(end - start);

    auto decomp_start = chrono::high_resolution_clock::now();
    string decompressed = lzw_decompress_str(compressed);
    auto decomp_end = chrono::high_resolution_clock::now();
    auto decomp_time = chrono::duration_cast<chrono::milliseconds>(decomp_end - decomp_start);

    CompressionResult r;
    r.algorithm_name = "LZW";
    r.original_size = input.size();
    r.compressed_size = compressed.size();
    r.compression_ratio = compressed.empty() ? 0.0 : (double)input.size() / compressed.size();
    r.compression_time_ms = comp_time.count();
    r.decompression_time_ms = decomp_time.count();
    r.integrity_ok = (input == decompressed);

    return r;
}

string litvinova_decompress(const string& compressed) {
    
    return lzw_decompress_str(compressed);
}