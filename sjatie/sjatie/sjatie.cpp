#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include "CompressionAlgorithms.h"

using namespace std;

string readFile(const string& filename) {
    ifstream file(filename);
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

void printResult(const CompressionResult& result) {
    cout << left << setw(25) << result.algorithm_name;
    cout << right << setw(10) << result.original_size;
    cout << right << setw(10) << result.compressed_size;
    cout << right << setw(12) << fixed << setprecision(2) << result.compression_ratio;
    cout << right << setw(15) << fixed << setprecision(3) << result.compression_time_ms;
    cout << right << setw(15) << fixed << setprecision(3) << result.decompression_time_ms;
    cout << right << setw(10) << (result.integrity_ok ? "PASS" : "FAIL");
    cout << endl;
}

int main() {
    string filename = "data6.txt";
    string text = readFile(filename);

    cout << "COMPRESSION ALGORITHMS TEST" << endl;
    cout << "File: " << filename << endl;
    cout << "Text size: " << text.size() << " bytes" << endl;
    cout << endl;

    cout << left << setw(25) << "Algorithm";
    cout << right << setw(10) << "Original";
    cout << right << setw(10) << "Compressed";
    cout << right << setw(12) << "Ratio";
    cout << right << setw(15) << "Cmp(ms)";
    cout << right << setw(15) << "Dec(ms)";
    cout << right << setw(10) << "Integrity";
    cout << endl;

    cout << string(97, '-') << endl;

    CompressionResult r1 = litvinova_compress(text);
    CompressionResult r2 = milyaeva_compress(text);
    CompressionResult r3 = doni_compress(text);
    CompressionResult r4 = kolesnikov_compress(text);

    printResult(r1);
    printResult(r2);
    printResult(r3);
    printResult(r4);

    return 0;
}