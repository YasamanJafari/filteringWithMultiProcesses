#include <iostream>
#include <utility>
#include <string>
#include <algorithm>
#include <vector>
#include <dirent.h>
#include <unistd.h>

using namespace std;

#define PROCESS_COUNT "prc_cnt"
#define DIRECTORY "dir"

#define ASCEND "ascend"
#define DESCEND "descend"

#define WORKER_EXEC "./WORKER"
#define PRESENTER_EXEC "./PRESENTER"

#define DELIMITER "-"
#define ASSIGN "="

#define QUIT "quit"

void tokenizeInput(string request, vector<pair<string, string> > &data, int &processCount, string &directory);
void printData(vector<pair<string, string> > &data);
bool getFilesOfDir(string name, vector <string> &files);
void createWorkers(int processCount, vector <pid_t> &workersPIDs, vector <vector<int> > &fds);
void createPresenter(pid_t &presenterPID);
void waitForChildren(vector <pid_t> workersPIDs, pid_t presenterPID);
void createWorkersPipes(int processCount, vector <vector<int> > &fds);
void shareDataOnWorkersPipe(vector <string> files, vector <vector<int> > fds, string workerData, int processCount, string directory);
string convertFilteringInfoToString(vector<pair<string, string> > data);

int main()
{
    string request, directory;
    vector<pair<string, string> > data;
    vector <pid_t> workersPIDs;
    pid_t presenterPID;
    vector <string> files;
    vector <vector<int> > fds;
    int processCount;
    string workerData;

    while(true) 
    {
        getline(cin, request);
        if(request == QUIT)
            break;

        tokenizeInput(request, data, processCount, directory);

        if(!getFilesOfDir(directory, files)) {continue;}

        createWorkersPipes(processCount, fds);
        workerData = convertFilteringInfoToString(data);

        // createPresenter(presenterPID);
        createWorkers(processCount, workersPIDs, fds);

        shareDataOnWorkersPipe(files, fds, workerData, processCount, directory);

        waitForChildren(workersPIDs, presenterPID);
    }
}

string convertFilteringInfoToString(vector<pair<string, string> > data)
{
    string dataToBeSent = "";
    if(data.size() != 0)
    {
        int indexLimit = ((data[data.size() - 1].second == ASCEND || data[data.size() - 1].second == DESCEND) ? data.size() - 1 : data.size()); //not sending the sort part 
        for(int i = 0; i < indexLimit; i++)
        {
            dataToBeSent = dataToBeSent + data[i].first + ASSIGN + data[i].second;
            if(i != indexLimit - 1)
                dataToBeSent += DELIMITER;
        }
    }
    return dataToBeSent;
}

void shareDataOnWorkersPipe(vector <string> files, vector <vector<int> > fds, string workerData, int processCount, string directory)
{   
    vector <string> data;
    for(int i = 0; i < processCount; i++)
    {
        string allData = "";
        if(workerData != "")
            allData += workerData + DELIMITER;
        allData += ("dir=" + directory + "*");
        data.push_back(allData);
    }
    
    int j = 0;
    for(int i = 0; i < files.size(); i++)
    {
        data[j] = data[j] + files[i] + '-';
        j = (j + 1) % processCount; 
    }
    for(int i = 0; i < fds.size(); i++)
    {
        close(fds[i][0]); //close reading endpoint
        write(fds[i][1], data[i].c_str(), data[i].size()+1); 
    }

    for(int i = 0; i < fds.size(); i++)
        close(fds[i][1]);
}

void createWorkersPipes(int processCount, vector <vector<int> > &fds)
{
    fds.clear();
    for(int i = 0; i < processCount; i++)
    {
        int fd[2];
        if(pipe(fd) == -1)
        {
            cerr << "Creating worker pipe failed." << endl;
            return;
        }
        vector <int> fdInfo;
        fdInfo.push_back(fd[0]);
        fdInfo.push_back(fd[1]);
        fds.push_back(fdInfo);
    }
}

void waitForChildren(vector <pid_t> workersPIDs, pid_t presenterPID)
{
    for(int i = 0; i < workersPIDs.size(); i++)
        waitpid(workersPIDs[i], NULL, WNOHANG);
    waitpid(presenterPID, NULL, WNOHANG);
}

void createPresenter(pid_t &presenterPID)
{
    pid_t pid;
    pid = fork();
    if(pid < 0)
    {
        cerr << "Couldn't create presenter process." << endl;
        return;
    }  
    if(pid == 0)
        execvp(PRESENTER_EXEC, NULL); 
    else
        presenterPID = pid;
}

void createWorkers(int processCount, vector <pid_t> &workersPIDs, vector <vector<int> > &fds)
{
    pid_t pid;
    char* argv[3];
    for(int i = 0; i < processCount; i++)
    {
        argv[0] = (char*)to_string(fds[i][0]).c_str();
        argv[1] = (char*)to_string(fds[i][1]).c_str();
        argv[2] = NULL;
        pid = fork();
        if(pid < 0)
        {
            cerr << "Couldn't create a new worker process." << endl;
            return;
        }
        else if(pid == 0)
            execvp(WORKER_EXEC, argv);
        else
            workersPIDs.push_back(pid);
    }
}

bool getFilesOfDir(string name, vector <string> &files)
{
    int len;
    DIR* dirp;
    struct dirent *dp;
    files.clear();

    dirp = opendir(name.c_str());
    if (dirp == NULL) 
    {
       cerr << "Cannot open directory " << name << endl;
       return false;
    }

    while ((dp = readdir(dirp)) != NULL) 
    {
        string fileName = dp->d_name;
        if(fileName.substr(fileName.find_first_of('.') + 1) == "dms")
            files.push_back(fileName);
    }
    closedir (dirp);
    return true;
}

void printData(vector<pair<string, string> > &data)
{
    for (int i = 0; i < data.size(); i++)
    {
        cerr << data[i].first << " " << data[i].second << endl;
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