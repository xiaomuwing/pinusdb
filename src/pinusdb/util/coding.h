/*
* Copyright (c) 2019 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http://www.gnu.org/licenses>
*/

#pragma once
#include <stdint.h>
#include <string.h>
#include <string>

#define PLATFORM_LITTLE_ENDIAN

  //       int32 ->     uint32
  // -------------------------
  //           0 ->          0
  //          -1 ->          1
  //           1 ->          2
  //          -2 ->          3
  //         ... ->        ...
  //  2147483647 -> 4294967294
  // -2147483648 -> 4294967295
  //
  //        >> encode >>
  //        << decode <<

class Coding
{
public:
  static uint32_t ZigzagEncode32(int32_t n)
  {
    return static_cast<uint32_t>((n << 1) ^ (n >> 31));
  }
  
  static int32_t ZigzagDecode32(uint32_t n)
  {
    return static_cast<int32_t>(n >> 1) ^ -static_cast<int32_t>(n & 1);
  }
  
  static uint64_t ZigzagEncode64(int64_t n)
  {
    return static_cast<uint64_t>((n << 1) ^ (n >> 63));
  }
  
  static int64_t ZigzagDecode64(uint64_t n)
  {
    return static_cast<int64_t>(n >> 1) ^ -static_cast<int64_t>(n & 1);
  }

  ///////////////////////////////////////////////////////////////////////
  static uint8_t* VarintEncode32(uint8_t* dst, uint32_t v) {
    static const int B = 128;
    unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
    if (v < (1 << 7)) {
      *(ptr++) = v;
    }
    else if (v < (1 << 14)) {
      *(ptr++) = v | B;
      *(ptr++) = v >> 7;
    }
    else if (v < (1 << 21)) {
      *(ptr++) = v | B;
      *(ptr++) = (v >> 7) | B;
      *(ptr++) = v >> 14;
    }
    else if (v < (1 << 28)) {
      *(ptr++) = v | B;
      *(ptr++) = (v >> 7) | B;
      *(ptr++) = (v >> 14) | B;
      *(ptr++) = v >> 21;
    }
    else {
      *(ptr++) = v | B;
      *(ptr++) = (v >> 7) | B;
      *(ptr++) = (v >> 14) | B;
      *(ptr++) = (v >> 21) | B;
      *(ptr++) = v >> 28;
    }
    return (ptr);
  }

  static uint8_t* VarintEncode64(uint8_t* dst, uint64_t v) {
    static const int B = 128;
    while (v >= B) {
      *(dst++) = (v & (B - 1)) | B;
      v >>= 7;
    }
    *(dst++) = static_cast<unsigned char>(v);
    return dst;
  }

  static const char* VarintDecode64(const char* p, const char* limit, uint64_t* value) {
    uint64_t result = 0;
    for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7)
    {
      uint64_t byte = *(reinterpret_cast<const unsigned char*>(p));
      p++;
      if (byte & 128) {
        result |= ((byte & 127) << shift);
      }
      else {
        result |= (byte << shift);
        *value = result;
        return p;
      }
    }

