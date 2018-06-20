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

/** 杂项工具函数库。包括：
 *      1. 共享内存的分配与释放。
 *      2. 系统时间函数
 */
//#include <shlobj.h>
//#include <windows.h>
#include "linux.h"
#include "kernel.h"
#include <time.h>
//#include <io.h>
#include <fcntl.h>
#include "./zlib/zlib.h"
#include <sys/stat.h>
#include <sys/mman.h>
//#include <tchar.h>
//#include <Sddl.h>
//#include <Aclapi.h>
#include <locale.h>

//此宏定义的原因请参见头文件中的说明。
#define _IN_LOG_C_FILE
#include "utility.h"
#undef  _IN_LOG_C_FILE

extern TCHAR *m_Path;

static SHAREDMEMORYINFO shared_info[MAX_SHARED_MEMORY_COUNT] = { 0, };

/*!
 * \brief 带有长度的复制字符串（为了进行移植的需要）。
 * \param target              目标地址
 * \param source              源地址
 * \return：无
*/
void CopyPartString(TCHAR *target, const TCHAR *source, int length)
{
    int i;

    for (i = 0; i < length && source[i]; i++)
        target[i] = source[i];

    target[i] = 0;
}

/*!
 * \brief 在LOG文件中写出信息。
 *  当LOG文件没有被初始化的时候，log信息将丢失。
 * \param   id              一般为文件名字与函数名字的组合
 * \param   format          输出格式，与printf相同
 *      ...             附带的可变参数
 * \return
 *      无
 */
void Log(const TCHAR *id, const TCHAR *format, ...)
{
    return;
}

/*!
 * \brief 将文件装载到缓冲区。
 *  请注意：缓冲区的大小必须满足文件读取的要求，不能小于文件的长度。
 * \param  file_name           文件全路径名称
 * \param  buffer              缓冲区
 * \param  buffer_length       缓冲区长度
 * \return
 *      成功：返回读取的长度，失败：-1
 */
int LoadFromFile(const TCHAR *file_name, void *buffer, int buffer_length)
{
    FILE *fd;
    int length;

    Log(LOG_ID, L"读取文件%s到内存%p", file_name, buffer);

        char filename[256];

    memset(filename, 0, sizeof(filename));

        if (-1 == wcstombs(filename, file_name, wcslen(file_name) + 1))
                return 0;

        fd = _tfopen(filename, "r");

//  fd = _tfopen(file_name, TEXT("rb"));
    if (!fd)
    {
        Log(LOG_ID, L"文件打开失败");
        return 0;
    }

    length = (int)fread(buffer, 1, buffer_length, fd);
    fclose(fd);

    if (length < 0)
    {
        Log(LOG_ID, L"文件读取失败");
        return 0;
    }

    Log(LOG_ID, L"文件读取成功, 长度:%d", length);
    return length;
}

/*!
 * \brief 将内存保存到文件。如果目标存在，则覆盖。
 * \param  file_name           文件全路径名称
 * \param  buffer              缓冲区指针
 * \param  buffer_length       文件长度
 * \return
 *      成功：1，失败：0
 */
int SaveToFile(const TCHAR *file_name, void *buffer, int buffer_length)
{
    FILE *fd;
    int length;

    Log(LOG_ID, L"保存内存%p到文件%s，长度：%d", buffer, file_name, buffer_length);

    char filename[256];

    memset(filename, 0, sizeof(filename));

    if (-1 == wcstombs(filename, file_name, wcslen(file_name) + 1))
        return 0;

    fd = _tfopen(filename, "wb");
    if (!fd)
    {
        TCHAR dir_name[MAX_PATH];
        char* dirname[MAX_PATH];

        int  i, index, ret;

        Log(LOG_ID, L"文件打开失败");

        //可能需要创建目录
        //1. 寻找当前文件的目录
        index = 0;
//      _tcscpy(dir_name, file_name);
        strcpy(dirname, filename);
//      for (i = 0; dirname[i]; i++)
//          if (dirname[i] == '/')
//              index = i;
        for (i = strlen(dirname) - 1; i >= 0 && dirname[i]; --i)
            if (dirname[i] == "/")
                index = i;
        if (!index)
            return 0;

        dirname[index] = 0;     //dir_name中包含有目录名字
//      ret = SHCreateDirectoryEx(0, dir_name, 0);
//      if (ret != ERROR_SUCCESS)
//          return 0;

        ret = mkdir(dirname, S_IRWXU | S_IRWXG);
        if (0 != ret)
            return 0;
        //创建目录成功，再次打开
        fd = _tfopen(filename, "wb");
        if (!fd)
            return 0;
    }

    length = (int)fwrite(buffer, 1, buffer_length, fd);
    fclose(fd);

    if (length != buffer_length)
    {
        Log(LOG_ID, L"文件写入失败");
        return 0;
    }

    Log(LOG_ID, L"文件写入成功");
    return length;
}

