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

/*  工具头文件。
 *  装载log，file，以及内存管理的头文件。
 */

#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <stdio.h>
#include "kernel.h"
//#include <windows.h>
#include "linux.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _SizeOf(x)      (sizeof((x)) / sizeof((x)[0]))
#define _IsNoneASCII(x) ((WORD)x >= 0x2B0)
#define _HanZiLen       1

//最多申请的共享内存数量
#define MAX_SHARED_MEMORY_COUNT         1024            //最多的共享对象

//键盘定义
#define KEY_LSHIFT              (1 << 0)
#define KEY_RSHIFT              (1 << 1)
#define KEY_SHIFT               (1 << 2)
#define KEY_LCONTROL            (1 << 3)
#define KEY_RCONTROL            (1 << 4)
#define KEY_CONTROL             (1 << 5)
#define KEY_LALT                (1 << 6)
#define KEY_RALT                (1 << 7)
#define KEY_ALT                 (1 << 8)
#define KEY_CAPITAL             (1 << 9)

//共享内存的信息
typedef struct tagSHAREDMEMORYINFO
{
    HANDLE  handle;
    void    *pointer;
} SHAREDMEMORYINFO;

typedef struct tagFILEMAPDATA
{
	HANDLE		h_map;			//映射句柄
	long long   length;			//文件的长度
}FILEMAPDATA, *FILEMAPHANDLE;

#define TYPE_USERAPP        0               //documents and setting/{username}/application data
#define TYPE_ALLAPP         1               //documents and setting/all users/application data
#define TYPE_PROGRAM        2               //program files/unispim6
#define TYPE_TEMP           3               //临时文件目录

//LOG相关
//声明LOG记录的标识，由程序文件名字与函数名称组合而成。
#define WIDEN(x)    L ## x
#define WIDEN2(x)   WIDEN(x)
#define LOG_ID      WIDEN2(__FUNCTION__)

extern void Log(const TCHAR *id, const TCHAR *format, ...);

//LOG相关结束

//文件处理相关
//从文件读入数据
extern int LoadFromFile(const TCHAR *file_name, void *buffer, int buffer_length);

//保存数据文件
extern int SaveToFile(const TCHAR *file_name, void *buffer, int buffer_length);

//获得文件长度
extern int GetFileLength(const TCHAR *file_name);

//复制部分字符串
extern void CopyPartString(TCHAR *target, const TCHAR *source, int length);

//获得共享内存区域指针
extern void *GetSharedMemory(const TCHAR *shared_name);
extern void *GetReadOnlySharedMemory(const TCHAR *shared_name);

//创建共享内存区
extern void *AllocateSharedMemory(const TCHAR *shared_name, int length);

//释放共享内存区
extern void FreeSharedMemory(const TCHAR *shared_name, char *pointer);

//获得当前用户的Application目录
extern TCHAR *GetUserAppDirectory(TCHAR *dir);

//组合目录与文件名
extern TCHAR *CombineDirAndFile(const TCHAR *dir, const TCHAR *file, TCHAR *result);

//获得文件的路径，App/allApp/program
TCHAR *GetFileFullName(int type, const TCHAR *file_name, TCHAR *result);

//Ansi字符串转换到UTF16
extern void AnsiToUtf16(const char *name, wchar_t *wname, int nSize);

extern void Utf16ToAnsi(const wchar_t *wchars, char *chars, int nSize);

extern void UCS32ToUCS16(const UC UC32Char, TCHAR *buffer);
extern void U16CopyInLinux(char* bufferDest, char* bufferU16, int sizeU16);
extern WORD* utf16ncpy(WORD *dest, const wchar_t *src, size_t n);

extern FILEMAPHANDLE FileMapOpen(const wchar_t *file_name);
extern int FileMapGetBuffer(FILEMAPHANDLE handle, char **buffer, int length);
extern int FileMapClose(FILEMAPHANDLE handle,const TCHAR *name);


#ifdef __cplusplus
}
#endif

#endif
