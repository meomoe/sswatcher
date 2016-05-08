//
//  sys_helper.hpp
//  sswatcher
//
//  Created by Jason Zhong on 5/2/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#ifndef sys_helper_hpp
#define sys_helper_hpp

#include <string>
#include <signal.h>
using std::string;

int get_pid_by_name(const string & name);
bool file_exist(const string & name);
string get_exec_path();
void send_sigusr(pid_t pid);

#endif /* sys_helper_hpp */
