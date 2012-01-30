/* big_file.c
 * Large file support ( > 2 GB ) for Windows platform
 */

#include <stdio.h>
#include <stdlib.h>
#include "big_file.h"

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER	((DWORD)-1)
#endif

HANDLE big_fopen (const char * file_name, const char * mode)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    if (mode[0] == 'r' && mode[1] == 0)
    {
        handle = CreateFile (file_name, GENERIC_READ, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, 0, NULL);
    }
    else if (mode[0] == 'w' && mode[1] == 0)
    {
        handle = CreateFile (file_name, GENERIC_WRITE, 0,
                NULL, CREATE_ALWAYS, 0, NULL);
    }
    return handle;
}

int big_fclose (HANDLE handle)
{
    return CloseHandle (handle) == 0;
}

size_t big_fwrite (const void *buffer, size_t size, size_t count,
                   HANDLE handle)
{
    DWORD written;
    WriteFile (handle, buffer, size*count, &written, NULL);
    return written / size;
}

size_t big_fread (void *buffer, size_t size, size_t count, HANDLE handle)
{
    DWORD read;
    ReadFile (handle, buffer, size*count, &read, NULL);
    return read / size;
}

int big_fseek (HANDLE handle, big_size_t offset, int origin)
{
    switch (origin)
    {
        case SEEK_CUR: origin = FILE_CURRENT; break;
        case SEEK_SET: origin = FILE_BEGIN; break;
        case SEEK_END: origin = FILE_END; break;
        default: return 1;
    }
    return (SetFilePointer (handle, BIG_LOW(offset), &BIG_HIGH(offset),
                origin) == INVALID_SET_FILE_POINTER &&
            GetLastError() != NO_ERROR);
    
}

big_size_t big_ftell (HANDLE handle)
{
    big_size_t pos;

    BIG_HIGH(pos) = 0;
    BIG_LOW(pos) = SetFilePointer (handle, 0, &BIG_HIGH(pos), FILE_CURRENT);
    if (BIG_LOW(pos) == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        pos = -1;
    return pos;
}

big_size_t big_fsize (HANDLE handle)
{
    big_size_t size;

    BIG_LOW(size) = GetFileSize (handle, &BIG_HIGH(size));
    if (BIG_LOW(size) == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
        size = -1;
    return size;
}

int big_feof (HANDLE handle)
{
    return big_fsize (handle) == big_ftell (handle);
}
