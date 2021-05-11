#ifndef UTILSTRING_H_
#define UTILSTRING_H_	1

#include <string>
#include <inttypes.h>

// read file
std::string file2string(const char *filename);

void string2file(const std::string &filename, const std::string &value);

std::string uint64_t2string(uint64_t value) ;

std::string double2string(double value, int precision);

std::string hexString
(
    const std::string &value
);

std::string asIsOrHex
(
    const std::string &value
);

/**
 * @brief Return binary data string
 * @param hex hex string
 * @return binary data string
 */
std::string stringFromHex(const std::string &hex);

#endif
