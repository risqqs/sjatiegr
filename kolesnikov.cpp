#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <fstream>

// Структура для узла дерева Хаффмана
struct Node {
    char data;
    int freq;
    Node *left, *right;

    Node(char data, int freq) {
        left = right = nullptr;
        this->data = data;
        this->freq = freq;
    }
};

// Функция для сравнения двух узлов. Нужна для очереди с приоритетом.
struct compare {
    bool operator()(Node* l, Node* r) {
        return (l->freq > r->freq);
    }
};

// Функция для печати кодов Хаффмана
void printCodes(struct Node* root, std::string str) {
    if (!root)
        return;

    if (root->data != '$')
        std::cout << root->data << ": " << str << "\n";

    printCodes(root->left, str + "0");
    printCodes(root->right, str + "1");
}

// Функция для построения дерева Хаффмана и его печати
void buildHuffmanTree(std::string text) {
    std::map<char, int> freq;
    for (char c : text) {
        freq[c]++;
    }

    std::priority_queue<Node*, std::vector<Node*>, compare> minHeap;

    for (auto pair : freq) {
        minHeap.push(new Node(pair.first, pair.second));
    }

    while (minHeap.size() != 1) {
        Node* left = minHeap.top();
        minHeap.pop();

        Node* right = minHeap.top();
        minHeap.pop();

        Node* top = new Node('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        minHeap.push(top);
    }

    printCodes(minHeap.top(), "");
}

// --- Функции для кодирования и декодирования ---

void generateCodes(Node* root, std::string str, std::map<char, std::string>& huffmanCode) {
    if (!root)
        return;

    if (root->data != '$') {
        huffmanCode[root->data] = str;
    }

    generateCodes(root->left, str + "0", huffmanCode);
    generateCodes(root->right, str + "1", huffmanCode);
}

std::string huffmanEncode(std::string text, std::map<char, std::string>& huffmanCode) {
    std::string encoded_string = "";
    for (char c : text) {
        encoded_string += huffmanCode[c];
    }
    return encoded_string;
}

std::string huffmanDecode(std::string encoded_string, Node* root) {
    std::string decoded_string = "";
    Node* curr = root;
    for (char bit : encoded_string) {
        if (bit == '0')
            curr = curr->left;
        else
            curr = curr->right;

        if (curr->left == nullptr && curr->right == nullptr) {
            decoded_string += curr->data;
            curr = root;
        }
    }
    return decoded_string;
}


int main() {
    std::ifstream inputFile("text.txt");
    if (!inputFile.is_open()) {
        std::cerr << "Could not open the file - 'text.txt'" << std::endl;
        return EXIT_FAILURE;
    }

    std::string text((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

    // 1. Подсчет частот
    std::map<char, int> freq;
    for (char c : text) {
        freq[c]++;
    }

    // 2. Построение дерева Хаффмана
    std::priority_queue<Node*, std::vector<Node*>, compare> minHeap;
    for (auto pair : freq) {
        minHeap.push(new Node(pair.first, pair.second));
    }

    if (minHeap.empty()) {
        std::cout << "Empty file, nothing to do.\n";
        return 0;
    }

    while (minHeap.size() != 1) {
        Node* left = minHeap.top();
        minHeap.pop();
        Node* right = minHeap.top();
        minHeap.pop();
        Node* top = new Node('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        minHeap.push(top);
    }
    Node* root = minHeap.top();

    // 3. Генерация кодов
    std::map<char, std::string> huffmanCode;
    generateCodes(root, "", huffmanCode);

    std::cout << "Huffman Codes for the entire text:\n";
    for (auto pair : huffmanCode) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }

    // 4. Кодирование
    std::string encoded_string = huffmanEncode(text, huffmanCode);
    std::cout << "\nEncoded string:\n" << encoded_string << "\n";

    // 5. Декодирование
    std::string decoded_string = huffmanDecode(encoded_string, root);
    std::cout << "Decoded string:\n" << decoded_string << "\n\n";

    inputFile.close();
    return 0;
}
