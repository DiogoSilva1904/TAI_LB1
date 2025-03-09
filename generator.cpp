#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace std;

class Generator {
public:
    vector<int> k_values; // Multiple k values
    int max_k; // The maximum k value
    double alpha;
    double threshold; // Threshold for candidate character selection
    vector<unordered_map<string, unordered_map<char, int>>> models; // One model per k value
    vector<unordered_map<string, int>> total_counts; // One total count per k value

    Generator(const vector<int>& ks, double alpha, double threshold = 0.6) : k_values(ks), alpha(alpha), threshold(threshold) {
        // Sort k values in descending order
        sort(k_values.begin(), k_values.end(), greater<int>());
        max_k = k_values[0];
        
        // Initialize models for each k value
        models.resize(k_values.size());
        total_counts.resize(k_values.size());
    }

    void train(const string &text) {
        if (text.size() <= max_k) {
            cerr << "Error: Training text must be longer than the maximum context length (k=" << max_k << ").\n";
            exit(1);
        }

        // Train models for each k value
        for (size_t i = 0; i < k_values.size(); i++) {
            int k = k_values[i];
            for (size_t j = k; j < text.size(); ++j) {
                string context = text.substr(j - k, k);
                char symbol = text[j];
                models[i][context][symbol]++;
                total_counts[i][context]++;
            }
        }
    }

    void save_model(const string &filename) {
        ofstream model_file(filename, ios::binary);
        if (!model_file) {
            cerr << "Error opening model file for writing!\n";
            exit(1);
        }

        // Save number of k values
        size_t num_k_values = k_values.size();
        model_file.write(reinterpret_cast<const char*>(&num_k_values), sizeof(num_k_values));

        // Save each k value
        for (size_t i = 0; i < num_k_values; i++) {
            int k = k_values[i];
            model_file.write(reinterpret_cast<const char*>(&k), sizeof(k));
        }

        // Save models for each k value
        for (size_t m = 0; m < num_k_values; m++) {
            size_t map_size = models[m].size();
            model_file.write(reinterpret_cast<const char*>(&map_size), sizeof(map_size));

            for (const auto &[context, symbols] : models[m]) {
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
        }
        model_file.close();
    }

    void load_model(const string &filename) {
        ifstream model_file(filename, ios::binary);
        if (!model_file) {
            cerr << "Error opening model file!\n";
            exit(1);
        }

        // Read number of k values
        size_t num_k_values;
        model_file.read(reinterpret_cast<char*>(&num_k_values), sizeof(num_k_values));
        
        // Read k values
        k_values.resize(num_k_values);
        for (size_t i = 0; i < num_k_values; i++) {
            model_file.read(reinterpret_cast<char*>(&k_values[i]), sizeof(int));
        }
        
        // Sort k values in descending order
        sort(k_values.begin(), k_values.end(), greater<int>());
        max_k = k_values[0];
        
        models.resize(num_k_values);
        total_counts.resize(num_k_values);

        // Read models for each k value
        for (size_t m = 0; m < num_k_values; m++) {
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

                    models[m][context][symbol] = count;
                    total_counts[m][context] += count;
                }
            }
        }
        model_file.close();
    }

    pair<char, int> generate_next(const string &context) {
        // Try each model in order (from highest k to lowest)
        for (size_t m = 0; m < k_values.size(); m++) {
            int k = k_values[m];
            
            // Extract the appropriate context length
            string current_context;
            if (context.length() >= k) {
                current_context = context.substr(context.length() - k, k);
            } else {
                continue; // Context too short for this k value
            }
            
            // Count consecutive spaces at the end of context
            int trailing_spaces = 0;
            for (int i = current_context.size() - 1; i >= 0; i--) {
                if (current_context[i] == ' ')
                    trailing_spaces++;
                else
                    break;
            }
            
            // If context exists in this model
            if (models[m].find(current_context) != models[m].end() && !models[m][current_context].empty()) {
                vector<pair<char, double>> candidates;
                double max_prob = 0.0;
                // Use total_counts to get the raw total count for this context
                int raw_total = total_counts[m][current_context];
                // Number of candidate symbols for this context
                int candidate_count = models[m][current_context].size();
                // Compute smoothed total: add alpha for each candidate symbol
                double smoothed_total = raw_total + alpha * candidate_count;
                
                // Compute smoothed probability for each candidate symbol
                for (const auto &[symbol, count] : models[m][current_context]) {
                    // Default probability calculation
                    double prob = (count + alpha) / smoothed_total;
                    // Penalize space if we already have several trailing spaces
                    if (trailing_spaces >= 3 && symbol == ' ')
                        prob = ((count / 10.0) + alpha) / smoothed_total;
                    
                    candidates.emplace_back(symbol, prob);
                    max_prob = max(max_prob, prob);
                }
                
                // Select candidates with probabilities at least threshold * max_prob
                vector<char> top_choices;
                for (const auto &[symbol, prob] : candidates) {
                    if (prob >= max_prob * threshold) {
                        top_choices.push_back(symbol);
                    }
                }
                
                if (!top_choices.empty()) {
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<int> dist(0, top_choices.size() - 1);
                    return {top_choices[dist(gen)], k};
                }
            }
        }
        
        // If no context match found in any model, return a space
        return {' ', 0};
    }

    string generate_text(const string &prior, int size, int delay_ms = 50, const string &write_file = "k_values.csv") {
        if (prior.size() < max_k) {
            cerr << "Error: The prior context must be at least " << max_k << " characters long." << endl;
            exit(1);
        }

        string output = prior;
        string context = prior;

        ofstream k_file(write_file);
        if (!k_file) {
            cerr << "Error opening " << write_file << " for writing!" << endl;
            exit(1);
        }

        // Write CSV header
        k_file << "Step,k_used\n";
    
        // First print the prior context
        cout << prior;
        cout << flush;

        for (int i = 0; i < size; ++i) {
            // Generate next character and get which k was used
            auto [next_char, k_used] = generate_next(context);
            
            // Print the character immediately with a small delay for visual effect
            cout << next_char << flush;
            
            // print which k was used (can enable for debug)
            //cout << "[k=" << k_used << "]" << flush;

            k_file << i + 1 << "," << k_used << "\n";
            
            // Add a small delay for vi
            if (delay_ms > 0) {
                this_thread::sleep_for(chrono::milliseconds(delay_ms));
            }
            
            // Update the output and context
            output += next_char;
            context = output.substr(output.size() - max_k, max_k);
        }

        k_file.close();
        
        cout << endl << endl;
        return output;
    }
};

