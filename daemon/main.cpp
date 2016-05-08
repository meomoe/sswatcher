//
//  main.cpp
//  sswatcher daemon
//
//  Created by Jason Zhong on 5/2/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//
//  Credit to Doug Potter http://www.itp.uzh.ch/~dpotter/howto/daemonize (CC-PD)
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <string>
#include <vector>
#include "json/json.h"
#include "shared_const.h"
#include "sys_helper.hpp"
#include "operation.hpp"
#include "communicator.hpp"

#define DAEMON_LOCK_FILE "/var/run/sswatcherd.pid"

using std::string;
using std::vector;

static int lfp = -1;
volatile sig_atomic_t should_connect = 0;

static void child_handler(int signum);

static void exit_handler(int signum);

static void connect_handler(int signum);

static void check_prereq();

static void daemonize(const char *lockfile);

static void handle_response(const string &response);

int main(int argc, const char * argv[]) {
    
    openlog(SSWATCHERD, LOG_PID, LOG_LOCAL5);
    syslog(LOG_NOTICE, "starting");
    
    check_prereq();
    
    char user[] = "";
    if (argc == 3) {
        string task(argv[1]);
        if (task.compare("user") == 0)
            strcpy(user, argv[2]);
    }
    
    // daemonize
    daemonize(DAEMON_LOCK_FILE);
    
    // set up connection with ssserver
    Communicator comm(Path::watcherd_sock());
    comm.send_request("ping");
    
    // drop user if there is one
    if (getuid() == 0 || geteuid() == 0) {
        struct passwd *pw = getpwnam(user);
        if (pw) {
            syslog(LOG_NOTICE, "dropping root, setting user to %s", user);
            setuid(pw->pw_uid);
        }
    }
    
    string response;
    
    signal(SIGUSR1, connect_handler);
    signal(SIGUSR2, connect_handler);
    
    while (true) {
        
        response = comm.receive();
        ConfigMgr::instance().load_default_conf();
        #ifdef DEBUG
            syslog(LOG_NOTICE, "%s", response.c_str());
        #endif
        
        if (response.substr(0, 5) == "stat:") {
            handle_response(response);
        } else if (response == "") {
            // timeout or failure occurred; re-establishing connection
            comm.send_request("ping");
            syslog(LOG_DEBUG, "trying to re-establish connection");
        }
        
        if (should_connect) {
            comm.send_request("ping");
            should_connect = 0;
        }
    }
    
    return 0;
}

static void handle_response(const string &response) {
    
    Json::Value root;
    Json::Reader reader;
    
    // stat: {"port": usage, ...}
    
    string res = response.substr(6);
    
    bool success = reader.parse(res.c_str(), root);
    if (success) {
        
        vector<uint16_t> ports = {};
        vector<uint64_t> delta = {};
        
        for (Json::ValueIterator i = root.begin(); i != root.end(); i++) {
            string port_str = i.key().asString();
            uint16_t port = atol(port_str.c_str());
            uint64_t d = root.get(port_str, "0").asUInt64();
            
            ports.push_back(port);
            delta.push_back(d);
            
            syslog(LOG_DEBUG, "port %d: %llu bytes", port, d);
        }
        
        ConfigMgr::instance().add_data_to_conf(ports, delta);
    } else {
        syslog(LOG_WARNING, "failed to parse data %s", reader.getFormattedErrorMessages().c_str());
    }
}


static void child_handler(int signum) {
    switch(signum) {
        case SIGALRM: exit(EXIT_FAILURE); break;
        case SIGUSR1:
            printf("%s (%s daemon) started\n", SSWATCHERD, SSWATCHER);
            exit(EXIT_SUCCESS); break;
        case SIGCHLD: exit(EXIT_FAILURE); break;
    }
}

static void exit_handler(int signum) {
    flock(lfp, LOCK_UN | LOCK_NB);
    close(lfp);
    exit(EXIT_SUCCESS);
}

static void connect_handler(int signum) {
    if (signum == SIGUSR1 || signum == SIGUSR2)
        should_connect = 1;
    signal(SIGUSR1, connect_handler);
    signal(SIGUSR2, connect_handler);
}

static void check_prereq() {
    if (getuid() != 0 || geteuid() != 0) {
        syslog(LOG_ERR, "error: " SSWATCHERD " requires root privilege");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid = get_pid_by_name(SSSERVER_REGEX);
    if (pid == -1) {
        syslog(LOG_ERR, "error: " SSSERVER " not running");
        exit(EXIT_FAILURE);
    }
}

static void daemonize(const char *lockfile) {
    pid_t pid, sid, parent;
    
    // already a daemon
    if (getppid() == 1) return;
    
    /* Create the lock file as the current user */
    if (lockfile && lockfile[0]) {
        lfp = open(lockfile, O_RDWR | O_CREAT, 0640);
        if (lfp < 0) {
            syslog(LOG_ERR, "unable to create lock file %s, code=%d (%s)",
                   lockfile, errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        int rc = flock(lfp, LOCK_EX | LOCK_NB);
        if (rc) {
            syslog(LOG_ERR, "lock file %s locked, code=%d (%s)",
                   lockfile, errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    // trap signals that we expect to recieve
    signal(SIGCHLD, child_handler);
    signal(SIGUSR1, child_handler);
    signal(SIGALRM, child_handler);
    
    // fork off the parent process
    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "unable to fork daemon, code=%d (%s)",
               errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
        
        /* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
         for two seconds to elapse (SIGALRM).  pause() should not return. */
        alarm(2);
        pause();
        
        exit(EXIT_FAILURE);
    }
    
    // at this point we are executing as the child process
    parent = getppid();
    
    /* Cancel certain signals */
    signal(SIGCHLD, SIG_DFL); /* A child process dies */
    signal(SIGTSTP, SIG_IGN); /* TTY signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP,  SIG_IGN); /* hangup signal */
    signal(SIGINT,  exit_handler);
    signal(SIGTERM, exit_handler);
    
    // Change the file mode mask
    umask(0);
    
    // create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "unable to create a new session, code %d (%s)",
               errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Redirect standard files to /dev/null
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    
    // save current PID for reference
    write(lfp, &pid, sizeof(pid));
    
    // tell the parent process that we have spawned
    kill(parent, SIGUSR1);
}
