#include <iostream>
#include <utility>
#include <string>
#include <algorithm>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <fstream>

#define READ_SIZE 1024

using namespace std;

#define DELIMITER "-"
#define FILE_AND_FILTER_DELIMITER "*"
#define ASSIGN "="
#define DIRECTORY "dir"

void tokenizeInput(string data, vector <string> &files, vector <pair <string, string> > &filters, string &directory);
void printData(vector<pair<string, string> > filters, vector <string> files, string directory);
void readFiles(vector <string> files, string directory);

int main(int argc, char* argv[])
{
    char info[READ_SIZE];
    vector <string> files;
    vector <pair <string, string> > filters;
    string directory;
    close(stoi(argv[1]));

    read(stoi(argv[0]), info, READ_SIZE); 

    close(stoi(argv[0])); 

    tokenizeInput(info, files, filters, directory);

    readFiles(files, directory);
}

void readFiles(vector <string> files, string directory)
{
    for(int i = 0; i < files.size(); i++)
    {
        string line;
        ifstream myFile(directory + "/" + files[i]);
        if (myFile.is_open())
        {
        while (getline(myFile,line))
        {
            cout << line << '\n';
        }
        myFile.close();
        }
        else cerr << "Unable to open file" << endl; 
    }
}

void printData(vector<pair<string, string> > filters, vector <string> files, string directory)
{
    cerr << "Directory: " << directory << endl;
    cerr << "Filters: " << endl;
    for (int i = 0; i < filters.size(); i++)
    {
        cerr << filters[i].first << " " << filters[i].second << endl;
    }

    cerr << "Files: " << endl;
    for(int i = 0; i < files.size() - 1; i++)
    {
        cerr << files[i] << endl;
    }
}
void tokenizeInput(string data, vector <string> &files, vector <pair <string, string> > &filters, string &directory)
{
    string filterInfo, fileInfo;

    filterInfo = data.substr(0, data.find_first_of(FILE_AND_FILTER_DELIMITER));
    fileInfo = data.substr(data.find_first_of(FILE_AND_FILTER_DELIMITER) + 1);

    string token = "";
    while (token != filterInfo)
    {
        int firstEqual = filterInfo.find_first_of(ASSIGN);
        if (firstEqual != -1)
        {
            pair<string, string> field;
            token = filterInfo.substr(0, firstEqual);
            filterInfo = filterInfo.substr(firstEqual + 1);
            field.first = token;
            token = filterInfo.substr(0, filterInfo.find_first_of(DELIMITER));
            field.second = token;
            if (field.first == DIRECTORY)
                directory = token;
            filters.push_back(field);
        }
        filterInfo = filterInfo.substr(filterInfo.find_first_of(DELIMITER) + 1);
    }

    token = "";

    while (token != fileInfo)
    {
        token = fileInfo.substr(0, fileInfo.find_first_of(DELIMITER));
        fileInfo = fileInfo.substr(fileInfo.find_first_of(DELIMITER) + 1);
        files.push_back(token);
    }
}