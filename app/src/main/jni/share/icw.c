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

/*  智能组词模块
 *
 *  智能组词的改进
 *  1. 估值函数中包含词频信息，现在去掉
 *  2. 用户自造词的词频加入到估值函数中
 *  3. 将公式修改为更简洁的方式
 *  4. 用动态规划法进行编写算法
 *  5. 组成ICW的词条越短越好，用除法进行操作
 *
 *
 *  TCOC: 最小词频：4096，对数放大倍数32
 */
#include "icw.h"
#include "gram.h"
#include "zi.h"
#include "ci.h"
#include "config.h"
#include "utility.h"
#ifdef __linux__
#define MAX_HZ_BUFFER_SIZE   0x40
#else
#define MAX_HZ_BUFFER_SIZE   0x20
#endif

int show_icw_info = 0;

FILEMAPHANDLE   bigram_handle;
GRAM_DATA       *bigram_data = 0;
int             bigram_data_length = 0;

typedef struct tagNEWICWITEM
{
    int         length;                     //候选项的长度
    HZ          *hz;                        //汉字
    SYLLABLE    *syllable;                  //音节
    int         freq;                       //字频或者词频
    double      value;                      //估值
    struct tagNEWICWITEM    *next;          //下一项
}NEWICWITEM;

typedef struct tagICWLIFE
{
    double  value;
    const NEWICWITEM *item[MAX_ICW_LENGTH];
}ICWLIFE;

typedef struct tagICWGROUPITEM
{
    int         count;
    NEWICWITEM  item[ICW_MAX_CI_ITEMS];
}ICWGROUPITEM;

typedef struct tagICWITEMSET
{
    int             group_count;
    ICWGROUPITEM    group_item[MAX_ICW_LENGTH];
}ICWITEMSET;


int ci_option_save = 0;

void SaveCiOption()
{
    ci_option_save = pim_config->ci_option;
    pim_config->ci_option &= ~CI_AUTO_VOW_FUZZY;
}

void RestoreCiOption()
{
    pim_config->ci_option = ci_option_save;
}

/*!
 * \brief 装载bigram数据
 */
int LoadBigramData(const TCHAR *name)
{
    if (bigram_data)            //已经装载
        return 1;

    bigram_handle = FileMapOpen(name);
    if (!bigram_handle)
        return 0;

    if ((bigram_data_length = FileMapGetBuffer(bigram_handle, (char**)&bigram_data, 0)) <= 0){
        bigram_data = 0;
        return 0;
    }

    decode_word_list(GetGramWordList(bigram_data), bigram_data->header.word_list_size);

    return 1;
}

/*!
 *\brief 加快bigram装载速度
 */
int MakeBigramFaster()
{
    int i, x;
    int *data;

    data = (int*)bigram_data;
    if (!data)
        return 0;

    for (x = 0, i = 0; i < bigram_data_length / 4; i++)
        x += data[i];

    return x;
}

/*!
 *\brief 释放bigram数据
 */
int FreeBigramData(const TCHAR *name)
{
    FileMapClose(bigram_handle,name);
    bigram_data = 0;
    bigram_handle = 0;
    return 1;
}

/*!
 *\brief 产生ICW的项
 */
