#ifndef COMMS_CODER_H_
#define COMMS_CODER_H_

#include <cstddef>
#include <cstdint>
#include <bit>
#include <iostream>
#include <type_traits>

namespace odd {

constexpr void swapEndian8(uint8_t* value)
{
}

constexpr void swapEndian16(uint16_t* value)
{
  uint16_t toSwap = *value;
  *value = (toSwap << 8) | (toSwap >> 8);
}

constexpr void swapEndian32(uint32_t* value)
{
  uint32_t toSwap = *value;
  toSwap = ((toSwap << 8) & 0xFF00FF00) | ((toSwap >> 8) & 0xFF00FF);
  *value = (toSwap << 16) | (toSwap >> 16);
}

constexpr void swapEndian64(uint64_t* value)
{
  uint64_t toSwap = *value;
  toSwap = ((toSwap << 8) & 0xFF00FF00FF00FF00ULL) | ((toSwap >> 8) & 0x00FF00FF00FF00FFULL);
  toSwap = ((toSwap << 16) & 0xFFFF0000FFFF0000ULL) | ((toSwap >> 16) & 0x0000FFFF0000FFFFULL);
  *value = (toSwap << 32) | (toSwap >> 32);
}

constexpr void swapEndianAny(uint8_t* firstByte, uint8_t* lastByte)
{
  while (firstByte < lastByte)
  {
    std::swap(*firstByte, *lastByte);
    firstByte++;
    lastByte--;
  }
}

template<typename T>
void swapEndian(T* value)
{
  if constexpr (sizeof(T) == 1)
  {
    return;
  }
  else if constexpr (sizeof(T) == 2)
  {
    swapEndian16(value);
  }
  else if constexpr (sizeof(T) == 4)
  {
    swapEndian32(value);
  }
  else if constexpr (sizeof(T) == 8)
  {
    swapEndian64(value);
  }
  else
  {
    auto* firstByte = reinterpret_cast<uint8_t*>(value);

    swapEndianAny(firstByte,
                  firstByte + sizeof(T) - 1);
  }
}

template<typename T>
void encodeSingleValue(const T* toEncode, uint8_t* encoded)
{
  *reinterpret_cast<T*>(encoded) = *toEncode;

  if constexpr (std::endian::native == std::endian::little)
  {
    swapEndian(reinterpret_cast<T*>(encoded));
  }
}

template<typename T>
void decodeSingleValue(uint8_t* toDecode, T* decoded)
{
  *decoded = *reinterpret_cast<T*>(toDecode);

  if constexpr (std::endian::native == std::endian::little)
  {
    swapEndian(reinterpret_cast<T*>(decoded));
  }
}

} // namespace odd

#endif // COMMS_CODER_H_
