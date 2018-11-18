#include <iostream>
#include <utility>
#include <string>
#include <algorithm>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include<algorithm> 

#define NAMED_PIPE_PATH "./named_pipe"

#define VAL_SIZE 1024
#define ASCEND "ascend"
#define DESCEND "descend"

using namespace std;

int column = 0;

void mergeData(vector <string> lines, vector <string> &result);
void sortReadData(vector <string> dataFromWorker, string header, pair <string, string> sortVal, vector <string> &result);
bool isInt(string data);
void insertInResult(vector <string> lines, vector <string> &result);
void printResult(pair <string, string> sortVal, vector <string> result);

int main()
{
    int fd; 
    char value[VAL_SIZE];
    pair <string, string> sortVal;
    int processCount;
    bool gotFromBalancer = false;
    string header, pipes;
    vector <string> dataFromWorker;
    vector <string> result;
  
    mkfifo(NAMED_PIPE_PATH, 0666); 

    fd = open(NAMED_PIPE_PATH, O_RDONLY);

    while(1)
    {
        if(read(fd, value, VAL_SIZE) > 0)
        {
            string dataRead = value;
            string sortData = dataRead.substr(0, dataRead.find_first_of("/"));
            dataRead = dataRead.substr(dataRead.find_first_of("/") + 1);
            sortVal.first = sortData.substr(0, sortData.find_first_of("="));
            sortVal.second = sortData.substr(sortData.find_first_of("=") + 1);
            string dataReadRemain = dataRead.substr(0, dataRead.find_first_of("@"));
            pipes = dataRead.substr(dataRead.find_first_of("@") + 1);
            processCount = stoi(dataReadRemain.substr(dataReadRemain.find_first_of("/") + 1));
            close(fd); 
            unlink(NAMED_PIPE_PATH);
            break;
        }
    }

    vector <string> pipeFDs;
    string word = "";
    while(word != pipes)
    {
        word = pipes.substr(0, pipes.find_first_of(" "));
        pipes = pipes.substr(pipes.find_first_of(" ") + 1);
        pipeFDs.push_back(word);
    }

    for(int i = 0; i < processCount; i++)
    {  
        mkfifo(("./" + pipeFDs[i]).c_str(), 0666); 
        fd = open(("./" + pipeFDs[i]).c_str(),O_RDONLY);
        if(read(fd, value, VAL_SIZE) >= 0)
        {   
            string dataRead = value;
            header = dataRead.substr(0, dataRead.find_first_of("^"));
            dataRead = dataRead.substr(dataRead.find_first_of("^") + 1);
            dataFromWorker.push_back(dataRead);
            sortReadData(dataFromWorker, header, sortVal, result);
        }
        unlink(("./" + pipeFDs[i]).c_str());
        close(fd);
    } 

    printResult(sortVal, result);
    return 0; 
}

void printResult(pair <string, string> sortVal, vector <string> result)
{
    if(sortVal.second == DESCEND)
    {
        for(int i = result.size() - 1; i >= 0; i--)
        {
            cout << result[i] << endl;
        }
    }
    else
    {
        for(int i = 0; i < result.size(); i++)
        {
            cout << result[i] << endl;
        }       
    }
}

bool isInt(string data)
{
    for(int i = 0; i < data.size(); i++)
    {
        if(data[i] > '9' || data[i] < '0')
            return false;
    }
    return true;
}

bool compare(string first, string second) 
{
    vector <string> firstTokens, secondTokens;
    string token = "";
    while(token != first)
    {
        token = first.substr(0, first.find_first_of(" "));
        first = first.substr(first.find_first_of(" ") + 1);
        firstTokens.push_back(token);          
    }
    token = "";
    while(token != second)
    {
        token = second.substr(0, second.find_first_of(" "));
        second = second.substr(second.find_first_of(" ") + 1);
        secondTokens.push_back(token);          
    }
    if(isInt(firstTokens[column]))
        return stoi(firstTokens[column]) < stoi(secondTokens[column]);
    return firstTokens[column] < secondTokens[column];
}

void sortReadData(vector <string> dataFromWorker, string header, pair <string, string> sortVal, vector <string> &result)
{
    vector <string> headerTokens;
    string token = "";
    while (token != header)
    {
        token = header.substr(0, header.find_first_of(" "));
        header = header.substr(header.find_first_of(" ") + 1);
        headerTokens.push_back(token);
    }

    token = "";
    int len = dataFromWorker.size() - 1;
    vector <string> lines;
    string line = "";
    while(dataFromWorker[len] != line)
    {
        line = dataFromWorker[len].substr(0, dataFromWorker[len].find_first_of("\n"));
        dataFromWorker[len] = dataFromWorker[len].substr(dataFromWorker[len].find_first_of("\n") + 1);
        lines.push_back(line);     
    }

    for(int i = 0; i < headerTokens.size(); i++)
        if(headerTokens[i] == sortVal.first) 
            column = i;

    sort(lines.begin(), lines.end(), compare);
    mergeData(lines, result);

}

void insertInResult(string line, vector <string> &result)
{
    vector <string> lineWords, temp;
    string completeLine = line;
    string token = "";
    while(token != line)
    {
        token = line.substr(0, line.find_first_of(" "));
        line = line.substr(line.find_first_of(" ") + 1);
        lineWords.push_back(token);          
    }
    for(int i = 0; i < result.size(); i++)
    {
        temp.clear();
        string tempResult = result[i];
        token = "";
        while(token != tempResult)
        {
            token = tempResult.substr(0, tempResult.find_first_of(" "));
            tempResult = tempResult.substr(tempResult.find_first_of(" ") + 1);
            temp.push_back(token);          
        }
        if(isInt(lineWords[column]))
        {
            if(stoi(lineWords[column]) < stoi(temp[column]))
            {
                // cerr << "INSERTING NUM" << endl;
                result.insert(result.begin() + i, completeLine);
                return;
            }
        }
        else
        {
            if((lineWords[column]) < (temp[column]))
            {
                // cerr << "INSERTING STR " << endl;
                result.insert(result.begin() + i, completeLine);
                return;
            }
        }
    }
    result.push_back(completeLine);
}

void mergeData(vector <string> lines, vector <string> &result)
{
    if(result.size() == 0)
    {
        for(int i = 0; i < lines.size(); i++)
            result.push_back(lines[i]);
    }

    else
    {
        for(int i = 0; i < lines.size(); i++)
        {
            // cerr << "@@@ " << lines[i] << " " << result.size() << endl;
            insertInResult(lines[i], result);
        }
    }
}
