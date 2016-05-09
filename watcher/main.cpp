//
//  main.cpp
//  sswatcher
//
//  Created by Jason Zhong on 3/5/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include "status.hpp"
#include "operation.hpp"
#include "sys_helper.hpp"
#include "shared_const.h"
#include "ver.h"
#include "commands.h"

using std::cout;
using std::endl;
using std::cin;
using std::string;

using std::ofstream;
using std::ifstream;

void show_usage(bool success);

void handle_input(int argc, const char * argv[]);

void send_connect_signal();

string remove_pref(const char * arg);

int main(int argc, const char * argv[]) {
    handle_input(argc, argv);
}


void handle_input(int argc, const char * argv[]) {
    if (argc <= 1) {
        cout << "status:" << endl;
        show_status();
        cout << "(\"" << SSWATCHER << " "<< CMD_HELP << "\" for help)" << endl;
        exit(EXIT_SUCCESS);
    } else {
        ConfigMgr * mgr = &ConfigMgr::instance();
        string task = remove_pref(argv[1]);
        
        if (argc == 2) {
            if (task == CMD_START)
                start_ss();
            else if (task == CMD_STOP)
                stop_ss();
            else if (task == CMD_RESTART)
                restart();
            else if (task == CMD_LOAD) {
                mgr->import_from(cin);
            } else if (task == CMD_STAT)
                mgr->export_to(cout, true);
            else if (task == CMD_RESET_ALL) {
                mgr->reset_usage_for_all();
                mgr->export_to(cout, true);
            } else if (task == CMD_STATUS)
                show_status();
            else if (task == CMD_DEF_LIM_1 || task == CMD_DEF_LIM_2) {
                cout << get_formatted_datacap() << endl;
            } else if (task == CMD_HELP)
                show_usage(true);
            else if (task == CMD_VER_1 || task == CMD_VER_2)
                cout << SSWATCHER " version " << WATCHER_VER_0 << "." << WATCHER_VER_1
                     << "." << WATCHER_VER_2 << endl;
            else {
                show_usage(false);
                exit(EXIT_FAILURE);
            }
        } else if (argc == 3) {
            if (task == CMD_REMOVE)
                mgr->remove_port(argv[2]);
            else if (task == CMD_LOAD) {
                ifstream input((string(argv[2])));
                mgr->import_from(input);
                input.close();
            } else if (task == CMD_EXPORT) {
                ofstream output((string(argv[2])));
                mgr->export_to(output);
                output.close();
            } else if (task == CMD_DEF_LIM_1 || task == CMD_DEF_LIM_2) {
                set_default_datacap(argv[2]);
            } else if (task == CMD_DEF_LIM_1 || task == CMD_DEF_LIM_2) {
                mgr->update_datacap_for_all(argv[2]);
            } else if (task == CMD_RESET) {
                mgr->reset_usage_for_port(argv[2]);
            } else {
                show_usage(false);
                exit(EXIT_FAILURE);
            }
        } else {
            if (task == CMD_ADD || task == CMD_UPDATE) {
                if (argc == 4) {
                    mgr->add_port(argv[2], argv[3]);
                } else if (argc == 5) {
                    mgr->add_port(argv[2], argv[3], argv[4]);
                } else {
                    show_usage(false);
                    exit(EXIT_FAILURE);
                }
            } else if (task == CMD_LIM) {
                mgr->update_datacap_for_port(argv[2], argv[3]);
            } else if (task == CMD_START) {
                
                string user;
                string path;
                string cmd1 = remove_pref(argv[2]);
                if (cmd1 == CMD_USER)
                    user = string(argv[3]);
                if (cmd1 == CMD_CONFIG)
                    path = string(argv[3]);
                if (argc == 6) {
                    string cmd2 = remove_pref(argv[4]);
                    if (cmd2 == CMD_USER)
                        user = string(argv[5]);
                    if (cmd2 == CMD_CONFIG)
                        path = string(argv[5]);
                }
                Path::set_ss_config(path);
                start_ss(user);
                
            } else {
                show_usage(false);
                exit(EXIT_FAILURE);
            }
        }
        
        if (task == CMD_LOAD ||
            task == CMD_RESET ||
            task == CMD_RESET ||
            task == CMD_REMOVE ||
            task == CMD_DEF_LIM_1 ||
            task == CMD_DEF_LIM_2 ||
            task == CMD_LIM ||
            task == CMD_LIM_ALL ||
            task == CMD_ADD ||
            task == CMD_UPDATE) {
            send_connect_signal();
        }
    }
}

string remove_pref(const char * arg) {
    string res(arg);
    if (res.substr(0,2) == LONG_PREF)
        res = res.substr(2);
    return res;
}

void send_connect_signal() {
    send_sigusr(get_pid_by_name(SSWATCHERD_REGEX));
}


void show_usage(bool success) {
    if (!success)
        cout << "error: unsupported option" << endl << endl;
    
    string spaces = "        ";
    
    cout << "usage: sswatcher <command> [option]" << endl << endl;
    cout << "supported commands:" <<endl;
    cout << spaces << CMD_STATUS << ", " << CMD_STAT << ", " << CMD_LOAD << ", " << CMD_VER_2 << endl;
    cout << spaces << CMD_ADD << " <port> <password> [limit]" << endl;
    cout << spaces << CMD_UPDATE << " <port> <password> [limit]" << endl;
    cout << spaces << CMD_LIM << " <port> <limit>" << endl;
    cout << spaces << CMD_LIM_ALL << " <limit>" << endl;
    cout << spaces << CMD_RESET << " <port>" << endl;
    cout << spaces << CMD_RESET_ALL << endl;
    cout << spaces << CMD_REMOVE << " <port>" << endl;
    cout << spaces << CMD_LOAD << " <file_path>" << endl;
    cout << spaces << CMD_EXPORT << " <file_path>" << endl;
    cout << spaces << CMD_DEF_LIM_2 << endl;
    cout << spaces << CMD_DEF_LIM_2 << " <limit>" << endl;
    
    cout << "  - run these commands as root:" << endl;
    cout << spaces << CMD_START << " [user <run_as_user>] [config <file_path>]" << endl;
    cout << spaces << CMD_STOP << ", " << CMD_RESTART << "" << endl;
}
