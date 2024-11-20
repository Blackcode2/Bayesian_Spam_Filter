#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <iomanip>

using namespace std;

class SpamFilter
{
private:
    map<string, int> spamWordCount;
    map<string, int> hamWordCount;
    int totalSpamWords = 0;
    int totalHamWords = 0;
    double spamPrior = 0.0;
    double hamPrior = 0.0;

    string preprocessString(const string &text)
    {
        string processed;
        for (char c : text)
        {
            if (isalnum(c) || isspace(c))
            {
                processed += tolower(c);
            }
        }
        return processed;
    }

    vector<string> tokenize(const string &text)
    {
        vector<string> tokens;
        istringstream stream(text);
        string word;
        while (stream >> word)
        {
            tokens.push_back(word);
        }
        return tokens;
    }

    double calculateProbability(const string &email)
    {
        string processed = preprocessString(email);
        vector<string> words = tokenize(processed);

        double spamScore = log(spamPrior);
        double hamScore = log(hamPrior);

        for (const string &word : words)
        {
            double p_word_spam = (spamWordCount[word] + 1.0) / (totalSpamWords + spamWordCount.size());
            double p_word_ham = (hamWordCount[word] + 1.0) / (totalHamWords + hamWordCount.size());
            spamScore += log(p_word_spam);
            hamScore += log(p_word_ham);
        }

        return exp(spamScore) / (exp(spamScore) + exp(hamScore));
    }

public:
    void train(const string &filename, bool isSpam)
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error: Cannot open file " << filename << endl;
            exit(1);
        }

        string line;
        while (getline(file, line))
        {
            string processed = preprocessString(line);
            vector<string> words = tokenize(processed);

            for (const string &word : words)
            {
                if (isSpam)
                {
                    spamWordCount[word]++;
                    totalSpamWords++;
                }
                else
                {
                    hamWordCount[word]++;
                    totalHamWords++;
                }
            }
        }
        file.close();
    }

    void setPriors(int spamCount, int hamCount)
    {
        spamPrior = (double)spamCount / (spamCount + hamCount);
        hamPrior = (double)hamCount / (spamCount + hamCount);
    }

    double classifyEmail(const string &email, double threshold)
    {
        double probability = calculateProbability(email);
        return probability >= threshold;
    }

    double evaluate(const string &testFile, bool isSpam, double threshold)
    {
        ifstream file(testFile);
        if (!file.is_open())
        {
            cerr << "Error: Cannot open file " << testFile << endl;
            exit(1);
        }

        string line;
        int totalEmails = 0;
        int correctPredictions = 0;

        while (getline(file, line))
        {
            bool prediction = classifyEmail(line, threshold);
            if (prediction == isSpam)
            {
                correctPredictions++;
            }
            totalEmails++;
        }
        file.close();

        return (double)correctPredictions / totalEmails * 100.0;
    }

    void classifyAndEvaluate(const string &spamTestFile, const string &hamTestFile, vector<double> thresholds)
    {
        for (double threshold : thresholds)
        {
            int correctPredictions = 0;

            cout << "Threshold: " << threshold << "\n";
            cout << "Classifying Spam Emails:\n";

            // Classify spam emails
            ifstream spamFile(spamTestFile);
            string line;
            int emailIndex = 1;
            while (getline(spamFile, line))
            {
                double probability = calculateProbability(line);
                bool isSpam = probability >= threshold;

                cout << "Email s" << setw(2) << setfill('0') << emailIndex << ":\n";
                cout << "  Probability: " << fixed << setprecision(4) << probability << "\n";
                cout << "  Predicted Label: " << (isSpam ? "Spam" : "Non-spam") << "\n";

                if (isSpam)
                {
                    correctPredictions++;
                }

                emailIndex++;
            }
            spamFile.close();

            cout << "\nClassifying Non-Spam Emails:\n";

            // Classify non-spam emails
            ifstream hamFile(hamTestFile);
            emailIndex = 1;
            while (getline(hamFile, line))
            {
                double probability = calculateProbability(line);
                bool isSpam = probability >= threshold;

                cout << "Email h" << setw(2) << setfill('0') << emailIndex << ":\n";
                cout << "  Probability: " << fixed << setprecision(4) << probability << "\n";
                cout << "  Predicted Label: " << (isSpam ? "Spam" : "Non-spam") << "\n";

                if (!isSpam)
                {
                    correctPredictions++;
                }

                emailIndex++;
            }
            hamFile.close();

            double accuracy = (double)correctPredictions / 40 * 100.0;
            cout << "Accuracy at Threshold " << threshold << ": " << fixed << setprecision(2) << accuracy << "%\n";
            cout << "-------------------------------------------\n";
        }
    }
};

int main()
{
    SpamFilter filter;

    // Train the filter
    filter.train("dataset_spam_train100.csv", true);
    filter.train("dataset_ham_train100.csv", false);

    // Set priors based on training data
    filter.setPriors(100, 100);

    // Thresholds for classification
    vector<double> thresholds = {0.6, 0.7, 0.8, 0.9, 0.95};

    // Classify and evaluate test data
    filter.classifyAndEvaluate("dataset_spam_test20.csv", "dataset_ham_test20.csv", thresholds);

    return 0;
}
