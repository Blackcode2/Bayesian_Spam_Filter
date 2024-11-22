#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <cctype>
#include <cstring>
#include <sstream>
#include <algorithm>

using namespace std;

const string train_ham = "dataset_ham_train100.csv";
const string train_spam = "dataset_spam_train100.csv";
const string test_ham = "dataset_ham_test20.csv";
const string test_spam = "dataset_spam_test20.csv";

class SpamFilter {
private:
    vector<string> stopwords = {"a", "about", "above", "after", "again", "against", /*...*/ "yourself", "yourselves"};
    vector<string> singleLetters = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", /*...*/ "m"};

    struct Data {
        vector<string> words;
        int length = 0;
        int real_length = 0;
        double ham_probability = 0.0;
        double spam_probability = 0.0;
        double probability = 0.0;
    };

    vector<Data> testHamData, testSpamData, trainHamData, trainSpamData;

    // Convert a string to lowercase
    string toLowerCase(const string &s) {
        string result;
        transform(s.begin(), s.end(), back_inserter(result), ::tolower);
        return result;
    }

    // Check if a word is unnecessary
    bool isUnnecessaryWord(const string &word) {
        if (find(stopwords.begin(), stopwords.end(), word) != stopwords.end() ||
            find(singleLetters.begin(), singleLetters.end(), word) != singleLetters.end() ||
            word == "ham" || word == "spam") {
            return true;
        }
        return false;
    }

    // Tokenize a line into words
    vector<string> tokenize(const string &line) {
        vector<string> words;
        stringstream ss(line);
        string word;
        while (ss >> word) {
            words.push_back(toLowerCase(word));
        }
        return words;
    }

    // Parse a file into data
    void parseFile(const string &filename, vector<Data> &data, int recordCount) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            exit(EXIT_FAILURE);
        }

        string line;
        int index = 0;
        while (getline(file, line) && index < recordCount) {
            vector<string> words = tokenize(line);
            Data record;
            for (auto &word : words) {
                if (!isUnnecessaryWord(word)) {
                    record.words.push_back(word);
                }
            }
            record.length = record.words.size();
            data.push_back(record);
            index++;
        }
        file.close();
    }

    // Remove duplicate words in each record and update real_length
    void removeDuplicateWords(vector<Data> &data) {
        for (auto &record : data) {
            sort(record.words.begin(), record.words.end());
            record.words.erase(unique(record.words.begin(), record.words.end()), record.words.end());
            record.real_length = record.words.size();
        }
    }

    // Calculate probabilities for test data
    void calculateProbabilities(vector<Data> &testData, const vector<Data> &trainData, bool isHam) {
        for (auto &testRecord : testData) {
            double probability = 0.0;
            for (const string &testWord : testRecord.words) {
                int count = 0;
                for (const auto &trainRecord : trainData) {
                    if (find(trainRecord.words.begin(), trainRecord.words.end(), testWord) != trainRecord.words.end()) {
                        count++;
                    }
                }
                count++; // Laplace smoothing
                probability += log10(count);
            }
            if (isHam) {
                testRecord.ham_probability = probability;
            } else {
                testRecord.spam_probability = probability;
            }
        }
    }

    // Calculate final probabilities for spam and ham
    void calculateFinalProbabilities(vector<Data> &data) {
        for (auto &record : data) {
            double spamProb = pow(10, record.spam_probability);
            double hamProb = pow(10, record.ham_probability);
            record.probability = spamProb / (spamProb + hamProb);
        }
    }

    // Print results
    void printResults(const vector<Data> &data, const string &label) {
        for (size_t i = 0; i < data.size(); i++) {
            cout << label << "-" << i + 1 << " : " << data[i].probability << endl;
        }
        cout << "-------------------------------------" << endl;
    }

public:
    void run() {
        // Parse files
        parseFile(test_ham, testHamData, 20);
        parseFile(test_spam, testSpamData, 20);
        parseFile(train_ham, trainHamData, 100);
        parseFile(train_spam, trainSpamData, 100);

        // Remove duplicate words
        removeDuplicateWords(testHamData);
        removeDuplicateWords(testSpamData);
        removeDuplicateWords(trainHamData);
        removeDuplicateWords(trainSpamData);

        // Calculate probabilities
        calculateProbabilities(testHamData, trainHamData, true);
        calculateProbabilities(testHamData, trainSpamData, false);
        calculateProbabilities(testSpamData, trainHamData, true);
        calculateProbabilities(testSpamData, trainSpamData, false);

        // Calculate final probabilities
        calculateFinalProbabilities(testHamData);
        calculateFinalProbabilities(testSpamData);

        // Print results
        printResults(testHamData, "ham");
        printResults(testSpamData, "spam");
    }
};

int main() {
    SpamFilter filter;
    filter.run();
    return 0;
}