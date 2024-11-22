#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <vector>
#include <sstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <set>

using namespace std;

const double epsilon = 1e-9;

class SpamFilter
{
private:
    map<string, int> spamWordCount;
    map<string, int> hamWordCount;
    map<string, double> wordSpamP; // P(word|spam)
    map<string, double> wordSpamQ;
    map<string, double> wordHamP;  // P(word|ham)
    map<string, double> wordHamQ; 
    int totalSpamWords = 0;
    int totalHamWords = 0;
    double spamPrior;
    double hamPrior;
    set<string> vocabulary;

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

    void calculateProbabilities()
    {
        int vocabSize = vocabulary.size();
        for (const auto &word : vocabulary)
        {
            wordSpamP[word] = (spamWordCount[word] + 1.0) / (totalSpamWords + vocabSize);
            wordHamP[word] = (hamWordCount[word] + 1.0) / (totalHamWords + vocabSize);
        }
    }

    string preprocess(const string &text)
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

    vector<string> extractEmail(const string &fileName)
    {
        ifstream file = openFile(fileName);
        string line;
        string email;
        vector<string> emails;
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

        file.close();

        vector<string> porcessedEmails;
        for (auto iter = emails.begin(); iter != emails.end(); iter++)
        {
            porcessedEmails.push_back(preprocess(*iter));
        }

        return porcessedEmails;
    }

    // void classifyEmail(const string fileName, bool isSpam)
    // {

    //     vector<double> thresholds = {0.6, 0.7, 0.8, 0.9, 0.95};
    //     int index = 1;

    //     vector<string> emails = extractEmail(fileName);
    //     cout << "\nindex |  label  |  probability  | 0.6\t| 0.7\t| 0.8\t| 0.9\t| 0.95" << endl;
    //     for (string email : emails)
    //     {
    //         double spamScore = 1.0;
    //         double hamScore = 1.0;
    //         vector<string> words = splitByWord(email);

    //         for (string word : words)
    //         {
    //             double temp = wordSpamP.count(word) ? wordSpamP[word] : 1.0 / (totalSpamWords + vocabulary.size()); 
    //             spamScore *= temp;
    //             hamScore *= (1-temp);
    //             // hamScore *= wordHamP.count(word) ? wordHamP[word] : 1.0 / (totalHamWords + vocabulary.size());
    //         }

    //         // Normalize scores to probabilities
            
    //         double probSpam = spamScore / (spamScore + hamScore);

    //         vector<string> tLabel;
    //         for (double t : thresholds)
    //         {
    //             string label = probSpam >= t ? "Spam" : "Ham";
    //             tLabel.push_back(label);
    //         }
    //         cout.width(6);
    //         cout << std::left << index;
    //         cout.width(4);
    //         cout << std::left << "| " << (isSpam ? "Spam\t| " : "Ham\t| ");
    //         cout.width(13);
    //         cout << std::left << probSpam << "\t| " << tLabel[0] << "\t| " << tLabel[1] << "\t| " << tLabel[2] << "\t| " << tLabel[3] << "\t| " << tLabel[4] << endl;
    //         index++;
    //     }
    // }

    void classifyEmail(const string fileName, bool isSpam)
    {
        
        vector<double> thresholds = {0.6, 0.7, 0.8, 0.9, 0.95};
        int index = 1;

        vector<string> emails = extractEmail(fileName);
        cout << "\nindex |  label  |  probability  | 0.6\t| 0.7\t| 0.8\t| 0.9\t| 0.95" << endl;
        for (string email : emails)
        {
            double spamScore = log(spamPrior);
        double hamScore = log(hamPrior);
        vector<string> words = splitByWord(email);

        for (string word : words)
        {
            if (vocabulary.count(word)) 
            {
                spamScore += log(wordSpamP[word] + epsilon);
                hamScore += log(wordHamP[word] + epsilon);
            } 
            else 
            {
                double unseenProbSpam = 1.0 / (totalSpamWords + vocabulary.size());
                double unseenProbHam = 1.0 / (totalHamWords + vocabulary.size());
                spamScore += log(unseenProbSpam + epsilon);
                hamScore += log(unseenProbHam + epsilon);
            }
        }

        // Normalize probabilities using the log-sum-exp trick
        double maxScore = max(spamScore, hamScore);
        double probSpam = exp(spamScore - maxScore) / (exp(spamScore - maxScore) + exp(hamScore - maxScore));

            // double probSpam = spamScore / (spamScore + hamScore);

            vector<string> tLabel;
            for (double t : thresholds)
            {
                string label = probSpam >= t ? "Spam" : "Ham";
                tLabel.push_back(label);
            }
            cout.width(6);
            cout << std::left << index;
            cout.width(4);
            cout << std::left << "| " << (isSpam ? "Spam\t| " : "Ham\t| ");
            cout.width(13);
            cout << std::left << probSpam << "\t| " << tLabel[0] << "\t| " << tLabel[1] << "\t| " << tLabel[2] << "\t| " << tLabel[3] << "\t| " << tLabel[4] << endl;
            index++;
        }
    }

public:
    void train(const string fileName, bool isSpam)
    {
        vector<string> emails = extractEmail(fileName);

        // DEBUG
        //     int i = 1;
        //     for(auto iter = emails.begin(); iter != emails.end(); iter++){
        //         cout<< i << " email: " << *iter << endl;
        //         i++;
        // }

        for (string email : emails)
        {
            vector<string> words = splitByWord(email);
            for (string word : words)
            {
                if (isSpam)
                {
                    spamWordCount[word] += 1;
                    totalSpamWords += 1;
                    vocabulary.insert(word);
                }
                else
                {
                    hamWordCount[word] += 1;
                    totalHamWords += 1;
                    vocabulary.insert(word);
                }
            }
        }
    }

    void trainAll(const string &spamFile, const string &hamFile)
    {
        train(spamFile, true);
        train(hamFile, false);
    }

    void setPriors(int spamEmails, int hamEmails)
    {
        spamPrior = (double)spamEmails / (spamEmails + hamEmails);
        hamPrior = (double)hamEmails / (spamEmails + hamEmails);
    }

    void evaluate(const string fileName, bool isSpam)
    {
        classifyEmail(fileName, isSpam);
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
    filter.trainAll(trainSpam, trainHam);
    // filter.printWord();

    // // Set priors based on training data
    filter.setPriors(100, 100);

    filter.evaluate(testSpam, true);
    filter.evaluate(testHam, false);

    return 0;
}