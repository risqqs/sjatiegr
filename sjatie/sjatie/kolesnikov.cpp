#include "CompressionAlgorithms.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <sstream>
#include <chrono>
#include <bitset>

struct HuffmanNode {
    char data;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char data, int freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
    ~HuffmanNode() {
        delete left;
        delete right;
    }
};

struct compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

HuffmanNode* buildHuffmanTree(const std::map<char, int>& freq) {
    if (freq.empty()) return nullptr;

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, compare> minHeap;
    for (const auto& pair : freq) {
        minHeap.push(new HuffmanNode(pair.first, pair.second));
    }

    while (minHeap.size() > 1) {
        HuffmanNode* left = minHeap.top(); minHeap.pop();
        HuffmanNode* right = minHeap.top(); minHeap.pop();

        HuffmanNode* top = new HuffmanNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        minHeap.push(top);
    }
    return minHeap.top();
}

void generateHuffmanCodes(HuffmanNode* root, std::string str, std::map<char, std::string>& huffmanCode) {
    if (!root) return;
    if (root->data != '$') {
        huffmanCode[root->data] = str;
    }
    generateHuffmanCodes(root->left, str + "0", huffmanCode);
    generateHuffmanCodes(root->right, str + "1", huffmanCode);
}

std::string bitsToBytes(const std::string& bits) {
    std::string bytes;
    for (size_t i = 0; i < bits.size(); i += 8) {
        std::string byte = bits.substr(i, 8);
        while (byte.size() < 8) byte += "0";
        bytes += static_cast<char>(std::bitset<8>(byte).to_ulong());
    }
    return bytes;
}

std::string bytesToBits(const std::string& bytes, size_t originalBitLength) {
    std::string bits;
    for (unsigned char byte : bytes) {
        bits += std::bitset<8>(byte).to_string();
    }
    return bits.substr(0, originalBitLength);
}

std::string kolesnikov_decompress(const std::string& compressed) {
    if (compressed.empty()) {
        return "";
    }

    size_t pos = 0;

    if (pos >= compressed.size()) return "";
    unsigned char freq_count = static_cast<unsigned char>(compressed[pos++]);

    std::map<char, int> freq;
    for (int i = 0; i < freq_count; ++i) {
        if (pos + 4 > compressed.size()) return "";
        char ch = compressed[pos++];
        uint32_t freq_val = (static_cast<unsigned char>(compressed[pos]) << 24) |
            (static_cast<unsigned char>(compressed[pos + 1]) << 16) |
            (static_cast<unsigned char>(compressed[pos + 2]) << 8) |
            static_cast<unsigned char>(compressed[pos + 3]);
        pos += 4;
        freq[ch] = freq_val;
    }

    if (pos + 4 > compressed.size()) return "";
    uint32_t bit_length = (static_cast<unsigned char>(compressed[pos]) << 24) |
        (static_cast<unsigned char>(compressed[pos + 1]) << 16) |
        (static_cast<unsigned char>(compressed[pos + 2]) << 8) |
        static_cast<unsigned char>(compressed[pos + 3]);
    pos += 4;

    HuffmanNode* root = buildHuffmanTree(freq);
    if (!root) {
        return "";
    }

    std::string encoded_data = compressed.substr(pos);
    std::string bits = bytesToBits(encoded_data, bit_length);

    std::string decoded_string;
    HuffmanNode* current = root;
    for (char bit : bits) {
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

    delete root;
    return decoded_string;
}

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

    std::map<char, int> freq;
    for (char c : input) {
        freq[c]++;
    }

    HuffmanNode* root = buildHuffmanTree(freq);
    if (!root) {
        CompressionResult result;
        result.algorithm_name = "Huffman (Kolesnikov)";
        result.original_size = input.size();
        result.compressed_size = 0;
        result.compression_ratio = 0.0;
        result.compression_time_ms = 0;
        result.decompression_time_ms = 0;
        result.integrity_ok = false;
        return result;
    }

    std::map<char, std::string> huffmanCode;
    generateHuffmanCodes(root, "", huffmanCode);

    std::string encoded_bits;
    for (char c : input) {
        encoded_bits += huffmanCode[c];
    }

    std::ostringstream header;
    header << static_cast<char>(freq.size());

    for (const auto& p : freq) {
        header << p.first;
        uint32_t freq_val = p.second;
        header << static_cast<char>((freq_val >> 24) & 0xFF);
        header << static_cast<char>((freq_val >> 16) & 0xFF);
        header << static_cast<char>((freq_val >> 8) & 0xFF);
        header << static_cast<char>(freq_val & 0xFF);
    }

    uint32_t bit_length = static_cast<uint32_t>(encoded_bits.size());
    header << static_cast<char>((bit_length >> 24) & 0xFF);
    header << static_cast<char>((bit_length >> 16) & 0xFF);
    header << static_cast<char>((bit_length >> 8) & 0xFF);
    header << static_cast<char>(bit_length & 0xFF);

    std::string header_str = header.str();
    std::string compressed_bits = bitsToBytes(encoded_bits);
    std::string compressed_data = header_str + compressed_bits;

    auto end_time = std::chrono::high_resolution_clock::now();
    auto compression_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    auto decomp_start = std::chrono::high_resolution_clock::now();
    std::string decompressed = kolesnikov_decompress(compressed_data);
    auto decomp_end = std::chrono::high_resolution_clock::now();
    auto decompression_time = std::chrono::duration_cast<std::chrono::microseconds>(decomp_end - decomp_start);

    delete root;

    CompressionResult result;
    result.algorithm_name = "Huffman (Kolesnikov)";
    result.original_size = input.size();
    result.compressed_size = compressed_data.size();
    result.compression_ratio = (input.size() > 0) ? static_cast<double>(input.size()) / compressed_data.size() : 1.0;
    result.compression_time_ms = static_cast<double>(compression_time.count()) / 1000.0;
    result.decompression_time_ms = static_cast<double>(decompression_time.count()) / 1000.0;
    result.integrity_ok = (input == decompressed);

    return result;
}