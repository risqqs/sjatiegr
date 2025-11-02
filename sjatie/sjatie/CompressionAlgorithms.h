#pragma once
#include <string>

struct CompressionResult {
    std::string algorithm_name;
    size_t original_size;
    size_t compressed_size;
    double compression_ratio;
    double compression_time_ms;
    double decompression_time_ms;
    bool integrity_ok;
};

CompressionResult huffman_kolesnikov(const std::string& input);
std::string huffman_kolesnikov(const std::string& compressed);