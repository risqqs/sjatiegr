#include <iostream>
#include <string>
#include "CompressionAlgorithms.h"

int main()
{
    std::string test_data = "Hello World! This is test data for compression algorithms.";

    CompressionResult result1 = huffman_kolesnikov(test_data);
    return 0;
}
