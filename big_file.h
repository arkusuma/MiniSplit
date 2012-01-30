/* big_file.h
 * Large file support ( > 2 GB ) for Windows platform
 */

#ifndef _BIG_FILE_H_
#define _BIG_FILE_H_

#include <windows.h>

typedef __int64 big_size_t;

#define BIG_LOW(x)  ((*(LARGE_INTEGER *) &(x)).LowPart)
#define BIG_HIGH(x) ((*(LARGE_INTEGER *) &(x)).HighPart)

HANDLE big_fopen (const char * file_name, const char * mode);
int big_fclose (HANDLE file_handle);

size_t big_fwrite (const void *buffer, size_t size, size_t count,
                   HANDLE handle);
size_t big_fread (void *buffer, size_t size, size_t count, HANDLE handle);

int big_fseek (HANDLE handle, big_size_t offset, int origin);
big_size_t big_ftell (HANDLE handle);

big_size_t big_fsize (HANDLE handle);

int big_feof (HANDLE handle);

#endif
