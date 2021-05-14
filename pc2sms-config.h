#ifndef PC2SMS_CONFIG_H_
#define PC2SMS_CONFIG_H_	1

#include <string>

#include "sms-listener.h"

/**
 * config file consists of lines:
 *  service address, e.g. localhost:50051
 *  login
 *  password
 */
int parseServiceConfig(
	std::string &listenAddress,
    std::string &login,
    std::string &password,
    NotifyPolicy &policy,
    const std::string &config
);

#endif
