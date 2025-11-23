#include "CompressionAlgorithms.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <sstream>
#include <chrono>

// Структура для узла дерева Хаффмана
struct HuffmanNode {
    char data;
    int freq;
    HuffmanNode* left, * right;

    HuffmanNode(char data, int freq) {
        left = right = nullptr;
        this->data = data;
        this->freq = freq;
    }
    // Деструктор для очистки памяти
    ~HuffmanNode() {
        delete left;
        delete right;
    }
};

// Структура для сравнения в очереди с приоритетом
struct compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return (l->freq > r->freq);
    }
};

// Функция для построения дерева Хаффмана
HuffmanNode* buildHuffmanTree(const std::map<char, int>& freq) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, compare> minHeap;
    for (auto pair : freq) {
        minHeap.push(new HuffmanNode(pair.first, pair.second));
    }

    if (minHeap.empty()) {
        return nullptr;
    }

    while (minHeap.size() != 1) {
        HuffmanNode* left = minHeap.top(); minHeap.pop();
        HuffmanNode* right = minHeap.top(); minHeap.pop();

        HuffmanNode* top = new HuffmanNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        minHeap.push(top);
    }
    return minHeap.top();
}

// Функция для генерации кодов Хаффмана
void generateHuffmanCodes(HuffmanNode* root, std::string str, std::map<char, std::string>& huffmanCode) {
    if (!root) return;
    if (root->data != '$') {
        huffmanCode[root->data] = str;
    }
    generateHuffmanCodes(root->left, str + "0", huffmanCode);
    generateHuffmanCodes(root->right, str + "1", huffmanCode);
}

// Функция сжатия
CompressionResult kolesnikov_compress(const std::string& input) {
    auto start_time = std::chrono::high_resolution_clock::now();

    if (input.empty()) {
        CompressionResult result;
        result.algorithm_name = "Huffman (Kolesnikov)";
        result.original_size = 0;
        result.compressed_size = 0;
        result.compression_ratio = 1.0;
        result.compression_time_ms = 0;
        result.decompression_time_ms = 0;
        result.integrity_ok = true;
        return result;
    }

    // 1. Подсчет частот
    std::map<char, int> freq;
    for (char c : input) {
        freq[c]++;
    }

    // 2. Построение дерева Хаффмана
    HuffmanNode* root = buildHuffmanTree(freq);

    // 3. Генерация кодов
    std::map<char, std::string> huffmanCode;
    generateHuffmanCodes(root, "", huffmanCode);

    // 4. Сериализация таблицы частот
    std::stringstream serialized_data;
    serialized_data << freq.size();
    for (auto const& [key, val] : freq) {
        serialized_data << ';' << key << ',' << val;
    }
    serialized_data << '|';

    // 5. Кодирование входной строки
    for (char c : input) {
        serialized_data << huffmanCode[c];
    }
    std::string compressed_data = serialized_data.str();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto compression_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Тест распаковки
    auto decomp_start = std::chrono::high_resolution_clock::now();
    std::string decompressed = kolesnikov_decompress(compressed_data);
    auto decomp_end = std::chrono::high_resolution_clock::now();
    auto decompression_time = std::chrono::duration_cast<std::chrono::milliseconds>(decomp_end - decomp_start);

    delete root; // Очистка памяти

    CompressionResult result;
    result.algorithm_name = "Huffman (Kolesnikov)";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    // ИСПРАВЛЕНИЕ: единый формат вычисления коэффициента сжатия
    result.compression_ratio = (input.size() > 0 && compressed_data.size() > 0) ?
        (double)input.size() / compressed_data.size() : 1.0;
    result.compression_time_ms = compression_time.count();
    result.decompression_time_ms = decompression_time.count();
    result.integrity_ok = (input == decompressed);

    return result;
}

// Функция распаковки
std::string kolesnikov_decompress(const std::string& compressed) {
    if (compressed.empty()) {
        return "";
    }

    std::stringstream ss(compressed);
    std::string freq_part;
    std::getline(ss, freq_part, '|');

    // 1. Десериализация таблицы частот
    std::stringstream freq_stream(freq_part);
    int freq_count;
    freq_stream >> freq_count;

    std::map<char, int> freq;
    for (int i = 0; i < freq_count; ++i) {
        char separator, ch;
        int f;
        // ИСПРАВЛЕНИЕ: правильное чтение формата char,freq
        if (freq_stream.peek() == ';') {
            freq_stream.get(); // пропускаем ';'
        }
        freq_stream.get(ch); // читаем символ
        if (freq_stream.peek() == ',') {
            freq_stream.get(); // пропускаем ','
        }
        freq_stream >> f;
        freq[ch] = f;
    }

    // 2. Восстановление дерева Хаффмана
    HuffmanNode* root = buildHuffmanTree(freq);
    if (!root) {
        return "";
    }

    // 3. Декодирование остальной части строки
    std::string encoded_data;
    std::getline(ss, encoded_data);

    std::string decoded_string;
    HuffmanNode* current = root;
    for (char bit : encoded_data) {
        if (bit == '0') {
            current = current->left;
        }
        else {
            current = current->right;
        }

        if (current && current->left == nullptr && current->right == nullptr) {
            decoded_string += current->data;
            current = root;
        }
    }

    delete root; // Очистка памяти

    return decoded_string;
}