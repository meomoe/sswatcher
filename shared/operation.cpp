//
//  operation.cpp
//  sswatcher
//
//  Created by Jason Zhong on 5/2/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#include "operation.hpp"
#include <vector>
#include <string>
#include <unistd.h>
#include <syslog.h>
#include "sys_helper.hpp"
#include <cstdio>
#include <sys/file.h>
#include "status.hpp"
#include "shared_const.h"
#include "converter.hpp"

using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;

#define PLACEHOLDER_PORT 8383

int64_t datacap_cache = -1;

uint64_t get_default_datacap() {
    
    if (datacap_cache >= 0)
        return datacap_cache;
    
    ifstream ifs(Path::datacap());
    string datacap;
    if (ifs.good())
        ifs >> datacap;
    ifs.close();
    datacap_cache = Converter::parse_size(datacap.c_str());
    return datacap_cache;
}

string get_formatted_datacap(uint64_t datacap) {
    uint64_t cap = datacap;
    if (cap == -1)
        cap = get_default_datacap();
    if (cap == 0)
        return "0 (unlimited)";
    else
        return Converter::convert_to_string(cap);
}

void set_default_datacap(const char * datacap) {
    uint64_t cap = Converter::parse_size(datacap, true);
    datacap_cache = cap;
    ofstream ofs(Path::datacap());
    ofs << datacap << endl;
    ofs.close();
}

#pragma mark - User

User::User() {
    datacap = get_default_datacap();
}

User::User(const string & password): User() {
    this->password = password;
}

User::User(const string & password, const uint64_t datacap):
User(password) {
    this->datacap = datacap;
}

User::User(const string & password, const uint64_t datacap, const uint64_t usage):
User::User(password, datacap){
    this->usage = usage;
}


#pragma mark - Config Manager

ConfigMgr::ConfigMgr() {
    load_default_conf();
}

unsigned ConfigMgr::load_conf(std::istream & input) {
    UserMap prev_users = users;
    users.clear();
    string line;
    unsigned i = 0;
    vector<string> args;
    
    while (getline(input, line)) {
        args.clear();
        args.push_back("");
        
        for (char ch : line) {
            if (ch != '\0' && ch != '\t' && ch != ' ')
                args[args.size() - 1] += ch;
            else if (args[args.size() - 1] != "")
                args.push_back("");
        }
        
        if (args[args.size() - 1] == "")
            args.pop_back();
        
        if (args.size() >= 2) {
            if (args[0][0] < '0' || args[0][0] > '9')
                continue;
            
            uint16_t port = atol(args[0].c_str());
            User c((string(args[1])));
            
            // set datacap (third argument)
            if (args.size() >= 3)
                c.datacap = Converter::parse_size(args[2].c_str());
            
            // set usage history (fourth argument)
            if (args.size() >= 4)
                c.usage = Converter::parse_size(args[3].c_str());
            
            users[port] = c;
            
            UserMapIter cur = users.find(port);
            UserMapIter prev = prev_users.find(port);
            if (prev != prev_users.end()) {
                
                cur->second.usage = prev->second.usage;
                if (args.size() >= 4)
                    cur->second.usage = Converter::parse_size(args[3].c_str());
                
                if (cur->second.datacap != prev->second.datacap) {
                    cur->second.is_over_datacap = is_over_datacap(port);
                    if (!cur->second.is_over_datacap)
                        send_add_request(port, cur->second.password);
                }
            }
            
            i++;
        }
    }
    
    return i;
}

void ConfigMgr::load_default_conf() {
    ifstream ifs(Path::watcher_config());
    load_conf(ifs);
    ifs.close();
}

void ConfigMgr::init_ss() {
    send_remove_request(PLACEHOLDER_PORT);
    for (UserMap::value_type u : users) {
        send_remove_request(u.first);
        if (!is_over_datacap(u.first))
            send_add_request(u.first, u.second.password);
        else
            u.second.is_over_datacap = true;
    }
}

void ConfigMgr::write_conf() {

    ofstream ofs(Path::watcher_config());
    export_to(ofs);
    ofs.close();
    
    // allow write
    if (getuid() == 0 || geteuid() == 0) {
        string cmd = "chown " + string(getlogin()) + " " + Path::watcher_config();
        system(cmd.c_str());
    }
    
    refresh_all_users();
}

bool ss_config_is_ok() {
    const vector<string> temp = { "server", "method", "port_password" };
    
    ifstream ifs(Path::ss_config());
    bool check[3] = { false, false, false };
    string line;
    
    while (getline(ifs, line)) {
        if (line.find(temp[0], 0) != string::npos)
            check[0] = true;
        if (line.find(temp[1], 0) != string::npos)
            check[1] = true;
        if (line.find(temp[2], 0) != string::npos) {
            check[2] = true;
        }
    }
    
    ifs.close();
    
    return (check[0] && check[1] && check[2]);
}

