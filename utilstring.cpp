#include "utilstring.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

static std::string file2string(std::istream &strm)
{
	if (!strm)
		return "";
	return std::string((std::istreambuf_iterator<char>(strm)), std::istreambuf_iterator<char>());
}

std::string file2string(const char *filename)
{
	if (!filename)
		return "";
	std::ifstream t(filename);
	return file2string(t);
}

void string2file(const std::string &filename, const std::string &value)
{
	std::ofstream file(filename);
	file << value;
}

std::string uint64_t2string(uint64_t value) 
{
  std::stringstream ss;
  ss << value << value;;
  return ss.str();
}

std::string double2string(double value, int precision) 
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(precision) << value;;
  return ss.str();
}

std::string hexString
(
    const std::string &value
)
{
  std::stringstream ss;
  for (int i = 0; i < value.size(); i++) {
      ss << std::hex << std::setw(2) << std::setfill('0') << (int) (uint8_t) value[i];
  }
  return ss.str();
}

std::string asIsOrHex
(
    const std::string &value
)
{
  bool nonascii = false;
  for (int i = 0; i < value.size(); i++) {
      if ((value[i] >= 0x20) && (value[i] <= 0x7f))
          continue;
      nonascii = true;
      break;
  }
  if (nonascii)
    return hexString(value);
  else
    return value;
}

/**
 * @brief Return binary data string
 * @param hex hex string
 * @return binary data string
 */
std::string stringFromHex(const std::string &hex)
{
	std::string r(hex.length() / 2, '\0');
	std::stringstream ss(hex);
	ss >> std::noskipws;
	char c[3] = {0, 0, 0};
	int i = 0;
	while (ss >> c[0]) {
		if (!(ss >> c[1]))
			break;
		unsigned char x = (unsigned char) strtol(c, NULL, 16);
		r[i] = x;
		i++;
	}
	return r;
}
