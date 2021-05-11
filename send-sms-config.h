#ifndef SEND_SMS_CONFIG_H_
#define SEND_SMS_CONFIG_H_	1

#include <string>

/**
 *  config file consists of lines:
 *  service address
 *  login
 *  password
 */
int parseClientConfig(
	std::string &serviceAddress,
    std::string &login,
    std::string &password,
    const std::string &config
);

#endif
