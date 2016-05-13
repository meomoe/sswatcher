//
//  status.hpp
//  sswatcher
//
//  Created by Jason Zhong on 5/2/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//
//  Start, stop, and show status of shadowsocks and sswatcherd
//

#ifndef status_hpp
#define status_hpp

#include <string>
using std::string;

void show_status();
bool ss_is_runnning();
bool daemon_is_running();
void start_ss(string user = "");
void stop_ss();
void restart();

#endif /* status_hpp */

