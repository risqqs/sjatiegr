#include <iostream>
#include <string>
#include <list>
#include <algorithm>

using namespace std;

struct SymbolRange {
    char symbol;
    double range_start;
    double range_end;
};

list<SymbolRange> calculateProbabilities(const string& message) {
    list<pair<char, int>> counts;

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

    for (auto& kv : counts) {
        double prob = static_cast<double>(kv.second) / total;
        probabilities.push_back({ kv.first, cumulative, cumulative + prob });
        cumulative += prob;
    }

    return probabilities;
}


int main() {
    string message;
    setlocale(LC_ALL, "Russian");
    cout << "Введите строку для кодирования: ";
    getline(cin, message);

    auto probabilities = calculateProbabilities(message);

    cout << "\nВероятности символов:\n";
    for (const auto& sr : probabilities) {
        cout << "'" << sr.symbol << "': [" << sr.range_start << ", " << sr.range_end << ")\n";
    }


    return 0;
}