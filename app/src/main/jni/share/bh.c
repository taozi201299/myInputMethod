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

/** 笔划输入模块
 *  提供两个笔划输入的功能：
 *  1. 顺序笔划输入模式
 *  2. 无顺序笔划输入模式
 */

#include "zi.h"
#include <assert.h>
#include <string.h>
#include "config.h"
#include "utility.h"
//#include <win32/pim_ime.h>
#include <string.h>
//#include <tchar.h>
#include "init_resource.h"

static BHDATA *bh_data      = 0;  //笔画文件映射内存块
static TCHAR *bh_share_name = TEXT("HYPIM_BH_SHARED_NAME");

//#pragma data_seg(HYPIM_SHARED_SEGMENT)
//static int    bh_loaded = 0;          //笔划是否已经在内存中
//#pragma   data_seg()

/*!
 * \brief 获得笔划输入的候选
 */
int GetBHCandidates(const TCHAR *input_string, CANDIDATE *candidates, int array_length)
{
    char bh_string[0x100];
    int  i, bh_length, min_bhs, hz_count, idx;
    INT_PTR maxp;
    int *index1, *index2;
    BHITEM *data;

    extern int LoadBHResource();

    //没有输入或者输入不为B则直接返回
    if (!array_length || !input_string || *input_string != 'B' || !input_string[1])
        return 0;

    if(!default_resource->bh_loaded)
        LoadBHResource();

    if (!bh_data)
    {
        bh_data = GetReadOnlySharedMemory(bh_share_name);

        //可能存在其他进程已经装载了，但是退出后共享内存被释放的问题
        if (!bh_data && default_resource->bh_loaded)
        {
            default_resource->bh_loaded = 0;
            LoadBHResource();
        }
    }

    if (!bh_data)
        return 0;

    //检查是否都为笔划字母与数字构成，否则返回；并确认是否为模糊匹配
    min_bhs = 0;
    for (bh_length = 0, i = 1; input_string[i] && i < sizeof(bh_string) - 1; i++)
    {
        switch (input_string[i])
        {
            case 'h':
                bh_string[bh_length] = '1';
                break;

            case 's':
                bh_string[bh_length] = '2';
                break;

            case 'p':
                bh_string[bh_length] = '3';
                break;

            case 'n':
                bh_string[bh_length] = '4';
                break;

            case 'd':
                bh_string[bh_length] = '4';
                break;

            case 'z':
                bh_string[bh_length] = '5';
                break;

            case '*':
                bh_string[bh_length] = '*';
                min_bhs--;
                break;

            case '?':
                bh_string[bh_length] = '?';
                break;

            default:
                return 0;
        }

        bh_length++;
    }

    bh_string[bh_length] = 0;
    if (!bh_length)
        return 0;

    min_bhs += bh_length;

    if (min_bhs <= 0)
        return 0;

    index1 = bh_data->index1;
    index2 = (int*)(bh_data->index1 + bh_data->maxstrockes);
    data   = (BHITEM*)(index2 + (bh_data->maxmcp - bh_data->minmcp + 1));
    idx    = bh_data->index1[min_bhs - 1];

    maxp = (INT_PTR)data + sizeof(BHITEM) * bh_data->itemcount;

    //取得汉字的候选
    for (hz_count = 0, i = 0; i < bh_data->itemcount && hz_count <array_length; i++)
    {
        BHITEM *p = (BHITEM*)((INT_PTR)bh_data + idx + i * sizeof(BHITEM));

        if((INT_PTR)p >= maxp)
            break;

        if (pim_config->scope_gbk == HZ_SCOPE_UNICODE)
        {
            if ( pim_config->hide_black_window)
                continue;
        }
    }

    return hz_count;
}

/*!
 * \brief 获得笔划的候选显示
 */
void GetBHDisplayString(CANDIDATE *candidate, TCHAR *buffer, int length)
{
    if (!buffer || !length)
        return;

    *buffer = 0;
    return;
}

/*!
 * \brief 加载笔划数据文件到内存。
 * \param file_name           笔划数据文件全路径
 * \return 成功：1 失败：0
 */
int LoadBHData(const TCHAR *file_name)
{
    int file_length;

    assert(file_name);

    if (default_resource->bh_loaded)
        return 1;

    file_length = GetFileLength(file_name);
    if (file_length <= 0)
        return 0;

    bh_data = AllocateSharedMemory(bh_share_name, file_length);
    if (!bh_data)
        return 0;

    if ((file_length = LoadFromFile(file_name, bh_data, file_length)) == -1)
    {
        FreeSharedMemory(bh_share_name, bh_data);
        Log(LOG_ID, L"笔划数据文件打开失败。name=%s", file_name);
        return 0;
    }

    if (!file_length)
        return 0;

    default_resource->bh_loaded = 1;

    return 1;
}

/*!
 * \brief 释放笔划数据文件
 */
int FreeBHData()
{
    default_resource->bh_loaded = 0;

    if (bh_data)
    {
        FreeSharedMemory(bh_share_name, bh_data);
        bh_data = 0;
    }

    return 1;
}
