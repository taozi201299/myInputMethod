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

/*  字相关函数。
 *  处理：
 *      1. 获得汉字候选
 *      2. 对候选进行排序
 *      3. 对候选汉字进行排重
 *
 *  对字频的处理：
 *      1. 字频信息分为原始字频与用户汉字使用度量两种
 *      2. 原始字频将不会再改变
 *      3. 用户字使用度用于标识用户输入过的字的次数
 *      4. 在比较字频的时候，首先进行用户字使用度的比较，
 *          如果不同则，使用度最高的优先
 *      5. 如果使用度相同，则进行原始字频的判断
 *      6. 如果采用最近汉字优先方式，则比较Cache中的使用标识号
 *
 */

#include <assert.h>
//#include <tchar.h>
#include "kernel.h"
#include "config.h"
#include "zi.h"
#include "ci.h"
#include "utility.h"
#include "editor.h"
#include "init_resource.h"

#define DEFAULT_TOP_POS     100

/*static*/ HZDATAHEADER *hz_data     = 0;
static TCHAR *hz_data_share_name = TEXT("HYPIM_HZ_DATA_SHARED_NAME");

//#pragma data_seg(HYPIM_SHARED_SEGMENT)
//
//HZCACHE       hz_cache = { ZICACHE_SIGNATURE, 1, {0}, {0}, }; //汉字Cache结构
//int           zi_cache_loaded   = 0;                          //是否已经装载了字Cache
//int           zi_cache_modified = 0;                          //是否改变
//
//TOPZIITEM topzi_table[MAX_SYLLABLES] =  { 0 };            //置顶字表
//int           topzi_table_items = 0;                          //置顶字表项数目
//int           topzi_loaded      = 0;                          //是否已经装入到内存
//
//static int  hz_data_loaded = 0;                               //汉字信息表是否已经装入
//#pragma   data_seg()


/** 判断汉字能否插入词库
 */
int IsAllCanInLibrary(HZ *hz, int length)
{
    int i;

    for (i = 0; i < length; i++)
        if (!_CanInLibrary(hz[2*i]))
            return 0;

    return 1;
}

#define IsGBHanzi(hz)

/** 判断是否为GB汉字
 */
int IsGB(HZ hz)
{
    return (((hz) % 0x100) >= 0xb0 && ((hz) % 0x100) <= 0xf7 && ((hz) / 0x100) >= 0xa1 && ((hz) / 0x100) <= 0xfe);
}

/*  获得汉字项在汉字集合中的序号。
 */
static int GetHzItemIndex(HZITEM *item)
{
    assert(item - hz_data->hz_item >= 0 && item - hz_data->hz_item < hz_data->hz_count);

    return (int)item->hz_id;
}

/*  基于汉字以及字频进行排序，用于汉字候选的排重。
 */
int CompareHzCodeFreq(const CANDIDATE *cand1, const CANDIDATE *cand2)
{
    assert(cand1 && cand2);

    //按内码排序
    if (cand1->hz.item->hz != cand2->hz.item->hz)
        return cand1->hz.item->hz - cand2->hz.item->hz;

    //按字频排序
    return cand2->hz.item->freq - cand1->hz.item->freq;
}

/*  基于小音节词的汉字进行排序，用于小音节词候选的排重。
 */
int CompareSmallCiCode(const CANDIDATE *cand1, const CANDIDATE *cand2)
{
    int ret;

    assert(cand1 && cand2);

    //先按长度排序
    if (cand1->hz.word_item->ci_length != cand2->hz.word_item->ci_length)
        return cand2->hz.word_item->ci_length - cand1->hz.word_item->ci_length;

    //再按内码排序
    ret = memcmp(
        GetItemHZPtr(cand2->hz.word_item),
        GetItemHZPtr(cand1->hz.word_item),
        sizeof(HZ) * cand1->hz.word_item->ci_length);

    return ret;
}

//去掉无法显示的汉字
int DeleteUnreadableZiCandidates(CANDIDATE *candidate_array, int count)
{
    int i, new_count = 0;

    for (i = 0; i < count; i++)
    {
        if ( pim_config->scope_gbk == HZ_SCOPE_UNICODE )
        {
            if ( pim_config->hide_black_window)
                continue;
        }
        if(candidate_array[i].hz.item->hz > 0xFFFF)
        {
            continue;
        }

        candidate_array[new_count] = candidate_array[i];
        new_count++;
    }

    return new_count;
}

