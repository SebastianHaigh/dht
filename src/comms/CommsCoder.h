#ifndef COMMS_CODER_H_
#define COMMS_CODER_H_

#include <cstddef>
#include <cstdint>
#include <bit>
#include <iostream>
#include <type_traits>

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

template<typename T>
void swapEndian(T* value)
{
  static_assert((sizeof(T) == 1) || (sizeof(T) == 2) || (sizeof(T) == 4) || (sizeof(T) == 8),
                "Cannot swap endian for this value size, must be 1, 2, 4, or 8 bytes");

  if constexpr (sizeof(T) == 1)
  {
    swapEndian8(value);
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
}

template<typename T>
void encodeSingleValue(T* toEncode, uint8_t* encoded)
{
  static_assert(sizeof(T) <= 8, "Single value must be less than 8 bytes to encode");

  *reinterpret_cast<T*>(encoded) = *toEncode;

  if constexpr (std::endian::native == std::endian::little)
  {
    swapEndian(reinterpret_cast<T*>(encoded));
  }
}

template<typename T>
void decodeSingleValue(uint8_t* toDecode, T* decoded)
{
  static_assert(sizeof(T) <= 8, "Single value must be less than 8 bytes to decode");

  *decoded = *reinterpret_cast<T*>(toDecode);

  if constexpr (std::endian::native == std::endian::little)
  {
    swapEndian(reinterpret_cast<T*>(decoded));
  }
}

#endif // COMMS_CODER_H_
