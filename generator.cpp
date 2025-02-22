//gerador por maior número de counts
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
        ifstream model_file(filename);
        if (!model_file) {
            cerr << "Error opening model file!\n";
            exit(1);
        }

        string line;
        cout << "Loading model from " << filename << "...\n";

        while (getline(model_file, line)) {
            size_t last_space = line.find_last_of(" ");
            size_t second_last_space = line.find_last_of(" ", last_space - 1);

            if (last_space == string::npos || second_last_space == string::npos) {
                cerr << "Error parsing line: " << line << "\n";
                continue;
            }

            string context = line.substr(0, second_last_space);
            char symbol = line[second_last_space + 1];
            int count = stoi(line.substr(last_space + 1));

            context_counts[context][symbol] = count;
            total_counts[context] += count;

            // Print what is being loaded
            //cout << "Context: '" << context << "', Symbol: '" << symbol 
            //    << "', Count: " << count << "\n";
        }
        
        model_file.close();
        cout << "Model loading complete.\n";
    }


    char generate_next(const string &context) {
        if (context_counts.find(context) == context_counts.end()) {
            cerr << "Warning: Unknown context '" << context << "'\n";
            return ' '; // Fallback for unknown contexts
        }

        vector<pair<char, int>> candidates;
        int max_count = 0;

        // Collect characters with their counts and find the max count
        for (const auto &[symbol, count] : context_counts[context]) {
            candidates.emplace_back(symbol, count);
            if (count > max_count) {
                max_count = count;
            }
        }

        // Filter candidates to only include those with high probability
        vector<char> top_choices;
        for (const auto &[symbol, count] : candidates) {
            if (count >= max_count * 0.7) {  // Allow some variation
                top_choices.push_back(symbol);
            }
        }

        // Randomly choose among the top candidates
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, top_choices.size() - 1);
        return top_choices[dist(gen)];
    }


    string generate_text(const string &prior, int size) {
        string output = prior;
        string context = prior;
        unordered_map<string, int> context_history;  // Track seen contexts

        for (int i = 0; i < size; ++i) {

            char next_char = generate_next(context);

            // Debugging: Print step-by-step generation
            cout << "Step " << i << ": Context = '" << context << "', Next Char = '" << next_char << "'\n";

            output += next_char;
            context = output.substr(output.size() - k, k); // Update context
        }

        return output;
    }


};

int main(int argc, char *argv[]) {
    if (argc < 7) {
        cerr << "Usage: ./generator -k <order> -a <alpha> -p <prior> -s <size>\n";
        return 1;
    }

    int k = -1, size = -1;
    double alpha = -1.0;
    string prior;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-k" && i + 1 < argc) {
            k = stoi(argv[++i]);
        } else if (arg == "-a" && i + 1 < argc) {
            alpha = stod(argv[++i]);
        } else if (arg == "-p" && i + 1 < argc) {
            prior = argv[++i];
        } else if (arg == "-s" && i + 1 < argc) {
            size = stoi(argv[++i]);
        }
    }

    if (prior.length() != k || k == -1 || alpha == -1.0 || size == -1) {
        cerr << "Invalid parameters. Ensure prior has length k.\n";
        return 1;
    }

    Generator gen(k, alpha);
    gen.load_model("model.txt");
    string generated_text = gen.generate_text(prior, size);

    cout << "Generated text:\n" << generated_text << endl;
    return 0;
}