/*!
 *  \brief 在候选数组中，排除掉重复的汉字，并且向前移动被排序的数组项
 *  为了加快速度，使用bitmap的方式进行查重。Unicode字符将不能用
 *  这种方法处理。
 *  \param   candidate_array         候选数组
 *  \param   count                   候选数目
 *  \return      排重后的候选数目
 */
int UnifyZiCandidates(CANDIDATE *candidate_array, int count)
{
    int i;
    int new_count;

    if (count < 2)          //至少两个才能进行比较
        return count;

    //基于汉字的内码以及字频进行排序，这样相同的汉字将
    //集中在一起存放，然后进行删除操作
    qsort(candidate_array, count, sizeof(CANDIDATE), CompareHzCodeFreq);

    new_count = 1;
    for (i = 1; i < count; i++)
    {
        if (candidate_array[i].hz.item->hz == candidate_array[i - 1].hz.item->hz)
            continue;

        candidate_array[new_count] = candidate_array[i];
        new_count++;
    }

    return new_count;
}

/*!
 * \brief 在候选数组中，排除掉重复的小音节词
 * \param candidate_array         候选数组
 * \param count                   候选数目
 * \return              排重后的候选数目
 */
int UnifySmallCiCandidates(CANDIDATE *candidate_array, int count)
{
    int i;
    int new_count;

    if (count < 2)          //至少两个才能进行比较
        return count;

    //基于小音节词的内码进行排序，这样具有相同的汉字词将
    //集中在一起存放，然后进行删除操作
    qsort(candidate_array, count, sizeof(CANDIDATE), CompareSmallCiCode);

    new_count = 1;
    for (i = 1; i < count; i++)
    {
        if (candidate_array[i].hz.word_item->ci_length == candidate_array[i - 1].hz.word_item->ci_length &&
            !memcmp(GetItemHZPtr(candidate_array[i].hz.word_item), GetItemHZPtr(candidate_array[i - 1].hz.word_item),
            sizeof(HZ) * candidate_array[i].hz.word_item->ci_length))
            continue;

        candidate_array[new_count] = candidate_array[i];
        new_count++;
    }

    return new_count;
}

/*!
 * \brief 调整词频数据到字频数据，用于进行排序的比较。
 *  字词都在同一个基准上进行。
 *
 *  词频计算方法：
 *      freq = freq_ori / 4;
 *      if (freq > 500000)
 *          freq = 500000 + freq / 2000;
 *      if (freq > (1 << 19) - 1)
 *          freq = (1 << 19) - 1;
 *
 *  字频计算方法：
 *      freq = freq_ori / 2;
 *      if (freq > 8000000)
 *          freq = 8000000 + freq / 2000;
 *      if (freq > (1 << 23) - 1)
 *          freq = (1 << 23) - 1;
 *  \param ci_freq   词频
 *  \return 调整的字频数据
 */
int AdjustWordFreqToHZFreq(int ci_freq)
{
    int zi_freq;

    //将词频转换到原始语料词频
    if (ci_freq > BASE_CI_FREQ)
        ci_freq = (ci_freq - BASE_CI_FREQ) * 2000;

    ci_freq *= 4;

    //将原始词频转换为字频
    zi_freq = ci_freq / 2;
    if (zi_freq > BASE_ZI_FREQ)
        zi_freq = BASE_ZI_FREQ + zi_freq / 2000;

    if (zi_freq > MAX_ZI_FREQ)
        zi_freq = MAX_ZI_FREQ;

    return zi_freq;
}

/*!
 * \brief 将汉字字频转换为原始字频
 * \param zi_freq       汉字字频
 * \return 汉字字频转换的原始字频
 */
int ConvertToRealHZFreq(int zi_freq)
{
    if (zi_freq > 8000000)
        return (zi_freq - 8000000) * 2000 * 2;

    return zi_freq * 2;
}

/*!
 * \brief 将词频转换为原始词频
 * \param  ci_freq   词频
 * \return 词频转换的原始词频
 */
int ConvertToRealCIFreq(int ci_freq)
{
    if (ci_freq > 500000)
        return (ci_freq - 500000) * 2000 * 4;

    return ci_freq * 4;
}

/*!
 * \brief 获得汉字的使用度
 * \param item 汉字项
 * \return 汉字的使用度
 */
