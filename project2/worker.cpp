#include <iostream>
#include <utility>
#include <string>
#include <algorithm>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <fcntl.h> 

#define READ_SIZE 4096

using namespace std;

#define DELIMITER "-"
#define FILE_DELIMITER " "
#define FILE_AND_FILTER_DELIMITER "*"
#define ASSIGN "="
#define END_CHAR "#"
#define DIRECTORY "dir"

void tokenizeInput(string data, vector <string> &files, vector <pair <string, string> > &filters, string &directory);
void printReqData(vector<pair<string, string> > filters, vector <string> files, string directory);
void readFiles(vector <string> files, string directory, vector <vector <string> > &fileData);
void printFile(vector <vector <string> > &fileData);
vector <vector<string> > filterData(vector <vector <string> > fileData, vector <pair <string, string> > filters);
void sendDataToPresenter(vector <vector<string> > filteredData, vector <vector <string> > fileData);

int main(int argc, char* argv[])
{
    char info[READ_SIZE];
    vector <string> files;
    vector <pair <string, string> > filters;
    string directory;
    vector <vector <string> > fileData;
    vector <vector<string> > filteredData;
    close(stoi(argv[1]));

    read(stoi(argv[0]), info, READ_SIZE); 

    close(stoi(argv[0])); 

    tokenizeInput(info, files, filters, directory);

    readFiles(files, directory, fileData);

    filteredData = filterData(fileData, filters);

    sendDataToPresenter(filteredData, fileData);
}

void sendDataToPresenter(vector <vector<string> > filteredData, vector <vector <string> > fileData)
{
    int fd; 
    
    string path = "./" + to_string(getpid());

    string filteredDataForPipe = "";

    for(int i = 0; i < fileData[0].size(); i++)
    {
        filteredDataForPipe += (fileData[0][i]);
        if(i == fileData[0].size() - 1)
            filteredDataForPipe += "^";
        else
            filteredDataForPipe += " ";
    }

    cerr << "HELLO " << endl;

    for(int i = 0; i < filteredData.size(); i++)
    {
        for(int j = 0; j < filteredData[i].size(); j++)
        {
            filteredDataForPipe += filteredData[i][j] + " ";
        }
        if(i != filteredData.size() - 1)
            filteredDataForPipe += '\n';
    }

    cerr << "BYE " << endl;

    mkfifo(path.c_str(), 0666); 
    fd = open(path.c_str(), O_WRONLY); 

    cerr << "WORKER " << filteredDataForPipe << endl;

    write(fd, filteredDataForPipe.c_str(), filteredDataForPipe.size()+1); 
    close(fd); 
}

vector <vector<string> > filterData(vector <vector <string> > fileData, vector <pair <string, string> > filters)
{
    vector <vector<string> > filteredData;
    for(int i = 0; i < fileData.size(); i++)
    {
        bool accepted = true;
        for(int j = 0; j < filters.size(); j++)
        {
            for(int k = 0; k < fileData[0].size(); k++)
            {
                if(fileData[0][k] == filters[j].first)
                {
                    if(fileData[i][k] != filters[j].second)
                        accepted = false;    
                }
            }
            if(accepted && i != 0)
            {
                accepted = false;
                filteredData.push_back(fileData[i]);
            }
        }
    }
    return filteredData;
}

void printFile(vector <vector <string> > &fileData)
{
    for(int i = 0; i < fileData.size(); i++)
    {
        for(int j = 0; j < fileData[i].size(); j++)
            cerr << fileData[i][j] << " ";
        cerr << endl;
    }
}

void readFiles(vector <string> files, string directory, vector <vector <string> > &fileData)
{
    fileData.clear();
    for(int i = 0; i < files.size(); i++)
    {
        string line, word;
        vector <string> lineData;
        ifstream myFile(directory + "/" + files[i]);
        if (myFile.is_open())
        {
            int lineCount = 0;
            word = "";
            while (getline(myFile,line))
            {
                lineData.clear();
                while (word != line)
                {
                    word = line.substr(0, line.find_first_of(FILE_DELIMITER));
                    line = line.substr(line.find_first_of(FILE_DELIMITER) + 1);
                    lineData.push_back(word);
                }
                lineCount++;
                fileData.push_back(lineData);
            }
            myFile.close();
        }
        else 
            cerr << "Unable to open file." << endl; 
    }
}

void printReqData(vector<pair<string, string> > filters, vector <string> files, string directory)
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