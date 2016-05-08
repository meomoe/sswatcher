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

using std::cout;
using std::endl;
using std::cin;
using std::string;

using std::ofstream;
using std::ifstream;

void show_usage(bool success);

void handle_input(int argc, const char * argv[]);

void send_connect_signal();

int main(int argc, const char * argv[]) {
    handle_input(argc, argv);
}


void handle_input(int argc, const char * argv[]) {
    if (argc <= 1) {
        cout << "status:" << endl;
        show_status();
        cout << "(\"" << SSWATCHER << " help\" for help)" << endl;
        exit(EXIT_SUCCESS);
    } else {
        ConfigMgr * mgr = &ConfigMgr::instance();
        string task(argv[1]);
        
        if (task.substr(0, 2) == "--")
            task = task.substr(2);
        
        if (argc == 2) {
            if (task == "start")
                start_ss();
            else if (task == "stop")
                stop_ss();
            else if (task == "restart")
                restart();
            else if (task == "load") {
                mgr->import_from(cin);
            } else if (task == "usage")
                mgr->export_to(cout);
            else if (task == "reset_usage_all") {
                mgr->reset_usage_for_all();
                mgr->export_to(cout);
            } else if (task == "status")
                show_status();
            else if (task == "default_datacap") {
                uint64_t cap = get_default_datacap();
                if (cap > 0)
                    cout << cap << " bytes" << endl;
                else
                    cout << "0 (infinite)" << endl;
            } else if (task == "help")
                show_usage(true);
            else if (task == "version")
                cout << SSWATCHER " version " << WATCHER_VER_0 << "." << WATCHER_VER_1
                     << "." << WATCHER_VER_2 << endl;
            else {
                show_usage(false);
                exit(EXIT_FAILURE);
            }
        } else if (argc == 3) {
            if (task == "remove")
                mgr->remove_port(atol(argv[2]));
            else if (task == "load_file") {
                ifstream input((string(argv[2])));
                mgr->import_from(input);
                input.close();
            } else if (task == "export_file") {
                ofstream output((string(argv[2])));
                mgr->export_to(output);
                output.close();
            } else if (task == "set_default_datacap") {
                set_default_datacap(atoll(argv[2]));
            } else if (task == "update_limit_all") {
                mgr->update_datacap_for_all(atol(argv[2]));
            } else if (task == "reset_usage") {
                mgr->reset_usage_for_port(atol(argv[2]));
            } else {
                show_usage(false);
                exit(EXIT_FAILURE);
            }
        } else {
            if (task == "add" || task == "update") {
                if (argc == 4) {
                    mgr->add_port(atol(argv[2]), string(argv[3]));
                } else if (argc == 5) {
                    mgr->add_port(atol(argv[2]), string(argv[3]), atoll(argv[4]));
                } else {
                    show_usage(false);
                    exit(EXIT_FAILURE);
                }
            } else if (task == "update_limit") {
                mgr->update_datacap_for_port(atol(argv[2]), atol(argv[3]));
            } else if (task == "start") {
                
                string user;
                string path;
                if (string(argv[2]) == "user")
                    user = string(argv[3]);
                if (string(argv[2]) == "config")
                    path = string(argv[3]);
                if (argc == 6) {
                    if (string(argv[4]) == "user")
                        user = string(argv[5]);
                    if (string(argv[4]) == "config")
                        path = string(argv[5]);
                }
                Path::set_ss_config(path);
                start_ss(user);
                
            } else {
                show_usage(false);
                exit(EXIT_FAILURE);
            }
        }
        
        if (task == "load" ||
            task == "reset_usage_all" ||
            task == "remove" ||
            task == "load_file" ||
            task == "set_default_datacap" ||
            task == "update_limit_all" ||
            task == "reset_usage" ||
            task == "add" || task == "update" ||
            task == "update_limit") {
            send_connect_signal();
        }
    }
}

void send_connect_signal() {
    send_sigusr(get_pid_by_name(SSWATCHERD_REGEX));
}


void show_usage(bool success) {
    if (!success)
        cout << "error: unsupported option" << endl << endl;
    
    cout << "usage: sswatcher <command> [option]" << endl << endl;
    cout << "supported commands:" <<endl;
    cout << "        status, usage, load, version" << endl;
    cout << "        add <port> <password> [datacap]" << endl;
    cout << "        update <port> <password> [datacap]" << endl;
    cout << "        update_limit <port> <limit>" << endl;
    cout << "        update_limit_all <limit>" << endl;
    cout << "        reset_usage <port>" << endl;
    cout << "        reset_usage_all" << endl;
    cout << "        remove <port>" << endl;
    cout << "        load_file <file_path>" << endl;
    cout << "        export_file <file_path>" << endl;
    cout << "        default_datacap" << endl;
    cout << "        set_default_datacap <datacap in bytes>" << endl;
    cout << "  - run these commands as root:" << endl;
    cout << "        start [user <run_as_user>] [config <file_path>]" << endl;
    cout << "        stop, restart" << endl;
}
