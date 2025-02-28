#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <random>

using namespace std;

class Generator {
public:
    int k;
    double alpha;
    unordered_map<string, unordered_map<char, int>> context_counts;
    unordered_map<string, int> total_counts;

    Generator(int k, double alpha) : k(k), alpha(alpha) {}

    void load_model(const string &filename) {
        ifstream model_file(filename, ios::binary);
        if (!model_file) {
            cerr << "Error opening model file!\n";
            exit(1);
        }

        size_t map_size;
        model_file.read(reinterpret_cast<char*>(&map_size), sizeof(map_size));

        for (size_t i = 0; i < map_size; ++i) {
            size_t context_length;
            model_file.read(reinterpret_cast<char*>(&context_length), sizeof(context_length));

            string context(context_length, ' ');
            model_file.read(&context[0], context_length);

            size_t symbol_count;
            model_file.read(reinterpret_cast<char*>(&symbol_count), sizeof(symbol_count));

            for (size_t j = 0; j < symbol_count; ++j) {
                char symbol;
                int count;
                model_file.read(reinterpret_cast<char*>(&symbol), sizeof(symbol));
                model_file.read(reinterpret_cast<char*>(&count), sizeof(count));

                context_counts[context][symbol] = count;
                total_counts[context] += count;
            }
        }
        model_file.close();
    }

    char generate_next(const string &context) {
        if (context_counts.find(context) == context_counts.end()) {
            return ' ';
        }

        vector<pair<char, int>> candidates;
        int max_count = 0;
        for (const auto &[symbol, count] : context_counts[context]) {
            candidates.emplace_back(symbol, count);
            max_count = max(max_count, count);
        }

        vector<char> top_choices;
        for (const auto &[symbol, count] : candidates) {
            if (count >= max_count * 0.7) {
                top_choices.push_back(symbol);
            }
        }

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, top_choices.size() - 1);
        return top_choices[dist(gen)];
    }

    string generate_text(const string &prior, int size) {
        string output = prior;
        string context = prior;

        // Ensure the context is exactly k characters long
        if (context.size() < k) {
            context = string(k - context.size(), ' ') + context;  // Pad with spaces
        } else {
            context = context.substr(context.size() - k, k);
        }

        for (int i = 0; i < size; ++i) {
            char next_char = generate_next(context);
            output += next_char;
            context = output.substr(output.size() - k, k);  // Always get the last k characters
        }

        return output;
    }

};

int main(int argc, char *argv[]) {
    if (argc < 7) {
        cerr << "Usage: ./generator -k <order> -a <alpha> -p <prior> -s <size>\n";
        return 1;
    }

    int k = stoi(argv[2]);
    double alpha = stod(argv[4]);
    string prior = argv[6];
    int size = stoi(argv[8]);

    Generator gen(k, alpha);
    gen.load_model("model.bin");
    string generated_text = gen.generate_text(prior, size);

    cout << "Generated text:\n" << generated_text << endl;
    return 0;
}

