//
//  status.cpp
//  sswatcher
//
//  Created by Jason Zhong on 5/2/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#include "status.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "sys_helper.hpp"
#include "operation.hpp"

#include "shared_const.h"

using std::ifstream;
using std::ofstream;
using std::cout;
using std::endl;
using std::system;

static void show_status(string pname, string regex) {
    pid_t pid = get_pid_by_name(regex);
    if (pid != -1)
        cout << pname << " is running. PID: " << pid << endl;
    else
        cout << pname << " is not running." << endl;
}

// wrapper function for showing status
void show_status() {
    cout << " - ";
    show_status(SSSERVER, SSSERVER_REGEX);
    cout << " - ";
    show_status(SSWATCHERD, SSWATCHERD_REGEX);
}

static void check_prev() {
    if (!(getuid() == 0 || geteuid() == 0)) {
        cout << "error: starting or stopping requires root privilege" << endl;
        exit(EXIT_FAILURE);
    }
}

void start_ss(string user) {
    
    // check for root privilege
    check_prev();
    
    string cmd;
    string login(getlogin());
    
    // make config dir as needed
    cmd = "mkdir -p " + Path::config_dir();
    system(cmd.c_str());
    cmd = "chown " + login + " " + Path::config_dir();
    system(cmd.c_str());
    
    
    // start ssserver
    if (get_pid_by_name(SSSERVER_REGEX) == -1) {
        if (file_exist(Path::ss_config())) {
            if (file_exist(Path::server_sock()))
                remove(Path::server_sock().c_str());
            cmd = SSSERVER " --manager-address " + Path::server_sock()
                        + " -c " + Path::ss_config() + " --user nobody -d start";
            system(cmd.c_str());
            
            /* ssserver will generate manager sock as root with 755
               changing it to 777 so that ss-daemon can write to it */
            cmd = "chmod 777 " + Path::server_sock();
            system(cmd.c_str());
            
            ConfigMgr::instance().init_ss();
        } else {
            cout << "error: config file not found" << endl;
            cout << "       please place a valid Shadowsocks config file at \"" << Path::ss_config() << "\", or specify a config file" << endl;
            exit(EXIT_FAILURE);
        }
        
    }
    
    // start sswatcher daemon
    if (get_pid_by_name(SSWATCHERD_REGEX) == -1) {
        cmd = get_exec_path() + SSWATCHERD;
        // set run-as user for sswatcherd
        if (user != "")
            cmd += " user " + user;
        else
            cmd += " user " + login;
        system(cmd.c_str());
    }
    
    show_status();
}

void stop_ss() {
    check_prev();
    
    pid_t pid = get_pid_by_name(SSWATCHERD_REGEX);
    if (pid != -1)
        kill(pid, SIGTERM);
    
    pid = get_pid_by_name(SSSERVER_REGEX);
    if (pid != -1) {
        system(SSSERVER " -d stop");
    }
    
    show_status();
}

void restart() {
    check_prev();
    stop_ss();
    start_ss();
}

