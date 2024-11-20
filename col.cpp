#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Function to extract the "text" column from a CSV file
vector<string> extractTextColumn(const string &filename, const string &targetColumn)
{
    ifstream file(filename);
    vector<string> textData;
    string line, cell;

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return textData;
    }

    // Read the header row to find the index of the "text" column
    getline(file, line);
    istringstream headerStream(line);
    vector<string> headers;
    while (getline(headerStream, cell, ','))
    {
        headers.push_back(cell);
    }

    // Find the index of the "text" column
    auto it = find(headers.begin(), headers.end(), targetColumn);
    if (it == headers.end())
    {
        cerr << "Column \"" << targetColumn << "\" not found in file: " << filename << endl;
        return textData;
    }
    int columnIndex = distance(headers.begin(), it);

    // Read each data row and extract the "text" column
    while (getline(file, line))
    {
        istringstream lineStream(line);
        vector<string> row;
        while (getline(lineStream, cell, ','))
        {
            row.push_back(cell);
        }

        // Ensure the row has enough columns
        if (row.size() > columnIndex)
        {
            textData.push_back(row[columnIndex]);
        }
        else
        {
            cerr << "Skipping row with insufficient columns: " << line << endl;
        }
    }

    file.close();
    return textData;
}

int main()
{
    string filename = "dataset_ham_test20.csv";
    string targetColumn = "text";

    vector<string> textColumn = extractTextColumn(filename, targetColumn);

    if (!textColumn.empty())
    {
        cout << "Extracted text from \"" << targetColumn << "\" column:" << endl;
        for (const string &text : textColumn)
        {
            cout << text << endl;
            cout << "--------------------" << endl; // Separate rows for clarity
        }
    }
    else
    {
        cout << "No data extracted or column not found." << endl;
    }

    return 0;
}
