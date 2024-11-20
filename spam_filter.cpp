#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <vector>
#include <sstream>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

class SpamFilter
{
private:
    const string fileName;
    map<string, int> spamCount;
    map<string, int> hamCount;
    int totalSpamWords;
    int totalHampWords;
    double spamPrior;
    double hamPrior;

    ifstream openFile(const string fileName)
    {
        ifstream file;
        file.open(fileName);
        if (!file.is_open())
        {
            cerr << "Error! can not open the file " << fileName << endl;
            exit(1);
        }
        return file;
    }

    string preprocessString(const string &text)
    {
        string processedText;
        for (char character : text)
        {
            if (isalnum(character))
            {
                processedText += tolower(character);
            }
            else if (isspace(character))
            {
                processedText += ' ';
            }
        }
        return processedText;
    }

    vector<string> splitByWord(const string &text)
    {
        vector<string> words;
        istringstream iss(text);
        string word;
        while (iss >> word)
        {
            words.push_back(word);
        }
        return words;
    }

    double calcaulateProbability(const string &email)
    {
        string processed = preprocessString(email);
        vector<string> words = splitByWord(processed);

        double spamScore = log(spamPrior);
        double hamScore = log(hamPrior);

        for (const string &word : words)
        {
            double p_word_spam = (spamCount[word] + 1) / (double)(totalSpamWords + spamCount.size());
            double p_word_ham = (hamCount[word] + 1) / (double)(totalHampWords + hamCount.size());
            spamScore += log(p_word_spam);
            hamScore += log(p_word_ham);
        }

        return exp(spamScore) / (exp(spamScore) + exp(hamScore));
    }

public:
    void countEachWord(const string fileName, bool isTrain)
    {
        ifstream file = openFile(fileName);
        string line;
        int i = 0;
        while (getline(file, line))
        {
            string preprocessed = preprocessString(line);
            vector<string> words = splitByWord(preprocessed);

            cout << "text: " << preprocessed << endl;

            i++;
            if (i == 20)
            {
                break;
            }

            for (string word : words)
            {
                if (isTrain)
                {
                    spamCount[word] += 1;
                    totalSpamWords += 1;
                }
                else
                {
                    hamCount[word] += 1;
                    totalHampWords += 1;
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

    void testAndEvaluate(const string &testFile, bool isSpam, double threshold)
    {
        ifstream file(testFile);
        if (!file.is_open())
        {
            cerr << "Error opening test file: " << testFile << endl;
            return;
        }

        int correct = 0, total = 0;
        string line;
        while (getline(file, line))
        {
            double spamScore = calcaulateProbability(line);
            bool prediction = spamScore >= threshold;
            if (prediction == isSpam)
                correct++;
            total++;
        }
        file.close();

        cout << "Threshold " << threshold << ": Accuracy = " << (double)correct / total * 100.0 << "%" << endl;
    }
};

int main()
{
    string trainSpam = "dataset_spam_train100.csv";
    string trainHam = "dataset_ham_train100.csv";

    string testSpam = "dataset_spam_test20.csv";
    string testHam = "dataset_ham_test20.csv";

    SpamFilter filter;

    // Train on provided data
    filter.countEachWord(trainSpam, true);
    // filter.countEachWord(trainHam, false);

    // // Set priors based on training data
    // filter.setPriors(100, 100);

    // // Evaluate on test data with different thresholds
    // cout << "Testing Spam Filter with Various Thresholds:" << endl;
    // for (double threshold : {0.6, 0.7, 0.8, 0.9, 0.95})
    // {
    //     cout << "Testing with threshold: " << threshold << endl;
    //     filter.testAndEvaluate(testSpam, true, threshold);
    //     filter.testAndEvaluate(testHam, false, threshold);
    // }

    return 0;
}