void ConfigMgr::set_comm() {
    if (!comm) {
        comm = new Communicator(Path::watcher_sock());
        if (getuid() == 0 || geteuid() == 0) {
            string cmd = "chown " + string(getlogin()) + " " + Path::watcher_sock();
            system(cmd.c_str());
        }
    }
}

void ConfigMgr::import_from(std::istream & input) {
    unsigned cnt = load_conf(input);
    write_conf();
    if (ss_is_runnning())
        init_ss();
    cout << cnt << " entries imported" << endl;
}

void ConfigMgr::export_to(std::ostream & output, bool readable) {
    output << "port\tpassword\tdata_cap\tusage" << endl;
    
    for (auto & u : users) {
        output << u.first << "\t" << u.second.password << "\t" ;
        if (readable)
            output << get_formatted_datacap(u.second.datacap) << "\t" << Converter::convert_to_string(u.second.usage) << endl;
        else
            output << u.second.datacap << "\t" << u.second.usage << endl;
    }
}

void ConfigMgr::export_user(std::ostream & output) {
    output << "port\tpassword" << endl;
    
    for (auto & u : users)
        output << u.first << "\t" << u.second.password << endl;
}

void ConfigMgr::send_remove_request(const uint16_t port) {
    set_comm();
    string remove_req = "remove: {\"server_port\": " + std::to_string(port) + "}";
    comm->send_request(remove_req.c_str());
}

void ConfigMgr::send_add_request(const uint16_t port, const string & password) {
    set_comm();
    string add_req = "add: {\"server_port\": " + std::to_string(port)
                     + ", \"password\": \"" + password + "\"}";
    comm->send_request(add_req.c_str());
}

void ConfigMgr::remove_port(const char * port) {
    
    uint16_t pt = atol(port);
    
    UserMapIter iter = users.find(pt);
    
    if (iter != users.end()) {
        users.erase(iter);
        
        send_remove_request(pt);
        
        write_conf();
    } else {
        cerr << "error: could not find entry with port " << port << endl;
        exit(EXIT_FAILURE);
    }
}


void ConfigMgr::add_port(const char * port, const char * password, const char * datacap) {
    
    uint16_t pt = atol(port);
    string pwd(password);
    uint64_t cap = Converter::parse_size(datacap, true);
    
    UserMapIter iter = users.find(pt);
    User u = User(password, cap);
    bool exist = false;
    
    if (iter != users.end()) {
        exist = true;
        
        // we need to first remove the existing port
        send_remove_request(pt);
        u.usage = iter->second.usage;
        u.is_over_datacap = false;  // will check in write_conf
    }
    
    users[pt] = u;
    
    send_add_request(pt, pwd);
    
    write_conf();
}

void ConfigMgr::add_data_to_conf(const vector<uint16_t> & ports, const vector<uint64_t> & delta) {
    if (ports.size() == delta.size()) {
        for (int i = 0; i < ports.size(); i++) {
            UserMapIter iter = users.find(ports[i]);
            
            // ignore users not in our managed list
            if (iter != users.end())
                iter->second.usage += delta[i];
        }
        
        write_conf();
    }
}

bool ConfigMgr::is_over_datacap(uint16_t port) {
    UserMapIter iter = users.find(port);
    if (iter != users.end()) {
        if (iter->second.usage >= iter->second.datacap && iter->second.datacap > 0) {
            return true;
        }
    }
    return false;
}

// remove user if exceeds limit
void ConfigMgr::refresh_all_users() {
    for (UserMap::value_type u : users)
        if (is_over_datacap(u.first)) {
            send_remove_request(u.first);
            u.second.is_over_datacap = true;
            syslog(LOG_NOTICE, "removed user at port %d: over data cap", u.first);
        }
}

void ConfigMgr::reset_usage_for_all() {
    for (auto & u : users)
        u.second.usage = 0;
    write_conf();
}

void ConfigMgr::reset_usage_for_port(const char * port) {
    uint16_t pt = atol(port);
    
    UserMapIter iter = users.find(pt);
    if (iter != users.end())
        iter->second.usage = 0;
    else {
        cerr << "error: port not in use or not managed by " << SSWATCHER << endl;
        exit(EXIT_FAILURE);
    }
    write_conf();
}

void ConfigMgr::update_datacap_for_all(const char * datacap) {
    uint64_t cap = Converter::parse_size(datacap);
    
    for (auto & u : users)
        u.second.datacap = cap;
    write_conf();
}

void ConfigMgr::update_datacap_for_port(const char * port, const char * datacap) {
    uint16_t pt = atol(port);
    uint64_t cap = Converter::parse_size(datacap);
    
    UserMapIter iter = users.find(pt);
    if (iter != users.end())
        iter->second.datacap = cap;
    else {
        cerr << "error: port not in use or not managed by " << SSWATCHER << endl;
        exit(EXIT_FAILURE);
    }
    write_conf();
}
