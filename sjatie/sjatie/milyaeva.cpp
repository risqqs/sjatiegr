#include <iostream>
#include <string>
#include <list>
#include <algorithm>

using namespace std;

// Структура для хранения символа и его вероятностного диапазона
struct SymbolRange {
    char symbol;
    double range_start;
    double range_end;
};

// Функция подсчета вероятностей и диапазонов через list
list<SymbolRange> calculateProbabilities(const string& message) {
    list<pair<char, int>> counts;

    // Подсчет количества каждого символа
    for (char c : message) {
        auto it = find_if(counts.begin(), counts.end(),
            c{ return p.first == c; });
        if (it != counts.end()) {
            it->second++;
        }
        else {
            counts.push_back({ c, 1 });
        }
    }

    int total = message.size();

    list<SymbolRange> probabilities;
    double cumulative = 0.0;

    // Вычисляем диапазоны для каждого символа
    for (auto& kv : counts) {
        double prob = static_cast<double>(kv.second) / total;
        probabilities.push_back({ kv.first, cumulative, cumulative + prob });
        cumulative += prob;
    }

    return probabilities;
}

// Функция для поиска диапазона символа
pair<double, double> findRange(const list<SymbolRange>& probabilities, char symbol) {
    for (const auto& sr : probabilities) {
        if (sr.symbol == symbol) {
            return { sr.range_start, sr.range_end };
        }
    }
    // Если символ не найден, возвращаем нулевой диапазон
    return { 0.0, 0.0 };
}

// Функция для сжатия
double arithmeticEncode(const string& message, const list<SymbolRange>& probabilities) {
    double low = 0.0;
    double high = 1.0;

    for (char symbol : message) {
        auto range = findRange(probabilities, symbol);
        double range_width = high - low;
        high = low + range_width * range.second;
        low = low + range_width * range.first;
    }
    return (low + high) / 2;
}

// Функция для распаковки
string arithmeticDecode(double code, size_t message_length, const list<SymbolRange>& probabilities) {
    string decoded_message = "";
    double value = code;

    for (size_t i = 0; i < message_length; ++i) {
        for (const auto& sr : probabilities) {
            if (value >= sr.range_start && value < sr.range_end) {
                decoded_message += sr.symbol;
                double range_width = sr.range_end - sr.range_start;
                value = (value - sr.range_start) / range_width;
                break;
            }
        }
    }

    return decoded_message;
}

int main() {
    string message;
    setlocale(LC_ALL, "Russian");
    cout << "Введите строку для кодирования: ";
    getline(cin, message);

    // Подсчет вероятностей и диапазонов
    auto probabilities = calculateProbabilities(message);

    cout << "\nВероятности символов:\n";
    for (const auto& sr : probabilities) {
        cout << "'" << sr.symbol << "': [" << sr.range_start << ", " << sr.range_end << ")\n";
    }

    // Сжатие
    double code = arithmeticEncode(message, probabilities);
    cout << "\nЗакодированное число: " << code << endl;

    // Распаковка
    string decoded_message = arithmeticDecode(code, message.size(), probabilities);
    cout << "\nДекодированное сообщение: " << decoded_message << endl;

    return 0;
}