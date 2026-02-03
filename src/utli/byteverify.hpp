#include <cstdint>
#include <qcontainerfwd.h>
#include <qstringview.h>

namespace ByteVerify {
template <typename T> T reverseBits(T num, int bits) {
  T res = 0;
  for (int i = 0; i < bits; ++i) {
    if ((num >> i) & 1)
      res |= (static_cast<T>(1) << (bits - 1 - i));
  }
  return res;
}

template <typename T>
T calcCRC(const QByteArray &data, T poly, T init, bool refIn, bool refOut,
          T xorOut) {
  T crc = init;
  int bits = sizeof(T) * 8;

  for (uint8_t byte : data) {
    T c = static_cast<unsigned char>(byte);
    if (refIn)
      c = reverseBits(c, 8);
    crc ^= (c << (bits - 8));
    for (int i = 0; i < 8; ++i) {
      if (crc & (static_cast<T>(1) << (bits - 1)))
        crc = (crc << 1) ^ poly;
      else
        crc = (crc << 1);
    }
  }

  if (bits == 16)
    crc &= 0xFFFF;
  if (bits == 32)
    crc &= 0xFFFFFFFF;

  if (refOut)
    crc = reverseBits(crc, bits);

  return crc ^ xorOut;
}

template <typename T> T calcCheckSum(const QByteArray &data, int mode = 0) {
  T sum = 0;
  for (uint8_t byte : data) {
    sum += byte;
  }
  if (mode == 1)
    return ~sum;
  if (mode == 2)
    return (static_cast<T>(~sum) + 1);
  return sum;
}

inline uint8_t calcLRC(const QByteArray &data) {
  uint8_t lrc = 0;
  for (uint8_t byte : data) {
    lrc += byte;
  }
  return static_cast<uint8_t>(-static_cast<int8_t>(lrc));
}

inline uint8_t calcBCC(const QByteArray &data) {
  uint8_t bcc = 0;
  for (uint8_t byte : data) {
    bcc ^= byte;
  }
  return bcc;
}

inline uint16_t crc16_ibm(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x8005, 0x0000, true, true, 0x0000);
}
inline uint16_t crc16_maxim(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x8005, 0x0000, true, true, 0xFFFF);
}
inline uint16_t crc16_usb(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x8005, 0xFFFF, true, true, 0xFFFF);
}
inline uint16_t crc16_modbus(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x8005, 0xFFFF, true, true, 0x0000);
}
inline uint16_t crc16_ccitt(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x1021, 0x0000, true, true, 0x0000);
}
inline uint16_t crc16_ccitt_false(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x1021, 0xFFFF, false, false, 0x0000);
}
inline uint16_t crc16_x25(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x1021, 0xFFFF, true, true, 0xFFFF);
}
inline uint16_t crc16_xmodem(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x1021, 0x0000, false, false, 0x0000);
}
inline uint16_t crc16_dnp(const QByteArray &data) {
  return calcCRC<uint16_t>(data, 0x3D65, 0x0000, true, true, 0xFFFF);
}
inline uint16_t crc32(const QByteArray &data) {
  return calcCRC<uint32_t>(data, 0x04C11DB7, 0xFFFFFFFF, true, true,
                           0xFFFFFFFF);
}
inline uint16_t crc32_mpeg2(const QByteArray &data) {
  return calcCRC<uint32_t>(data, 0x04C11DB7, 0xFFFFFFFF, false, false,
                           0x00000000);
}

inline uint8_t checksum8(const QByteArray &data) {
  return calcCheckSum<uint8_t>(data);
}
inline uint16_t checksum16(const QByteArray &data) {
  return calcCheckSum<uint16_t>(data);
}
} // namespace ByteVerify
