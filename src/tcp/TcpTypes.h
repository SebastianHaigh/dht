#ifndef TCP_TYPES_H_
#define TCP_TYPES_H_

#include <string>

namespace odd::tcp
{

using IpAddressString = std::string;
using IpAddressV4 = uint32_t;
using PortNumber = uint16_t;

} // namespace odd::tcp

#endif // TCP_TYPES_H_
