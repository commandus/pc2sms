
#include "send-sms-config.h"
#include <sstream>

#define DEF_SVC_HOST "167.172.99.203"

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
) {
    std::stringstream ss(config);
    std::string s;
    if (std::getline(ss, s,'\n')) {
        serviceAddress = s;
    }
    if (std::getline(ss, s,'\n')) {
        login = s;
    }
    if (std::getline(ss, s,'\n')) {
        password = s;
    }
	if (serviceAddress.empty())
        serviceAddress = DEF_SVC_HOST;
	return 0;
}
