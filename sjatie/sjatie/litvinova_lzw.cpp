#include "CompressionAlgorithms.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <sstream>
#include <string>
#include <bitset>
#include <climits>

using namespace std;

// --- Вспомогательные функции для LZW ---

// Преобразование вектора целых (кодов) в бинарную строку
string codesToBinaryString(const vector<int>& codes, int initialCodeSize) {
    string bitStream = "";
    int codeSize = initialCodeSize + 1; // Начальный размер кода (9 бит)
    int dict_size = 258; // 256 (символы) + 2 (Clear и End)
    int maxCode = (1 << codeSize); // 2^codeSize

    for (int code : codes) {
        bitStream += bitset<32>(code).to_string().substr(32 - codeSize);
        // Увеличиваем размер кода, если словарь заполняется
        if (dict_size >= maxCode && maxCode < 65536) {
            codeSize++;
            maxCode = (1 << codeSize);
        }
        dict_size++;
    }

    // Дополняем нулями до кратности 8
    while (bitStream.size() % 8 != 0) {
        bitStream += '0';
    }

    // Преобразуем битовую строку в байты
    string byteString;
    for (size_t i = 0; i < bitStream.size(); i += 8) {
        string byte_str = bitStream.substr(i, 8);
        byteString += static_cast<char>(bitset<8>(byte_str).to_ulong());
    }
    return byteString;
}

// Преобразование бинарной строки (байтов) в вектор целых (кодов)
vector<int> binaryStringToCodes(const string& binary_str, int initialCodeSize) {
    // Сначала преобразуем байты в битовую строку
    string bitStream;
    for (unsigned char byte : binary_str) {
        bitStream += bitset<8>(byte).to_string();
    }

    vector<int> codes;
    int codeSize = initialCodeSize + 1; // 9 бит
    int bit_pos = 0;
    int maxCode = (1 << codeSize);
    int dict_size = 258; // 256 (символы) + 2 (Clear и End)

    while (bit_pos + codeSize <= bitStream.size()) {
        string code_str = bitStream.substr(bit_pos, codeSize);
        int code = static_cast<int>(bitset<32>(code_str).to_ulong());
        codes.push_back(code);
        bit_pos += codeSize;

        // Увеличиваем размер кода, если словарь заполняется
        if (dict_size >= maxCode && maxCode < 65536) {
            codeSize++;
            maxCode = (1 << codeSize);
        }
        dict_size++;
    }
    return codes;
}


string lzw_compress_binary(const string& s) {
    if (s.empty()) return "";

    const int CLEAR_CODE = 256;
    const int END_CODE = 257;

    unordered_map<string, int> dict;
    int dict_size = 258; // 256 (символы) + 2 (Clear и End)
    int code_size = 9; // Начальный размер кода 9 бит

    // Инициализация словаря (0-255) и специальных кодов
    for (int i = 0; i < 256; i++) {
        dict[string(1, static_cast<char>(i))] = i;
    }
    dict["CLEAR"] = CLEAR_CODE;
    dict["END"] = END_CODE;

    string w;
    vector<int> compressed;

    //compressed.push_back(CLEAR_CODE); // Не используем CLEAR в этой простой версии

    for (char c : s) {
        string wc = w + c;
        if (dict.find(wc) != dict.end()) {
            w = wc;
        }
        else {
            compressed.push_back(dict[w]);
            if (dict_size < 65536) { // Максимальный размер словаря
                dict[wc] = dict_size++;
            }
            w = string(1, c);
        }
    }

    if (!w.empty()) {
        compressed.push_back(dict[w]);
    }
    //compressed.push_back(END_CODE); // Не используем END в этой простой версии

    return codesToBinaryString(compressed, 8); // 8 бит для начального размера символа
}

string lzw_decompress_binary(const string& compressed) {
    if (compressed.empty()) return "";

    vector<int> codes = binaryStringToCodes(compressed, 8); // 8 бит для начального размера символа
    if (codes.empty()) return "";

    const int CLEAR_CODE = 256;
    const int END_CODE = 257;

    // Восстановление словаря
    vector<string> dict(258); // 256 (символы) + 2 (Clear и End)
    for (int i = 0; i < 256; i++) {
        dict[i] = string(1, static_cast<char>(i));
    }
    // dict[CLEAR_CODE] и dict[END_CODE] не используются напрямую

    int dict_size = 258;
    int code_size = 9; // Начальный размер кода
    int maxCode = (1 << code_size);

    if (codes.empty()) return "";
    int old_code = codes[0];
    string result = dict[old_code];
    string current_entry = result;

    for (size_t i = 1; i < codes.size(); i++) {
        int code = codes[i];
        string entry;

        if (code == CLEAR_CODE) {
            // Сброс словаря - не реализовано в этой версии
            continue;
        }
        else if (code == END_CODE) {
            // Конец данных - не реализовано в этой версии
            break;
        }
        else if (code < dict_size) {
            entry = dict[code];
        }
        else if (code == dict_size) {
            // Специальный случай: entry = current_entry + current_entry[0]
            entry = current_entry + current_entry[0];
        }
        else {
            // Ошибка
            return "";
        }

        result += entry;

        if (dict_size < 65536) { // Максимальный размер словаря
            dict.push_back(current_entry + entry[0]);
            dict_size++;
        }

        current_entry = entry;
        old_code = code;
    }

    return result;
}

// Функция распаковки
string litvinova_decompress(const string& compressed) {
    if (compressed.empty()) { // Проверка в начале
        return "";
    }
    return lzw_decompress_binary(compressed);
}

// Функция сжатия
CompressionResult litvinova_compress(const string& input) {
    auto start = chrono::high_resolution_clock::now();
    string compressed = lzw_compress_binary(input);
    auto end = chrono::high_resolution_clock::now();
    auto comp_time = chrono::duration_cast<chrono::milliseconds>(end - start);

    auto decomp_start = chrono::high_resolution_clock::now();
    string decompressed = litvinova_decompress(compressed);
    auto decomp_end = chrono::high_resolution_clock::now();
    auto decomp_time = chrono::duration_cast<chrono::milliseconds>(decomp_end - decomp_start);

    CompressionResult r;
    r.algorithm_name = "LZW Binary";
    r.original_size = input.size();
    r.compressed_size = compressed.size();
    r.compression_ratio = compressed.empty() ? 1.0 : static_cast<double>(input.size()) / compressed.size();
    r.compression_time_ms = static_cast<double>(comp_time.count());
    r.decompression_time_ms = static_cast<double>(decomp_time.count());
    r.integrity_ok = (input == decompressed);

    return r;
}