//
//  sys_helper.cpp
//  sswatcher
//
//  Created by Jason Zhong on 5/2/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#include "sys_helper.hpp"

#include <vector>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __APPLE__
    #include <mach-o/dyld.h>
#elif !defined __linux__
    #error "unknown platform"
#endif /* __APPLE__ */

using std::vector;
using std::ifstream;

pid_t get_pid_by_name(const string & name) {
    
    pid_t pid = -1;
    
    string cmd = "pgrep -f " + name;
    FILE * proc = popen(cmd.c_str(), "r");
    
    if (proc) {
        fscanf(proc, "%d", &pid);
        pclose(proc);
    }
    
    return pid;
}

bool file_exist(const string & name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}
// http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c


string get_exec_path() {
    char buf[2048] = "";
    
    // TODO: error handling
    #ifdef __APPLE__
        uint32_t size = sizeof(buf) - 1;
        _NSGetExecutablePath(buf, &size);
    #elif defined __linux__
        ssize_t size = sizeof(buf) - 1;
        ssize_t len = readlink("/proc/self/exe", buf, size);
        if (len != -1)
            buf[len] = '\0';
    #endif

    string path = string(buf);
    path = path.substr(0, path.find_last_of("/"));
    path += "/";
    return string(path);
}
/*
 - http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
 - https://www.securecoding.cert.org/confluence/display/c/POS30-C.+Use+the+readlink%28%29+function+properly
 */

void send_sigusr(pid_t pid) {
    if (pid > 0)
        kill(pid, SIGUSR1);
}