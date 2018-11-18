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

using namespace std;

int column = 0;

void mergeVectors(vector <int> first, vector <int> second, vector <int> result);
void sortReadData(vector <string> dataFromWorker, string header, pair <string, string> sortVal);
bool isInt(string data);

int main()
{
    int fd; 
    char value[VAL_SIZE];
    pair <string, string> sortVal;
    int processCount;
    bool gotFromBalancer = false;
    string header, pipes;
    vector <string> dataFromWorker;
  
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
        cerr << "START " << endl;
        fd = open(("./" + pipeFDs[i]).c_str(),O_RDONLY);
        cerr << "END " << endl;
        if(read(fd, value, VAL_SIZE) >= 0)
        {   
            cerr << "READING..." << endl;
            string dataRead = value;
            header = dataRead.substr(0, dataRead.find_first_of("^"));
            dataRead = dataRead.substr(dataRead.find_first_of("^") + 1);
            dataFromWorker.push_back(dataRead);
            sortReadData(dataFromWorker, header, sortVal);
        }
        close(fd);
    } 
    return 0; 
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

void sortReadData(vector <string> dataFromWorker, string header, pair <string, string> sortVal)
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

    for(int i = 0; i < lines.size(); i++)
    {
        cerr << lines[i] << endl;
    }
}

void mergeVectors(vector <int> first, vector <int> second, vector <int> result)
{
    int i = 0, j = 0, k = 0; 
  
    while (i < first.size() && j < second.size()) 
    { 
        if (first[i] < second[j]) 
            result[k++] = first[i++]; 
        else
            result[k++] = second[j++]; 
    } 
  
    while (i < first.size()) 
        result[k++] = first[i++]; 
   
    while (j < second.size()) 
        result[k++] = second[j++]; 
} 