/*!
 * \brief 获得文件长度。
 * \param file_name           文件名称
 * \return
 *      文件长度，-1标识出错。
 */
int GetFileLength(const TCHAR *file_name)
{
    struct _stat f_data;

    char filename[256];
    memset(filename, 0, sizeof(filename));
    if (-1 == wcstombs(filename, file_name, wcslen(file_name)+ 1))
        return -1;
    if (_tstat(filename, &f_data))
        return -1;

    return (int) f_data.st_size;
}

/*!
 * \brief 获得共享内存区域指针。
 * \param shared_name         共享内存名称
 * \return
 *      没有找到：0
 *      找到：指向内存的指针
 * \todo 接口暂不使用，后续整理
 */
void *GetSharedMemory(const TCHAR *shared_name)
{
    int fd = -1;
        int ret = -1;

        char sharedname[MAX_PATH];

    memset(sharedname, 0, sizeof(sharedname));

        if (-1 == wcstombs(sharedname, shared_name, wcslen(shared_name) + 1)) {
                return 0;
        }

      //  fd = shm_open(sharedname, O_RDWR, 00777);
        if (-1 == fd) {
                return 0;
        }

        struct stat statbuf;
        if (-1 == fstat(fd, &statbuf)) {
                return 0;
        }

        char* p = mmap(NULL, statbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    return p;
}

/*!
 * \brief 获得只读的共享内存区域指针（针对IE7）
 * \param  shared_name         共享内存名称
 * \return
 *      没有找到：0
 *      找到：指向内存的指针
 * \todo 接口暂不使用，后续整理
 */
void *GetReadOnlySharedMemory(const TCHAR *shared_name)
{
        int fd = -1;
        int ret = -1;

        char sharedname[MAX_PATH];

    memset(sharedname, 0, sizeof(sharedname));

        if (-1 == wcstombs(sharedname, shared_name, wcslen(shared_name) + 1)) {
                return 0;
        }

//        fd = shm_open(sharedname, O_RDONLY, 00644);
        if (-1 == fd) {
                return 0;
        }
   /// 　获得文件或共享内存区的信息
        struct stat statbuf;
        if (-1 == fstat(fd, &statbuf)) {
                return 0;
        }

        char* p = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

        return p;

}

/*!
 * \brief   创建共享内存区
 * \param   shared_name         共享内存名称
 * \param   length              共享内存长度
 * \return
 *      创建失败：0
 *      成功：指向内存的指针
 * \todo 接口暂不使用，后续整理
 */
void *AllocateSharedMemory(const TCHAR *shared_name, int length)
{
    int fd = -1;
    int ret = -1;

    char sharedname[MAX_PATH];

    memset(sharedname, 0, sizeof(sharedname));

    if (-1 == wcstombs(sharedname, shared_name, wcslen(shared_name) + 1)) {
        return 0;
    }

//    fd = shm_open(sharedname, O_RDWR|O_CREAT, 00777);
    if (-1 == fd) {
        return 0;
    }

    ret = ftruncate(fd, length);
    if (-1 == ret) {
        return 0;
    }

    char *p = (char *)mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    return p;
}

/*!
 * \brief  释放共享内存区
 * \param  shared_name         共享内存名称
 * \param  pointer             共享内存指针
 * \return 无
 * \todo 接口暂不使用，后续整理
 */
void FreeSharedMemory(const TCHAR *shared_name, char *pointer)
{
    int fd = -1;
    int ret = -1;

        char sharedname[MAX_PATH];

    memset(sharedname, 0, sizeof(sharedname));

        if (-1 == wcstombs(sharedname, shared_name, wcslen(shared_name) + 1)) {
                return;
        }

//        fd = shm_open(sharedname, O_RDONLY, 00777);
        if (-1 == fd) {
                return;
        }

    struct stat statbuf;
    if (-1 == fstat(fd, &statbuf)) {
        return;
    }

    if (-1 == munmap(pointer, statbuf.st_size)) {
        return;
    }

  //  shm_unlink(sharedname);
}

/*!
 * \brief 获取词库文件所有目录
 * \param dir 词库文件路径的内存指针
 * \return 词库文件所有目录
 */
TCHAR *GetUserAppDirectory(TCHAR *dir)
{
    _tcscpy_s(dir, MAX_PATH, m_Path);
    return dir;
}

/*!
 * \brief 组合目录与文件，形成新的完整文件
 * \param   dir         目录名
 * \param   file        文件名
 * \param   result      结果
 * \return 新的完整文件
 */
TCHAR *CombineDirAndFile(const TCHAR *dir, const TCHAR *file, TCHAR *result)
{
    if (result != dir)
        _tcscpy(result, dir);

    if (dir[_tcslen(dir) - 1] != '/')
        _tcscat(result, TEXT("//"));

    if (file[0] == '/')
        file++;

    _tcscat(result, file);

    return result;
}

/*!
 * \brief 获得文件的绝对路径。
 *  存储于Config中的文件名字，可以为相对路径名字。
 *  如："theme/abc.jpg"，在/usr/app/unispim6目录中无法找到
 *  的话，则在/alluser/app/unispim6/中寻找，如果还找不到，则返回0
 *  如果文件为绝对路径，则找到直接返回。
 *  \param  file_name
 *  \param  result
 *  \return
 *      0：没有找到文件（/usr、/allusr下皆没有）
 *      其他：新文件名（即原来的result）
 */
TCHAR *GetFileFullName(int type, const TCHAR *file_name, TCHAR *result)
{

    TCHAR dir[MAX_PATH];

    switch(type)
    {
    case TYPE_USERAPP:
        case TYPE_ALLAPP:
        if (!GetUserAppDirectory(dir))
            return 0;
        break;
    default:
        return 0;
    }

    CombineDirAndFile(dir, file_name, result);

    return result;
}

/*!
 * \brief  Ansi字符串转换到UTF16
 */
void AnsiToUtf16(const char *name, wchar_t *wname, int nSize)
{
    MultiByteToWideChar(936, 0, name, (int)strlen(name) + 1, wname, nSize);
}

void U16CopyInLinux(char* bufferDest, char* bufferU16, int sizeU16)
{
    int i = 0;

    // no check on overlap, since no buffer length here.
    if (!bufferDest || !bufferU16) {return;}

    for (i = 0; i < sizeU16; i++)
    {
        *bufferDest++ = *bufferU16++;
        *bufferDest++ = *bufferU16++;
        *bufferDest++ = 0;
        *bufferDest++ = 0;
    }

    return;
}
void Utf16ToAnsi(const wchar_t *wchars, char *chars, int nSize)
{

#ifdef __linux__
    char * res = setlocale(LC_ALL, "zh_CN.GBK");
    if(res == NULL){
       printf("name is %s",res);
    }
#endif
    int ret = WideCharToMultiByte(936, 0, wchars, -1, chars, nSize, NULL, FALSE);
#ifdef __linux__
    if(ret < 0)
        chars[0] = '\0';
#endif
}

void UCS32ToUCS16(const UC UC32Char, TCHAR *buffer)
{
    buffer[1] = 0;

    if (UC32Char > 0x10FFFF || (UC32Char >= 0xD800 && UC32Char <= 0xDFFF))
    {
        buffer[0] = '?';
        return;
    }

    if (UC32Char < 0x10000)
        buffer[0] = (TCHAR)UC32Char;
    else
    {
        buffer[0] = (UC32Char - 0x10000) / 0x400 + 0xD800;
        buffer[1] = (UC32Char - 0x10000) % 0x400 + 0xDC00;
        buffer[2] = 0;
    }
}

/*!
 * \brief 将UTF32的字符串复制成UTF16的字符串
 * \param dest  目标字符串，复制后的UTF16字符串，其内存占据最小为 src 的一半，理论上应与 src 内存占据等长
 * \param src  源字符串，宽字符类型，Linux下字符单位为4字节，Windows下为2字节
 * \param n  复制字符个数，此函数不检查该值的合法性，应由调用者确保复制不会溢出
 * \return 复制后的UTF16的字符串
 */
WORD* utf16ncpy(WORD *dest, const wchar_t *src, size_t n)
{
    TCHAR buffer[3];
    TCHAR *pbuf = NULL;
    WORD *pdest = dest;

    if(sizeof(wchar_t) == 2)
        return (_tcsncpy(dest, src, n));

    while(n-- > 0 && *src)
    {
        UCS32ToUCS16(*src++, buffer);
        pbuf = buffer;
        while(*pbuf)
        {
            *pdest++ = (WORD)(*pbuf++);
        }
    }
    *pdest = 0;
    return pdest;
}


/*!
 * \brief 文件映射
 * \param 文件name
 * \return 文件映射handle结构
 */
FILEMAPHANDLE FileMapOpen(const wchar_t *file_name)
{
    FILEMAPHANDLE handle = NULL;
    struct stat status;
    char sharedname[MAX_PATH];
    memset(sharedname, 0, sizeof(sharedname));

    handle = (FILEMAPHANDLE) malloc(sizeof(FILEMAPDATA));
    if(!handle)
        return 0;

    if (-1 == wcstombs(sharedname, file_name, wcslen(file_name) + 1))
    {
        free(handle);
        return 0;
    }

    int h_file = open(sharedname,O_RDWR);

    if(h_file < 0)
    {
        free(handle);
        return 0;
    }

    handle->length = 0;
    int r = fstat(h_file,&status);
    if(r == -1){
        close(h_file);
        return 0;
    }

    if(fstat(h_file, &status) < 0)
    {
        free(handle);
        close(h_file);
        return 0;
    }

    handle->length = status.st_size;
    handle->h_map = mmap(0, handle->length, PROT_READ|PROT_WRITE, MAP_SHARED, h_file, 0);
    if(handle->h_map == NULL || handle->h_map == (void *) -1){
        if(errno == EACCES){
            close(h_file);
            return 0;
        } else if(errno == EAGAIN){
            close(h_file);
            return 0;

        }else if(errno == EINVAL){
            close(h_file);
            return 0;

        }else if(errno == ENFILE){
            close(h_file);
            return 0;

        }else if(errno == ENODEV){
            close(h_file);
            return 0;
        }else if(errno == ENOMEM){
            close(h_file);
            return 0;
        }else if(errno == EPERM){
            close(h_file);
            return 0;
        }else if(errno == ETXTBSY){
            close(h_file);
            return 0;
        }else if(errno == SIGSEGV){
            close(h_file);
            return 0;
        }else if(errno == SIGBUS){
            close(h_file);
            return 0;
        }
        close(h_file);
        return 0;
    }

    close(h_file);

    return handle;
}
/*!
 * \brief 映射文件大小
 * \param handle   文件映射handle
 * \param buffer   映射文件内存指针
 * \param length   映射文件大小
 * \return 映射文件大小
 */
int FileMapGetBuffer(FILEMAPHANDLE handle, char **buffer, int length)
{
    *buffer = (char *)handle->h_map;
    return handle->length;
}
/*!
 * \brief 关闭文件映射
 * \param handle  文件映射handle
 * \param name  该参数不使用
 * \return 成功 0 失败-1
 * \todo 整理接口参数
 */
int FileMapClose(FILEMAPHANDLE handle,const TCHAR *name)
{

    if(handle)
        {
        if (-1 == munmap(handle->h_map, handle->length)) {
            return -1;
        }
        handle->length = 0;
        free (handle);
        }

    return 0;
}