int GetHzUsedCount(HZITEM *item)
{
    return default_resource->hz_cache.used_count[GetHzItemIndex(item)];
}

//对汉字进行排序，简单比较字频以及固顶字以及是否在cache中。
/*
static int CompareZi(const CANDIDATE *cand1, const CANDIDATE *cand2)
{
    int used_count1, used_count2;
    int cache_pos1, cache_pos2;
    int freq1, freq2;

    //返回值大于0时cand2在cand1前，返回值小于0时cand1在cand2前

    //排序方式：固顶字在最前(固顶字之间按ini文件里的顺序)；若非固顶字，
    //并设置了快速调整字频，若都是字，在Cache中序号大的(后进入Cache)在
    //前，若不都是字，使用度高的在前；若设置了慢速调整字频，使用度高的
    //在前；若没有设置字频调整，若都是字，字频高的在前，若不都是字，将
    //是词的候选项的词频换算为字频再比较，字频高的在前

    //特别注意CiCache和ZiCache结构区别很大，其cache_pos意义也不同，请
    //认真阅读其定义

    //xian->西安，xian->现
    //在字、词混合排序的时候，用“使用度”进行；在没有使用度的情况下，用
    //调整后的词频、字频进行判断
    if (cand1->hz.is_word)
    {
        freq1       = cand1->hz.word_item->freq;
        freq1       = AdjustWordFreqToHZFreq(freq1);
        cache_pos1  = cand1->hz.cache_pos;  //CiCache中的位置

        if (cand1->hz.cache_pos != -1)
            used_count1 = GetCiUsedCount(cache_pos1);
        else
            used_count1 = 0;
    }
    else
    {
        freq1       = cand1->hz.item->freq;
        cache_pos1  = GetZiCachePos(cand1->hz.item);  //ZiCache中的序号(这个pos不代表位置，详见HZCACHE中的注释)
        used_count1 = GetHzUsedCount(cand1->hz.item);

        //当前是全集模式 and 候选是繁体字不是简体字，减小字频
        if (pim_config->hz_output_mode == HZ_OUTPUT_HANZI_ALL &&
            cand1->hz.item->traditional && !cand1->hz.item->simplified)
            freq1 = freq1 >> 10;
    }

    if (cand2->hz.is_word)
    {
        freq2       = cand2->hz.word_item->freq;
        freq2       = AdjustWordFreqToHZFreq(freq2);
        cache_pos2  = cand2->hz.cache_pos;

        if (cand2->hz.cache_pos != -1)
            used_count2 = GetCiUsedCount(cache_pos2);
        else
            used_count2 = 0;
    }
    else
    {
        freq2       = cand2->hz.item->freq;
        cache_pos2  = GetZiCachePos(cand2->hz.item);
        used_count2 = GetHzUsedCount(cand2->hz.item);

        //当前是全集模式 and 候选是繁体字不是简体字，减小字频
        if (pim_config->hz_output_mode == HZ_OUTPUT_HANZI_ALL &&
            cand2->hz.item->traditional && !cand2->hz.item->simplified)
            freq2 = freq2 > 10;
    }

    //判断固顶字。注意在输入繁体的时候失效！！！
    if (!(pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL) && (pim_config->hz_option & HZ_USE_FIX_TOP))
    {
        //词与字在这个问题上判断相同
        if (cand1->hz.top_pos != cand2->hz.top_pos)
            return cand1->hz.top_pos - cand2->hz.top_pos;
    }

    //快速调整字频
    if (pim_config->hz_option & HZ_ADJUST_FREQ_FAST)
    {
        if (!cand1->hz.is_word && !cand2->hz.is_word)
        {
            if (cache_pos1 != cache_pos2)
                return cache_pos2 - cache_pos1;
        }
        else
        {
            if (used_count1 != used_count2)
                return used_count2 - used_count1;
        }
    }

    //慢速调整字频
    if (pim_config->hz_option & HZ_ADJUST_FREQ_SLOW)
    {
        if (used_count1 != used_count2)
            return used_count2 - used_count1;
    }

    //比较字频
    if (pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL)
        if (!cand1->hz.is_word && !cand2->hz.is_word)
            return (cand2->hz.item->freq - cand1->hz.item->freq);

    return freq2 - freq1;
}
*/

