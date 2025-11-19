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