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

#define NAMED_PIPE_PATH "./named_pipe"

#define VAL_SIZE 1024

using namespace std;

int main()
{
    int fd; 
  
    mkfifo(NAMED_PIPE_PATH, 0666); 
  
    char value[VAL_SIZE];

    pair <string, string> sortVal;

    int processCount;

    bool gotFromBalancer = false;

    while (1) 
    {  
        fd = open(NAMED_PIPE_PATH,O_RDONLY);
        if(read(fd, value, VAL_SIZE) >= 0 && gotFromBalancer)
            cerr << "Presenter speaking: " << value << endl;

        if(!gotFromBalancer)
        {
            string dataRead = value;
            string sortData = dataRead.substr(0, dataRead.find_first_of("/"));
            sortVal.first = sortData.substr(0, sortData.find_first_of("="));
            sortVal.second = sortData.substr(sortData.find_first_of("=") + 1);
            processCount = stoi(dataRead.substr(dataRead.find_first_of("/") + 1));
            gotFromBalancer = true;

            cerr << sortVal.first << " " << sortVal.second << " " << processCount << endl;
        }
  
        close(fd); 
    } 
    return 0; 
}