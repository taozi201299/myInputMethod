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

/*  输入上下文管理
 */
#include "kernel.h"
#include "editor.h"
#include "context.h"
#include "config.h"
#include "utility.h"
#include "linux.h"
//#include <win32/pim_ime.h>
//#include <win32/main_window.h>
//#include <tchar.h>

//全局上下文数据，也是默认的数据
//PIMCONTEXT *context;          //全局（线程级别）Context，用于窗口的显示

PIMCONTEXT demo_context =
{
    STATE_START,
    HZ_LESS_USED,               //当前汉字集合的level

    //返回结果
    TEXT(""),                   //返回结果字符串
    0,                          //返回结果长度
    { {0}, },                   //结果音节数组
    0,                          //结果音节计数

    //上下文数据
    TEXT("hua'yu'pin'yin"), //用户输入的文字
    TEXT("hua'yu'pin'yin"), //用户输入的文字
    14,                         //输入串长度
    0,                          //正在处理的输入串位置
    9,                          //光标位置索引（以当前正在处理的syllable为准）
    0,                          //用户输入的符号
    0,                          //上次输入是否为数字
    0,                          //剩余的字符
    0,                          //上次输入的是否为点
    0,                          //上上次输入的是否为点

    //当前音节
	{ 0, },						//Syllables 逆向划分音节表
	{ 0, },						//正向划分音节表
    { 0, },                     //syllable start position
    4,                          //音节计数
	4,							//正向划分音节计数
    0,                          //当前处理音节位置
    0,
    { 0, },                     //默认汉字串
    { 0, },                     //默认汉字串的音节

    //已经选择的项目
    { {0}, },                   //已经进行了选择的数据
    0,                          //数据项计数

    //智能编辑状态数据
    { 0, },                     //已经选好的汉字
    { 0, },                     //智能编辑状态的音节位置

    //写作
    TEXT("hua2'yu3'pin'yin"),   //写作窗口的文字
    TEXT(""),                   //已经选择的写作窗口文字
    12,                         //写作串长度
    8,                          //光标位置

    //候选
    {
        { CAND_TYPE_ICW },
    },                          //候选集合
    42,                         //候选数目
    0,                          //显示候选的第一条索引
    1,                          //被选中的候选索引

    //当前页需要显示的候选
    {
        TEXT("元心拼音"),   TEXT("元心"), TEXT("化"), TEXT("话"), TEXT("花"), TEXT("画"), TEXT("华"), TEXT("滑"), TEXT("划"),
        TEXT("哗"), TEXT("桦"), TEXT("骅"), TEXT("婲"), TEXT("铧"),  TEXT("嬅"),  TEXT("婳"),  TEXT("婠"), TEXT("粿"),
        TEXT("猾"), TEXT("埖"), TEXT("吪"), TEXT("蕐"), TEXT("姡"), TEXT("搳"),   TEXT("芲"),  TEXT("觟"), TEXT("螖"),
        TEXT("摦"), TEXT("檴"), TEXT("磆"), TEXT("槬"), TEXT("蘳"), TEXT("杹"),   TEXT("夻"),  TEXT("黊"), TEXT("呚"),
        TEXT("砉")
    },
    45,                         //当前页中的候选数目
    { 0, },
};

/*!
 *\brief 清除上一次的结果
 */
void ClearResult(PIMCONTEXT *context)
{
    context->result_length         = 0;
    context->result_string[0]      = 0;
    context->result_syllable_count = 0;
}

/*!
 * \brief 初始化输入上下文
 */
void ResetContext(PIMCONTEXT *context)
{
    context->candidate_count        = 0;
    context->candidate_index        = 0;
    context->candidate_page_count   = 0;
    context->compose_cursor_index   = 0;
    context->compose_length         = 0;
    context->compose_string[0]      = 0;
    context->cursor_pos             = 0;
    context->last_symbol            = 0;
    context->input_length           = 0;
    context->input_pos              = 0;
    context->input_string[0]        = 0;
    context->origin_string[0]       = 0;
    context->selected_item_count    = 0;
    context->syllable_count         = 0;
    context->syllable_pos           = 0;
    context->syllable_mode          = 0;
    context->last_dot               = 0;
    context->next_to_last_dot       = 0;
    context->state = STATE_START;
}

/*!
 * \brief 第一次进行上下文的初始化
 */
void FirstTimeResetContext(PIMCONTEXT *context)
{
    BYTE key_state[0x100];

    memcpy(context, &demo_context, sizeof(PIMCONTEXT));

    //启动时默认半角输入
    pim_config->hz_option |= HZ_SYMBOL_HALFSHAPE;

    ResetContext(context);
}

/*!
 * \brief 处理上下文音节与字节之间的关系
 */
void MakeSyllableStartPosition(PIMCONTEXT *context)
{
    int i, index = 0;
    TCHAR pinyin[0x10];

    for (i = 0; i < context->syllable_count; i++)
    {
        context->syllable_start_pos[i] = index;

        index += GetSyllableString(context->syllables[i], pinyin, _SizeOf(pinyin), /*context->syllable_correct_flag[i],*/ 0);

        if (context->input_string[index] == SYLLABLE_SEPARATOR_CHAR)
            index++;
    }

    context->syllable_start_pos[i] = index;
}