/*!
  * \brief 对汉字字频进行比较
  * \param cand1 候选项
  * \param cand2 候选项
  * \return
  *  返回值大于0时cand2在cand1前，返回值小于0时cand1在cand2前
  * */
static int CompareZi(const CANDIDATE *cand1, const CANDIDATE *cand2)
{
    int used_count1, used_count2;
    int freq1, freq2;

    if (cand1->hz.is_word)
    {
        freq1       = cand1->hz.word_item->freq;
        freq1       = AdjustWordFreqToHZFreq(freq1);

        if (cand1->hz.cache_pos != -1)
            used_count1 = GetCiUsedCount(cand1->hz.cache_pos);
        else
            used_count1 = 0;
    }
    else
    {
        freq1       = cand1->hz.item->freq;
        used_count1 = GetHzUsedCount(cand1->hz.item);

        //当前是全集模式 and 候选是繁体字不是简体字，减小字频
        if (pim_config->hz_output_mode == HZ_OUTPUT_HANZI_ALL &&
            cand1->hz.item->traditional && !cand1->hz.item->simplified)
            freq1 = freq1 >> 10;
    }

    if (cand2->hz.is_word)
    {
        freq2       = cand2->hz.word_item->freq;
        freq2       = AdjustWordFreqToHZFreq(freq2);

        if (cand2->hz.cache_pos != -1)
            used_count2 = GetCiUsedCount(cand2->hz.cache_pos);
        else
            used_count2 = 0;
    }
    else
    {
        freq2       = cand2->hz.item->freq;
        used_count2 = GetHzUsedCount(cand2->hz.item);

        //当前是全集模式且候选是繁体字不是简体字，减小字频
        if (pim_config->hz_output_mode == HZ_OUTPUT_HANZI_ALL &&
            cand2->hz.item->traditional && !cand2->hz.item->simplified)
            freq2 = freq2 > 10;
    }

    //判断固顶字。注意在输入繁体的时候失效！！！
    if (!(pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL) && (pim_config->hz_option & HZ_USE_FIX_TOP))
    {
        //词与字在这个问题上判断相同
        if (cand1->hz.top_pos != cand2->hz.top_pos)
            return cand1->hz.top_pos - cand2->hz.top_pos;
    }

    //快速或慢速调整字频
    if (pim_config->hz_option & HZ_ADJUST_FREQ_FAST || pim_config->hz_option & HZ_ADJUST_FREQ_SLOW)
    {
        if (used_count1 != used_count2)
            return used_count2 - used_count1;
    }

    //比较字频
    if (pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL)
        if (!cand1->hz.is_word && !cand2->hz.is_word)
            return (cand2->hz.item->freq - cand1->hz.item->freq);

    return freq2 - freq1;
}

/*!
 * \brief 快速字频调整下对候选进行排序
 * \param candidate_array  候选数组
 * \param count   候选个数
 */
static void SortZiCandidatesInternal(CANDIDATE *candidate_array, int count)
{
    int i, j, k;
    CANDIDATE temp;

    //繁体字不考虑固定字问题
    if (!(pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL) && (pim_config->hz_option & HZ_USE_FIX_TOP))
    {
        for (i = 0; i < count; i++)
        {
            if (candidate_array[i].hz.top_pos == DEFAULT_TOP_POS)
            {
                break;
            }
        }

        //固定字不参与下面的排序
        candidate_array += i;
        count -= i;
    }

    for (i = 0; i < count; i++)
    {
        if (candidate_array[i].hz.is_word)
        {
            continue;
        }

        k = i;
        for (j = i + 1; j < count; j++)
        {
            if (candidate_array[j].hz.is_word)
            {
                continue;
            }

            //注意hz.cache_pos里只存小音节词的cache位置
            if (GetZiCachePos(candidate_array[j].hz.item) > GetZiCachePos(candidate_array[k].hz.item))
            {
                k = j;
            }
        }

        if (k != i)
        {
            //移动而非交换(保证选择排序的无序区里是按使用度有序的)
            temp = candidate_array[k];
            for (j = k - i - 1; j >= 0; j--)
            {
                memcpy(candidate_array + i + j + 1, candidate_array + i + j, sizeof(CANDIDATE));
            }
            candidate_array[i] = temp;
        }
    }
}

/*!
 * \brief 对汉字候选进行排序
 * \param candidate_array  候选数组
 * \param count   候选个数
 */

