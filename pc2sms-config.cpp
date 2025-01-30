#include "pc2sms-config.h"
#include <sstream>

/**
 * config file consists of lines:
 *  listen address
 *  allowed login
 *  allowed password
 *  allow user to add device himself
 */
int parseServiceConfig(
    Pc2SmsConfig &retVal,
    const std::string &configLines
) {
    std::stringstream ss(configLines);
    std::string s;
    if (std::getline(ss, s,'\n')) {
        retVal.listenAddress = s;
    }
    if (retVal.listenAddress.empty())
        retVal.listenAddress = "0.0.0.0:50053";
    if (std::getline(ss, s,'\n')) {
        retVal.login = s;
    }
    if (std::getline(ss, s,'\n')) {
        retVal.password = s;
    }
    if (std::getline(ss, s,'\n')) {
        if (s.empty()) {
            retVal.policy = NP_ROUND_ROBIN;
        } else {
            retVal.policy = (s.find('a') == std::string::npos) ? NP_ALL : NP_ROUND_ROBIN;
        }
    }
    if (std::getline(ss, s,'\n')) {
        if (s.empty()) {
            retVal.daemonize = false;
        } else {
            retVal.daemonize = (s.find('d') != std::string::npos);
        }
    }
	return 0;
}
