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

/*  输入法配置程序。
 *  配置程序从注册表中装载配置信息。当注册表中不存在配置信息时，设置
 *  默认的配置到注册表。
 */
#include "kernel.h"
#include "config.h"
#include "context.h"
#include "utility.h"
#include "pim_resource.h"
#include "linux.h"
#include "init_resource.h"

//制作主题的函数名
typedef int (_stdcall *pMAKE_THEME) (PIMCONFIG *);
typedef int (_stdcall *pBACKUP) (const TCHAR *);

PIMCONFIG *pim_config;                  //全局使用的config指针

#pragma data_seg(HYPIM_SHARED_SEGMENT)

PIMCONFIG default_config =              //默认的输入法配置数据
{
    //输入风格
    STYLE_CSTAR,

    //启动的输入方式：中文、英文
    STARTUP_CHINESE,

    //拼音模式
    PINYIN_QUANPIN,

    //是否使用汉字音调辅助
    1,

    //是否使用ICW（智能组词）
    1,

    //首字母输入的最小汉字数目
    4,

    //汉字输出的方式：简体、繁体。同时繁体字包含未分类汉字。
    HZ_OUTPUT_SIMPLIFIED,

    //输入字的选项
    HZ_RECENT_FIRST         |       //最近输入的字优先（默认）
    HZ_ADJUST_FREQ_FAST     |       //字输入调整字频（默认）
    HZ_USE_TAB_SWITCH_SET   |       //使用TAB切换汉字的集合
    HZ_USE_TWO_LEVEL_SET    |       //使用两种集合切分方式
    HZ_SYMBOL_CHINESE       |       //使用汉字符号
    HZ_SYMBOL_HALFSHAPE     |       //半角符号
    HZ_USE_FIX_TOP,                 //使用固顶字

    //词输入选项
    CI_AUTO_FUZZY           |       //输入词的时候，自动使用z/zh, c/ch, s/sh的模糊（默认）
    CI_SORT_CANDIDATES      |       //候选词基于词频进行排序（默认）
    CI_ADJUST_FREQ_FAST     |       //快速调整词频（默认）
    CI_WILDCARD             |       //输入词的时候，使用通配符（默认）
    CI_USE_FIRST_LETTER     |       //使用首字母输入词（默认）
    CI_RECENT_FIRST         |       //最新输入的词优先（默认）
    CI_AUTO_VOW_FUZZY,              //使用韵母自动匹配

    //是否使用模糊音
    0,

    //模糊音选项
    0,                              //全部不模糊

    //词库文件名称
    {
        TEXT("unispim6//wordlib//user.uwl"),
        TEXT("unispim6//wordlib//sys.uwl"),
    },

    //词库数量
    2,

    //显示在候选页中的候选的个数
    8,

    //单个短语候选时，显示位置；使用上面废弃的配置项；默认为3
    //spw_position
    3,

    //使用词语联想
    1,

    //从第x个音节开始联想
    4,

    //联想词位于候选词第x位
    2,

    //联想词个数
    2,

    //只输出GBK集合，scope_gbk
    HZ_SCOPE_GBK,

    //开启屏蔽天窗功能，默认不开启
    0,

};

#pragma data_seg()

/*  将配置写入到注册表中。
 *  内部函数，用于本模块的调用，不进行比较以及Resource的重新装载
 */
void SaveConfigInternal(PIMCONFIG *config)
{
    //保存config后，需要进行更新的操作。
    if (pim_config && pim_config != config)
        memcpy(pim_config, config, sizeof(PIMCONFIG));
}