void SortZiCandidates(CANDIDATE *candidate_array, int count)
{
    if (count < 2)          //至少两个才能进行比较
        return;

    qsort(candidate_array, count, sizeof(CANDIDATE), CompareZi);

    //若是快速字频调整，还要进一步处理，详见bugsys A00000022
    if (pim_config->hz_option & HZ_ADJUST_FREQ_FAST)
    {
        SortZiCandidatesInternal(candidate_array, count);
    }
}

/*!
 * \brief   判断汉字是否包含特定的音调。
 * \param     hz              汉字
 * \param     syllable        汉字音节
 * \param     tone            被判断的音调
 * \return    候选汉字数目。
 */
int ZiContainTone(HZ hz, SYLLABLE syllable, int tone)
{
    int start = 0;
    int end = hz_data->hz_count - 1;
    int mid;

    if (tone == TONE_0)             //没有指定音调
        return 1;

    //用二分法查找到这个汉字
    while(start <= end)
    {
        mid = (start + end) / 2;

        //声母有区别
        if (hz_data->hz_item[mid].syllable.con > syllable.con)
        {
            end = mid - 1;
            continue;
        }
        else if (hz_data->hz_item[mid].syllable.con < syllable.con)
        {
            start = mid + 1;
            continue;
        }

        //韵母有区别
        if (hz_data->hz_item[mid].syllable.vow > syllable.vow)
        {
            end = mid - 1;
            continue;
        }
        else if (hz_data->hz_item[mid].syllable.vow < syllable.vow)
        {
            start = mid + 1;
            continue;
        }

        //字有区别
        if (hz_data->hz_item[mid].hz > hz)
        {
            end = mid - 1;
            continue;
        }

        if (hz_data->hz_item[mid].hz < hz)
        {
            start = mid + 1;
            continue;
        }

        //找到！
        break;
    }

    if (start > end)        //没有找到
        return 0;

    return (hz_data->hz_item[mid].syllable.tone & tone) != 0;
}

/*!
 * \brief  获得汉字候选。
 * \param      syllable        音节
 * \param      candidate_array 候选数组
 * \param      array_length    候选数组长度
 * \param      fuzzy_mode      模糊设置
 * \param      set_mode        汉字集合选项：常用（进化）、全集
 * \param      output_mode     字输出选项：繁体、简体
 * 注：如果为繁体，则肯定是全集汉字集合。
 * \return       候选汉字数目。
 */
int GetZiCandidates(SYLLABLE syllable, CANDIDATE *candidate_array, int array_length, int fuzzy_mode, int set_mode, int output_mode)
{
    int i, j, count;        //候选计数器
    int topzi_count = 0;        //置顶字数目
    HZ  top_zi[MAX_TOPZI];  //置顶字
    int check_top_zcs_fuzzy = 0;    //置顶字zcs模糊
    extern int LoadHZDataResource();

    if (!array_length)
        return 0;

    if (!default_resource->hz_data_loaded)
        LoadHZDataResource();

    if (!hz_data)
    {
       // hz_data = GetReadOnlySharedMemory(hz_data_share_name);

        //可能存在其他进程已经装载了，但是退出后共享内存被释放的问题
        if (!hz_data && default_resource->hz_data_loaded)
        {
            default_resource->hz_data_loaded = 0;
            LoadHZDataResource();
        }
    }

    if (!hz_data)
        return 0;

    count = 0;

    //检索汉字
     for (i = 0; i < hz_data->hz_count; i++)
    {
        //判断简繁体是否符合
        if (!(
            ((output_mode & HZ_OUTPUT_HANZI_ALL)) ||                                //输出全集汉字
            ((output_mode & HZ_OUTPUT_ICW_ZI) && hz_data->hz_item[i].icw_hz) ||         //输出ICW使用的汉字
            ((output_mode & HZ_OUTPUT_SIMPLIFIED) && hz_data->hz_item[i].simplified) || //输出简体字，字是简体
            ((output_mode & HZ_OUTPUT_TRADITIONAL) && (hz_data->hz_item[i].traditional || hz_data->hz_item[i].other))   //输出繁体字，字是繁体或该字未分类
            ))
            continue;

        //jieping notes: 置顶字已取消，不必判断
//        if (check_top_zcs_fuzzy)
//        {
//            if (!ContainSyllableWithTone(syllable, hz_data->hz_item[i].syllable, fuzzy_mode | FUZZY_Z_ZH | FUZZY_C_CH | FUZZY_S_SH))
//                continue;

//            //判断是否为置顶字
//            for (j = 0; j < topzi_count; j++)
//                if (top_zi[j] == LOWORD(hz_data->hz_item[i].hz))
//                    break;

//            //不是置顶字的话，需要重新进行音调的判断
//            if (j == topzi_count)
//                if (!ContainSyllableWithTone(syllable, hz_data->hz_item[i].syllable, fuzzy_mode))
//                    continue;           //拼音不相符
//        }
        /*else*/ if (!ContainSyllableWithTone(syllable, hz_data->hz_item[i].syllable, fuzzy_mode))
            continue;           //拼音不相符
        else if (syllable.vow == 0 && hz_data->hz_item[i].freq < 0x8000)
            continue;
        else if ((output_mode & HZ_OUTPUT_ICW_ZI) && hz_data->hz_item[i].freq < 0x8000)
            continue;

        candidate_array[count].type       = CAND_TYPE_ZI;
        candidate_array[count].hz.is_word = 0;
        candidate_array[count].hz.item    = &hz_data->hz_item[i];
        candidate_array[count].hz.hz_type = ZI_TYPE_NORMAL;

        //jieping notes: 置顶字已取消，不必判断
        //判断是否为置顶字
//        for (j = 0; j < topzi_count; j++)
//            if (top_zi[j] == LOWORD(hz_data->hz_item[i].hz))
//                break;

        //设置置顶字序号，如果没有，则设为最大值
//        if (j == topzi_count)
              j = DEFAULT_TOP_POS;

        candidate_array[count].hz.top_pos = j;

        count++;
        if (count >= array_length)          //数组过长
            break;
    }

    return count;
}

