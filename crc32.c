#define CRC_POLY 0xEDB88320

unsigned long crc_table[256];

void crcCreateTable (void)
{
    int i, j;
    unsigned long c;

    for (i = 0; i < 256; i++)
    {
        c = i;
        for (j = 0; j < 8; j++)
            c = (c >> 1) ^ (c & 1 ? CRC_POLY : 0);
        crc_table[i] = c;
    }
}

unsigned long crcBytes (unsigned long crc, const char *codes, int count)
{
    int i;

    crc ^= 0xffffffff;
    for (i = 0; i < count; i++)
        crc = crc_table[(codes[i] ^ crc) & 0xff] ^ (crc >> 8);
    return crc ^ 0xffffffff;
}
