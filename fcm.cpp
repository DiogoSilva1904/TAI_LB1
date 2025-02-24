/* fcm.cpp - Finite-Context Model (FCM) for Information Content Calculation */
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <cctype>

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

    void save_model(const string &filename) {
        ofstream model_file(filename);
        for (const auto &context : context_counts) {
            for (const auto &entry : context.second) {
                model_file << context.first << " " << entry.first << " " << entry.second << "\n";
            }
        }
        model_file.close();
    }


    double compute_entropy(const string &text, const string &output_filename) {
        double H = 0.0;
        ofstream entropy_output(output_filename);
        entropy_output << "position,entropy_value" << endl;

        for (size_t i = k; i < text.size(); ++i) {
            string context = text.substr(i - k, k);
            char symbol = text[i];
            int count = context_counts[context][symbol] + alpha;
            int total = total_counts[context] + alpha * 256; // Assume ASCII
            double prob = (double)count / total;
            double entropy_value = -log2(prob);
            H += log2(prob);
            entropy_output << i << "," << entropy_value << endl;
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

    string filename;
    int k = -1;
    double alpha = -1.0;

    filename = argv[1];

    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-k" && i + 1 < argc) {
            k = stoi(argv[++i]);
        } else if (arg == "-a" && i + 1 < argc) {
            alpha = stod(argv[++i]);
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

    string text, line;
    while (getline(file, line)) {
        for (char ch : line) {
            if (isprint(ch)) text += ch;
        }
    }
    file.close();

    FCM model(k, alpha);
    model.train(text);
    model.save_model("model.txt");


    ofstream output("context_counts.csv");
    output << "context,symbol,count" << endl;
    for (auto &context : model.context_counts) {
        for (auto &symbol : context.second) {
            output << context.first << "," << symbol.first << "," << symbol.second << endl;
        }
    }
    output.close();

    cout << "Average Information Content: " << model.compute_entropy(text, "entropy_data.csv") << " bits/symbol" << endl;
    return 0;
}