void print_usage() {
    cerr << "Usage:\n"
         << "Train mode: ./enerator train -f <text_file> -k <max_order> -a <alpha>\n"
         << "Generate mode: ./generator generate -k <max_order> -a <alpha> -p <prior> -s <size> [-d <delay_ms>] [-t <threshold>]\n"
         << "Example: ./generator train -f input.txt -k 10 -a 0.1\n"
         << "Example: ./generator generate -k 10 -a 0.1 -p \"Hello world this is a test\" -s 100 -d 50 -t 0.2\n";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    string mode = argv[1];
    string write_file = "k_values.csv";  // Default output file

    if (mode == "train") {
        if (argc < 7) {
            print_usage();
            return 1;
        }

        string filename;
        int max_k = 0;
        double alpha = 0.0;

        for (int i = 2; i < argc; i += 2) {
            string flag = argv[i];
            if (flag == "-f" && i + 1 < argc) {
                filename = argv[i + 1];
            } else if (flag == "-k" && i + 1 < argc) {
                max_k = stoi(argv[i + 1]);
            } else if (flag == "-a" && i + 1 < argc) {
                alpha = stod(argv[i + 1]);
            }
        }

        if (filename.empty() || max_k <= 0 || alpha <= 0) {
            cerr << "Invalid parameters for training.\n";
            print_usage();
            return 1;
        }

        // Create a range of k values: 2, 3, ..., max_k
        vector<int> k_values;
        for (int k = 2; k <= max_k; k++) {
            if (k == 2 || k == 3 || k == 5 || k == 8 || k == max_k || (k % 5 == 0 && k < max_k)) {
                k_values.push_back(k);
            }
        }
        if (find(k_values.begin(), k_values.end(), max_k) == k_values.end()) {
            k_values.push_back(max_k);
        }

        // Read the input file
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr << "Error opening file " << filename << "\n";
            return 1;
        }

        string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        if (text.size() <= max_k) {
            cerr << "Error: The input file must contain more than " << max_k << " characters.\n";
            return 1;
        }

        cout << "Training with k values: ";
        for (int k : k_values) {
            cout << k << " ";
        }
        cout << endl;

        // Create and train the model
        Generator model(k_values, alpha);
        model.train(text);
        model.save_model("multi_k_model.bin");

    } 
    else if (mode == "generate") {
        if (argc < 9) {
            print_usage();
            return 1;
        }

        int max_k = 0;
        double alpha = 0.0;
        string prior;
        int size = 0;
        int delay_ms = 15; // Default delay in milliseconds
        double threshold = 0.6; // Default threshold


        for (int i = 2; i < argc; i += 2) {
            string flag = argv[i];
            if (flag == "-k" && i + 1 < argc) {
                max_k = stoi(argv[i + 1]);
            } else if (flag == "-a" && i + 1 < argc) {
                alpha = stod(argv[i + 1]);
            } else if (flag == "-p" && i + 1 < argc) {
                prior = argv[i + 1];
            } else if (flag == "-s" && i + 1 < argc) {
                size = stoi(argv[i + 1]);
            } else if (flag == "-d" && i + 1 < argc) {
                delay_ms = stoi(argv[i + 1]);
            } else if (flag == "-t" && i + 1 < argc) {
                threshold = stod(argv[i + 1]);
            }
            else if (flag == "-w" && i + 1 < argc) {
                write_file = argv[i + 1];  // Get custom output file name
            }
        }

        if (max_k <= 0 || alpha <= 0 || size <= 0) {
            cerr << "Invalid parameters for generation.\n";
            print_usage();
            return 1;
        }

        // Create a temporary model just to load the actual model
        vector<int> temp_k = {max_k};
        Generator gen(temp_k, alpha, threshold);
        gen.load_model("multi_k_model.bin");

        cout << "Loaded model with k values: ";
        for (int k : gen.k_values) {
            cout << k << " ";
        }
        cout << endl;

        // Validate that the prior context is long enough
        if (prior.size() < gen.max_k) {
            cerr << "Error: The prior context must be at least " << gen.max_k << " characters long." << endl;
            cerr << "Current prior length: " << prior.size() << " characters." << endl;
            cerr << "Please provide a longer prior string or use a smaller k value." << endl;
            return 1;
        }

        // Use the new delay parameter in the generate_text call
        string generated_text = gen.generate_text(prior, size, delay_ms, write_file);
        
        cout << "Generation complete! Total length: " << generated_text.size() << " characters." << endl;
    } 
    else {
        cerr << "Unknown mode: " << mode << ". Use 'train' or 'generate'.\n";
        print_usage();
        return 1;
    }

    return 0;
}