int GenerateICWItems(ICWITEMSET *icw_items, SYLLABLE *syllable, int syllable_count)
{
    //CANDIDATE candidates[ICW_MAX_ITEMS];
    CANDIDATE   *candidates;    //改为在堆上分配空间，避免堆栈溢出
    int         count, iPrevFreq, iCurFreq;
    int         i, j, iItemNum = 0;
    const int   ciMaxTempItemNum = 20;
    const int   ciMaxICWItem = 30; // ICW_MAX_ITEMS
    const double cfChangeRateThreshold = 0.05;

    candidates = malloc(sizeof(CANDIDATE) * ciMaxICWItem);

    //音节数目
    icw_items->group_count = syllable_count;

    //提取每一个音节的候选
    for (i = 0; i < syllable_count; i++)
    {
        count = 0;
        for (j = 2; j <= min(8, syllable_count - i); j++)
            count += ProcessCiCandidate(syllable + i, j, 0, candidates + count, ciMaxICWItem - count, 1);

        if (count >= ICW_MAX_CI_ITEMS)
        {
            count = ICW_MAX_CI_ITEMS;
        }
        else
        {
            //获得汉字
            count += GetZiCandidates(syllable[i], candidates + count, ciMaxICWItem - count, pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0, HZ_MORE_USED, HZ_OUTPUT_ICW_ZI);
            if (!count)     //在没有任何项的情况下，才找更大集合的汉字
                count += GetZiCandidates(syllable[i], candidates + count, ciMaxICWItem - count, pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0, HZ_MORE_USED, HZ_OUTPUT_SIMPLIFIED);

            count = (count > ICW_MAX_CI_ITEMS) ? ICW_MAX_CI_ITEMS : count;

            if (!count)     //没有找到候选(可能是拼音错误，如:chua)，或无法分配内存
            {
                free(candidates);
                return 0;
            }
        }

        //设置汉字数组以及长度
        iItemNum = 0;
        for (j = 0; j < count; j++)
        {
            if (j == ciMaxTempItemNum)
            {
                if (candidates[j].type == CAND_TYPE_ZI)
                {
                    iPrevFreq = ConvertToRealHZFreq((int)(candidates[j].hz.item->freq));
                }
                else if (candidates[j].type == CAND_TYPE_CI &&
                    candidates[j].word.item->ci_length == candidates[j].word.item->syllable_length)
                {
                    iPrevFreq = ConvertToRealCIFreq((int)candidates[j].word.item->freq);
                }
            }

            if (j > ciMaxTempItemNum)
            {
                if (candidates[j].type == CAND_TYPE_ZI)
                {
                    iCurFreq = ConvertToRealHZFreq((int)(candidates[j].hz.item->freq));
                    if (((float)iCurFreq / (float)iPrevFreq) <= cfChangeRateThreshold)
                    {
                        break;
                    }
                    iPrevFreq = iCurFreq;
                }
                else if (candidates[j].type == CAND_TYPE_CI &&
                    candidates[j].word.item->ci_length == candidates[j].word.item->syllable_length)
                {
                    iCurFreq = ConvertToRealCIFreq((int)candidates[j].word.item->freq);
                    if (((float)iCurFreq / (float)iPrevFreq) <= cfChangeRateThreshold)
                    {
                        break;
                    }
                    iPrevFreq = iCurFreq;
                }
            }

            if (candidates[j].type == CAND_TYPE_ZI)
            {
                icw_items->group_item[i].item[iItemNum].length      = 1;
                icw_items->group_item[i].item[iItemNum].hz          = (HZ*)&candidates[j].hz.item->hz;
                icw_items->group_item[i].item[iItemNum].syllable    = &candidates[j].hz.item->syllable;
                icw_items->group_item[i].item[iItemNum].freq        = ConvertToRealHZFreq((int)(candidates[j].hz.item->freq));
                iItemNum++;
            }
            else if (candidates[j].type == CAND_TYPE_CI &&
                candidates[j].word.item->ci_length == candidates[j].word.item->syllable_length)
            {
                icw_items->group_item[i].item[iItemNum].length      = candidates[j].word.item->ci_length;
                icw_items->group_item[i].item[iItemNum].hz          = candidates[j].word.hz;
                icw_items->group_item[i].item[iItemNum].syllable    = candidates[j].word.syllable;
                icw_items->group_item[i].item[iItemNum].freq        = ConvertToRealCIFreq((int)candidates[j].word.item->freq);
                iItemNum++;
            }
        }

        //本音节的候选数目
        icw_items->group_item[i].count = (iItemNum > count) ? count : iItemNum;
    }

    free(candidates);

    return 1;
}

/*!
 * \brief 估计组价值
 */
void NewEvaluateGroup(ICWITEMSET *icw_items, int group_no)
{
    NEWICWITEM *items, *next_items;
    int i, j, index, next_group_no;
    int start, end;
    int count, next_count;
    char ci0[MAX_HZ_BUFFER_SIZE], ci1[MAX_HZ_BUFFER_SIZE];
    double value, max_value;


    items = icw_items->group_item[group_no].item;
    count = icw_items->group_item[group_no].count;
    if (group_no == icw_items->group_count - 1)         //最后一个
        end = 1;

    if (!group_no)                                      //第一个
        start = 1;

    for (i = 0; i < count; i++)
    {
        char c0[MAX_HZ_BUFFER_SIZE] = {0};
        memset(c0,0,sizeof(char)*MAX_HZ_BUFFER_SIZE);
#ifdef __linux__
        U16CopyInLinux(c0, (char*)items[i].hz, items[i].length);
#else
        memcpy(c0, (char*)items[i].hz, items[i].length * 2);
#endif
        Utf16ToAnsi((TCHAR*)c0, ci0, sizeof(ci0));

        next_group_no = group_no + items[i].length;
        if (next_group_no == icw_items->group_count)
        {

            items[i].value = GetBigramValue(bigram_data, ci0, triangleMark);
            items[i].next = 0;
            continue;
        }

        //在后续的组中找出最佳值
        next_items = icw_items->group_item[next_group_no].item;
        next_count = icw_items->group_item[next_group_no].count;
        max_value = -1.0;
        index = 0;
        for (j = 0; j < next_count; j++)
        {
            char c1[MAX_HZ_BUFFER_SIZE] = {0};
#ifdef __linux__
        U16CopyInLinux(c1, (char*)next_items[j].hz, next_items[j].length);
#else
        memcpy(c1, (char*)next_items[j].hz, next_items[j].length * 2);
#endif
            //strncpy_s(ci1, sizeof(ci1), (char*)next_items[j].hz, next_items[j].length * 2);
            Utf16ToAnsi((TCHAR*)c1, ci1, sizeof(ci1));

            value = GetBigramValue(bigram_data, ci0, ci1);
            value *= next_items[j].value;
            if (value > max_value)
            {
                index = j;
                max_value = value;
            }
        }
        items[i].value = max_value;
        items[i].next = &next_items[index];

        if (!group_no)              //开始位置，需要计算开始的结果
            items[i].value *= GetBigramValue(bigram_data, triangleMark, ci0);
    }
}

