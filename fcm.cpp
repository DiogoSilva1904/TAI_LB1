#include <iostream>
#include <fstream>
#include <unordered_map>
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
        ofstream model_file(filename, ios::binary);
        if (!model_file) {
            cerr << "Error opening model file for writing!\n";
            exit(1);
        }

        size_t map_size = context_counts.size();
        model_file.write(reinterpret_cast<const char*>(&map_size), sizeof(map_size));

        for (const auto &[context, symbols] : context_counts) {
            size_t context_length = context.length();
            model_file.write(reinterpret_cast<const char*>(&context_length), sizeof(context_length));
            model_file.write(context.data(), context_length);

            size_t symbol_count = symbols.size();
            model_file.write(reinterpret_cast<const char*>(&symbol_count), sizeof(symbol_count));

            for (const auto &[symbol, count] : symbols) {
                model_file.write(reinterpret_cast<const char*>(&symbol), sizeof(symbol));
                model_file.write(reinterpret_cast<const char*>(&count), sizeof(count));
            }
        }
        model_file.close();
    }

    double compute_entropy(const string &text, const string &output_filename) {
        double H = 0.0;
        ofstream entropy_output(output_filename);
        entropy_output << "position,entropy_value\n";

        for (size_t i = k; i < text.size(); ++i) {
            string context = text.substr(i - k, k);
            char symbol = text[i];

            int count = context_counts[context][symbol] + alpha;
            int total = total_counts[context] + alpha * 256;
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
    model.save_model("model.bin");

    cout << "Average Information Content: " << model.compute_entropy(text, "entropy_data.csv") << " bits/symbol" << endl;
    return 0;
}

