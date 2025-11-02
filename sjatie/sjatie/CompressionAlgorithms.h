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

CompressionResult kolesnikov_ñompress(const std::string& input);
std::string kolesnikov_decompress(const std::string& compressed);

CompressionResult litvinova_compress(const std::string& input);
std::string litvinova_decompress(const std::string& compressed);

CompressionResult milyaeva_compress(const std::string& input);
std::string milyaeva_decompress(const std::string& compressed);

CompressionResult doni_compress(const std::string& input);
std::string doni_decompress(const std::string& compressed);