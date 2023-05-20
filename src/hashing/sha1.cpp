#include "sha1.h"
#include <cstdint>
#include <cstdio>
#include <iostream>

void sha1(const uint8_t *message, uint32_t messageLength, uint8_t *digest)
{
  uint8_t* paddedMessage_p = padMessage(message, messageLength);

  uint32_t numChunks = (messageLength * 8 + numberOfPaddingBits(messageLength) + 64) / 512;  // 512 bits per chunk

  uint32_t h0 = H0;
  uint32_t h1 = H1;
  uint32_t h2 = H2;
  uint32_t h3 = H3;
  uint32_t h4 = H4;

  uint32_t words[80];

  for (int chunk = 0; chunk < numChunks; chunk++)
  {
    // Break this chunk into sixteen 32 bit big endian words
    for (int i = 0; i < 16; i++)
    {
      uint32_t word = 0;
      memcpy(&word, paddedMessage_p + chunk * 64 + i * 4, 4);
      if (isLittleEndian())
      {
        word = swapEndianU32(word);
      }
      words[i] = word;
    }

    for (int i = 16; i < 80; i++)
    {
      words[i] = words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16];
      words[i] = rotateLeft(words[i], 1);
    }

    uint32_t a = h0;
    uint32_t b = h1;
    uint32_t c = h2;
    uint32_t d = h3;
    uint32_t e = h4;

    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[0]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[1]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[2]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[3]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[4]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[5]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[6]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[7]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[8]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[9]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[10]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[11]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[12]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[13]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[14]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[15]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[16]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[17]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[18]);
    sha1Round<ShaFunction::F1>(a, b, c, d, e, words[19]);

    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[20]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[21]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[22]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[23]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[24]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[25]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[26]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[27]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[28]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[29]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[30]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[31]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[32]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[33]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[34]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[35]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[36]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[37]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[38]);
    sha1Round<ShaFunction::F2>(a, b, c, d, e, words[39]);

    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[40]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[41]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[42]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[43]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[44]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[45]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[46]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[47]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[48]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[49]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[50]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[51]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[52]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[53]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[54]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[55]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[56]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[57]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[58]);
    sha1Round<ShaFunction::F3>(a, b, c, d, e, words[59]);

    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[60]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[61]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[62]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[63]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[64]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[65]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[66]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[67]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[68]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[69]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[70]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[71]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[72]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[73]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[74]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[75]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[76]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[77]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[78]);
    sha1Round<ShaFunction::F4>(a, b, c, d, e, words[79]);

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
  }

  if (isLittleEndian())
  {
    h0 = swapEndianU32(h0);
    h1 = swapEndianU32(h1);
    h2 = swapEndianU32(h2);
    h3 = swapEndianU32(h3);
    h4 = swapEndianU32(h4);
  }
  memcpy(digest, &h0, 4);
  memcpy(digest + 4, &h1, 4);
  memcpy(digest + 8, &h2, 4);
  memcpy(digest + 12, &h3, 4);
  memcpy(digest + 16, &h4, 4);

  delete[] paddedMessage_p;
}