/*!
 * \brief 该接口暂不使用
 */
HZITEM* GetSingleZiCandidate(TCHAR zi)
{
    int i;

    for (i = 0; i < hz_data->hz_count; i++)
    {
        if (zi == hz_data->hz_item[i].hz)
            return &hz_data->hz_item[i];
    }

    return 0;
}

/*!
 *  \brief 完整处理音节到汉字候选。
 *  处理过程：
 *      1. 检索汉字候选
 *      2. 汉字候选排重
 *      3. 候选排序
 *  本函数将依照系统配置与IC配置中的当前模糊音、检索结果集以及输出集合
 *  设置进行处理。
 *
 *  \param       syllable            音节
 *  \param       *candidate_array    候选数组
 *  \param       array_length        候选数组长度
 *  \return      返回候选数目
 */
int ProcessZiCandidates(SYLLABLE syllable, CANDIDATE *candidate_array, int array_length, int zi_level)
{
    int zi_count = 0, normal_zi_count = 0, small_count = 0, small_ci_count = 0, small_zi_count = 0;
    SYLLABLE small_syllables[2];
    CANDIDATE small_word_candidates[0x10];

    //寻找类似xian输入西安的候选，双拼不用考虑
    if (GetSmallSyllables(syllable, small_syllables))
    {
        int i;

        small_ci_count = ProcessCiCandidate(small_syllables,
                                         2,
                                         0,
                                         small_word_candidates,
                                         _SizeOf(small_word_candidates),
                                         1);

        for (i = 0; i < small_ci_count && i < array_length; i++)
        {
            candidate_array[i].type = CAND_TYPE_ZI;
            candidate_array[i].hz.is_word = 1;
            candidate_array[i].hz.hz = small_word_candidates[i].word.hz;
            candidate_array[i].hz.word_item = small_word_candidates[i].word.item;
            candidate_array[i].hz.origin_syllable = syllable;
            candidate_array[i].hz.top_pos = DEFAULT_TOP_POS;
            candidate_array[i].hz.cache_pos =
                GetCiCacheInfo(small_word_candidates[i].word.hz, candidate_array[i].hz.word_item->ci_length);
        }

        //检索小音节拆分字
        small_zi_count = GetZiCandidates(small_syllables[0],
            candidate_array + small_ci_count,
            array_length - small_ci_count,
            pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0,
            zi_level,
            pim_config->hz_output_mode);

        //如果没有找到汉字，如eng，则必须扩大汉字的范围进行查找
        if (!small_zi_count)
            small_zi_count = GetZiCandidates(small_syllables[0],
            candidate_array + small_ci_count,
            array_length - small_ci_count,
            pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0,
            HZ_ALL_USED,
            HZ_OUTPUT_HANZI_ALL);

        for (i = small_ci_count; i < small_ci_count + small_zi_count; i++)
        {
            candidate_array[i].hz.hz_type = ZI_TYPE_OTHER;
        }

        small_count = small_ci_count + small_zi_count;
    }

    //检索普通字
    normal_zi_count = GetZiCandidates(syllable,
                            candidate_array + small_count,
                            array_length - small_count,
                            pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0,
                            zi_level,
                            pim_config->hz_output_mode);

    //如果没有找到汉字，如eng，则必须扩大汉字的范围进行查找
    if (!normal_zi_count)
        normal_zi_count = GetZiCandidates(syllable,
                                candidate_array + small_count,
                                array_length - small_count,
                                pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0,
                                HZ_ALL_USED,
                                HZ_OUTPUT_HANZI_ALL);

    zi_count = small_zi_count + normal_zi_count;

    //去掉无法显示的汉字
    zi_count = DeleteUnreadableZiCandidates(candidate_array + small_ci_count, zi_count);

    //排重(只针对字)
    zi_count = UnifyZiCandidates(candidate_array + small_ci_count, zi_count);

    //排序(字和小音节词都要排)
    SortZiCandidates(candidate_array, small_ci_count + zi_count);

    return small_ci_count + zi_count;
}

