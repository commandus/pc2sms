
#include "send-sms-config.h"
#include <sstream>

#define DEF_SVC_HOST "82.97.249.193:50053"

/**
 *  config file consists of lines:
 *  service address
 *  login
 *  password
 */
int parseClientConfig(
    Pc2SmsClientConfig &retVal,
    const std::string &config
) {
    std::stringstream ss(config);
    std::string s;
    if (std::getline(ss, s,'\n')) {
        retVal.serviceAddress = s;
    }
    if (std::getline(ss, s,'\n')) {
        retVal.login = s;
    }
    if (std::getline(ss, s,'\n')) {
        retVal.password = s;
    }
	if (retVal.serviceAddress.empty())
        retVal.serviceAddress = DEF_SVC_HOST;
	return 0;
}
