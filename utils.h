#ifndef UTIL_H
#define UTIL_H

#include "big_file.h"

typedef struct
{
  WIN32_FIND_DATA time;
  DWORD attr;
}
TIME_ATTR;

int FileExists (const char *FileName);
/* Return TRUE if FileName exists */

void GetFileName (const char *file, char *buf);
/* Get file name only, without leading path */

void GetFilePath (const char *file, char *buf);
/* Get file path only (file name and '\' removed, except for X:\ ) */

void GetNoExtFileName (const char *file, char *buf);
/* Trim file extension, if there's no extension output is the same */

int GetFileTimeAttributes (const char *fname, TIME_ATTR * attr);
/* Get information about file creation time and attributes */

void SetFileTimeAttributes (const char *fname, TIME_ATTR * attr);
/* Set file creation time and attributes */

char *GetSystemErrorMessage (void);

void BigIntToStr (big_size_t N, char *out);

char *FormatInt (big_size_t N);

#endif