/*!
 * \brief 向Cache中增加汉字项。
 * \param item            汉字项
 */
static void AddHzToCache(HZITEM *item)
{
    int i;

    //如果字为固定方式，则不需要重新写入
    if (pim_config->hz_option & HZ_ADJUST_FREQ_NONE)
        return;

    default_resource->hz_cache.cache[GetHzItemIndex(item)] = default_resource->hz_cache.max_id;   //将本汉字赋予最新标志
    default_resource->hz_cache.max_id++;                                                       //累计当前最大的id

    //达到最大值后，将所有的值都减去最大值即可
    if (default_resource->hz_cache.max_id >= HZ_CACHE_MAX_ID)
    {
        for (i = 0; i < _SizeOf(default_resource->hz_cache.cache); i++)
            default_resource->hz_cache.cache[i] -= HZ_CACHE_MAX_ID;

        default_resource->hz_cache.max_id -= HZ_CACHE_MAX_ID;
    }

    default_resource->zi_cache_modified = 1;
}

/*!
 * \brief 获得汉字项在Cache中的ID。
 * \param  item            汉字项指针
 * \return 在Cache中的使用ID（越大越近使用）
 */
/*static*/ int GetZiCachePos(HZITEM *item)
{
    return default_resource->hz_cache.cache[GetHzItemIndex(item)];
}

/*!
 *  \brief 调整字的使用度
 *  \param  item 汉字项
 */

static void AddHzToUsedCount(HZITEM *item)
{
    //如果字为固定方式，则不需要重新写入
    if (pim_config->hz_option & HZ_ADJUST_FREQ_NONE)
        return;

    default_resource->hz_cache.used_count[GetHzItemIndex(item)]++;
    default_resource->zi_cache_modified = 1;
}

/*!
 * \brief 处理汉字被选中后的更新事务。
 *  调整字频，更新Cache。
 *  \param item        汉字项
 */
void ProcessZiSelected(HZITEM *item)
{
    //更新汉字使用度信息
    AddHzToUsedCount(item);

    //更新汉字Cache信息
    AddHzToCache(item);
}

/*!
 * \brief 处理以词定字的字的更新
 *  找到与该字对应的HZITEM（最大字频），进行处理
 * \todo 该接口暂不使用，删除
 */
void ProcessZiSelectedByWord(HZ hz, SYLLABLE syllable)
{
    int i, index = -1;
    int max_freq = -1;

    //todo:可能需要对音调进行处理

    //检索汉字
    for (i = 0; i < hz_data->hz_count; i++)
    {
        if (hz_data->hz_item[i].hz != hz)
            continue;

        if (!ContainSyllableWithTone(syllable, hz_data->hz_item[i].syllable, 0))
            continue;           //拼音不相符

        if ((int)hz_data->hz_item[i].freq > max_freq)
        {
            index = i;
            max_freq = hz_data->hz_item[i].freq;
        }
    }

    if (index != -1)        //找到
        ProcessZiSelected(&hz_data->hz_item[index]);
}

