/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2014, 2015 Beijing Yuan Xin Technology Co.,LTD.
 * All rights reserved.
 *
 * This file is part of Yuan Xin Pinyin Input Engine.
 *
 * Yuan Xin Pinyin Input Engine is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Yuan Xin Pinyin Input Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Autonr: Beijing Yuan Xin Technology Co.,LTD.
 * Contact: yuanxin@syberos.com
 * Web: http://www.syberos.com/
 */

#ifndef _LINUX_H_
#define _LINUX_H_

/**
 *  Windows data type redefined for linux.
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "gram.h"

#define TCHAR wchar_t
#define UINT unsigned int
#define UINT_PTR unsigned int
#define DWORD unsigned long
#define HWND void*
//#define LPINPUTCONTEXT void*
#define byte unsigned char
//#define BYTE unsigned char
#define TEXT(x) L##x
#define __stdcall __attribute__((stdcall))
#define _stdcall
#define HANDLE void*
//#define RECT void*
//#define POINT void*
#define CONST const
#define HINSTANCE void*
#define BOOL bool
#define LPBYTE unsigned char*
#define INT_PTR int*
#define __int64 long long
#define MAX_STATUS_LIST_SIZE                    100             //状态栏列表数
#define __FUNCTION__ ""
#define WINAPI
#define MAX_PATH 260
#define _MAX_PATH MAX_PATH

#define VK_CAPITAL 0x14
#define VK_BACK 0x8
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11

#define VK_MENU 0x12
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5

#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28

#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24

#define VK_DELETE 0x2E

#define VK_HELP 0x2F

#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69

#define VK_F9 0x78

#define DWORD unsigned long
#define NEAR
#define FAR
#define LONG long
#define __wchar_t wchar_t
//#define HIMCC DWORD
#define LF_FACESIZE 32
#define WCHAR wchar_t
#define MAX_SEARCH_URL_LENGTH           0x200   //最大搜索的URL长度
#define SOFTKBD_NUMBER          15      /* 软键盘数目 */
#define SOFTKBD_KEY_NUMBER      48      /* 软键盘字符数目 */

#define STIF_DEFAULT        0x00000000L
#define STIF_SUPPORT_HEX    0x00000001L

#define LRESULT void*
#define WPARAM void*
#define LPARAM void*

#define TRUE true

#define FILE_ATTRIBUTE_NORMAL 0x80
#define _A_SUBDIR 0x10

#define WM_USER                         0x0400
#define UIM_MODE_CHANGE                         (WM_USER + 0x101)               //改变输入模式消息
#define UIM_INPUT_HZ                            (WM_USER + 0x102)               //输入汉字消息（偏旁部首检字）
#define UIM_NOTIFY                                      (WM_USER + 0x103)               //内部窗口控制消息
//#define       UIM_UPDATE_NOTIFY                       (WM_USER + 0x104)               //更新通知消息
#define UIM_SEL_CANDIDATE                       (WM_USER + 0x105)               //鼠标选择候选
#define UIM_NEXT_PAGE                           (WM_USER + 0x106)               //翻动到下一页
#define UIM_PREV_PAGE                           (WM_USER + 0x107)               //翻动到上一页
#define UIM_RECENT_CI                           (WM_USER + 0x108)               //最近输入的词
#define UIM_ENGLISH_INPUT                       (WM_USER + 0x109)               //切换英文输入法
#define UIM_STATUS_WINDOW_CHANGED       (WM_USER + 0x110)               //状态窗口位置改变
#define FALSE false

/* Workaround defination for linux building. */
#define __1i64 (int64_t)0x1
#define __try
#define __except(x)
#define _O_U16TEXT 0x20000
#define ERROR_SUCCESS 0x0
#define _stat stat
#define FILE_MAP_READ 0x4
#define FILE_MAP_WRITE 0x2

#define _fputc_nolock fputc_unlocked
#define _ltoa ltoa
#define _unlink unlink
#define strncat_s strncat
#define strcat_s strcat

#define __declspec(x)

#define _tcsicmp        wcscasecmp
#define PathFileExists(x)    access(x, F_OK)
#define _tcscpy wcscpy
#define _tcsnccpy wcsncpy
#define MoveFile rename
#define _stscanf_s sscanf
#define _tstoi(x) wcstol(x, NULL, 10)
#define _tunlink unlink
#define _tcstok(x, y) wcstok(x, y, NULL)
#define _tcslen wcslen
#define _tcsncpy_s(x, y, z, t) wcsncpy(x, z, t)
#define _tcsncat_s(x, y, z, t) wcsncat(x, z, t)
#define _tcscmp wcscmp
#define _tcscat_s(x, y, z) wcscat(x, z)
// update needed
#define _tcslwr_s(x, y)
#define _stprintf_s swprintf
#define ZeroMemory(x, y) memset(x, 0, y)
#define _tcschr wcschr
#define _tcsncmp wcsncmp
#define _tcsrchr wcsrchr
#define _tcsstr wcsstr
#define _tcscpy_s(x, y, z) wcscpy(x, z)
#define _fgettc fgetwc
#define _istdigit iswdigit
#define memcpy_s(x, y, z, t) memcpy(x, z, t)
// update needed
#define WideCharToMultiByte(x, y, z, a, b, c, d, e)  wcstombs(b, z, c)
// update needed
#define MultiByteToWideChar(x, y, z, a, b, c)  mbstowcs(b, z, c)
#define _tcsdup wcsdup
#define _strnicmp strncasecmp
#define _fileno fileno
#define _trename rename
//#define StrToIntEx(x, y, z) \
//  do { *z=strtol(x, NULL, 16) \
//        } while(0)
#define _tcsncpy wcsncpy
#define _tcsnccmp wcsncmp
#define SHCreateDirectoryEx(x, y, z) mkdir(y, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define _tcscat wcscat
#define _ftprintf fwprintf
#define GetLastError() errno
#define Sleep(x) usleep(1000*x)
#define _tstat stat
#define strncpy_s(x, y, z, t) strncpy(x, z, t)
#define _tfopen fopen

