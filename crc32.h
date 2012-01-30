#ifndef CRC32_H
#define CRC32_H

void crcCreateTable (void);
unsigned long crcBytes (unsigned long crc, const char *codes, int count);

#endif
