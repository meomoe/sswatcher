//
//  path.cpp
//  sswatcher
//
//  Created by Jason Zhong on 5/6/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#include "path.hpp"
#include "sys_helper.hpp"
#include <stdlib.h>

#define CONFIG_DIR "/.config/sswatcher/"
#define SSWATCHER_CONFIG CONFIG_DIR + "sswatcher.conf"
#define SSWATCHER_SOCK CONFIG_DIR + "sswatcher.sock"
#define SSSERVER_SOCK "/tmp/ssmanager.sock"
#define DATACAP_FILE CONFIG_DIR + "sswatcher-datacap"
#define DAEMON_UNIX_SOCK "/tmp/sswatcherd.sock"


string Path::path = "";

string Path::home_path() {
    return string(getenv("HOME"));
}

string Path::ss_config() {
    if (path == "")
        return SS_CONFIG;
    else
        return path;
}

void Path::set_ss_config(string path) {
    Path::path = path;
}

string Path::watcher_config() {
    return home_path() + SSWATCHER_CONFIG;
}

string Path::watcher_sock() {
    return home_path() + SSWATCHER_SOCK;
}

string Path::server_sock() {
    return SSSERVER_SOCK;
}

string Path::datacap() {
    return home_path() + DATACAP_FILE;
}

string Path::watcherd_sock() {
    return DAEMON_UNIX_SOCK;
}

string Path::config_dir() {
    return home_path() + CONFIG_DIR;
}
