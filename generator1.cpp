//generator com métodos probabilísticos
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

        // Only sum probabilities over observed characters
        int total = total_counts[context] + alpha * context_counts[context].size();
        vector<pair<char, double>> probabilities;
        double cumulative_prob = 0.0;

        for (const auto &[symbol, count] : context_counts[context]) {
            double prob = (count + alpha) / total;
            cumulative_prob += prob;
            probabilities.emplace_back(symbol, cumulative_prob);
        }

        if (probabilities.empty()) {
            cerr << "Warning: No valid symbols found for context '" << context << "'\n";
            return ' '; // Avoid crashing
        }

        // Randomly select based on cumulative probability
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> dist(0.0, 1.0);
        double rnd = dist(gen);

        for (const auto &[symbol, prob] : probabilities) {
            if (rnd <= prob) {
                return symbol;
            }
        }

        return probabilities.back().first; // Fallback
    }



    string generate_text(const string &prior, int size) {
        string output = prior;
        string context = prior;

        for (int i = 0; i < size; ++i) {
            char next_char = generate_next(context);

            // Debugging: Print step-by-step generation
            cout << "Step " << i << ": Context = '" << context << "', Next Char = '" << next_char << "'\n";

            output += next_char;
            context = output.substr(output.size() - k, k); // Update context

            // Safety Check: If stuck generating spaces or same char infinitely, break
            if (i > 5 && output.substr(output.size() - 5) == string(5, next_char)) {
                cerr << "Warning: Possible infinite loop detected! Breaking...\n";
                break;
            }
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

