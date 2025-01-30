#ifndef SEND_SMS_CONFIG_H_
#define SEND_SMS_CONFIG_H_	1

#include <string>

class Pc2SmsClientConfig {
public:
    std::string serviceAddress;
    std::string login;
    std::string password;
};

/**
 *  config file consists of lines:
 *  service address
 *  login
 *  password
 */
int parseClientConfig(
    Pc2SmsClientConfig &retVal,
    const std::string &config
);

#endif
