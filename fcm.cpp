/* fcm.cpp - Finite-Context Model (FCM) for Information Content Calculation */
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cmath>

using namespace std;

class FCM {
public:
    int k;
    double alpha;
    unordered_map<string, unordered_map<char, int>> context_counts;
    unordered_map<string, int> total_counts;

public:
    FCM(int k, double alpha) : k(k), alpha(alpha) {}

    void train(const string &text) {
        for (size_t i = k; i < text.size(); ++i) {
            string context = text.substr(i - k, k);
            char symbol = text[i];
            context_counts[context][symbol]++;
            total_counts[context]++;
        }
    }

    double compute_entropy(const string &text) {
        double H = 0.0;
        for (size_t i = k; i < text.size(); ++i) {
            string context = text.substr(i - k, k);
            char symbol = text[i];
            int count = context_counts[context][symbol] + alpha;
            int total = total_counts[context] + alpha * 256; // Assume ASCII
            double prob = (double)count / total;
            H += log2(prob);
        }
        return -H / text.size();
    }
};

int main(int argc, char *argv[]) {
    if (argc != 6) {
        cerr << "Usage: ./fcm <text_file> -k <order> -a <alpha>\n";
        return 1;
    }

    string filename;
    int k = -1;
    double alpha = -1.0;

    // Positional argument for the file
    filename = argv[1];  // The first argument is the filename

    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-k") {
            k = stoi(argv[++i]);   // Next argument is k
        } else if (arg == "-a") {
            alpha = stod(argv[++i]); // Next argument is alpha
        }
    }

    if (filename.empty() || k == -1 || alpha == -1.0) {
        cerr << "Invalid arguments. Usage: ./fcm <text_file> -k <order> -a <alpha>\n";
        return 1;
    }

    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file " << filename << "\n";
        return 1;
    }

    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    FCM model(k, alpha);
    model.train(text);
    //print context counts
    for (auto &context : model.context_counts) {
        cout << context.first << ": ";
        for (auto &symbol : context.second) {
            cout << symbol.first << "=" << symbol.second << " ";
        }
        cout << endl;
    }
    //
    ofstream output("context_counts.csv");
    for (auto &context : model.context_counts) {
        for (auto &symbol : context.second) {
            output << context.first << "," << symbol.first << "," << symbol.second << endl;
        }
    }
    output.close();

    cout << "Average Information Content: " << model.compute_entropy(text) << " bits/symbol" << endl;
    return 0;
}