/*!
 *\brief  获得ICW候选，动态规划方法
 */
int NewGetIcwCandidates(SYLLABLE *syllable, int syllable_count, CANDIDATE *candidate, double *max_value)
{
    int i, index, part_syllable_count;
//  ICWITEMSET icw_items;
    ICWITEMSET *icw_items;                  //为了避免堆栈越界的错误，必须采用在堆中分配的方式 2008-03-06.

    NEWICWITEM  *icw_item;
    HZ *icw_hz;
    SYLLABLE *icw_syllable;

    //先赋初值，避免函数返回0时*max_value没有初始化
    *max_value = -1.0;

    if (!bigram_data || syllable_count < 2 || syllable_count > MAX_ICW_LENGTH)
        return 0;

    //检查音节是否为全音节，如果非全音节数目超过5，则不计算
    for (i = 0, part_syllable_count = 0; i < syllable_count; i++)
        if (syllable[i].vow == VOW_NULL)
            part_syllable_count++;

    if (part_syllable_count >= ICW_MAX_PART_SYLLABLES)
        return 0;

    //由于icw组词不需要超级模糊，所以要在pim_config中清除该位
    SaveCiOption();

    icw_items = malloc(sizeof(ICWITEMSET));
    memset(icw_item, sizeof(ICWITEMSET),0);
    memset(icw_items->group_item, sizeof(icw_items->group_item),0);
    if (!icw_items)
        return 0;

    //1. 产生每一个候选
    if (!GenerateICWItems(icw_items, syllable, syllable_count))
    {
        RestoreCiOption();
        free(icw_items);
        return 0;
    }

    RestoreCiOption();

    //2. 倒序估计每一个组的价值
    for (i = icw_items->group_count - 1; i >= 0; i--)
        NewEvaluateGroup(icw_items, i);

    //3. 查找最大值
    index = 0;
    for (i = 0; i < icw_items->group_item[0].count; i++)
    {
        if (icw_items->group_item[0].item[i].value > *max_value)
        {
            *max_value = icw_items->group_item[0].item[i].value;
            index = i;
        }
    }

    //4. 填充candidate数据
    icw_item = &icw_items->group_item[0].item[index];
    i = 0;
    icw_hz = candidate->icw.hz;
    icw_syllable = candidate->icw.syllable;
    while(i < icw_items->group_count && icw_item)
    {
        for (i = 0; i < icw_item->length; i++)
        {
            *icw_hz++ = icw_item->hz[i];
            *icw_syllable++ = icw_item->syllable[i];
        }

        icw_item = icw_item->next;
    }

    candidate->icw.length = icw_items->group_count;
    candidate->type = CAND_TYPE_ICW;

    free(icw_items);
    return 1;
}

/*!
 *\brief 输出计算的过程
 */
void OutputBigramProcess(int ci_count, char **ci)
{
    int i;
    double b_count;
    double value = 1.0;
    char *ci0, *ci1;

    ci0 = ci[0];
    b_count = GetBigramValue(bigram_data, triangleMark, ci0);
    printf("%s\t%s\t%6.8g\n", triangleMark, ci0, b_count);
    value *= b_count;

    for (i = 0; i < ci_count - 1; i++)
    {
        ci0 = ci[i];
        ci1 = ci[i + 1];

        b_count = GetBigramValue(bigram_data, ci0, ci1);
        printf("%s\t%s\t%6.8g\n", ci0, ci1, b_count);
        value *= b_count;
    }

    ci0 = ci[ci_count - 1];
    b_count = GetBigramValue(bigram_data, ci0, triangleMark);
    printf("%s\t%s\t%6.8g\n", ci0, triangleMark, b_count);
    value *= b_count;

    printf("value:%6.8g\n", value);
}
