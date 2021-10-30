#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>

#include <sys/types.h>
#include <dirent.h>

using namespace std;


/*mutex changeMutex;


void applyChange(bool& changeIs, bool newValue) {
    changeMutex.lock();
    changeIs = newValue;
    
}*/

void checkTheChanges(DIR* localRepo, bool& changeIs){
   
    struct dirent* context;

    vector <string> oldArgs;
    vector <string> newArgs;
    string arg;
    
    
    
    while ((context = readdir(localRepo)) != NULL){
        arg = context->d_name;
            
        if (arg == "." || arg == "..") continue;
            
        oldArgs.push_back(arg);
    }

    while(1){
        
        this_thread::sleep_for(std::chrono::milliseconds(1000));

        printf("\r\033[1A%c",  system("date")); // \r: set cursor to the first char of the line; \033[1A: move cursor up 1 line;
	fflush(stdout);	//clear output buffer
        
        rewinddir(localRepo);
        
        while ((context = readdir(localRepo)) != NULL){
            arg = context->d_name;
            
            if (arg == "." || arg == "..") continue;
            
            newArgs.push_back(arg);
        }
        
        if (newArgs == oldArgs) {
            newArgs.clear();
            continue;
        }
        
        oldArgs.clear();
        oldArgs = newArgs;
        newArgs.clear();
        
        changeIs = true;
    }  
}


void synchronizeFolders(const string pathRep, const string pathSub, bool& changeIs){
    
    string cmd = "cp -r " + pathSub + "/* " +  pathRep + "/";
    
    while(1){
        if (changeIs == true){
            system(cmd.c_str());
            changeIs = false;
        }
    }
}





int main(int argc, char **argv)
{
    if (argc == 1) {
        cout << "You did not enter any argument!" << endl;
    }
    
    if (argc == 2) {
        cout << "You must enter 2 arguments!" << endl;
    }
    
    if (argc > 3) {
        cout << "You must enter too many arguments!" << endl;
    }
    
    string argTMP1(argv[1]);
    argTMP1.erase(0,1);
    string argTMP2(argv[2]);
    argTMP2.erase(0,1);
    
    string user = getenv("USER");
        
    string pathSub = "/home/" + user + "/" + argTMP2;
    string pathRep = "/home/" + user + "/" + argTMP1;
    cout << pathSub << endl;
    DIR* localRepo = opendir(pathSub.c_str());
    bool changeIs = false;
    
        if (localRepo == NULL) {
        cout << "Directory cannot be opened." << endl;
        exit(0);
    }
    
   
 
    thread checker(checkTheChanges, ref(localRepo), ref(changeIs));
    thread worker(synchronizeFolders, pathRep, pathSub, ref(changeIs));
    
    
    worker.join();
    checker.join();
        
    return 0;
}

