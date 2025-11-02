#include <iostream>
#include <string>
#include "CompressionAlgorithms.h"

int main()
{
    std::string test_data = "Hello World! This is test data for compression algorithms.";

    CompressionResult result1 = kolesnikov_сompress(test_data);
    CompressionResult result2 = litvinova_compress(test_data);
    CompressionResult result3 = milyaeva_compress(test_data);
    CompressionResult result4 = doni_compress(test_data);

    return 0;
}
