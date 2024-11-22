#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <algorithm>

using namespace std;

// Struct definitions
struct Word {
    string word;
    int count = 0;
    double p = 0.0; // Probability P(word|spam) or P(word|ham)
    double q = 0.0; // Complement probability (1 - P)
};

struct EmailSet {
    int num = 0;    // Email index
    double r = 0.0; // Probability
};

// Function declarations
void readTrain(const string &filename, unordered_map<string, Word> &wordMap);
void calculateProbabilities(unordered_map<string, Word> &spamWords, unordered_map<string, Word> &hamWords);
void readTest(const string &filename, const unordered_map<string, Word> &trainWords, vector<EmailSet> &results, char type);
double calculateEmailProbability(const unordered_map<string, Word> &trainWords, const unordered_map<string, int> &testWordMap);

// Main function
int main() {
    // File paths
    string trainSpamFile = "dataset_spam_train100.csv";
    string trainHamFile = "dataset_ham_train100.csv";
    string testSpamFile = "dataset_spam_test20.csv";
    string testHamFile = "dataset_ham_test20.csv";

    // Data structures
    unordered_map<string, Word> spamWords, hamWords;
    vector<EmailSet> spamResults, hamResults;

    // Read training data
    readTrain(trainSpamFile, spamWords);
    readTrain(trainHamFile, hamWords);

    // Calculate probabilities
    calculateProbabilities(spamWords, hamWords);

    // Classify test data
    readTest(testSpamFile, spamWords, spamResults, 's');
    readTest(testHamFile, hamWords, hamResults, 'h');

    return 0;
}

// Read and preprocess training data
void readTrain(const string &filename, unordered_map<string, Word> &wordMap) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    getline(file, line); // Skip header

    while (getline(file, line)) {
        stringstream processedLine;
        for (char c : line) {
            if (isalpha(c)) {
                processedLine << static_cast<char>(tolower(c));
            } else if (isspace(c)) {
                processedLine << ' ';
            }
        }

        string word;
        while (processedLine >> word) {
            wordMap[word].word = word;
            wordMap[word].count++;
        }
    }
    file.close();
}

// Calculate probabilities P(word|spam) and P(word|ham)
void calculateProbabilities(unordered_map<string, Word> &spamWords, unordered_map<string, Word> &hamWords) {
    for (auto &[word, spamWord] : spamWords) {
        int hamCount = hamWords.count(word) ? hamWords[word].count : 1; // Smoothing
        spamWord.p = static_cast<double>(spamWord.count) / (spamWord.count + hamCount);
        spamWord.q = 1.0 - spamWord.p;
    }

    for (auto &[word, hamWord] : hamWords) {
        int spamCount = spamWords.count(word) ? spamWords[word].count : 1; // Smoothing
        hamWord.p = static_cast<double>(hamWord.count) / (hamWord.count + spamCount);
        hamWord.q = 1.0 - hamWord.p;
    }
}

// Read and classify test data
void readTest(const string &filename, const unordered_map<string, Word> &trainWords, vector<EmailSet> &results, char type) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    getline(file, line); // Skip header
    int emailIndex = 0;

    while (getline(file, line)) {
        unordered_map<string, int> testWordMap;

        stringstream processedLine;
        for (char c : line) {
            if (isalpha(c)) {
                processedLine << static_cast<char>(tolower(c));
            } else if (isspace(c)) {
                processedLine << ' ';
            }
        }

        string word;
        while (processedLine >> word) {
            testWordMap[word]++;
        }

        double pProd = calculateEmailProbability(trainWords, testWordMap);
        double qProd = 1.0 - pProd;
        double probability = pProd / (pProd + qProd);

        results.push_back({++emailIndex, probability});
    }
    file.close();

    // Print results
    cout << "----------- " << (type == 's' ? "Spam" : "Ham") << " -----------\n";
    for (const auto &result : results) {
        cout << type << " - " << setw(2) << result.num << " : " << fixed << setprecision(3) << result.r << endl;
    }
}

// Calculate email probability
double calculateEmailProbability(const unordered_map<string, Word> &trainWords, const unordered_map<string, int> &testWordMap) {
    double pProd = 1.0, qProd = 1.0;

    for (const auto &[word, count] : testWordMap) {
        if (trainWords.count(word)) {
            const Word &trainWord = trainWords.at(word);
            pProd *= trainWord.p;
            qProd *= trainWord.q;
        }
    }

    return (pProd > 0.0) ? pProd : 0.0;
}