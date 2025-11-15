// milyaeva_arithmetic coding.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <map>


using namespace std;

// Функция для подсчета вероятностей символов
map<char, pair<double, double>> calculateProbabilities(const string& message) {
    map<char, int> counts;
    for (char c : message) {
        counts[c]++;
    }
    int total = message.size();
    map<char, pair<double, double>> probabilities;
    double cumulative = 0.0;
    for (auto& kv : counts) {
        double prob = static_cast<double>(kv.second) / total;
        probabilities[kv.first] = { cumulative, cumulative + prob };
        cumulative += prob;
    }
    return probabilities;
}


int main() {
    string message;
    setlocale(LC_ALL, "Russian");
    cout << "Введите строку для кодирования: ";
    getline(cin, message);

    // Подсчет вероятностей
    auto probabilities = calculateProbabilities(message);

    cout << "\nВероятности символов:\n";
    for (auto& kv : probabilities) {
        cout << "'" << kv.first << "': [" << kv.second.first << ", " << kv.second.second << ")\n";
    }
    return 0;
}