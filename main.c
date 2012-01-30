/* File   : main.c
 * Author : AR Kusuma
 * Update : 19 February 2004
 *
 * Desc.  : Main source for MiniSplit.
 *          Contains split engine and GUI management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <shlobj.h>
 
#include "utils.h"
#include "crc32.h"
#include "big_file.h"
#include "resource.h"

#define PROGRESSBAR_RANGE	250
#define BUF_SIZE		(1 << 18)
#define MAX_PARTS               1000

#define ST_NONE         0
#define ST_SPLITTING    1
#define ST_JOINING      2

HWND hMain;
int state;
int stop;
int AskUser = TRUE;

big_size_t SplitSize = 1440 * 1000;

static const char *msgError = "Error";
static const char *msgConfirm = "Confirmation";
static const char *msgInfo = "Information";

static const char *reg_key = "Software\\LZE Software\\MiniSplit";
static const char *reg_split_size = "Size";
static const char *reg_split_type = "Type";

typedef struct
{
  char *name;
  int bytes;
} SIZE_TYPE;

static const SIZE_TYPE split_type[] =
{ { "Parts", 0 },
  { "Bytes", 1 },
  { "Kilo Bytes (KB)",            1000 },
  { "Mega Bytes (MB)",       1000*1000 },
  { "Giga Bytes (GB)",  1000*1000*1000 },
  { "Kibi Bytes (KiB)",           1024 },
  { "Mebi Bytes (MiB)",      1024*1024 },
  { "Gibi Bytes (GiB)", 1024*1024*1024 },
  { "1.44 MB Floppy",        1440*1000 },
  { "2.88 MB Floppy",        2880*1000 },
  { "8 MB Flash",          8*1000*1000 },
  { "16 MB Flash",        16*1000*1000 },
  { "32 MB Flash",        32*1000*1000 },
  { "64 MB Flash",        64*1000*1000 },
  { "128 MB Flash",      128*1000*1000 },
  { "256 MB Flash",      256*1000*1000 },
  { "512 MB Flash",      512*1000*1000 },
  { "650 MB CD-R/W",     650*1000*1000 },
  { "700 MB CD-R/W",     700*1000*1000 },
  { 0, 0 }
};

#define PARTS  0
#define BYTES  1
#define KB     2
#define MB     3
#define GB     4
#define KIB    5
#define MIB    6
#define GIB    7
#define CUSTOM 8

/*========================================================*/

void normalize_float (char * num)
{
    int len, i;
    len = strlen (num) - 1;
    while (len >= 0 && num[len] == '0')
        len--;
    i = len;
    while (i >= 0 && isdigit(num[i]))
        i--;
    if (i >= 0)
    {
        if (isdigit(num[len]))
        {
            len++;
        }
        num[len] = 0;
    }
}

void UpdateSplitSize (void)
{
    char buf[32], file[MAX_PATH];
    int count, sel, i;
    HANDLE hdl;
    big_size_t fsize;

    sel = SendDlgItemMessage (hMain, IDC_SPLIT_TYPE, CB_GETCURSEL, 0, 0);
    switch (sel)
    {
    	case PARTS:
    	    if (SplitSize > 0)
            {
                GetDlgItemText (hMain, IDC_FILE, file, sizeof (file));
                if ((hdl = big_fopen (file, "r")) != INVALID_HANDLE_VALUE)
                {
                    fsize = big_fsize (hdl);
                    big_fclose (hdl);

                    count = (fsize + SplitSize - 1) / SplitSize;
                    SetDlgItemInt (hMain, IDC_SPLIT_SIZE, count, FALSE);
                }
            }
            break;
        case BYTES:
            BigIntToStr (SplitSize, buf);
            SetDlgItemText (hMain, IDC_SPLIT_SIZE, buf);
            break;
        default:
            if (sel < CUSTOM)
            {
                sprintf (buf, "%.3f", SplitSize / (float) split_type[sel].bytes);
                normalize_float (buf);
                SetDlgItemText (hMain, IDC_SPLIT_SIZE, buf);
            }
            else
            {
            	SplitSize = split_type[sel].bytes;
                SetDlgItemText (hMain, IDC_SPLIT_SIZE, "");
            }
            break;
    }
    EnableWindow(GetDlgItem(hMain, IDC_SPLIT_SIZE), sel < CUSTOM);
}

