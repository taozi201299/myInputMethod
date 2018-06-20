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

#ifndef _SHARE_SEGMENT_H_
#define _SHARE_SEGMENT_H_

//#include <windows.h>
#include "linux.h"
#include "config.h"
#include "ci.h"
#include "kernel.h"

#include "wordlib.h"
#include "zi.h"

#ifdef __cplusplus
extern "C" {
#endif

static TCHAR *segment_share_name = TEXT("HYPIM_SEGMENT_SHARED_NAME");

typedef struct tagSHARE_SEGMENT
{
    int process_count;                                              //与IME连接的进程计数

    int is_default_ime;                                             //是否为默认输入法
    int global_config_update_time;                                  //配置装载时间
    int config_loaded;                                              //是否已经装载Config

    PIMCONFIG all_config;                                           //系统正在使用的Config

    int q1_index;                                                   //单引号
    int q2_index;                                                   //双引号
    int symbol_loaded;                                              //是否已经装载符号表

    int user_wordlib_id;                                            //用户词库标识
    int user_wl_modified;                                           //是否改变
    int can_save_user_wordlib;                                      //是否可以保存用户词库（向用户词库Import词汇可能被覆盖）

    TCHAR wordlib_name[MAX_WORDLIBS * 2][WORDLIB_FILE_NAME_LENGTH]; //词库的名称
    int wordlib_length[MAX_WORDLIBS * 2];                           //词库长度数组
    int wordlib_deleted[MAX_WORDLIBS * 2];                          //词库是否已经被删除

    int bh_loaded;                                                  //笔划是否已经在内存中

    CICACHE ci_cache;                                               //词cache
    int ci_cache_loaded;                                            //是否已经装入
    int ci_cache_modified;                                          //是否改变

    int resource_loaded;                                            //资源尚未装载

    int spw_count;                                                  //短语数目
    int spw_length;                                                 //缓冲区数据长度
    int spw_loaded;                                                 //短语是否已经在内存中

    SYLLABLEMAP syllable_map[462];                                  //拼音－音节转换表
    int syllable_map_items;

    HZCACHE hz_cache;                                               //汉字Cache结构
    int zi_cache_loaded;                                            //是否已经装载了字Cache
    int zi_cache_modified;                                          //是否改变
    int hz_data_loaded;                                             //汉字信息表是否已经装入
}DEFAULT_RESOURCE;

extern DEFAULT_RESOURCE *default_resource;

extern int LoadDefaultResource();
extern int FreeDefaultResource();

#ifdef __cplusplus
}
#endif

#endif
