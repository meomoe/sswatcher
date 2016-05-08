//
//  communicator.hpp
//  sswatcher
//
//  Created by Jason Zhong on 5/3/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#ifndef communicator_hpp
#define communicator_hpp

#include <sys/un.h>
#include <string>

#define SS_COMM "ss-comm"

using std::string;

class Communicator {
public:
    void send_request(const char * request);
    string receive();
    
    Communicator(string client);
    ~Communicator();
    
private:
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    int sockfd = -1;    
};

#endif /* communicator_hpp */
