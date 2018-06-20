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

/* 词库工具头文件
 */

#ifndef _WORDLIB_H_
#define _WORDLIB_H_

#include "kernel.h"
#include "syllable.h"

#ifdef __cplusplus
extern "C" {
#endif

//extern int user_wl_modified;
//extern int can_save_user_wordlib;

#define WORDLIB_MAX_FREQ                ((1 << 19) - 1)             //词库中的最大词频
#define BASE_CI_FREQ                    (500000)                    //词频基准
#define USER_BASE_FREQ                  100000                      //用户自定词的默认词频

//词库相关定义
#define DEFAULT_USER_WORDLIB_NAME   TEXT("用户词库")
#define DEFAULT_USER_WORDLIB_AUTHOR TEXT("元心拼音输入法")
#define WORDLIB_NAME_PREFIX         TEXT("\\unispim6\\wordlib\\")       //词库文件的前缀
#define WORDLIB_USER_SHORT_NAME     TEXT("user.uwl")                    //用户词库文件的短名称
#define WORDLIB_SYS_FILE_NAME       TEXT("unispim6//wordlib//sys.uwl")  //系统词库文件名

#define MAX_WORDLIBS                    2                           //内存中最多的词库数目
#define WORDLIB_PAGE_SIZE               1024                        //词库页大小
#define WORDLIB_NAME_LENGTH             16                          //词库名称长度
#define WORDLIB_AUTHOR_LENGTH           16                          //词库作者名称
#define PAGE_END                        -1                          //结束页
#define WORDLIB_EXTRA_LENGTH            0x100000                    //用户词库的扩充空间
#define WORDLIB_PAGE_DATA_LENGTH        (WORDLIB_PAGE_SIZE - 4 * sizeof(int))   //页中可用数据长度
#define WORDLIB_FILE_NAME_LENGTH        256                         //词库文件名称的最大长度
#define WORDLIB_FEATURE_LENGTH          sizeof(int)                 //词项中的特性描述长度，目前用一个整数
#define WORDLIB_CREATE_EXTRA_LENGTH     0x100000                    //创建新词库时，增加额外的1M数据。
#define WORDLIB_NORMAL_EXTRA_LENGTH     0x40000                     //普通增加容量便于添加词汇

typedef struct tagWINCHART
{
    char high;
    char low;
} WINCHART;

//词库头定义
typedef struct tagWORDLIBHEADER
{
    int         signature;                          //词库的签名
#ifdef __linux__
    WINCHART    name[WORDLIB_NAME_LENGTH];
    WINCHART    author_name[WORDLIB_AUTHOR_LENGTH];
#else
    TCHAR       name[WORDLIB_NAME_LENGTH];          //词库的名称
    TCHAR       author_name[WORDLIB_AUTHOR_LENGTH]; //词库作者名称
#endif
    int         word_count;                         //词汇数目
    int         page_count;                         //已分配的页数
    int         can_be_edit;                        //是否可以编辑
    int         pim_version;                        //输入法版本号（兼容性考虑）
    int         index[CON_NUMBER][CON_NUMBER];      //索引表
} WORDLIBHEADER;

//页定义
typedef struct tagPAGE
{
    int         page_no;                            //页号
    int         next_page_no;                       //下一个页号，-1标识结束
    int         length_flag;                        //本页包含的词汇长度的标志
    int         data_length;                        //已经使用的数据长度
    char        data[WORDLIB_PAGE_DATA_LENGTH];     //数据开始
} PAGE;

//词库定义
typedef struct tagWORDLIB
{
    union
    {
        //词库文件头数据
        WORDLIBHEADER   header;

        //用于对齐页边界
        PAGE            header_data[sizeof(WORDLIBHEADER) / WORDLIB_PAGE_SIZE + 1];
    };
    PAGE                pages[1];                   //页数据
} WORDLIB;

//V5词库定义
typedef struct tagWORDLIBV5
{
    int     magic_number;
    int     index_table[8][18][18];
    int     win_time;
    int     free_address;
}WORDLIBV5;

#define WLUP_OPER_ADD       1               //增加词条
#define WLUP_OPER_DEL       2               //删除词条
#define WLUP_OPER_UPDATE    3               //更新词条

typedef struct tagUPDATEITEM
{
    const TCHAR *ci_string;                 //词字符串
    const TCHAR *py_string;                 //拼音字符串
    int freq;                               //词频
    int operation;                          //操作：OPER_ADD (1), OPER_DELETE(2), OPER_UPDATE(3),
}UPDATEITEM;

#define ENCODERV5       0xfdef              //V5汉字的加密数字

#define WLF_CONTINUE_ON_ERROR       (1 << 0)
#define WLF_HALT_ON_ERROR           (1 << 1)

#define WLF_CLEAR_WORDLIB           (1 << 2)
#define WLF_DELETE_WORDLIB          (1 << 3)
#define MAX_USER_WORD       (1 << 16)

#define WLE_CI                      (-1)
#define WLE_YIN                     (-2)
#define WLE_FREQ                    (-3)
#define WLE_NAME                    (-4)
#define WLE_ITEMS                   (-5)
#define WLE_NOCI                    (-6)
#define WLE_OTHER                   (-10)

//装载词库文件。
extern int LoadWordLibrary(const TCHAR *wordlib_name);

//保存词库文件
extern int SaveWordLibrary(int wordlib_id);

//获得词库的指针
extern WORDLIB *GetWordLibrary(int wordlib_id);

//建新页面
extern int NewWordLibPage(int wordlib_id);

//向词库中增加词项
extern int AddCiToWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int freq);
extern int AddCiToUserWordLibrary(HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int freq);
//确定词是否在词库中
extern WORDLIBITEM *GetCiInWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length);

//在词库中删除词汇。
extern int DeleteCiFromWordLib(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int clear_syllable);

//装载词库文件，附带额外的数据长度（一般用于创建词库文件或者装载用户词库文件）
extern int LoadWordLibraryWithExtraLength(const TCHAR *lib_name, int extra_length, int check_exist);

//装载用户词库文件
extern int LoadUserWordLibrary(const TCHAR *wordlib_name);

//释放词库数据。
extern void CloseWordLibrary(int wordlib_id);

//释放所有的词库数据。
extern void CloseAllWordLibrary();

//创建空的词库文件
extern int CreateEmptyWordLibFile(const TCHAR *wordlib_file_name, const TCHAR *name, const TCHAR *author, int can_be_edit);

//获得下一个词库标识，用于词库的遍历。
int GetNextWordLibId(int cur_id);
const TCHAR *GetWordLibFileName(int wl_id);

//获得用户词库标识
int GetUserWordLibId();

int LoadAllWordLibraries();
void MaintainWordLibPointer();
int GetWordLibraryLoaded(const TCHAR *lib_name);

//删词
int __stdcall DeleteCiFromAllWordLib(TCHAR *ci_str, int ci_length, TCHAR *py_str, int py_length);

#ifdef __cplusplus
}
#endif

#endif
