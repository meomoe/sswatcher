//
//  operation.hpp
//  sswatcher
//
//  Created by Jason Zhong on 5/2/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//
//  Controls shadowsocks and sswatcher configurations
//

#ifndef operation_hpp
#define operation_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "communicator.hpp"

using std::string;
using std::vector;
using std::map;

uint64_t get_default_datacap();
void set_default_datacap(const char * datacap);
string get_formatted_datacap(uint64_t datacap = -1);

struct User {
    string password;
    uint64_t datacap;
    uint64_t usage = 0;
    bool is_over_datacap = false;
    User();
    User(const string & password);
    User(const string & password, const uint64_t datacap);
    User(const string & password, const uint64_t datacap, const uint64_t usage);
};

class ConfigMgr {
public:
    typedef map<uint16_t, User> UserMap;
    typedef UserMap::iterator UserMapIter;
    
    static ConfigMgr & instance() {
        static ConfigMgr mgr;
        return mgr;
    }
    
    void load_default_conf();
    // on start, give ss info about our managed users
    void init_ss();
    
    void import_from(std::istream & input);
    void export_to(std::ostream & output, bool readable = false);
    void remove_port(const char * port);
    void add_port(const char * port, const char * password, const char * datacap = 0);
    
    void update_datacap_for_all(const char * datacap);
    void update_datacap_for_port(const char * port, const char * datacap = 0);
    
    void add_data_to_conf(const vector<uint16_t> & ports, const vector<uint64_t> & delta);
    void reset_usage_for_all();
    void reset_usage_for_port(const char * port);
    
    void refresh_all_users();
    
private:
    
    ConfigMgr();
    map<uint16_t, User> users;
    Communicator * comm;
    
    void write_conf();
    
    unsigned load_conf(std::istream & input);
    void set_comm();
    
    void send_remove_request(const uint16_t port);
    void send_add_request(const uint16_t port, const string & password);
    
    bool is_over_datacap(const uint16_t port);
    
    
public:
    ConfigMgr(ConfigMgr const&)        = delete;
    void operator = (ConfigMgr const&) = delete;
};

#endif /* operation_hpp */
