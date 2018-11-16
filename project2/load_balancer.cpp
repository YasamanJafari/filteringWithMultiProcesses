#include <iostream>
#include <utility>
#include <string>
#include <algorithm>
#include <vector>
#include <dirent.h>

using namespace std;

#define FIELD1 "brand"
#define FIELD2 "model"
#define FIELD3 "ram"
#define FIELD4 "hdd"
#define FIELD5 "price"

#define PROCESS_COUNT "prc_cnt"
#define DIRECTORY "dir"

#define ASCEND "ascend"
#define DESCEND "descend"

#define DELIMITER "-"
#define ASSIGN "="

void tokenizeInput(string request, vector<pair<string, string> > &data, int &processCount, string &directory);
void printData(vector<pair<string, string> > &data);
void getFilesOfDir(string name);

int main()
{
    string request, directory;
    vector<pair<string, string> > data;
    int processCount;

    getline(cin, request);

    tokenizeInput(request, data, processCount, directory);

    getFilesOfDir(directory);

    cout << directory << endl;
}

void getFilesOfDir(string name)
{
    int len;
    DIR* dirp;
    struct dirent *dp;

    dirp = opendir(name.c_str());
    if (dirp == NULL) 
       cout << "Cannot open directory" << name << endl;

    while ((dp = readdir(dirp)) != NULL) {
        cout << dp->d_name << endl;
    }
    closedir (dirp);
}

void printData(vector<pair<string, string> > &data)
{
    for (int i = 0; i < data.size(); i++)
    {
        cout << data[i].first << " " << data[i].second << endl;
    }
}

void tokenizeInput(string request, vector<pair<string, string> > &data, int &processCount, string &directory)
{
    data.clear();
    request.erase(remove(request.begin(), request.end(), ' '), request.end());
    string token;
    while (token != request)
    {
        int firstEqual = request.find_first_of(ASSIGN);
        if (firstEqual != -1)
        {
            pair<string, string> field;
            token = request.substr(0, firstEqual);
            request = request.substr(firstEqual + 1);
            field.first = token;
            token = request.substr(0, request.find_first_of(DELIMITER));
            field.second = token;
            if (field.first == PROCESS_COUNT)
                processCount = stoi(token);
            else if (field.first == DIRECTORY)
                directory = token;
            else
                data.push_back(field);
        }
        request = request.substr(request.find_first_of(DELIMITER) + 1);
    }
}