#define INLINE
#define min(x, y) ((x) < (y) ? x : y)

#define WORD unsigned short
#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))

typedef unsigned long _fsize_t;
typedef unsigned long SIZE_T;

struct _finddata_t
{
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    _fsize_t size;
    char name[MAX_PATH];
};

typedef DWORD   HIMCC;

typedef struct tagSIZE {
  LONG cx;
  LONG cy;
} SIZE, *PSIZE;

typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT;

typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT, *PPOINT;

typedef struct {
 BYTE                 lfHeight;
 BYTE                 lfWidth;
 BYTE                 lfEscapement;
 BYTE                 lfOrientation;
 BYTE                 lfWeight;
  BYTE      lfItalic;
  BYTE      lfUnderline;
  BYTE      lfStrikeOut;
  BYTE      lfCharSet;
  BYTE      lfOutPrecision;
  BYTE      lfClipPrecision;
  BYTE      lfQuality;
  BYTE      lfPitchAndFamily;
  __wchar_t lfFaceName[LF_FACESIZE];
} LOGFONTA;

typedef struct {
 BYTE             lfHeight;
 BYTE             lfWidth;
 BYTE             lfEscapement;
 BYTE             lfOrientation;
 BYTE             lfWeight;
  BYTE  lfItalic;
  BYTE  lfUnderline;
  BYTE  lfStrikeOut;
  BYTE  lfCharSet;
  BYTE  lfOutPrecision;
  BYTE  lfClipPrecision;
  BYTE  lfQuality;
  BYTE  lfPitchAndFamily;
  WCHAR lfFaceName[LF_FACESIZE];
} LOGFONTW;

typedef struct tagCOMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} COMPOSITIONFORM, *PCOMPOSITIONFORM, NEAR *NPCOMPOSITIONFORM, FAR *LPCOMPOSITIONFORM;

typedef struct tagCANDIDATEINFO {
    DWORD               dwSize;
    DWORD               dwCount;
    DWORD               dwOffset[32];
    DWORD               dwPrivateSize;
    DWORD               dwPrivateOffset;
} CANDIDATEINFO, *PCANDIDATEINFO, NEAR *NPCANDIDATEINFO, FAR *LPCANDIDATEINFO;

typedef struct tagCANDIDATEFORM {
    DWORD dwIndex;
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} CANDIDATEFORM, *PCANDIDATEFORM, NEAR *NPCANDIDATEFORM, FAR *LPCANDIDATEFORM;

typedef struct tagCANDIDATELIST {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    DWORD dwOffset[1];
} CANDIDATELIST, *PCANDIDATELIST, NEAR *NPCANDIDATELIST, FAR *LPCANDIDATELIST;

typedef struct tagCOMPOSITIONSTRING {
    DWORD dwSize;
    DWORD dwCompReadAttrLen;
    DWORD dwCompReadAttrOffset;
    DWORD dwCompReadClauseLen;
    DWORD dwCompReadClauseOffset;
    DWORD dwCompReadStrLen;
    DWORD dwCompReadStrOffset;
    DWORD dwCompAttrLen;
    DWORD dwCompAttrOffset;
    DWORD dwCompClauseLen;
    DWORD dwCompClauseOffset;
    DWORD dwCompStrLen;
    DWORD dwCompStrOffset;
    DWORD dwCursorPos;
    DWORD dwDeltaStart;
    DWORD dwResultReadClauseLen;
    DWORD dwResultReadClauseOffset;
    DWORD dwResultReadStrLen;
    DWORD dwResultReadStrOffset;
    DWORD dwResultClauseLen;
    DWORD dwResultClauseOffset;
    DWORD dwResultStrLen;
    DWORD dwResultStrOffset;
    DWORD dwPrivateSize;
    DWORD dwPrivateOffset;
} COMPOSITIONSTRING, *PCOMPOSITIONSTRING, NEAR *NPCOMPOSITIONSTRING, FAR  *LPCOMPOSITIONSTRING;

typedef struct tagINPUTCONTEXT {
    HWND                hWnd;
    BOOL                fOpen;
    POINT               ptStatusWndPos;
    POINT               ptSoftKbdPos;
    DWORD               fdwConversion;
    DWORD               fdwSentence;
    union   {
        LOGFONTA        A;
        LOGFONTW        W;
    } lfFont;
    COMPOSITIONFORM     cfCompForm;
    CANDIDATEFORM       cfCandForm[4];
    HIMCC               hCompStr;
    HIMCC               hCandInfo;
    HIMCC               hGuideLine;
    HIMCC               hPrivate;
    DWORD               dwNumMsgBuf;
    HIMCC               hMsgBuf;
    DWORD               fdwInit;
    DWORD               dwReserve[3];
} INPUTCONTEXT, *PINPUTCONTEXT, NEAR *NPINPUTCONTEXT, FAR *LPINPUTCONTEXT;

typedef struct _MEMORYSTATUS {
  DWORD  dwLength;
  DWORD  dwMemoryLoad;
  SIZE_T dwTotalPhys;
  SIZE_T dwAvailPhys;
  SIZE_T dwTotalPageFile;
  SIZE_T dwAvailPageFile;
  SIZE_T dwTotalVirtual;
  SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;


typedef enum _COMPOSESTRING
{
    pinyinString,
    selectString,
    resultString
}COMPOSESTRING;
extern int resource_thread_finished;

#ifdef __cplusplus
}
#endif


#endif