/*!
 * \brief 装载字Cache数据文件
 * \param zi_cache_name 字Cache数据文件name
 * \return 成功：1，失败：0
 */
int LoadZiCacheData(const TCHAR *zi_cache_name)
{
    if (default_resource->zi_cache_loaded)
        return 1;

    if (LoadFromFile(zi_cache_name, (char*)&default_resource->hz_cache, sizeof(default_resource->hz_cache)) > 0)
        default_resource->zi_cache_loaded = 1;
    else
        default_resource->zi_cache_loaded = 0;

    return default_resource->zi_cache_loaded;
}

/*!
 * \brief 保存汉字Cache数据
 * \param zi_cache_name 字Cache数据文件name
 * \return 成功：1，失败：0
 */
int SaveZiCacheData(const TCHAR *zi_cache_name)
{
    if (default_resource->zi_cache_modified)
    {
        default_resource->zi_cache_modified = 0;
        return SaveToFile(zi_cache_name, (char*)&default_resource->hz_cache, sizeof(default_resource->hz_cache)) != -1;
    }
    return 1;
}

/*!
 * \brief 释放ZiCacheData
 */
int FreeZiCacheData(const TCHAR *zi_cache_name)
{
    SaveZiCacheData(zi_cache_name);
    default_resource->zi_cache_loaded = 0;

    return 1;
}

/*!
 * \brief 装载字数据文件
 * \param hz_data_name 字数据文件name
 */
int LoadHZData(const TCHAR *hz_data_name)
{
    int file_length;

    assert(hz_data_name);

    if (default_resource->hz_data_loaded)
        return 1;

    file_length = GetFileLength(hz_data_name);
    if (file_length <= 0)
        return 0;
    hz_data = (HZDATAHEADER *)malloc(file_length);
    if(!hz_data)
        return 0 ;

    if ((LoadFromFile(hz_data_name, hz_data, file_length)) == -1)
    {
        free(hz_data);
        hz_data = NULL;
        Log(LOG_ID, L"汉字信息表文件打开失败。name=%s", hz_data_name);
        return 0;
    }

    default_resource->hz_data_loaded = 1;

    return 1;
}

/*!
 * \brief 释放汉字信息表文件
 */
int FreeHZData()
{
    default_resource->hz_data_loaded = 0;

    if (hz_data)
    {
      //  FreeSharedMemory(hz_data_share_name, hz_data);
        free(hz_data);
        hz_data = 0;
    }

    return 1;
}

/*!
 * \brief 获得汉字笔划的拼音串，用于显示笔划候选
 * \todo  笔画输入法接口 暂不使用，后续整理
 */
void GetZiBHPinyin(UC zi, TCHAR *buffer, int length)
{
    int  i, first;
    TCHAR s_str[0x30];
    int  s_len;
    SYLLABLE s;

    if (!buffer || !length || length < sizeof(HZ) + 1)
        return;

    if (!default_resource->hz_data_loaded)
        LoadHZDataResource();

    if (!hz_data)
    {
        hz_data = GetReadOnlySharedMemory(hz_data_share_name);

        //可能存在其他进程已经装载了，但是退出后共享内存被释放的问题
        if (!hz_data && default_resource->hz_data_loaded)
        {
            default_resource->hz_data_loaded = 0;
            LoadHZDataResource();
        }
    }

    if (!hz_data)
    {
        buffer[0] = 0;
        return;
    }

    UCS32ToUCS16(zi, buffer);

    first = 1;
    for (i = 0; i < hz_data->hz_count; i++)
    {
        if (hz_data->hz_item[i].hz != zi)
            continue;

        s      = hz_data->hz_item[i].syllable;
        s_len  = GetSyllableString(s, s_str, _SizeOf(s_str),/* 0,*/ 1);

        if (s_len + (int)_tcslen(buffer) + 2 >= length - 1)
            return;

        if (first)
            _tcscat_s(buffer, length, TEXT("("));
        else
            _tcscat_s(buffer, length, TEXT(","));

        _tcscat_s(buffer, length, s_str);

        first = 0;
    }

    if (!first)
        _tcscat_s(buffer, length, TEXT(")"));
}

