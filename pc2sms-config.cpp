#include "pc2sms-config.h"
#include <sstream>

/**
 * config file consists of lines:
 *  conninfo
 *  login
 *  password
 *  allow user to add device himself
 */
int parseServiceConfig(
	std::string &listenAddress,
    std::string &login,
    std::string &password,
    NotifyPolicy &policy,
    const std::string &config
) {
    std::stringstream ss(config);
    std::string s;
    if (std::getline(ss, s,'\n')) {
        listenAddress = s;
    }
    if (std::getline(ss, s,'\n')) {
        login = s;
    }
    if (std::getline(ss, s,'\n')) {
        password = s;
    }
    if (std::getline(ss, s,'\n')) {
        if (s.empty()) {
            policy = NP_ROUND_ROBIN;
        } else {
            policy = (s.find('a') == std::string::npos) ? NP_ALL : NP_ROUND_ROBIN;
        }
        
    }
	if (listenAddress.empty())
        listenAddress = "0.0.0.0:50053";
	return 0;
}