big_size_t GetSplitSize (void)
{
    char tmp[100], fname[MAX_PATH];
    double d_size;
    HANDLE hdl;
    big_size_t size, fsize;
    int sel;

    sel = SendDlgItemMessage(hMain, IDC_SPLIT_TYPE, CB_GETCURSEL, 0, 0);
    GetDlgItemText(hMain, IDC_SPLIT_SIZE, tmp, sizeof (tmp));
    d_size = atof (tmp);
    switch (sel)
    {
        case PARTS:
            GetDlgItemText(hMain, IDC_FILE, fname, sizeof (fname));
            if (d_size == 0 ||
                    (hdl = big_fopen (fname, "r")) == INVALID_HANDLE_VALUE)
                return 0;
            fsize = big_fsize (hdl);
            big_fclose (hdl);
            size = (d_size == 0) ? 0 : (fsize + d_size - 1) / d_size;
            break;
        default:
            size = split_type[sel].bytes;
            if (sel < CUSTOM)
                size *= d_size;
            break;
    }
    SplitSize = size;
    return size;
}

/*========================================================*/

void ProcessMessages(void)
{
    MSG msg;

    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        if (!IsDialogMessage(hMain, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
}

/*========================================================*/

#define myreturn(x) { state = ST_NONE; return (x); }

void save_error_code(int save)
{
    static int last_code = 0;

    if (save)
        last_code = GetLastError();
    else
        SetLastError(last_code);
}

#define DEBUG(x) MessageBox (hMain, x, "Debug", 0)

int Split(const char *file, const char *dir, big_size_t size)
{
    /* ifp, ifname -> input file
     * ofp, ofname -> output file (*.000, *.001, and so on)
     * cfp, cfname -> crc file
     */
    HANDLE ifp, ofp, cfp;
    char ifname[MAX_PATH], ofname[MAX_PATH], cfname[MAX_PATH];
    char basename[MAX_PATH], buf[BUF_SIZE], Temp[128];
    int fileno, i, fail;
    unsigned long crc;
    unsigned long crc_list[MAX_PARTS];
    big_size_t counter, total, fsize;
    size_t read;
    TIME_ATTR attr;

    state = ST_SPLITTING;
    GetFileTimeAttributes(file, &attr);
    BIG_LOW(fsize) = attr.time.nFileSizeLow;
    BIG_HIGH(fsize) = attr.time.nFileSizeHigh;

    if (fsize <= size)
    {
        MessageBox(hMain, "File too small, it won't be splited.",
                msgInfo, MB_ICONINFORMATION | MB_OK);
        myreturn(0);
    }

    GetFileName(file, ifname);
    strcpy(basename, dir);
    i = strlen(basename);
    if (basename[i - 1] != '\\')
    {
        basename[i] = '\\';
        basename[i + 1] = 0;
    }
    strcat(basename, ifname);

    i = (size + fsize - 1) / size;
    if (i > MAX_PARTS)
    {
        if (AskUser)
        {
            sprintf(Temp,
                    "More than a thousand file parts (%s) will be produced.\n"
                    "Can't continue splitting!", FormatInt(i));
            MessageBox(hMain, Temp, msgError, MB_OK | MB_ICONERROR);
        }
        myreturn(0);
    }

    if (AskUser)
    {
        sprintf(Temp,
                "The file will be splited into %d parts "
                "with maximum size %s bytes.\n"
                "Is it okay to continue ?", i, FormatInt(size));
        if (MessageBox(hMain, Temp, msgConfirm, MB_OKCANCEL |
                    MB_ICONQUESTION) != IDOK)
            myreturn(0);
    }

    if ((ifp = big_fopen(file, "r")) == INVALID_HANDLE_VALUE)
        myreturn(-1);

    fileno = 0;
    total = 0;
    stop = FALSE;
    fail = FALSE;
    do
    {
        sprintf(ofname, "%s.%03d", basename, fileno);
        SetFileAttributes(ofname, 0);
        if ((ofp = big_fopen(ofname, "w")) == INVALID_HANDLE_VALUE)
        {
            save_error_code(TRUE);
            fail = TRUE;
            break;
        }

        crc = 0;
        counter = size;
        while (!big_feof(ifp) && counter > 0)
        {
            ProcessMessages();

            read = (BUF_SIZE <= counter) ? BUF_SIZE : counter;
            read = big_fread(buf, 1, read, ifp);
            if (!read || !big_fwrite(buf, read, 1, ofp))
            {
                save_error_code(TRUE);
                fail = TRUE;
                break;
            }

            crc = crcBytes(crc, buf, read);
            counter -= read;
            total += read;

            SendDlgItemMessage(hMain, IDC_PROGRESS, PBM_SETPOS,
                    (int) (PROGRESSBAR_RANGE * total / fsize), 0);
        }
        big_fclose(ofp);
        SetFileTimeAttributes(ofname, &attr);

        crc_list[fileno++] = crc;
    } while (total < fsize && !stop && !fail);

    big_fclose(ifp);
    SetFileTimeAttributes(cfname, &attr);

    /* Write error checking (CRC) information */
    strcpy(cfname, basename);
    strcat(cfname, ".crc");
    SetFileAttributes(cfname, 0);
    if ((cfp = big_fopen(cfname, "w")) == INVALID_HANDLE_VALUE)
    {
        save_error_code(TRUE);
        fail = TRUE;
    }
    else
    {
        if (!big_fwrite(&fsize, sizeof(fsize), 1, cfp) ||
            !big_fwrite(crc_list, fileno * sizeof(crc), 1, cfp))
        {
            save_error_code(TRUE);
            fail = TRUE;
        }
        big_fclose(cfp);
        SetFileTimeAttributes(cfname, &attr);
    }
    
    SendDlgItemMessage(hMain, IDC_PROGRESS, PBM_SETPOS, 0, 0);

    if (fail)
    {
        save_error_code(FALSE);
        myreturn(-1);
    }
	
	if (IsDlgButtonChecked(hMain, IDC_DELETE_SOURCE))
	{
		char msg[MAX_PATH+64];

		sprintf(msg, "Source file (%s) will be deleted. Do you want to continue ?", ifname);
		if (MessageBox(hMain, msg, msgConfirm, MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
		{
			SetFileAttributes(file, 0);
			DeleteFile(ifname);
		}
	}

    myreturn(0);
}

/*========================================================*/

int find_file_parts(const char *file, int *parts, big_size_t *size)
{
    char ifname[MAX_PATH], basename[MAX_PATH];
    big_size_t found_size, s;
    int i, found_count, limit;
    HANDLE hdl;

#define LIMIT_DELTA 20

    GetNoExtFileName(file, basename);
    found_count = 0;
    found_size = 0;
    limit = LIMIT_DELTA;
    for (i = 0; i < limit && i < MAX_PARTS; i++)
    {
        sprintf(ifname, "%s.%03d", basename, i);
        if (FileExists(ifname))
        {
            hdl = big_fopen (ifname, "r");
            s = big_fsize (hdl);
            big_fclose (hdl);
            parts[found_count++] = i;
            found_size += s;
            limit = i + LIMIT_DELTA + 1;
        }
    }
    *size = found_size;
    return found_count;
}

/*========================================================*/

int Join(const char *file, const char *dir)
{
    HANDLE ifp, ofp;
    char ifname[MAX_PATH], ofname[MAX_PATH], basename[MAX_PATH];
    char buf[BUF_SIZE], tmp[128];
    int i, crc_exists, fail;
    unsigned long crc;
    unsigned long crc_list[MAX_PARTS];
    int file_parts[MAX_PARTS], error_list[MAX_PARTS];
    int file_count, error_count, crc_count;
    big_size_t file_size, crc_size, counter;
    size_t read;
    TIME_ATTR attr;

    state = ST_JOINING;
    GetFileTimeAttributes(file, &attr);

    /* Read CRC information */
    GetNoExtFileName(file, basename);
    sprintf(ifname, "%s.crc", basename);
    crc_exists = 0;
    crc_count = 0;
    if ((ifp = big_fopen(ifname, "r")) != INVALID_HANDLE_VALUE)
    {
        crc_count = ((big_fsize (ifp) - sizeof (big_size_t)) / 4);
        if (crc_count > 0 && crc_count <= MAX_PARTS)
        {
            if (big_fread(&crc_size, sizeof (crc_size), 1, ifp) != 0 &&
                big_fread(crc_list, 4 * crc_count, 1, ifp) != 0)
            {
                crc_exists = 1;
            }
        }
        big_fclose(ifp);
    }

    GetFileName(basename, ifname);
    strcpy(ofname, dir);
    if (dir[strlen(dir) - 1] != '\\')
        strcat(ofname, "\\");
    strcat(ofname, ifname);

    file_count = find_file_parts(file, file_parts, &file_size);

    if (AskUser && file_count == 0)
    {
        MessageBox(hMain, "Not a valid file to join.", msgError, MB_OK |
                MB_ICONERROR);
        myreturn(0);
    }

    if (AskUser && crc_exists && (file_size != crc_size))
    {
        strcpy(tmp, FormatInt(file_size));
        sprintf(buf, "Found %d file parts with total size of %s bytes.\n"
                "Should be %d file parts with total size of %s bytes.\n\n"
                "Do you still want to create '%s' ?",
                file_count, tmp, crc_count, FormatInt(crc_size), ifname);
        if (MessageBox(hMain, buf, msgConfirm, MB_OKCANCEL |
                    MB_ICONWARNING) != IDOK)
            myreturn(0);
    }
    else if (AskUser)
    {
        sprintf(buf, "Found %d file parts with total size of %s bytes.\n"
                "Is it okay to create '%s' ?", file_count,
                FormatInt(file_size), ifname);
        if (MessageBox(hMain, buf, msgConfirm, MB_OKCANCEL |
                    MB_ICONQUESTION) != IDOK)
            myreturn(0);
    }

    if (AskUser && FileExists(ofname))
    {
        sprintf(buf, "File %s already exists.\nDo you want to overwrite ?",
                ofname);
        if (MessageBox(hMain, buf, msgConfirm, MB_YESNO | MB_ICONQUESTION) !=
                IDYES)
            myreturn(0);
    }

    SetFileAttributes(ofname, FILE_ATTRIBUTE_NORMAL);
    if ((ofp = big_fopen(ofname, "w")) == INVALID_HANDLE_VALUE)
        myreturn(-1);

    counter = 0;
    error_count = 0;
    stop = FALSE;
    fail = FALSE;
    for (i = 0; i < file_count && !stop; i++)
    {
        sprintf(ifname, "%s.%03d", basename, file_parts[i]);
        if ((ifp = big_fopen(ifname, "r")) == INVALID_HANDLE_VALUE)
        {
            save_error_code(TRUE);
            fail = TRUE;
            break;
        }

        crc = 0;
        for (; !big_feof(ifp);)
        {
            ProcessMessages();

            read = big_fread(buf, 1, BUF_SIZE, ifp);
            if ((read != BUF_SIZE && !big_feof(ifp)) ||
                big_fwrite(buf, read, 1, ofp) == 0)
            {
                save_error_code(TRUE);
                fail = TRUE;
                break;
            }

            crc = crcBytes(crc, buf, read);
            counter += read;
            SendDlgItemMessage(hMain, IDC_PROGRESS, PBM_SETPOS,
                    (int) (PROGRESSBAR_RANGE * counter / file_size), 0);
        }
        CloseHandle(ifp);

        if (fail)
            break;

        if (crc_exists && crc_count > file_parts[i] &&
                crc != crc_list[file_parts[i]])
            error_list[error_count++] = file_parts[i];
    }
    big_fclose(ofp);
    SetFileTimeAttributes(ofname, &attr);

    SendDlgItemMessage(hMain, IDC_PROGRESS, PBM_SETPOS, 0, 0);

    if (fail)
    {
        save_error_code(FALSE);
        myreturn(-1);
    }

    if (AskUser && error_count > 0)
    {
        char *ptr;

        GetFileName(file, ifname);
        GetNoExtFileName(ifname, basename);
        strcpy(buf, "The following files are corrupted:\n");
        ptr = buf;
        for (i = 0; i < error_count && ptr - buf < BUF_SIZE - MAX_PATH; i++)
        {
            ptr = &ptr[strlen(ptr)];
            sprintf(ptr, "\n%s.%03d", basename, error_list[i]);
        }
        MessageBox(hMain, buf, msgError, MB_OK | MB_ICONERROR);
    }

	if (error_count == 0 && IsDlgButtonChecked(hMain, IDC_DELETE_SOURCE))
	{
		if (MessageBox(hMain, "Source files will be deleted. Do you want to continue ?",
			           msgConfirm, MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
		{
			for (i = 0; i < file_count && !stop; i++)
			{
				sprintf(ifname, "%s.%03d", basename, file_parts[i]);
				SetFileAttributes(ofname, 0);
				DeleteFile(ifname);
			}
			if (crc_exists)
			{
				sprintf(ifname, "%s.crc", basename);
				SetFileAttributes(ofname, 0);
				DeleteFile(ifname);
			}
		}
	}

    myreturn(0);
}

/*========================================================*/

void OnRadio(void)
{
    int val, sel;

    val = (IsDlgButtonChecked(hMain, IDC_SPLIT_RADIO) == BST_CHECKED);
    sel = SendDlgItemMessage (hMain, IDC_SPLIT_TYPE, CB_GETCURSEL, 0, 0);
    EnableWindow(GetDlgItem(hMain, IDC_SPLIT_SIZE), val && sel < CUSTOM);
    EnableWindow(GetDlgItem(hMain, IDC_SPLIT_TYPE), val);
}

/*========================================================*/

void SelectFile(const char *file)
{
    int select_item, i, len;
    char fname[MAX_PATH];

    strcpy(fname, file);
    select_item = IDC_SPLIT_RADIO;
    len = strlen(fname);
    for (i = len - 1; i >= 0; i--)
        if (fname[i] == '.')
            break;

    if (stricmp(&fname[++i], "crc") == 0)
        select_item = IDC_JOIN_RADIO;
    else if (i > 0 && len - i == 3)
    {
        select_item = IDC_JOIN_RADIO;
        for (; fname[i] != 0; i++)
            if (!isdigit(fname[i]))
            {
                select_item = IDC_SPLIT_RADIO;
                break;
            }
    }
    CheckRadioButton(hMain, IDC_SPLIT_RADIO, IDC_JOIN_RADIO, select_item);
    OnRadio();

    SetDlgItemText(hMain, IDC_FILE, fname);
    GetFilePath(fname, fname);
    SetDlgItemText(hMain, IDC_FOLDER, fname);
    
    if (select_item == IDC_SPLIT_RADIO)
        GetSplitSize ();
}

/*========================================================*/

void OnFileBrowse(void)
{
    static char Filter[] =
        "All Files (*.*)\x00*.*\x00"
        "First Files (*.000)\x00*.000\x00";
    char FileBuffer[MAX_PATH];
    OPENFILENAME ofn;

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hInstance = GetModuleHandle(0);
    ofn.hwndOwner = hMain;
    GetDlgItemText(hMain, IDC_FILE, FileBuffer, MAX_PATH);
    ofn.lpstrFile = FileBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = Filter;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (IsDlgButtonChecked(hMain, IDC_SPLIT_RADIO) == BST_CHECKED)
    {
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = "Select File To Split";
    }
    else
    {
        ofn.nFilterIndex = 2;
        ofn.lpstrTitle = "Select File To Join";
    }
    if (GetOpenFileName(&ofn))
        SelectFile(FileBuffer);
}

/*========================================================*/

int WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    char tmp[MAX_PATH];

    if (uMsg == BFFM_INITIALIZED)
    {
        GetDlgItemText((HWND) lpData, IDC_FOLDER, tmp, sizeof(tmp));
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM) tmp);
    }
    return 0;
}

/*========================================================*/

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE	0x40
#endif

#ifndef BIF_EDITBOX
#define BIF_EDITBOX		0x10
#endif

void OnFolderBrowse(void)
{
    BROWSEINFO bi;
    LPITEMIDLIST item;
    char FileBuffer[MAX_PATH];

    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = hMain;
    bi.lpszTitle = "Select Destination Folder:";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lParam = (LPARAM) hMain;
    bi.lpfn = BrowseCallbackProc;
    if ((item = SHBrowseForFolder(&bi)) != 0)
    {
        SHGetPathFromIDList(item, FileBuffer);
        SetDlgItemText(hMain, IDC_FOLDER, FileBuffer);
    }
}

/*========================================================*/

void UpdateCaption(int mode)
{
    SetDlgItemText(hMain, IDC_DO_IT, (mode == 0) ? "S&top" : "&Do It");
    EnableWindow(GetDlgItem(hMain, IDC_CLOSE), mode != 0);
}

void OnDoIt(void)
{
    char File[MAX_PATH], Dir[MAX_PATH];
    big_size_t size;
    
    GetDlgItemText(hMain, IDC_FILE, File, sizeof (File));
    GetDlgItemText(hMain, IDC_FOLDER, Dir, sizeof (Dir));

    if (state != ST_NONE)
        stop = 1;
    else
    {
        if (IsDlgButtonChecked(hMain, IDC_SPLIT_RADIO) == BST_CHECKED)
        {
            if (!FileExists(File))
            {
                MessageBox(hMain, GetSystemErrorMessage(), msgError,
                        MB_ICONERROR | MB_OK);
                return;
            }

            size = GetSplitSize ();

            if (size <= 0)
            {
                MessageBox(hMain, "Invalid split size.", msgError, MB_OK |
                        MB_ICONERROR);
                return;
            }

            UpdateCaption(0);
            if (Split(File, Dir, size) != 0)
                MessageBox(hMain, GetSystemErrorMessage(), msgError,
                        MB_ICONERROR | MB_OK);
            UpdateCaption(1);
        }
        else
        {
            if (!FileExists(File))
            {
                MessageBox(hMain, GetSystemErrorMessage(), msgError,
                        MB_ICONERROR | MB_OK);
                return;
            }

            UpdateCaption(0);
            if (Join(File, Dir) != 0)
                MessageBox(hMain, GetSystemErrorMessage(), msgError,
                        MB_ICONERROR | MB_OK);
            UpdateCaption(1);
        }
    }
}

/*========================================================*/

void OnAbout(void)
{
    static const char *Text =
        "MiniSplit 1.3a\n"
        "Product of LZE Software 2002-2004\n"
        "Written by: AR Kusuma\n\n"
        "Homepage:\n"
        "http://www.lzesoftware.com\n\n"
        "Mail: arkusuma@lzesoftware.com\n\n"
        "Last update: 30 April 2004";
    MSGBOXPARAMS mbp;

    memset(&mbp, 0, sizeof(mbp));
    mbp.cbSize = sizeof(mbp);
    mbp.hwndOwner = hMain;
    mbp.hInstance = GetModuleHandle(NULL);
    mbp.lpszText = Text;
    mbp.lpszCaption = "About MiniSplit";
    mbp.dwStyle = MB_OK | MB_USERICON;
    mbp.lpszIcon = MAKEINTRESOURCE(IDI_MAIN);
    MessageBoxIndirect(&mbp);
}

/*========================================================*/

#ifndef REG_QWORD
#define REG_QWORD                   ( 11 )  // 64-bit number
#endif 

void OnClose(void)
{
    HKEY hkey;
    char size[100];
    int type;

    if (state == ST_NONE)
    {
        if (RegCreateKey (HKEY_CURRENT_USER, reg_key, &hkey) == ERROR_SUCCESS)
        {
            GetDlgItemText (hMain, IDC_SPLIT_SIZE, size, sizeof (size));
            type = SendDlgItemMessage (hMain, IDC_SPLIT_TYPE, CB_GETCURSEL,
                    0, 0);
            RegSetValueEx (hkey, reg_split_size, 0, REG_SZ,
                    size, strlen(size) + 1);
            RegSetValueEx (hkey, reg_split_type, 0, REG_DWORD,
                    (char *) &type, sizeof (type));
            RegCloseKey (hkey);
        }
        EndDialog(hMain, 0);
    }
}

/*========================================================*/

void CenterWindow(HWND wnd, HWND owner)
{
    RECT rcWnd, rcOwner;
    int x, y;

    GetWindowRect(wnd, &rcWnd);
    GetWindowRect(owner, &rcOwner);

#define WndWidth        (rcWnd.right - rcWnd.left)
#define WndHeight       (rcWnd.bottom - rcWnd.top)
#define OwnerWidth      (rcOwner.right - rcOwner.left)
#define OwnerHeight     (rcOwner.bottom - rcOwner.top)

    x = rcOwner.left +(OwnerWidth - WndWidth) / 2;
    y = rcOwner.top +(OwnerHeight - WndHeight) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    SetWindowPos(wnd, 0, x, y, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

/*========================================================*/


/*========================================================*/

typedef struct
{
    unsigned int event;
    void (*handler)(void);
} HANDLER;

static const HANDLER ev[] =
{
    { IDC_FILE_BROWSE,    OnFileBrowse },
    { IDC_FOLDER_BROWSE,  OnFolderBrowse },
    { IDC_SPLIT_RADIO,    OnRadio },
    { IDC_JOIN_RADIO,     OnRadio },
    { IDC_DO_IT,          OnDoIt },
    { IDC_ABOUT,          OnAbout },
    { IDC_CLOSE,          OnClose }
};

#define MAX_HANDLER (sizeof(ev) / sizeof(HANDLER))

int WINAPI DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            {
                int i;
                char str[100];
                HKEY hkey;
				LONG style;

                hMain = hWnd;
                SendMessage(hMain, WM_SETICON, ICON_BIG,
                        (LPARAM) LoadIcon(GetModuleHandle(0),
                                          MAKEINTRESOURCE(IDI_MAIN)));

                crcCreateTable();
                            
                for (i = 0; split_type[i].name != 0; i++)
                    SendDlgItemMessage(hMain, IDC_SPLIT_TYPE, CB_ADDSTRING, 0,
                            (LPARAM) split_type[i].name);

                SendDlgItemMessage(hMain, IDC_PROGRESS, PBM_SETRANGE, 0,
                        PROGRESSBAR_RANGE << 16);

                /* Restore setting from previous session */
                strcpy (str, "1440");
                i = KB;
                if (RegOpenKey (HKEY_CURRENT_USER, reg_key, &hkey) == ERROR_SUCCESS)
                {
                    DWORD Type, Size;

                    Size = sizeof (str);
                    RegQueryValueEx (hkey, reg_split_size, 0, &Type,
                            (LPBYTE) &str, &Size);
                    str[sizeof(str)-1] = 0;
                    Size = sizeof (i);
                    RegQueryValueEx (hkey, reg_split_type, NULL, &Type,
                            (LPBYTE) &i, &Size);
                    RegCloseKey (hkey);
                }
                SetDlgItemText(hMain, IDC_SPLIT_SIZE, str);
                SendDlgItemMessage(hMain, IDC_SPLIT_TYPE, CB_SETCURSEL, i, 0);
				
		        GetSplitSize ();
                UpdateSplitSize ();

                SelectFile((char *) lParam);

                return TRUE;
            }
        case WM_CLOSE:
            OnClose();
            return TRUE;
        case WM_ACTIVATE:
            {
                DWORD pid;

                GetWindowThreadProcessId((HWND) lParam, &pid);
                if (wParam == WA_INACTIVE && GetCurrentProcessId() == pid)
                    CenterWindow((HWND) lParam, hMain);
                return TRUE;
            }
        case WM_DROPFILES:
            {
                char fname[MAX_PATH];

                DragQueryFile((HDROP) wParam, 0, fname, sizeof(fname));
                DragFinish((HDROP) wParam);

                if (!FileExists(fname))
                    SetDlgItemText(hWnd, IDC_FOLDER, fname);
                else
                    SelectFile(fname);

                return TRUE;
            }
        case WM_COMMAND:
            {
                int i;

                for (i = 0; i < MAX_HANDLER; i++)
                    if (ev[i].event == wParam)
                    {
                        ev[i].handler();
                        return TRUE;
                    }
                if (wParam == MAKEWPARAM(IDC_SPLIT_TYPE, CBN_SELCHANGE))
                {
                    UpdateSplitSize ();
                    return TRUE;
                }
                else if (wParam == MAKEWPARAM(IDC_SPLIT_SIZE, EN_CHANGE))
                {
                    GetSplitSize ();
                    return TRUE;
                }
            }
    }
    return FALSE;
}

/*========================================================*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow)
{
    InitCommonControls();
    CoInitialize(0);
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc,
            (LPARAM) lpCmdLine);
    CoUninitialize();
    return 0;
}
 