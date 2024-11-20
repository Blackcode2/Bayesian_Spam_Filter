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
    map<string, int> spamWordCount;
    map<string, int> hamWordCount;
    int totalSpamWords;
    int totalHamWords;
    double spamPrior;
    double hamPrior;
    vector<string> emails;

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
        bool isText = false;
        bool isFirstLine = false;
        if (text.find("\"Subject") != string::npos)
        {
            isFirstLine = true;
        }
        // cout << "f text: " << text << endl;
        for (char character : text)
        {

            if (isFirstLine)
            {
                if (character == '"')
                {
                    isText = true;
                }
                if (isText)
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
            }
            else
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

    double calcaulateProbability(const string &email) {
    vector<string> words = splitByWord(email);

    double spamScore = log(spamPrior);
    double hamScore = log(hamPrior);

    for (const string &word : words) {
        double p_word_spam = (spamWordCount[word] + 1.0) / (totalSpamWords + spamWordCount.size());
        double p_word_ham = (hamWordCount[word] + 1.0) / (totalHamWords + hamWordCount.size());

        // Logarithmic addition to prevent underflow
        spamScore += log(p_word_spam);
        hamScore += log(p_word_ham);

        // Debug intermediate probabilities
        cout << "Word: " << word << ", P(word|spam): " << p_word_spam << ", P(word|ham): " << p_word_ham << endl;
    }

    // Log-sum-exp trick for stability
    double maxScore = max(spamScore, hamScore);
    double denominator = log(exp(spamScore - maxScore) + exp(hamScore - maxScore)) + maxScore;
    return exp(spamScore - denominator);
}

  

    void openTestFile(const string &fileName)
    {
        ifstream file = openFile(fileName);
        string line;
        string email;
        bool isText = false;

        while (getline(file, line))
        {
            if (line.find("index,label,text") != string::npos)
            {
                continue;
            }

            size_t subjectPos = line.find("\"Subject");
            if (subjectPos != string::npos)
            {

                if (isText && !email.empty())
                {
                    emails.push_back(email);
                    email.clear();
                }

                isText = true;
                email = line.substr(subjectPos + 1);
            }
            else if (isText)
            {

                email += " " + line;
            }

            if (isText && line.find_last_of('"') == line.length() - 1)
            {
                email.pop_back();
                isText = false;
                emails.push_back(email);
                email.clear();
            }
        }

        if (isText && !email.empty())
        {
            emails.push_back(email);
        }
    }

public:
    void train(const string fileName, bool isSpam)
    {
        ifstream file = openFile(fileName);
        string line;
        int i = 0;
        // line += " ";
        while (getline(file, line))
        {
            if (i == 0)
            {
                i++;
                continue;
            }

            string preprocessed = preprocessString(line);
            vector<string> words = splitByWord(preprocessed);

            // DEBUG
            // cout << "text: " << preprocessed << endl;
            // if (i == 20)
            // {
            //     break;
            // }

            for (string word : words)
            {
                if (isSpam)
                {
                    spamWordCount[word] += 1;
                    totalSpamWords += 1;
                }
                else
                {
                    hamWordCount[word] += 1;
                    totalHamWords += 1;
                }
            }
        }
        file.close();
    }

    void setPriors(int spamEmails, int hamEmails)
    {
        spamPrior = (double)spamEmails / (spamEmails + hamEmails);
        hamPrior = (double)hamEmails / (spamEmails + hamEmails);
    }

    void printEmail()
    {
        int i = 1;
        for (auto iter = emails.begin(); iter != emails.end(); iter++)
        {
            cout << i << "email: " << *iter << endl;
            cout << *iter << endl;
            cout << "======= end ======" << endl;
            i++;
        }
    }

    void evaluate(const string fileName, bool isSpam)
    {
        int index = 1;
        vector<double> thresholds = {0.6, 0.7, 0.8, 0.9, 0.95};
        emails.clear();

        openTestFile(fileName);
        cout << "\nindex |  label  |  probability  | 0.6\t| 0.7\t| 0.8\t| 0.9\t| 0.95" << endl;
        for (string email : emails)
        {
            double probability = calcaulateProbability(email);
            vector<string> tLabel;
            // for (double t : thresholds)
            // {
            //     string label = probability >= t ? "Spam" : "Ham";
            //     tLabel.push_back(label);
            // }
            // cout.width(6);
            // cout << std::left << index;
            // cout.width(4);
            // cout << std::left << "| " << (isSpam ? "Spam\t| " : "Ham\t| ");
            // cout.width(13);
            // cout << std::left << probability << "\t| " << tLabel[0] << "\t| " << tLabel[1] << "\t| " << tLabel[2] << "\t| " << tLabel[3] << "\t| " << tLabel[4] << endl;
            // index++;
        }
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
    filter.train(trainSpam, true);
    filter.train(trainHam, false);

    // // Set priors based on training data
    filter.setPriors(20, 20);

    // filter.openTestFile(testHam);
    // filter.printEmail();

    filter.evaluate(testSpam, true);
    filter.evaluate(testHam, false);

    return 0;
}