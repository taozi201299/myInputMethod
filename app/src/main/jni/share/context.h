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

/*  输入上下文头文件
 */

#ifndef _INPUTCONTEXT_H_
#define _INPUTCONTEXT_H_

#include "kernel.h"
//#include <windows.h>
#include "linux.h"
//#include <win32/immdev.h>

#ifdef __cplusplus
extern "C" {
#endif

#define     CHINESE_MODE        (1 << 0)        //中文输入模式
#define     ENGLISH_MODE        (1 << 1)        //英文输入模式
#define     CAPTICAL_MODE       (1 << 2)        //大写方式
#define     QP_MODE             (1 << 3)        //全拼模式
//#define       SP_MODE             (1 << 4)        //双拼模式

#define     STATE_START         0               //起始状态
#define     STATE_EDIT          1               //编辑状态
#define     STATE_SELECT        2               //选择候选状态
#define     STATE_ENGLISH       3               //英文状态
#define     STATE_RESULT        4               //候选上屏状态
#define     STATE_ABC_SELECT    11              //ABC风格选择状态
#define     STATE_IEDIT         12              //智能编辑状态

typedef struct tagEditItem
{
    int         syllable_start_pos;             //起始音节位置
    int         syllable_length;                //占用的音节数目
    CANDIDATE   candidate;                      //本项的候选信息
    int         left_or_right;                  //是否为以词定字，并标识左侧、右侧
}SELECT_ITEM;

struct tagUICONTEXT;

//输入上下文结构
typedef struct tagPIMCONTEXT
{
    int         state;                                                  //输入法当前状态
    int         zi_set_level;                                           //当前汉字集合的level

    //返回结果
    TCHAR       result_string[MAX_RESULT_LENGTH + 1];                   //上屏的结果
    int         result_length;                                          //结果长度（以字节为单位）
    SYLLABLE    result_syllables[MAX_SYLLABLE_PER_INPUT];               //结果音节
    int         result_syllable_count;                                  //音节计数

    //上下文数据
    TCHAR       input_string[MAX_INPUT_LENGTH + 0x10];                  //用户输入的文字
    TCHAR       origin_string[MAX_INPUT_LENGTH + 0x10];
    int         input_length;                                           //输入串长度
    int         input_pos;                                              //正在处理的输入串位置
    int         cursor_pos;                                             //光标位置索引（以当前正在处理的syllable为准）
    TCHAR       last_symbol;                                            //用户输入的符号
    int         last_digital;                                           //上次输入的是否为数字
    int         last_char;                                              //上次输入剩余的字母（用于立即进行输入）
    int         last_dot;                                               //上次输入的是否为点
    int         next_to_last_dot;                                       //上上次输入的是否为点

    //当前音节
	SYLLABLE	syllables[MAX_SYLLABLE_PER_INPUT + 0x10];				//逆向划分音节表

	SYLLABLE	fore_syllables[MAX_SYLLABLE_PER_INPUT + 0x10];			//正向划分音节表
    int         syllable_start_pos[MAX_SYLLABLE_PER_INPUT + 0x10];      //音节在输入串中的起始位置表
	int			syllable_count;											//逆向划分音节计数
	int			fore_syllable_count;									//正向划分音节计数
    int         syllable_pos;                                           //当前处理到的音节位置
    int         syllable_mode;                                          //音节模式，当候选为短语时、输入拼音串中包括通配符时，此状态为0
    HZ          default_hz[MAX_SYLLABLE_PER_INPUT + 0x10];              //默认汉字串
    SYLLABLE    default_hz_syllables[MAX_SYLLABLE_PER_INPUT + 0x10];    //默认汉字串的音节(与默认汉字串中的每个汉字由正确的一一对应关系)

    //已经选择的项目
    SELECT_ITEM selected_items[MAX_SYLLABLE_PER_INPUT];                 //已经进行了选择的数据
    int         selected_item_count;                                    //数据项计数

    //智能编辑状态数据
    HZ          iedit_hz[MAX_SYLLABLE_PER_INPUT + 0x10];                //已经选好的汉字
    int         iedit_syllable_index;                                   //智能编辑状态的音节位置

    //写作
    TCHAR       compose_string[MAX_COMPOSE_LENGTH];                     //写作窗口的文字
    TCHAR       selected_compose_string[MAX_COMPOSE_LENGTH];            //已经选择过的写作窗口文字
    int         compose_length;                                         //写作串长度
    int         compose_cursor_index;                                   //光标所在位置

    //候选
    CANDIDATE   candidate_array[MAX_CANDIDATES];                        //候选集合
    int         candidate_count;                                        //候选数目
    int         candidate_index;                                        //显示候选的第一条索引
    int         candidate_selected_index;                               //被选中的候选索引

    //当前页需要显示的候选
    TCHAR       candidate_string[MAX_CANDIDATES_PER_LINE * MAX_CANDIDATE_LINES][MAX_CANDIDATE_STRING_LENGTH + 2];
    int         candidate_page_count;                                   //当前页中的候选数目
    TCHAR       candidate_trans_string[MAX_CANDIDATES_PER_LINE * MAX_CANDIDATE_LINES][MAX_TRANSLATE_STRING_LENGTH + 2];
}PIMCONTEXT;

extern PIMCONTEXT demo_context;

//extern void ClearContext();
extern void ClearResult(PIMCONTEXT *context);
extern void ResetContext(PIMCONTEXT *context);
extern void FirstTimeResetContext(PIMCONTEXT *context);
extern void ProcessInputChar(PIMCONTEXT *context, char ch);
extern void MakeSyllableStartPosition(PIMCONTEXT *context);

#ifdef __cplusplus
}
#endif

#endif
