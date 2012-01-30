/* File   : util.c
 * Author : AR Kusuma
 * Update : 17 February 2003
 *
 * Desc.  : Miscellaneous file-routines
 */

#include <windows.h>
#include <stdio.h>
#include "utils.h"

/*========================================================*/

int FileExists (const char *FileName)
{
    HANDLE fh;

    fh = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (fh != INVALID_HANDLE_VALUE)
    {
        CloseHandle(fh);
        return TRUE;
    }
    return FALSE;
}

/*========================================================*/

void GetFileName (const char *file, char *buf)
{
    int i;

    i = strlen (file) - 1;
    while (i >= 0)
    {
        if (file[i] == '\\')
            break;
        i--;
    }
    strcpy (buf, &file[i + 1]);
}

/*========================================================*/

void GetFilePath (const char *file, char *buf)
{
    int i;

    i = strlen (file) - 1;
    while (i > 0 && file[i] != '\\')
        i--;
    if (i <= 2)
        i++;

    strncpy (buf, file, i);
    buf[i] = 0;
}

/*========================================================*/

void GetNoExtFileName (const char *file, char *buf)
{
    int i;

    strcpy (buf, file);
    i = strlen (buf) - 1;
    while (i > 0 && buf[i] != '.')
        i--;
    if (buf[i] == '.')
        buf[i] = 0;
}

/*========================================================*/

int GetFileTimeAttributes (const char *fname, TIME_ATTR * attr)
{
    HANDLE handle;

    handle = FindFirstFile (fname, &attr->time);
    if (handle != INVALID_HANDLE_VALUE)
    {
        FindClose (handle);
        attr->attr = GetFileAttributes (fname);
        return 1;
    }
    return 0;
}

/*========================================================*/

void SetFileTimeAttributes (const char *fname, TIME_ATTR * attr)
{
    HANDLE handle;

    handle = CreateFile (fname, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    SetFileTime (handle, &attr->time.ftCreationTime,
            &attr->time.ftLastAccessTime,
            &attr->time.ftLastWriteTime);
    CloseHandle (handle);
    SetFileAttributes (fname, attr->attr);
}

/*========================================================*/

char * GetSystemErrorMessage (void)
{
    char *msg;
    int flags;

    flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS;
    FormatMessage(flags, 0, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &msg, 0, 0);
    return msg;
}

void BigIntToStr (big_size_t N, char *out)
{
    int i, j, neg;
    char tmp;

    neg = (N < 0);
    i = 0;
    do
    {
        out[i++] = (N % 10) + '0';
        N /= 10;
    } while (N != 0);
    if (neg)
    {
        out[i++] = '-';
    }
    out[i--] = 0;
    j = 0;
    while (j < i)
    {
        tmp = out[i];
        out[i--] = out[j];
        out[j++] = tmp;
    }
}

char * FormatInt (big_size_t N)
{
    NUMBERFMT fmt;
    static char result[32];
    char buf[32];
    char sep[5];

    fmt.NumDigits = 0;
    fmt.LeadingZero = 0;
    fmt.Grouping = 3;
    fmt.lpDecimalSep = sep;
    fmt.lpThousandSep = sep;
    fmt.NegativeOrder = 1;
    GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, sep, sizeof(sep));

    BigIntToStr (N, buf);
    
    GetNumberFormat (LOCALE_USER_DEFAULT, 0, buf, &fmt, result, sizeof(result));

    return result;
}
