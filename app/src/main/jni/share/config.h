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

/*  输入法配置头文件。
 */

//TODO: *****程序员符号模式，即在非中文的字符之间的/以及*符号一直为半角（需要进行很好的编程以及试验）

//注：本源代码在TAB设置为4，Indent设置为4的编辑器设置上显示才能正常。
/* Vim ts=4;sw=4 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "wordlib.h"
#include "linux.h"

//配置程序运行后，改变的配置项
#define MODIFY_NONE                 0
#define MODIFY_WORDLIB              (1 << 0)
#define MODIFY_THEME                (1 << 1)
#define MODIFY_INI                  (1 << 2)
#define MODIFY_ZI                   (1 << 3)
#define MODIFY_WORDLIB_FORCE_RELOAD (1 << 4) //词库有变动，强制重新装载词库(恢复数据的时候)

#define MAX_FILE_NAME_LENGTH        256             //文件名字的最大长度

//输入法风格
#define STYLE_CSTAR                 0               //中文之星风格（默认）
#define STYLE_ABC                   1               //智能ABC风格
#define STYLE_MS2                   2               //微软风格

//拼音方式
#define PINYIN_QUANPIN              0               //全拼
#define PINYIN_SHUANGPIN            1               //双拼

//启动输入法状态
#define STARTUP_CHINESE             0               //启动时为英文输入状态

//候选选择方式
#define SELECTOR_DIGITAL            0               //使用数字选择候选（默认）
#define SELECTOR_LETTER             1               //使用字母选择候选

//简繁定义
#define HZ_OUTPUT_SIMPLIFIED        (1 << 0)        //输出简体字（默认）
#define HZ_OUTPUT_TRADITIONAL       (1 << 1)        //输出繁体字
#define HZ_OUTPUT_HANZI_ALL         (1 << 2)        //输出全集
#define HZ_OUTPUT_ICW_ZI            (1 << 4)        //输出ICW使用的汉字集合
#define HZ_SYMBOL_CHINESE           (1 << 5)        //中文符号
#define HZ_SYMBOL_HALFSHAPE         (1 << 6)        //半角符号

#define HZ_SCOPE_GBK                (1 << 0)
#define HZ_SCOPE_UNICODE            (1 << 1)

//字选项
#define HZ_RECENT_FIRST             (1 << 0)        //最近输入的字优先（默认）
#define HZ_ADJUST_FREQ_FAST         (1 << 2)        //快速调整字频（默认）
#define HZ_ADJUST_FREQ_SLOW         (1 << 3)        //慢速调整字频
#define HZ_ADJUST_FREQ_NONE         (1 << 4)        //不调整字频
#define HZ_USE_TAB_SWITCH_SET       (1 << 6)        //使用TAB切换汉字的集合（默认）
#define HZ_USE_TWO_LEVEL_SET        (1 << 7)        //使用两种集合切分方式（默认）
#define HZ_USE_THREE_LEVEL_SET      (1 << 8)        //使用三种集合切分方式
#define HZ_USE_FIX_TOP              (1 << 9)        //使用固顶字（默认）

//词选项
#define CI_AUTO_FUZZY               (1 << 0)        //输入词的时候，自动使用z/zh, c/ch, s/sh的模糊（默认）
#define CI_SORT_CANDIDATES          (1 << 1)        //候选词基于词频进行排序（默认）
#define CI_ADJUST_FREQ_FAST         (1 << 2)        //快速调整词频（默认）
#define CI_ADJUST_FREQ_SLOW         (1 << 3)        //慢速调整词频（默认）
#define CI_ADJUST_FREQ_NONE         (1 << 4)        //不调整词频（默认）
#define CI_WILDCARD                 (1 << 5)        //输入词的时候，使用通配符（默认）
#define CI_RECENT_FIRST             (1 << 6)        //最新输入的词优先（默认）
#define CI_USE_FIRST_LETTER         (1 << 7)        //使用首字母输入词（默认）
#define CI_AUTO_VOW_FUZZY           (1 << 8)        //输入词的时候自动进行韵母首字母模糊匹配

//配置选项采用在注册表中开辟内存DUMP区的方式
//进行存储。这样读取以及写入配置的过程可以得到很大的简化

//输入选项结构
typedef struct tagHYPIMCONFIG
{
    //输入风格
    int     input_style;                            //默认STYLE_CSTAR

    //启动的输入方式：中文、英文
    int     startup_mode;                           //默认为STARTUP_CHINESE

    //拼音模式
    int     pinyin_mode;                            //默认为PINYIN_QUANPIN

    //是否使用汉字音调辅助
    int     use_hz_tone;                            //默认为1

    //是否使用ICW（智能组词）
    int     use_icw;                                //默认为1

    //首字母输入词汇。输入tagl，则产生候选“提案管理”。汉字数太少，有很大的问题。
    int     first_letter_input_min_hz;              //首字母输入的最小汉字数目，默认为3。

    //汉字输出的方式：简体、繁体
    int     hz_output_mode;                         //默认为HZ_OUTPUT_SIMPLIFIED

    //输入字的选项
    int     hz_option;                              //默认为全部

    //词输入选项
    int     ci_option;                              //默认为全部

    //是否使用模糊音，由于用户可能设置模糊音，不使用的时候，还应该保留，所以加入了这个选项。
    int     use_fuzzy;                              //默认为0

    //模糊音选项
    int     fuzzy_mode;                             //默认为0，全部不模糊

    //词库文件名称
    TCHAR   wordlib_name[MAX_WORDLIBS][MAX_FILE_NAME_LENGTH];
            //第一个默认为"wordlib/usr.uwl"(userapp)  用户词库
            //第二个默认为"wordlib/sys.uwl"(alluserapp)   系统词库

    //词库数量
    int     wordlib_count;                          //默认为2，系统词库与用户词库不在本表中记录

    //显示在候选页中的候选的最多个数
    int     candidates_per_line;                    //默认为5

    //单个短语候选时，显示位置；使用上面废弃的配置项
    ////为兼容以往的配置，spw_position的数值从11~19，使用时直接减10
    int    spw_position;                            //默认为13

    //使用词语联想
    int     use_word_suggestion;                    //默认为1

    //从第x个音节开始联想
    int     suggest_syllable_location;              //默认为2

    //联想词位于候选词第x位
    int     suggest_word_location;                  //默认为2

    //联想词个数
    int     suggest_word_count;                     //默认为2

    //输出字符集范围：gbk、unicode
    int     scope_gbk;                              //是否只输出gbk集合

    //显示Unicode Ext-B和Ext-C汉字
    int     hide_black_window;                      //开启屏蔽天窗功能

} PIMCONFIG;

//全局共享配置信息
extern PIMCONFIG *pim_config;
extern PIMCONFIG default_config;

extern void SaveConfigInternal(PIMCONFIG *config);

#ifdef __cplusplus
}
#endif

#endif
