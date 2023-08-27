#ifndef IO_TCP_TYPES_H_
#define IO_TCP_TYPES_H_

#include <string>

namespace odd::io::tcp
{

using IpAddressString = std::string;
using IpAddressV4 = uint32_t;
using PortNumber = uint16_t;

} // namespace odd::io::tcp

#endif // IO_TCP_TYPES_H_
