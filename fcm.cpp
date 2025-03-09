#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

class FCM {
public:
    int k;
    double alpha;
    unordered_map<string, unordered_map<char, int>> context_counts;
    unordered_map<string, int> total_counts;
    unordered_set<char> alphabet;

    FCM(int k, double alpha) : k(k), alpha(alpha) {}

    void train(const string &text) {
        for (size_t i = k; i < text.size(); ++i) {
            string context = text.substr(i - k, k);
            char symbol = text[i];
            context_counts[context][symbol]++;
            total_counts[context]++;
            alphabet.insert(symbol);
        }
    }

    double compute_entropy(const string &text, const string &output_filename) {
        double H = 0.0;
        ofstream entropy_output(output_filename);
        entropy_output << "position,entropy_value\n";

        size_t alphabet_size = alphabet.size();
        double smoothing_factor = alpha * alphabet_size;

        for (size_t i = k; i < text.size(); ++i) {
            string context = text.substr(i - k, k);
            char symbol = text[i];

            int count = context_counts[context][symbol] + alpha;
            int total = total_counts[context] + smoothing_factor;
            double prob = (double)count / total;
            double entropy_value = -log2(prob);

            H += log2(prob);
            entropy_output << i << "," << entropy_value << "\n";
        }
        entropy_output.close();
        return -H / text.size();
    }
};

int main(int argc, char *argv[]) {
    if (argc < 5) {
        cerr << "Usage: ./fcm <text_file> -k <order> -a <alpha>\n";
        return 1;
    }

    string filename = argv[1];
    int k = stoi(argv[3]);
    double alpha = stod(argv[5]);

    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error opening file " << filename << "\n";
        return 1;
    }

    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    FCM model(k, alpha);
    model.train(text);

    cout << "Alphabet: ";
    for (char c : model.alphabet) {
        cout << c << " ";
    }
    cout << endl;


    cout << "Alphabet size: " << model.alphabet.size() << endl;
    cout << "Average Information Content: " << model.compute_entropy(text, "entropy_data.csv") << " bits/symbol" << endl;
    return 0;
}