    return nullptr;
  }

  static const char* VarintDecode32(const char* p, const char* limit, uint32_t* value) {
    uint32_t result = 0;
    for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7)
    {
      uint32_t byte = *(reinterpret_cast<const unsigned char*>(p));
      p++;
      if (byte & 128) {
        result |= ((byte & 127) << shift);
      }
      else {
        result |= (byte << shift);
        *value = result;
        return p;
      }
    }

    return nullptr;
  }

  ///////////////////////////////////////////////////////////////////////

  static void PutVarint32(std::string* pDstStr, uint32_t value)
  {
    static const int B = 128;
    char buf[10];
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    while (value >= B) {
      *(ptr++) = (value & (B - 1)) | B;
      value >>= 7;
    }
    *ptr = static_cast<uint8_t>(value);
    ptr++;
    pDstStr->append(buf, (ptr - reinterpret_cast<uint8_t*>(buf)));
  }

  static void PutVarint64(std::string* pDstStr, uint64_t value)
  {
    static const int B = 128;
    char buf[10];
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    while (value >= B) {
      *(ptr++) = (value & (B - 1)) | B;
      value >>= 7;
    }
    *ptr = static_cast<uint8_t>(value);
    ptr++;
    pDstStr->append(buf, (ptr - reinterpret_cast<uint8_t*>(buf)));
  }

  static void PutFixed16(std::string* pDstStr, uint16_t value)
  {
    uint8_t buf[2];
#ifdef PLATFORM_LITTLE_ENDIAN
    memcpy(buf, &value, 2);
#else
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
#endif
    pDstStr->append(reinterpret_cast<char*>(buf), 2);
  }

  static void PutFixed32(std::string* pDstStr, uint32_t value)
  {
    uint8_t buf[4];
#ifdef PLATFORM_LITTLE_ENDIAN
    memcpy(buf, &value, 4);
#else
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
#endif
    pDstStr->append(reinterpret_cast<char*>(buf), 4);
  }

  static void PutFixed64(std::string* pDstStr, uint64_t value)
  {
    uint8_t buf[8];
#ifdef PLATFORM_LITTLE_ENDIAN
    memcpy(buf, &value, 8);
#else
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
    buffer[4] = static_cast<uint8_t>(value >> 32);
    buffer[5] = static_cast<uint8_t>(value >> 40);
    buffer[6] = static_cast<uint8_t>(value >> 48);
    buffer[7] = static_cast<uint8_t>(value >> 56);
#endif
    pDstStr->append(reinterpret_cast<char*>(buf), 8);
  }

  inline static uint32_t EncodeFloat(float val)
  {
    union { float f; uint32_t i; };
    f = val;
    return i;
  }

  inline static float DecodeFloat(uint32_t val)
  {
    union { float f; uint32_t i; };
    i = val;
    return f;
  }

  inline static uint64_t EncodeDouble(double val)
  {
    union { double d; uint64_t i; };
    d = val;
    return i;
  }

  inline static double DecodeDouble(uint64_t val)
  {
    union { double d; uint64_t i; };
    i = val;
    return d;
  }

  //////////////////////////////////////////////////////////////////////

  static void FixedEncode16(char* dst, uint16_t value)
  {
#ifdef PLATFORM_LITTLE_ENDIAN
    memcpy(dst, &value, sizeof(value));
#else
    dst[0] = value & 0xff;
    dst[1] = (value >> 8) & 0xff;
#endif
  }

  static void FixedEncode32(char* dst, uint32_t value)
  {
#ifdef PLATFORM_LITTLE_ENDIAN
    memcpy(dst, &value, sizeof(value));
#else
    dst[0] = value & 0xff;
    dst[1] = (value >> 8) & 0xff;
    dst[2] = (value >> 16) & 0xff;
    dst[3] = (value >> 24) & 0xff;
#endif
  }

  static void FixedEncode64(char* dst, uint64_t value)
  {
#ifdef PLATFORM_LITTLE_ENDIAN
    memcpy(dst, &value, sizeof(value));
#else
    dst[0] = value & 0xff;
    dst[1] = (value >> 8) & 0xff;
    dst[2] = (value >> 16) & 0xff;
    dst[3] = (value >> 24) & 0xff;
    dst[4] = (value >> 32) & 0xff;
    dst[5] = (value >> 40) & 0xff;
    dst[6] = (value >> 48) & 0xff;
    dst[7] = (value >> 56) & 0xff;
#endif
  }

  static uint16_t FixedDecode16(const char* p)
  {
#ifdef PLATFORM_LITTLE_ENDIAN
    uint16_t result;
    memcpy(&result, p, sizeof(result));
    return result;
#else
    return ((static_cast<uint16_t>(static_cast<unsigned char>(p[0])))
      | (static_cast<uint16_t>(static_cast<unsigned char>(p[1])) << 8));
#endif
  }

  static uint32_t FixedDecode32(const char* p)
  {
#ifdef PLATFORM_LITTLE_ENDIAN
    uint32_t result;
    memcpy(&result, p, sizeof(result));
    return result;
#else
    return ((static_cast<uint32_t>(static_cast<unsigned char>(p[0])))
      | (static_cast<uint32_t>(static_cast<unsigned char>(p[1])) << 8)
      | (static_cast<uint32_t>(static_cast<unsigned char>(p[2])) << 16)
      | (static_cast<uint32_t>(static_cast<unsigned char>(p[3])) << 24));
#endif
  }
  
  static uint64_t FixedDecode64(const char* p)
  {
#ifdef PLATFORM_LITTLE_ENDIAN
    uint64_t result;
    memcpy(&result, p, sizeof(result));
    return result;
#else
    return ((static_cast<uint64_t>(static_cast<unsigned char>(p[0])))
      | (static_cast<uint64_t>(static_cast<unsigned char>(p[1])) << 8)
      | (static_cast<uint64_t>(static_cast<unsigned char>(p[2])) << 16)
      | (static_cast<uint64_t>(static_cast<unsigned char>(p[3])) << 24)
      | (static_cast<uint64_t>(static_cast<unsigned char>(p[4])) << 32)
      | (static_cast<uint64_t>(static_cast<unsigned char>(p[5])) << 40)
      | (static_cast<uint64_t>(static_cast<unsigned char>(p[6])) << 48)
      | (static_cast<uint64_t>(static_cast<unsigned char>(p[7])) << 56));
#endif
  }

};