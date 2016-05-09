//
//  communicator.cpp
//  sswatcher
//
//  Created by Jason Zhong on 5/3/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#include "communicator.hpp"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <syslog.h>
#include "shared_const.h"
#include "sys_helper.hpp"

Communicator::Communicator(string client) {
    
    openlog(SS_COMM, LOG_PID, LOG_LOCAL5);
    
    if (file_exist(client)) {
        if (remove(client.c_str()) != 0) {
            syslog(LOG_WARNING, "unable to remove existing socket file, code=%d (%s)",
                   errno, strerror(errno));
        }
    }
    
    // unix datagram socket
    errno = 0;
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        syslog(LOG_ERR, "unable to create socket, code=%d (%s)",
               errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // set server and client socket addr
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, Path::server_sock().c_str());
    
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, client.c_str());
    
    // bind client sock
    if (bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
        syslog(LOG_ERR, "unable to bind socket, code=%d (%s)",
               errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        syslog(LOG_ERR, "unable to connect to server, code=%d (%s)",
               errno, strerror(errno));
        printf("error: unable to connect to ssserver, code=%d (%s)\n", errno, strerror(errno));
        printf("       consider stopping ssserver first:\n");
        printf("       sudo ssserver -d stop");
        exit(EXIT_FAILURE);
    }
}

void Communicator::send_request(const char * request) {
    
    if (send(sockfd, request, strlen(request), 0) == -1) {
        syslog(LOG_ERR, "unable to send data, code=%d (%s)",
               errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

string Communicator::receive() {
    char data[2048] = "";
    const size_t len = 2048;
    
    fd_set set;
    struct timeval timeout;
    int rv;
    
    FD_ZERO(&set);
    FD_SET(sockfd, &set);
    
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;
    
    rv = select(sockfd + 1, &set, NULL, NULL, &timeout);
    
    if(rv == -1) {
        if (errno != EINTR)
            syslog(LOG_WARNING, "unable to receive data, code=%d (%s)",
                   errno, strerror(errno));
        else
            strcpy(data, "interrupt");
    } else if(rv == 0) {
        syslog(LOG_DEBUG, "connection with ssserver timeout");
    } else {
        if (read(sockfd, data, len) == -1) {
            syslog(LOG_WARNING, "unable to receive data, code=%d (%s)",
                   errno, strerror(errno));
        }
    }
    
    return string(data);
}


Communicator::~Communicator() {
    close(sockfd);
    closelog();
}

