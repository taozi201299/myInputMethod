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


#include "zi.h"
#include "ci.h"
#include "icw.h"
#include "config.h"
#include "utility.h"
#include "wordlib.h"
#include "init_resource.h"

/*!
 * \brief 装载全部词库文件。
 *  其中，用户词库与系统词库为默认加载；外挂词库基于config中的设定进行加载
 * \return
 *      0：失败，一般为系统词库或者用户词库设置错误
 *      1：成功，但可能存在某些外挂词库错误的情况
 */
int LoadWordLibraryResource()
{
    return LoadAllWordLibraries();
}

/*!
 * \brief 释放词库的资源
 */
int FreeWordLibraryResource()
{
    //关闭所有词库文件
    CloseAllWordLibrary();

    return 1;
}

/*!
 *\brief 装载汉字Cache数据
 * \return 成功：1，失败：0
 */
int LoadZiCacheResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_USERAPP, ZICACHE_FILE_NAME, name);

    return LoadZiCacheData(name);
}

/*!
 * \brief 释放汉字Cache数据
 * \return 成功：1，失败：0
 */
int FreeZiCacheResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_USERAPP, ZICACHE_FILE_NAME, name);

    return FreeZiCacheData(name);
}

/*!
 *\brief 装载汉字数据
 * \return 成功：1，失败：0
 */
int LoadHZDataResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_USERAPP, HZDATA_FILE_NAME, name);
    if (LoadHZData(name))
        return 1;

    GetFileFullName(TYPE_ALLAPP, HZDATA_FILE_NAME, name);
    return LoadHZData(name);
}

/*!
 * \brief 释放汉字数据
 * \return 成功：1，失败：0
 */
int FreeHZDataResource()
{
    return FreeHZData();
}

/*!
 * \brief 保存汉字Cache数据
 */
int SaveZiCacheResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_USERAPP, ZICACHE_FILE_NAME, name);

    return SaveZiCacheData(name);
}

/*!
 * \brief 装载词Cache数据
 * \return  成功：1    失败：0
 */
int LoadCiCacheResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_USERAPP, CICACHE_FILE_NAME, name);

    return LoadCiCacheData(name);
}

/*!
 * \brief 保存词Cache数据
 */
int SaveCiCacheResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_USERAPP, CICACHE_FILE_NAME, name);

    return SaveCiCacheData(name);
}

/*!
 * \brief 释放词Cache数据
 */
int FreeCiCacheResource()
{
    SaveCiCacheResource();
    return FreeCiCacheData();
}

/*!
 * \brief 装载bigram资源
 */
int LoadBigramResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_ALLAPP, BIGRAM_FILE_NAME, name);

    return LoadBigramData(name);
}

/*!
 * \brief 释放bigram资源
 */
int FreeBigramResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_ALLAPP, BIGRAM_FILE_NAME, name);

    return FreeBigramData(name);
}

/*!
 *\brief 装载笔划数据文件
*/
int LoadBHResource()
{
    TCHAR name[MAX_PATH];

    GetFileFullName(TYPE_USERAPP, HZBH_FILE_NAME, name);
    if(LoadBHData(name))
        return 1;

    GetFileFullName(TYPE_ALLAPP, HZBH_FILE_NAME, name);
    return LoadBHData(name);
}

int FreeBHResource()
{
    FreeBHData();
    return 1;
}

/*!
 *\brief 释放输入法资源
 */
int PIM_FreeResources()
{
    if (!default_resource->resource_loaded)
        return 1;

    default_resource->resource_loaded = 0;

    FreeWordLibraryResource();
    FreeBigramResource();

    FreeZiCacheResource();
    FreeCiCacheResource();
    FreeHZDataResource();

    return 1;
}

/*!
 * \brief 保存当前的数据资源
 */
int PIM_SaveResources()
{
    SaveWordLibrary(GetUserWordLibId());
    SaveZiCacheResource();
    SaveCiCacheResource();

    return 1;
}

/*!
 * \brief 装载输入法使用的资源，如果失败，则输入法不起作用
 * \return
 *      成功：1，失败：0
 */
int PIM_LoadResources()
{
    LoadWordLibraryResource();          //词库
    LoadBigramResource();               //Bigram数据

    if (default_resource->resource_loaded)
        return 1;

    default_resource->resource_loaded = 1;

    LoadZiCacheResource();              //汉字Cache
    LoadHZDataResource();               //装载汉字数据
    LoadCiCacheResource();              //词Cache

    return 1;
}

