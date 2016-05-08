//
//  path.hpp
//  sswatcher
//
//  Created by Jason Zhong on 5/6/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#ifndef path_hpp
#define path_hpp

#include <string>

#define SS_CONFIG "/etc/shadowsocks.json"
#define SSSERVER_REGEX "\"ssserve(r( ){0,}){1}\""
#define SSWATCHERD_REGEX "\"sswatcher(d( ){0,}){1}\""

using std::string;

class Path {
public:
    static string ss_config();
    static void set_ss_config(string path);
    static string watcher_config();
    static string watcher_sock();
    static string watcherd_sock();
    static string server_sock();
    static string datacap();
    static string config_dir();
    
private:
    static string path;
    static string home_path();
};

#endif /* path_hpp */
