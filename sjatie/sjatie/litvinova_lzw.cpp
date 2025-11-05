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

CompressionResult litvinova_compress(const string& input) {
    
    return CompressionResult{};
}

string litvinova_decompress(const string& compressed) {
    
    return compressed;
}