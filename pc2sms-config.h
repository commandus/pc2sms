#ifndef PC2SMS_CONFIG_H_
#define PC2SMS_CONFIG_H_	1

#include <string>

#include "sms-listener.h"

class Pc2SmsConfig {
public:
    std::string listenAddress;
    std::string login;
    std::string password;
    NotifyPolicy policy;
    bool daemonize;
    int verbose;
    std::string pidfile;
    std::string toString() const;
};

/**
 * config file consists of lines:
 *  service address, e.g. localhost:50051
 *  login
 *  password
 */
int parseServiceConfig(
    Pc2SmsConfig &retVal,
    const std::string &configLines
);

#endif
