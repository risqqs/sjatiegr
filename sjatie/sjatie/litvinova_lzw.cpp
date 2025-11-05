#include "CompressionAlgorithms.h"

#include <unordered_map>
#include <sstream>

using namespace std;

string litvinova_compress_impl(const string& s) {
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

string litvinova_decompress_impl(const string& compressed) {
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
    
    return CompressionResult{};
}

string litvinova_decompress(const string& compressed) {
    
    return compressed;
}