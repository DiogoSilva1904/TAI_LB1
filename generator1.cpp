#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <random>
#include <string>
#include <cstdlib>

using namespace std;

class Generator {
public:
    int k;
    double alpha;
    unordered_map<string, unordered_map<char, int>> context_counts;
    unordered_map<string, int> total_counts;
    unordered_map<char, int> global_counts;
    int global_total = 0;

    // Persistent random generator.
    mt19937 gen;

    Generator(int k, double alpha) : k(k), alpha(alpha), gen(random_device{}()) {}

    void load_model(const string &filename) {
        ifstream model_file(filename);
        if (!model_file) {
            cerr << "Error opening model file!\n";
            exit(1);
        }

        string line;
        cout << "Loading model from " << filename << "...\n";

        while (getline(model_file, line)) {
            // Find the last space (separator before count)
            size_t pos_last = line.rfind(' ');
            if (pos_last == string::npos) {
                cerr << "Error parsing line (no count found): " << line << "\n";
                continue;
            }
            string count_str = line.substr(pos_last + 1);
            int count = stoi(count_str);

            // Remove the count (and its preceding space) from the line
            string remaining = line.substr(0, pos_last);

            // Now find the last space in the remaining string (separator before symbol)
            size_t pos_symbol = remaining.rfind(' ');
            if (pos_symbol == string::npos) {
                cerr << "Error parsing line (no symbol found): " << line << "\n";
                continue;
            }

            // The symbol is the character right after this space.
            string symbol_str = remaining.substr(pos_symbol + 1);
            if (symbol_str.empty()) {
                cerr << "Error: empty symbol in line: " << line << "\n";
                continue;
            }
            char symbol = symbol_str[0];

            // The context is the substring before the symbol separator.
            string context = remaining.substr(0, pos_symbol);

            // Update the model maps.
            context_counts[context][symbol] = count;
            total_counts[context] += count;
        }
        
        model_file.close();
        cout << "Model loading complete.\n";
    }


    // Helper: tries to find a matching context by backing off.
    // If no match is found, returns an empty string.
    string find_fallback_context(const string &context) {
        string ctx = context;
        while (!ctx.empty() && context_counts.find(ctx) == context_counts.end()) {
            ctx = ctx.substr(1); // drop the first character
        }
        return ctx;
    }

    // Generate the next character using the context or a fallback.
    char generate_next(const string &context) {
        string effective_context = context;
        if (context_counts.find(effective_context) == context_counts.end()) {
            effective_context = find_fallback_context(context);
            if (effective_context.empty()) {
                // Fallback to overall (global) distribution.
                return generate_from_global();
            } else {
                cerr << "Warning: Falling back from context '" << context 
                     << "' to lower-order context '" << effective_context << "'\n";
            }
        }

        // Use effective_context.
        int observedSymbols = context_counts[effective_context].size();
        int total = total_counts[effective_context] + static_cast<int>(alpha * observedSymbols);
        vector<pair<char, double>> probabilities;
        double cumulative_prob = 0.0;

        for (const auto &[symbol, count] : context_counts[effective_context]) {
            double prob = (count + alpha) / total;
            cumulative_prob += prob;
            probabilities.emplace_back(symbol, cumulative_prob);
        }

        // Safety check.
        if (probabilities.empty()) {
            return generate_from_global();
        }

        uniform_real_distribution<double> dist(0.0, 1.0);
        double rnd = dist(gen);

        for (const auto &[symbol, cum_prob] : probabilities) {
            if (rnd <= cum_prob) {
                return symbol;
            }
        }

        return probabilities.back().first; // Fallback
    }

    // Generate a character from the global symbol distribution.
    char generate_from_global() {
        if (global_total == 0) return ' '; // should not happen
        vector<pair<char, double>> probabilities;
        double cumulative_prob = 0.0;
        for (const auto &[symbol, count] : global_counts) {
            double prob = (count + alpha) / (global_total + alpha * global_counts.size());
            cumulative_prob += prob;
            probabilities.emplace_back(symbol, cumulative_prob);
        }
        uniform_real_distribution<double> dist(0.0, 1.0);
        double rnd = dist(gen);
        for (const auto &[symbol, cum_prob] : probabilities) {
            if (rnd <= cum_prob) {
                return symbol;
            }
        }
        return probabilities.back().first;
    }

    string generate_text(const string &prior, int size) {
        string output = prior;
        string context = prior;

        for (int i = 0; i < size; ++i) {
            char next_char = generate_next(context);

            // Debug: print each generation step.
            cout << "Step " << i << ": Context = '" << context 
                 << "', Next Char = '" << next_char << "'\n";

            output.push_back(next_char);
            // Ensure we always update context to be the last k characters.
            if (output.size() >= static_cast<size_t>(k))
                context = output.substr(output.size() - k, k);
            else
                context = output; // for initial steps if prior is shorter than k

            // Safety check: if the last 5 characters are the same, break out.
            if (i > 5 && output.substr(output.size() - 5) == string(5, next_char)) {
                cerr << "Warning: Detected repetitive output. Breaking to avoid infinite loop...\n";
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

    if (prior.length() != static_cast<size_t>(k) || k == -1 || alpha < 0 || size <= 0) {
        cerr << "Invalid parameters. Ensure prior has length k and other parameters are valid.\n";
        return 1;
    }

    Generator gen(k, alpha);
    gen.load_model("model.txt");
    string generated_text = gen.generate_text(prior, size);

    cout << "\nGenerated text:\n" << generated_text << endl;
    return 0;
}
