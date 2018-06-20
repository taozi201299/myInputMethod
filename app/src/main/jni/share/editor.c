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

/*
 *  用户输入：wo3'men2da'jia
 *  解析：
 *      音节         wo3     men2    da      jia
 *      有无'        y       n       y       n
 *      有无调       y       y       n       n
 *      汉字         -       -       -       -
 *      字符串       wo3     men2    da'     jia
 *      字符串长度   3       4       3       3
 *  Real光标：13；显示光标：13。
 *
 *  当用户选择了“我们”这个词的时候
 *      音节         wo3     men2    da      jia
 *      有无'        y       n       y       n
 *      有无调       y       y       n       n
 *      汉字         我      们      -       -
 *      字符串       我      们      da'     jia
 *      字符串长度   2       2       3       3
 *  当前光标：13；显示光标：11。
 *
 *
 */
#include "kernel.h"
#include "config.h"
#include "zi.h"
#include "ci.h"
#include "editor.h"
#include "context.h"
#include "utility.h"
#include "linux.h"
#include <assert.h>
#include <android/log.h>

extern PIMCONFIG *pim_config;

static BOOL ProcessPinyinParsing(PIMCONTEXT *context);
static int GenUnselectedPinyinStr(PIMCONTEXT *context, TCHAR *compose_string);
static BOOL IsMatchedSyllable(TCHAR *pinyin, TCHAR *search_str, BOOL *new_syllable);
static void CheckSyllableStringIUV(PIMCONTEXT *context, int index, TCHAR *pinyin, BOOL support_correct);

#define  LOG_TAG "Log"

/*!
 * \brief 获得候选的音节
*/
int GetCandidateSyllable(CANDIDATE *candidate, SYLLABLE *syllables, int length)
{
    assert(length >= 1);

    switch(candidate->type)
    {
    case CAND_TYPE_ZI:
        if (length < 1)
            return 0;

        //判断是否为类似xian的词
        if (!candidate->hz.is_word)
        {
            syllables[0] = candidate->hz.item->syllable;
            return 1;
        }

        if (length < (int)candidate->hz.word_item->syllable_length)
            return 0;

        memcpy(syllables, candidate->hz.word_item->syllable, sizeof(SYLLABLE) * candidate->hz.word_item->syllable_length);
        return candidate->hz.word_item->syllable_length;

    case CAND_TYPE_ICW:
        if (length < candidate->icw.length)
            return 0;

        memcpy(syllables, candidate->icw.syllable, sizeof(SYLLABLE) * candidate->icw.length);
        return candidate->icw.length;

    case CAND_TYPE_CI:
        if (length < (int)candidate->word.item->syllable_length)
            return 0;

        memcpy(syllables, candidate->word.syllable, sizeof(SYLLABLE) * candidate->word.item->syllable_length);
        return candidate->word.item->syllable_length;

    case CAND_TYPE_ZFW:
        if (length < 1)
            return 0;

        syllables[0] = candidate->zfw.syllable;
        return 1;
    }

    return 0;
}

/*!
 * \brief 获得候选的音节长度
 */
int GetCandidateSyllableCount(CANDIDATE *candidate)
{
    SYLLABLE syllable[0x40];

    return GetCandidateSyllable(candidate, syllable, sizeof(syllable)/sizeof(syllable[0]));
}

/*!
 *\brief 获得已经选择项的音节数组
 */
int GetSelectedItemSyllable(SELECT_ITEM *item, SYLLABLE *syllables, int length)
{
    if (length < 1)
        return 0;

    if (item->left_or_right == ZFW_NONE)
        return GetCandidateSyllable(&item->candidate, syllables, length);

    if (item->candidate.type != CAND_TYPE_CI)
        return 0;

    if (item->left_or_right == ZFW_LEFT)
    {
        if (item->candidate.type == CAND_TYPE_CI)
        {
            syllables[0] = item->candidate.word.syllable[0];
            return 1;
        }

        if (item->candidate.type == CAND_TYPE_ICW)
        {
            syllables[0] = item->candidate.icw.syllable[0];
            return 1;
        }

        return 0;
    }

    if (item->left_or_right == ZFW_RIGHT)
    {
        if (item->candidate.type == CAND_TYPE_CI)
        {
            syllables[0] = item->candidate.word.syllable[item->candidate.word.item->syllable_length - 1];
            return 1;
        }

        if (item->candidate.type == CAND_TYPE_ICW)
        {
            syllables[0] = item->candidate.icw.syllable[item->candidate.icw.length - 1];
            return 1;
        }

        return 0;
    }

    return 0;
}

/*!
* \brief 获得候选的汉字
* \return  候选字符串的长度（以字节为单位）
*/
int GetCandidateString(PIMCONTEXT *context, CANDIDATE *candidate, TCHAR *buffer, int length)
{
    assert(length >= 1);

    wmemset(buffer, 0, length);

    switch(candidate->type)
    {
    case CAND_TYPE_ZI:
        if (candidate->hz.is_word)
        {
            if (length < (int)candidate->hz.word_item->ci_length)
                return 0;

            int count = candidate->hz.word_item->ci_length;
            int i = 0;
            for( i = 0 ;i <count; i++){
                memcpy(buffer + i,candidate->hz.hz + i, sizeof(HZ));
            }
          //  memcpy(buffer, candidate->hz.hz, candidate->hz.word_item->ci_length * sizeof(HZ));
            return candidate->hz.word_item->ci_length;
        }

        if (length < 1)
            return 0;

        if (candidate->hz.item->hz > 0xFFFF)
        {
            UCS32ToUCS16(candidate->hz.item->hz, buffer);
            return 2;
        }
        else
        {
            *(HZ*)buffer = candidate->hz.item->hz;
            return 1;
        }

    case CAND_TYPE_ICW:
        if (length < (int)(candidate->icw.length))
            return 0;
            int count = candidate->icw.length;
            int i = 0;
            for( i = 0; i < count ;i ++){
                memcpy(buffer + i,candidate->icw.hz +i, sizeof(HZ));
            }
       // memcpy(buffer, candidate->icw.hz, sizeof(HZ) * candidate->icw.length);
        buffer[candidate->icw.length] = 0;

        return candidate->icw.length;

    case CAND_TYPE_CI: {
        if (length < (int) (candidate->word.item->ci_length))
            return 0;
        int i  = 0;
        for ( i = 0; i < candidate->word.item->ci_length; i++) {
            memcpy(buffer + i, candidate->word.hz + i, sizeof(HZ));
        }
//        memcpy(buffer, candidate->word.hz, sizeof(HZ));
//        memcpy(buffer + 1, candidate->word.hz + 1, sizeof(HZ));
//        memcpy(buffer, candidate->word.hz, sizeof(HZ) * candidate->word.item->ci_length);
        buffer[candidate->word.item->ci_length] = 0;

        return candidate->word.item->ci_length;
    }

    case CAND_TYPE_ZFW:
        if (length < 1)
            return 0;

        *(TCHAR*)buffer = candidate->zfw.hz;
        return 1;
    }

    return 0;
}

/*!
* \brief 获得候选的汉字的显示字符串（过长的要缩短）
* \return 候选字符串的长度（以字节为单位）
*/
int GetCandidateDisplayString(PIMCONTEXT *context, CANDIDATE *candidate, TCHAR *buffer, int length, int first_candidate)
{
    int   str_length;
    TCHAR str_buffer[MAX_SPW_LENGTH + 2];

    str_length = GetCandidateString(context, candidate, str_buffer, MAX_SPW_LENGTH + 2);

    if (first_candidate && !context->selected_item_count &&
            context->compose_cursor_index && context->compose_cursor_index < context->compose_length)
    {
        TCHAR str[MAX_SPW_LENGTH + 2];
        int syllable_index = GetSyllableIndexByComposeCursor(context, context->compose_cursor_index);

        _tcsncpy_s(str, _SizeOf(str), context->default_hz, syllable_index);
        _tcscat_s(str, _SizeOf(str), str_buffer);
        _tcscpy_s(str_buffer, _SizeOf(str_buffer), str);
    }
    int buf_len = (int)_tcslen(str_buffer);
    _tcscpy_s(buffer,buf_len+1,str_buffer);
    return 0 ;

    //return PackStringToBuffer(str_buffer, str_length, buffer, length);
}

/*!
 * \brief 获得当前已经选择的项的字符串
 */
int GetSelectedItemString(PIMCONTEXT *context, SELECT_ITEM *item, TCHAR *buffer, int length)
{
    TCHAR ft_ci[0x100];
    memset(ft_ci,0, sizeof(ft_ci));

    if (length <= 1)
        return 0;

    //特殊处理以词定字
    if (item->candidate.type == CAND_TYPE_CI)
    {
        if (item->left_or_right == ZFW_LEFT)
        {
            if (!(pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL))
            {
                *(HZ*)buffer = item->candidate.word.hz[0];
                *(buffer + 1) = 0;
            }
            else
            {
                memcpy(ft_ci, item->candidate.word.hz, sizeof(HZ) * item->candidate.word.item->ci_length);

//                for(int i = 0; i < item->candidate.word.item->ci_length; i++){
//                    memcpy(ft_ci[i],item->candidate.word.hz[i], sizeof(HZ));
//                }
                ft_ci[item->candidate.word.item->ci_length] = 0;
                *(HZ*)buffer = *(HZ*)ft_ci;
                *(buffer + 1) = 0;
            }

            return 1;
        }

        if (item->left_or_right == ZFW_RIGHT)
        {
            if (!(pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL))
            {
                *(HZ*)buffer = item->candidate.word.hz[item->candidate.word.item->ci_length - 1];
                *(buffer + 1) = 0;
            }
            else
            {
                memcpy(ft_ci, item->candidate.word.hz, sizeof(HZ) * item->candidate.word.item->ci_length);
//                for(int i = 0; i < item->candidate.word.item->ci_length; i++){
//                    memcpy(ft_ci[i],item->candidate.word.hz[i], sizeof(HZ));
//                }
                ft_ci[item->candidate.word.item->ci_length] = 0;
                *(HZ*)buffer = *((HZ*)ft_ci + item->candidate.word.item->ci_length - 1);
                *(buffer + 1) = 0;
            }

            return 1;
        }
    }

    //特殊处理以词定字
    if (item->candidate.type == CAND_TYPE_ICW)
    {
        if (item->left_or_right == ZFW_LEFT)
        {
            if (!(pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL))
            {
                *(HZ*)buffer = item->candidate.icw.hz[0];
                *(buffer + 1) = 0;
            }
            else
            {
                memcpy(ft_ci, item->candidate.icw.hz, sizeof(HZ) * item->candidate.icw.length);
                ft_ci[item->candidate.icw.length] = 0;
                *(HZ*)buffer = *((HZ*)ft_ci);
                *(buffer + 1) = 0;
            }

            return 1;
        }

        if (item->left_or_right == ZFW_RIGHT)
        {
            if (!(pim_config->hz_output_mode & HZ_OUTPUT_TRADITIONAL))
            {
                *(HZ*)buffer = item->candidate.icw.hz[item->candidate.icw.length - 1];
                *(buffer + 1) = 0;
            }
            else
            {
                memcpy(ft_ci, item->candidate.icw.hz, sizeof(HZ) * item->candidate.icw.length);
                ft_ci[item->candidate.icw.length] = 0;
                *(HZ*)buffer = *((HZ*)ft_ci + item->candidate.icw.length - 1);
                *(buffer + 1) = 0;
            }

            return 1;
        }
    }

    return GetCandidateString(context, &item->candidate, buffer, length);
}

/*!
 * \brief 用户选择后续处理
 */
void PostResult(PIMCONTEXT *context)
{
    if (!context || !context->result_syllable_count)
        return;

    //是不是一次选择
    if (context->selected_item_count == 1)      //只有一个
    {
        //ICW
        if (context->selected_items[0].candidate.type == CAND_TYPE_ICW)
        {
            return;
        }

        //单字
        if (context->selected_items[0].candidate.type == CAND_TYPE_ZI)
        {
            //输入类似xian的词汇
            if (context->selected_items[0].candidate.hz.is_word)
                InsertCiToCache((HZ*)context->result_string,
                                 context->selected_items[0].candidate.hz.word_item->ci_length,
                                 context->selected_items[0].candidate.hz.word_item->syllable_length,
                                 0);
            else
                ProcessZiSelected(context->selected_items[0].candidate.hz.item);
        }

        //词
        if (context->selected_items[0].candidate.type == CAND_TYPE_CI)
        {
            //非以词定字
            if (context->selected_items[0].left_or_right == ZFW_NONE)
            {
                ProcessCiSelected(context->result_syllables,
                                  context->result_syllable_count,
                                  (HZ*)context->result_string,
                                  context->selected_items[0].candidate.hz.word_item->ci_length);
                return;
            }

            //以词定字
            ProcessZiSelectedByWord(*(HZ*)context->result_string, context->result_syllables[0]);
        }

        return;
    }

    //多次选择，组成新词
    AddCi(context->result_syllables, context->result_syllable_count, (HZ*)context->result_string, context->result_length);
    //加入到Cache中
    InsertCiToCache((HZ*)context->result_string, context->result_length, context->result_syllable_count, 0);
}

/*!
 * \brief 生成结果串以及结果音节数组
 */
void MakeResult(PIMCONTEXT *context)
{
    int         item_count = context->selected_item_count;
    HZ          *p_result = context->result_string;
    SYLLABLE    *p_syllable = context->result_syllables;
    int         syllable_count;
    TCHAR       cand_string[MAX_RESULT_LENGTH + 1];
    SYLLABLE    syllables[MAX_SYLLABLE_PER_INPUT];
    int         i, length;
    const TCHAR *symbol_string;

    //非法输入串，以用户输入为结果串
    if (context->syllable_count == 0 && context->selected_item_count == 0)
    {
        _tcsncpy(p_result, context->input_string, MAX_RESULT_LENGTH);
        context->result_syllable_count = 0;
        return;
    }

    //遍历用户已经选择的词/字/
    context->result_length = 0;
    for (i = 0; i < context->selected_item_count; i++)
    {
        length = GetSelectedItemString(context, &context->selected_items[i], cand_string, _SizeOf(cand_string));
        if (length + (p_result - (HZ*)context->result_string) >= MAX_RESULT_LENGTH)
            continue;       //跳过越界的候选

        syllable_count = GetSelectedItemSyllable(&context->selected_items[i], syllables, MAX_SYLLABLE_PER_INPUT);
        if (syllable_count + (p_syllable - context->result_syllables) > MAX_SYLLABLE_PER_INPUT)
            continue;

        memcpy(p_result, cand_string, 2*length * sizeof(HZ));
        p_result += 2*length;
        context->result_length += length;

        memcpy(p_syllable, syllables, syllable_count * sizeof(SYLLABLE));
        p_syllable += syllable_count;
    }

    *p_result = 0;

    context->result_syllable_count = (int)(p_syllable - context->result_syllables);
    context->state = STATE_RESULT;

    //处理最后的结果，汉字增加频度、词Cache等
    PostResult(context);
}

/*!
 * \brief 获得当前上下文的候选信息
 */
void MakeCandidate(PIMCONTEXT *context)
{
    int i, compose_cursor_index, cursor_pos, candidate_count = 0;
     memset(context->candidate_array, 0, sizeof(context->candidate_array));

    if (context->state == STATE_IEDIT)
    {
        int syllable_count  = context->iedit_syllable_index == context->syllable_count ?
                    context->syllable_count : context->syllable_count - context->iedit_syllable_index;
        SYLLABLE *syllables = context->iedit_syllable_index == context->syllable_count ?
                    context->syllables : context->syllables + context->iedit_syllable_index;

        context->candidate_count =
                GetCandidates(context, 0, syllables, syllable_count, context->candidate_array, MAX_CANDIDATES, 0);

        return;
    }

    //***当光标位于拼音串中间(而非首位)时，第一个候选项应该是所有尚未转化为汉字的音节得出的候选项，从第二个候选项
    //开始才是光标之后的音节得出的候选项，第一个候选项的汉字通常被存储在context->syllable_hz中，来处理光标位于拼音
    //串中间时的选择问题
    if (context->syllable_mode && context->compose_cursor_index && context->compose_cursor_index < context->compose_length)
    {
        //希望能给所有尚未转化为汉字的拼音找一个汉字串作为第一个候选
        //(默认的候选结果)，但context->compose_cursor_index和context->cursor_pos
        //会影响这一逻辑(见GetCandidates代码，例如光标(context->compose_cursor_index
        //)位于拼音串中间时，候选的结果通常应该是光标之后的音节对应的
        //词(而不是所有未转化为汉字的音节对应的词))，所以这里先将二者
        //置0，获得候选后再恢复
        //例：不将context->cursor_pos置0
        //输入zi'guang'h'w'h'q，本来默认的候选为"紫光海外华侨"，按left
        //键4次后变为"海外华侨"

        //保存之前的context->compose_cursor_index
        compose_cursor_index          = context->compose_cursor_index;
        context->compose_cursor_index = 0;

        //保存之前的context->cursor_pos
        cursor_pos          = context->cursor_pos;
        context->cursor_pos = 0;

        context->candidate_count =
                GetCandidates(context,
                              context->input_string + context->input_pos,
                              context->syllables + context->syllable_pos,
                              context->syllable_count - context->syllable_pos,
                              context->candidate_array,
                              MAX_CANDIDATES,
                              !context->syllable_pos);

        if (context->candidate_count)
            candidate_count = context->candidate_count = 1;

        //恢复之前的context->compose_cursor_index
        context->compose_cursor_index = compose_cursor_index;

        //恢复之前的context->cursor_pos
        context->cursor_pos = cursor_pos;
    }

    //获得候选
    context->candidate_count = candidate_count +
            GetCandidates(context,
                          //如果已经有输入，则不应该读取首字母输入
                          context->input_string + context->input_pos,
                          context->syllables + context->syllable_pos,
                          context->syllable_count - context->syllable_pos,
                          context->candidate_array + candidate_count,
                          MAX_CANDIDATES - candidate_count,
                          !context->syllable_pos);

    //更新默认汉字串
    //if (context->candidate_count &&
    //(0 == context->compose_cursor_index || context->compose_length == context->compose_cursor_index ||
    //(context->syllable_count != (int)_tcslen(context->syllable_hz))))
    //    GetCandidateString(context, &context->candidate_array[0], context->syllable_hz, _SizeOf(context->syllable_hz));
    //上面代码废弃的原因：1.默认汉字串的意义应该是为所有尚未转化为汉字的音节指定一个默认的候选结果，由于移动光标
    //并不会造成任何汉字转化行为，因此context->syllable_hz应该与光标无关；2.context->syllable_hz应该尽量与尚未转
    //化为汉字的音节数保持一致，且是最新的
    if (context->candidate_count)
    {
        TCHAR candidate_hz[MAX_SYLLABLE_PER_INPUT + 0x10] = {0};
        GetCandidateString(context, &context->candidate_array[0], candidate_hz, _SizeOf(candidate_hz));

        if ((int)_tcslen(candidate_hz) >= context->syllable_count - context->syllable_pos)
        {
            _tcscpy_s((TCHAR*)context->default_hz, _SizeOf(context->default_hz), candidate_hz);
            GetCandidateSyllable(&context->candidate_array[0], context->default_hz_syllables, MAX_SYLLABLE_PER_INPUT + 0x10);
        }
    }

    //处理第一条候选和第二条候选相等的情况，应该是为了避免和上面的***处存在重复
    if (candidate_count && context->candidate_count > 1)
    {
        TCHAR cand_str1[MAX_SPW_LENGTH + 2], cand_str2[MAX_SPW_LENGTH + 2];
        int is_same;

        GetCandidateString(context, &context->candidate_array[0], cand_str1, MAX_SPW_LENGTH);
        GetCandidateString(context, &context->candidate_array[1], cand_str2, MAX_SPW_LENGTH);

        is_same = !_tcscmp(cand_str1, cand_str2);
        if (!is_same)
        {
            int len1 = (int)_tcslen(cand_str1);
            int len2 = (int)_tcslen(cand_str2);

            if (len1 > len2)
                is_same = !_tcscmp(cand_str1 + len1 - len2, cand_str2);
        }

        if (is_same)
        {
            for (i = 1; i < context->candidate_count - 1; i++)
                context->candidate_array[i] = context->candidate_array[i + 1];

            context->candidate_count--;
        }
    }

    if (context->candidate_index >= context->candidate_count)
        context->candidate_index = 0;

    context->candidate_selected_index = 0;
}

/*!
 * \brief 设置非法模式下的写作串
 */
void SetIllegalComposeString(PIMCONTEXT *context)
{
    TCHAR *pc = context->compose_string;
    TCHAR *pi = context->input_string;
    TCHAR at_count = 0;

    while(*pi)
    {
        if (*pi == '@')
        {
            at_count++;
            if (at_count == 2)
            {
                pi++;
                continue;
            }
        }
        *pc++ = *pi++;
    }
    *pc = 0;

    context->compose_length = (int)_tcslen(context->compose_string);
    context->compose_cursor_index = context->cursor_pos - (at_count >= 2 ? 1 : 0);
}

/*!
 * \brief 检查用户输入的音节是V还是U，需要与用户输入保持一致
 */
void CheckSyllableStringVAndU(PIMCONTEXT *context, int index, TCHAR *pinyin)
{
    CheckSyllableStringIUV(context, index, pinyin, TRUE);
}

/*!
 * \brief 检查用户输入的音节是I, V还是U，需要与用户输入保持一致
 */
static void CheckSyllableStringIUV(PIMCONTEXT *context, int index, TCHAR *pinyin, BOOL support_correct)
{
    int pos = context->syllable_start_pos[index];
    int i;

    for (i = 0; pinyin[i]; i++)
    {
        if (context->origin_string[i + pos] == 'u' && pinyin[i] == 'v')
            pinyin[i] = 'u';
        if (context->origin_string[i + pos] == 'v' && pinyin[i] == 'u')
            pinyin[i] = 'v';

        if(support_correct)
        {
            if (context->origin_string[i + pos] == 'v' && pinyin[i] == 'b')
                pinyin[i] = 'v';
            if (context->origin_string[i + pos] == 'u' && pinyin[i] == 'w')
                pinyin[i] = 'u';
            if (context->origin_string[i + pos] == 'i' && pinyin[i] == 'y')
                pinyin[i] = 'i';
        }
    }
}

//get url hint message,put it the cache
/*void CalcCurrentURLStr(PIMCONTEXT *context)
{
    //config->useurl
    if(pim_config->use_url_hint)
        CalcURLString(context);
    else
        ClearURLString();
}*/

/*!
 * \brief 处理上下文。
 *  上下文处理的基础：
 *  0. state
 *  1. input_string
 *  2. input_pos
 *  3. syllables
 *  4. syllable_pos
 *  5. selected_items
 *  6. selected_item_count
*/
void ProcessContext(PIMCONTEXT *context)
{
    TCHAR cand_string[MAX_RESULT_LENGTH + 1];
    int selected_length;
    int new_cursor_pos;
    int i, j;
    SYLLABLE* pSyllable = NULL;
    TCHAR lastInputChar = context->origin_string[context->input_length - 1];
    TCHAR *p;
    int iFlag = 0;
/**
 *  划分拼音音节
 */
    if(!ProcessPinyinParsing(context))
        return;

    //3. 制作每一个音节在输入串中位置数据
    MakeSyllableStartPosition(context);

    //4. 生成已经选择的汉字串
    p = context->compose_string;
    *p = 0;
    for (i = 0; i < context->selected_item_count; i++)
    {
        GetSelectedItemString(context, &context->selected_items[i], cand_string, _SizeOf(cand_string));

        //链接选择的汉字
        for (j = 0; cand_string[j]; j++)
            *p++ = cand_string[j];
    }

    //现在写作串前面为已经选择的汉字
    selected_length = (int)(p - context->compose_string);
    _tcsncpy_s(context->selected_compose_string, _SizeOf(context->selected_compose_string), context->compose_string, selected_length);
    context->selected_compose_string[selected_length] = 0;

    //5. 确定候选信息
    if (pim_config->input_style == STYLE_CSTAR || context->state == STATE_ABC_SELECT)
        MakeCandidate(context);

    //6 生成尚未选择的拼音串
    new_cursor_pos = GenUnselectedPinyinStr(context, p);

    //7. 设定新的光标位置
    context->compose_cursor_index = new_cursor_pos - context->input_pos + selected_length;
    context->compose_length       = (int)_tcslen(context->compose_string);
}

/*!
 * \brief 音节划分.
 *  本地函数，ProcessContext中的一个步骤，处理逆向及正向划分
 *  \param       context          音节
 *  \param       compose_string   组织的上屏显示字符串，包括汉字及拼音等
 *  \return       字符串长度，0标识没有字符。
 */
static BOOL ProcessPinyinParsing(PIMCONTEXT *context)
{
    SYLLABLE new_syllables[MAX_SYLLABLE_PER_INPUT];
    int new_syllable_count;
    int legal_length;
    int i;

    //1. 重新计算音节逆向分割，已选择的部分不变
    new_syllable_count =
            ParsePinYinStringReverse(context->input_string + context->input_pos,
                                     new_syllables + context->syllable_pos,
                                     MAX_SYLLABLE_PER_INPUT - context->syllable_pos,
                                     pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0,
                                     pim_config->pinyin_mode);

    legal_length = GetLegalPinYinLength(context->input_string);

    //非法拼音串，第二个条件的作用在于用户可能希望对于非法串有不同的处理
    //方式，例如输入aB，虽然B不合法，但由于a是合法的，候选结果中有"啊"，
    //此时用户按空格，上屏内容应为"啊B"，即选择的汉字"啊"应该上屏；又如输
    //入wangfuzong(见下面几行的处的注释)，此时又希望已经选择的汉字"王"被
    //消去。我们用if的第二个条件来区分这两种情况，大体上，legal_length是
    //输入串中首个大写字母之前的长度，因此条件二的意思是如果未输入大写字母
    //就应该考虑重新拆分音节而不是上屏。此外，双拼不受此影响
    if (!new_syllable_count && legal_length == context->input_length)
    {
        return FALSE;
    }

    for (i = 0; i < context->syllable_pos; i++)
    {
        new_syllables[i] = context->syllables[i];
    }

    new_syllable_count += context->syllable_pos;

    /**
     * 当前输入法的处理中都没有考虑已经选择的音节发生变化的情况
     */
    //2. 判断已经选择的音节是否与现在的音节相符合
    for (i = 0; i < new_syllable_count && i < context->syllable_pos; i++)
    {
        if (!SameSyllable(new_syllables[i], context->syllables[i]))
            break;
    }

    if (i != context->syllable_pos)         //前面的已经发生的改变，全部展开
    {
        context->selected_item_count = 0;
        context->syllable_pos = 0;
        context->input_pos = 0;
    }
    else    //不需要全部展开，将新的音节补充上
    {
        for (i = context->syllable_pos; i < new_syllable_count; i++)
        {
            context->syllables[i] = new_syllables[i];
        }
        context->syllable_count = new_syllable_count;
    }

    //3. 重新计算音节正向分割，已选择的部分不变
    new_syllable_count =
            ParsePinYinString(context->input_string + context->input_pos,
                              new_syllables + context->syllable_pos,
                              MAX_SYLLABLE_PER_INPUT - context->syllable_pos,
                              pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0);
    context->fore_syllable_count = 0;
    for (i = context->syllable_pos; i < new_syllable_count + context->syllable_pos; i++)
    {
        context->fore_syllables[i] = new_syllables[i];
        //判断是否正向逆向音节完全相同，如果完全相同，则放弃正向划分结果
        if((context->fore_syllable_count == 0) && !SameSyllable(context->fore_syllables[i], context->syllables[i]))
        {
            context->fore_syllable_count = new_syllable_count + context->syllable_pos;
        }
    }

    return TRUE;
}

/*!
 * \brief 生成尚未选择的拼音串.
 *  本地函数，ProcessContext中的一个步骤
 *  \param       context          音节
 *  \param       compose_string   组织的上屏显示字符串，包括汉字及拼音等
 *  \return       字符串长度，0标识没有字符。
 */
static int GenUnselectedPinyinStr(PIMCONTEXT *context, TCHAR *compose_string)
{
    int i, iCount, iCandSylPos;
    int pypos = 0;
    int htpos = 0;
    int input_pos = context->input_pos;
    int new_cursor_pos = context->cursor_pos;
    TCHAR pinyin[MAX_INPUT_LENGTH*6];           ///<用以保存首个备选词对应的完整音节
    TCHAR hint[MAX_INPUT_LENGTH*6];             ///<用以保存上屏显示的音节划分
    TCHAR input_str[MAX_INPUT_LENGTH + 0x10];   ///<用以保存需处理的输入拼音
    TCHAR origin_str[MAX_INPUT_LENGTH + 0x10];  ///<用以保存需处理的原始输入拼音

    memset(pinyin,0,sizeof(pinyin));
    memset(hint,0,sizeof(hint));
    memcpy(input_str, context->input_string + context->input_pos, sizeof(TCHAR) * context->input_length);
    memcpy(origin_str, context->origin_string + context->input_pos, sizeof(TCHAR) * context->input_length);
    input_str[context->input_length - context->input_pos] = L'\0';
    origin_str[context->input_length - context->input_pos] = L'\0';

    //取音节长度
    iCount = 0;
    switch (context->candidate_array[0].type)
    {
    case CAND_TYPE_CI:
        iCount = context->candidate_array[0].word.item->syllable_length;
        break;
    case CAND_TYPE_ICW:
        iCount = context->candidate_array[0].icw.length;
        break;
    case CAND_TYPE_ZI:
        iCount = 1;
        if (context->candidate_array[0].hz.is_word)
        {
            iCount = context->candidate_array[0].hz.word_item->syllable_length;
        }
        break;
    }

    //组织首个备选词的完整含音节划分拼音串
    for (i = 0, iCandSylPos = 0; i < iCount; i++, iCandSylPos++)
    {
        int py_len = 0;
        TCHAR pinyin_syllable[0x10];
        SYLLABLE one_syllable = {0,0,0};

        memset(pinyin_syllable, 0, sizeof(pinyin_syllable));

        switch (context->candidate_array[0].type)
        {
        case CAND_TYPE_CI:
            one_syllable = (context->candidate_array[0].word.item->syllable[iCandSylPos]);
            break;
        case CAND_TYPE_ICW:
            one_syllable = (context->candidate_array[0].icw.syllable[iCandSylPos]);
            break;
        case CAND_TYPE_ZI:
            if (context->candidate_array[0].hz.is_word)
            {
                one_syllable = (context->candidate_array[0].hz.word_item->syllable[iCandSylPos]);
            }
            else
            {
                one_syllable = (context->candidate_array[0].hz.item->syllable);
            }
            break;
        }

        if (iCount > 0 && iCandSylPos <iCount)
        {
            one_syllable.tone = 0;      //避免词库中出现的垃圾tone值，如词库清理过，此句可删除
            py_len = GetSyllableString(one_syllable, pinyin_syllable, _SizeOf(pinyin_syllable), 0);
            CheckSyllableStringIUV(context, i, pinyin_syllable, FALSE);
        }
        if (pinyin[0])
        {
            TCHAR sep_str[2] = {SYLLABLE_SEPARATOR_CHAR, 0};
            _tcscat_s(pinyin, MAX_INPUT_LENGTH*6, sep_str);
        }
        _tcscat_s(pinyin, MAX_INPUT_LENGTH*6, pinyin_syllable);
    }

    //组织含音节划分显示拼音串
    hint[0] = origin_str[0];     //首拼音直接写入不必判断
    pypos = 1;
    htpos = 1;

    for (i = 1; (i < context->input_length) && *(pinyin + pypos); i++)
    {
        BOOL ismatch = FALSE;
        BOOL new_syllable = FALSE;

        if(*(pinyin + pypos) != *(input_str + i))
        {
            //首个备选词全拼与当前拼音不符，则意味着当前拼音为新音节
            //移动当前全拼到当前音节尾
            while(*(pinyin + pypos) && (*(pinyin + pypos) != SYLLABLE_SEPARATOR_CHAR))
                pypos++;
            if(*(pinyin + pypos) && (*(input_str + i) != SYLLABLE_SEPARATOR_CHAR))
                pypos++;
            new_syllable = TRUE;
        }
        else
        {
            while(!ismatch && *(pinyin + pypos))
            {
                ismatch = IsMatchedSyllable(pinyin + pypos, input_str + i, &new_syllable);
                //取下一个拼音
                if(!ismatch)
                    pypos++;
            }
        }

        if(*(pinyin + pypos))
        {
            input_pos++;
            if((*(input_str + i) != SYLLABLE_SEPARATOR_CHAR) && new_syllable)
            {
                hint[htpos++] = SYLLABLE_SEPARATOR_CHAR;
                if(input_pos < context->cursor_pos)
                    new_cursor_pos++;
            }
            hint[htpos++] = origin_str[i];
            pypos++;
        }
        else
        {
            break;
        }
    }
    if(i < context->input_length - context->input_pos)
    {
        //输入拼音比首个备选词的拼音长，复制剩余拼音
        if(*(input_str + i) != SYLLABLE_SEPARATOR_CHAR)
        {
            //用户已输入分隔符，则不需再添加
            hint[htpos++] = SYLLABLE_SEPARATOR_CHAR;
            if(input_pos < context->cursor_pos)
                new_cursor_pos++;
        }
        htpos += context->input_length - context->input_pos - i;
        _tcscat_s(hint, MAX_INPUT_LENGTH*6, origin_str + i);
    }
    hint[htpos] = L'\0';

    for (i = 0; i <= htpos; i++)
        *compose_string++ = hint[i];

    return new_cursor_pos;       //光标要跳到结尾，返回新光标位置
}

/*!
 * \brief 判断当前待搜索拼音串首拼音是否属于当前音节.
 *  本地函数，为GenUnselectedPinyinStr提供判断依据
 * \param   pinyin           首个备选所对应的完整划分后的拼音串
 * \param   search_str       待搜索拼音串
 * \param   new_syllable     输出当前待搜索拼音串首拼音是否属于新音节
 * \return
 *        如果当前当前待搜索拼音串首拼音归为当前音节后，后续划分仍合理，则返回TRUE，否则返回FALSE
 */
static BOOL IsMatchedSyllable(TCHAR *pinyin, TCHAR *search_str, BOOL *new_syllable)
{
    BOOL ret = TRUE;
    int seperator = 0;
    int pinyin_pos = -1;
    TCHAR *pPinyin = pinyin;
    TCHAR *pSearch = search_str;

    while(*pSearch && *pPinyin)
    {
        if(*pPinyin == SYLLABLE_SEPARATOR_CHAR)
        {
            seperator++;
            if(seperator >= 2)  //连续的分隔符不合法
            {
                ret = FALSE;
                break;
            }
        }
        if(*pSearch == *pPinyin)
        {
            if(pinyin_pos < 0)
            {
                //记录首次找到的位置
                pinyin_pos = pPinyin - pinyin;
                if(*pSearch != SYLLABLE_SEPARATOR_CHAR)
                {
                    //分隔符可以看作是当前音节的结尾，而不是下一个音节起首
                    *new_syllable = (seperator > 0);
                    if(!*new_syllable && (pinyin_pos > 0))
                    {
                        ret = FALSE;
                        break;
                    }
                }
            }
            pSearch++;
            seperator = 0;      //新音节开始，分隔符记录清零
        }
        pPinyin++;
    }

    return ret;
}

int GetInputPos(PIMCONTEXT *context)
{
    int i, j;
    int pos = 0;

    //遍历当前的音节，找到需要开始的输入串位置
    for (i = 0; i < context->selected_item_count; i++)
    {
        for (j = context->selected_items[i].syllable_start_pos;
             j < context->selected_items[i].syllable_start_pos + context->selected_items[i].syllable_length;
             j++)
        {
            TCHAR pinyin[0x10];
            int  py_len;
            /**
             * 选择的汉字数
             */
            SYLLABLE one_syllable = context->syllables[j];

            if(context->selected_items[i].candidate.type == CAND_TYPE_CI)
                if(context->selected_items[i].candidate.word.type == CI_TYPE_OTHER)
                    one_syllable = context->fore_syllables[j];

            py_len = GetSyllableString(one_syllable, pinyin, _SizeOf(pinyin),/* context->syllable_correct_flag[j],*/ 0);

            pos += py_len;
            if (context->input_string[pos] == SYLLABLE_SEPARATOR_CHAR)
                pos++;
        }
    }

    return pos;
}

/*!
 * \brief 项输入串中增加字符
*/
void AddChar(PIMCONTEXT *context, TCHAR ch, int is_numpad, IUV_FUZZY fuzzyState)
{
    int i, index, syllable_remains;
    CANDIDATE *candidate;
    const TCHAR *symbol_string;

    if (!ch || context->input_length >= MAX_INPUT_LENGTH || context->syllable_count >= MAX_SYLLABLE_PER_INPUT)
        return;

    if (context->last_dot && !context->next_to_last_dot && ch != '.')
    {
        //依次向右移动一个字符
        for (i = context->input_length + 1; i > context->cursor_pos; i--)
            context->input_string[i] = context->input_string[i - 1];

        //填入当前的字符
        context->input_string[context->cursor_pos] = '.';
        context->cursor_pos++;
        context->input_length++;
        context->candidate_index  = 0;
    }

    //依次向右移动一个字符
    for (i = context->input_length + 1; i > context->cursor_pos; i--){
        context->input_string[i] = context->input_string[i - 1];
        context->origin_string[i] = context->origin_string[i-1];
    }

    //填入当前的字符
    switch (fuzzyState)
    {
    case I_TO_Y:
        context->input_string[context->cursor_pos] = 'y';
        break;

    case U_TO_W:
        context->input_string[context->cursor_pos] = 'w';
        break;

    case V_TO_B:
        context->input_string[context->cursor_pos] = 'b';
        break;

    default:
        context->input_string[context->cursor_pos] = ch;
        break;
    }
    context->origin_string[context->cursor_pos] = ch;


    context->cursor_pos++;
    context->input_length++;
    context->candidate_index  = 0;
    context->last_dot         = 0;
    context->next_to_last_dot = 0;

    ProcessContext(context);
}

/*!
 * \brief 项输入串中删除字符
*/
void BackspaceChar(PIMCONTEXT *context)
{
    int i;

    if (!context->cursor_pos)       //打头的时候，不需要后退
        return;

    //依次向右移动一个字符
    for (i = context->cursor_pos - 1; i < context->input_length; i++){
        context->input_string[i] = context->input_string[i + 1];
        context->origin_string[i]=context->origin_string[i+1];
    }

    context->cursor_pos--;
    context->input_length--;
    context->candidate_index  = 0;
    context->last_dot         = 0;
    context->next_to_last_dot = 0;

    if (context->cursor_pos == context->input_pos)      //与前面已经选择的重合
    {
        if (context->selected_item_count)
        {
            context->syllable_pos = context->selected_items[context->selected_item_count - 1].syllable_start_pos;
            context->selected_item_count--;
            context->input_pos = GetInputPos(context);
        }
    }

    if (context->input_length)
        ProcessContext(context);
}

/*!
 * \brief 求pos之前完整的音节数
 */
int GetSyllableIndexByComposeCursor(PIMCONTEXT *context, int pos)
{
    int i, ret = 0;
    int legal_length = GetLegalPinYinLength(context->input_string);

    if (pos <= 0)
        ret = 0;
    else if (context->compose_length == pos || context->input_pos > legal_length)
        ret = context->syllable_count;
    else
    {
        for (i = 0; i < pos && i < context->compose_length; i++)
            if (SYLLABLE_SEPARATOR_CHAR == context->compose_string[i])
                ret++;

        if (i < context->compose_length && SYLLABLE_SEPARATOR_CHAR == context->compose_string[i])
            ret++;

        for (i = 0; i < context->selected_item_count; i++)
            ret += context->selected_items[i].syllable_length;
    }

    return ret;
}

/*!
 * \brief 将GetSyllableIndexByComposeCursor的结果转化为default_hz_syllables中的结果
 */
int GetSyllableIndexInDefaultString(PIMCONTEXT *context, int syllable_index)
{
    int i, j, pos, has_star, fuzzy_mode, new_syllable_index;
    int syllable_diff, remain_syllable_len, default_syllable_len, checked_syllable_len;
    TCHAR checked_syllable_string[MAX_SYLLABLE_PER_INPUT * MAX_PINYIN_LENGTH + 1]  = {0};
    SYLLABLE parsed_syllables[MAX_SYLLABLE_PER_INPUT + 0x10] = {0};
    SYLLABLE reparsed_syllables[MAX_SYLLABLE_PER_INPUT + 0x10] = {0};
    SYLLABLE current_syllable;

    default_syllable_len = (int)_tcslen((TCHAR*)context->default_hz);
    remain_syllable_len = syllable_index - context->syllable_pos;
    syllable_diff = default_syllable_len - (context->syllable_count - context->syllable_pos);

    //此if还有待进一步思考，目前这样写的依据是syllable_diff < 0时下面的for
    //不会执行，不满足循环条件
    if (syllable_diff < 0)
        return syllable_index;

    //注意，此种情况不能返回syllable_index，由remain_syllable_len的计算公式
    //可知syllable_index == remain_syllable_len + context->syllable_pos，但
    //后者显然不一定等于_tcslen(context->selected_compose_string)，(如选择了
    //"西安")
    if (!remain_syllable_len || !syllable_diff)
        return remain_syllable_len + _tcslen(context->selected_compose_string);

    //用户输入里有通配符的情况
    has_star = 0;
    for (i = context->syllable_pos; i < syllable_index; i++)
    {
        if (context->syllables[i].con == CON_ANY && context->syllables[i].vow == VOW_ANY)
        {
            has_star = 1;
            break;
        }
    }

    //避免因声调引起不匹配，因为音节解析结果与声调无关
    for (i = 0; i < remain_syllable_len; i++)
    {
        parsed_syllables[i] = context->syllables[i + context->syllable_pos];
        parsed_syllables[i].tone = TONE_0;
    }

    //对context->default_hz_syllables的至少前remain_syllable_len个音节进行重解析(逆向解析)，
    //如果解析结果包含在context->syllables中，则当前位置就是光标在default_hz_syllables中的
    //对应位置
    new_syllable_index = -1;
    for (i = remain_syllable_len; i <= remain_syllable_len + syllable_diff; i++)
    {
        //获取i以前的音节对应的拼音
        pos = 0;
        for (j = 0; j < i; j++)
        {
            //排除声调可能引起的干扰
            current_syllable = context->default_hz_syllables[j];
            current_syllable.tone = TONE_0;

            pos += GetSyllableString(
                        current_syllable,
                        checked_syllable_string + pos,
                        MAX_SYLLABLE_PER_INPUT * MAX_PINYIN_LENGTH + 1 - pos,
                        0);
        }

        //重新解析这些拼音
        fuzzy_mode = pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0;
        if (pim_config->ci_option & CI_AUTO_FUZZY)
            fuzzy_mode |= FUZZY_ZCS_IN_CI;

        //由于智能组词结果中不含韵母首字母模糊(见SaveCiOption、CI_AUTO_VOW_FUZZY、
        //FUZZY_SUPER，这里为了避免潜在的问题，暂时不考虑韵母首字母模糊)
        //if (pim_config->ci_option & CI_AUTO_VOW_FUZZY)
        //  fuzzy_mode |= FUZZY_SUPER;

        checked_syllable_len =
                ParsePinYinStringReverse(
                    checked_syllable_string,
                    reparsed_syllables,
                    MAX_SYLLABLE_PER_INPUT,
                    fuzzy_mode,
                    PINYIN_QUANPIN);

        if (!has_star)
        {
            if (checked_syllable_len == remain_syllable_len)
            {
                if (CompareSyllables(parsed_syllables, reparsed_syllables, checked_syllable_len, fuzzy_mode))
                {
                    new_syllable_index = i + _tcslen(context->selected_compose_string);
                    return new_syllable_index;
                }
            }
        }
        else
        {
            //对于有通配符的，要找最大匹配的情况，例如:
            //先造一个词"西安市人民银行"，输入xian*|yinhang，此时default_hz为"西安市人民银行",
            //即"西安市""西安市人""西安市人民"的音节均可匹配xian*，必须选最长的"西安市人民"
            if (WildCompareSyllables(parsed_syllables, remain_syllable_len, reparsed_syllables, checked_syllable_len, fuzzy_mode))
                new_syllable_index = i + _tcslen(context->selected_compose_string);
        }
    }

    if (has_star && new_syllable_index >= 0)
        return new_syllable_index;

    //若未转化成功，仍然返回原来的值
    return syllable_index;
}

/*!
 * \brief 空格或者选择候选上屏，包含以词定字
 * \param  index               选择的候选索引
 * \param  left_or_right       以词定字标示
 */
static void SelectCandidateInternal(PIMCONTEXT *context, int index, int left_or_right)
{
    CANDIDATE *candidate;
    SELECT_ITEM *items = context->selected_items;
    int count = context->selected_item_count;
    int cursor_pos;
    int legal_length;
    int fuzzy_mode = (pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0);

    if (count >= MAX_SYLLABLE_PER_INPUT)
        return;

    candidate = &context->candidate_array[index];

    // no more command execution.
    items[count].left_or_right = ZFW_NONE;
    items[count].candidate     = *candidate;

    switch(candidate->type)
    {
    case CAND_TYPE_ICW:     //ICW
        items[count].syllable_start_pos = context->syllable_pos;
        items[count].syllable_length    = candidate->icw.length;
        items[count].left_or_right      = left_or_right;
        break;

    case CAND_TYPE_CI:      //word
        items[count].syllable_start_pos = context->syllable_pos;
        items[count].left_or_right      = left_or_right;

        if (candidate->word.type == CI_TYPE_OTHER)
            items[count].syllable_length = candidate->word.origin_syllable_length;
        else
            items[count].syllable_length = candidate->word.item->syllable_length;

        break;

    case CAND_TYPE_ZI:      //zi
        {
            SYLLABLE syllable;
            items[count].syllable_start_pos = context->syllable_pos;
            items[count].syllable_length    = 1;

            //处理声调问题：一些候选字待有声调，但改字在context->input_string中并无声调，例如liangang，可能查liang的
            //候选字得到"量"(CON_L，VOW_IANG，TONE_2，只是举例实际中不一定)，如果选择了该字，GetInputPos测得该字的拼
            //音长度为3(包括声调)，但实际上lianggang里的liang是没有声调的，这将造成context->input_pos计算错误
            if (items[count].candidate.hz.is_word)
            {
                syllable = items[count].candidate.hz.origin_syllable;
            }
            else
            {
                syllable = items[count].candidate.hz.item->syllable;
            }

            //除了普通的逆向解析(非小音节字，小音节词)，其他情况(逆向解析小音节字、所有正向解析)都应该是没有声调的。
            //此处以前写为：
            //if (context->syllables[context->syllable_pos].con != items[count].candidate.hz.origin_syllable.con ||
            //    context->syllables[context->syllable_pos].vow != items[count].candidate.hz.origin_syllable .vow)
            //不妥，未考虑到模糊音的情况，如"圣修堂"，设置了模糊，输入senxiutang，选择"圣"，满足if条件，不合理
            if (!ContainSyllable(context->syllables[context->syllable_pos],  syllable, fuzzy_mode))
            {
                //替换掉音节，因为下面的GetInputPos函数正是利用context->syllables(而不是items[count]中的音节)来计算
                //context->input_pos的(这样做可以解决二者不一致的问题，如items[count]中的音节有声调(如字库中选出的候
                //选字通常都是有声调的)，而context->syllables中的音节没有声调，context->input_pos应该以context->syllables
                //为准)
                context->syllables[context->syllable_pos] = syllable;
                context->syllables[context->syllable_pos].tone = TONE_0;
            }

            if (!ContainSyllable(context->fore_syllables[context->syllable_pos],  syllable, fuzzy_mode))
            {
                //替换掉音节，因为下面的GetInputPos函数正是利用context->syllables(而不是items[count]中的音节)来计算
                //context->input_pos的(这样做可以解决二者不一致的问题，如items[count]中的音节有声调(如字库中选出的候
                //选字通常都是有声调的)，而context->syllables中的音节没有声调，context->input_pos应该以context->syllables
                //为准)
                context->fore_syllables[context->syllable_pos] = syllable;
                context->fore_syllables[context->syllable_pos].tone = TONE_0;
            }

            //如果是小音节字，增加一个音节。注意小音节词仍然算一个音节，即输入xianren，如果
            //直接选择"西安"是一个音节，如果分别选择"西"和"安"是两个音节。按backspace键可以
            //明确地看到这一区别，当compose_string为"西安ren"时，按一次backspace前者变为"xianren"
            //，后者变为"西anren"
            if (!items[count].candidate.hz.is_word && items[count].candidate.hz.hz_type == ZI_TYPE_OTHER)
            {
                context->syllable_count++;
                context->fore_syllable_count++;
            }
        }
        break;

    case CAND_TYPE_ZFW:     //以词定字
        items[count].syllable_start_pos = context->syllable_pos;
        items[count].syllable_length    = candidate->zfw.word->item->syllable_length;
        break;

    default:
        return;
    }

    context->selected_item_count++;

    cursor_pos = context->cursor_pos;

    context->candidate_index  = 0;
    context->cursor_pos       = context->input_length;
    context->input_pos        = GetInputPos(context);
    context->syllable_pos    += items[count].syllable_length;

    if (context->selected_item_count && cursor_pos != context->input_length)
    {
        int index = context->selected_item_count - 1;

        //虽然此时context->syllables尚未重新解析，但至少当前的音节已经更新(见上面CAND_TYPE_ZI)的代码，此时
        //先计算一下context->syllable_start_pos，以保证光标位置正确(例如|xianren，选择了"西"，正确的结果应
        //该为"西|anren"，但如果缺少下面这句，结果为"西an|ren")，在ProcessContext里重新解析音节后还会再计算
        //context->syllable_start_pos
        MakeSyllableStartPosition(context);

        index = context->selected_items[index].syllable_start_pos + context->selected_items[index].syllable_length;
        context->cursor_pos = context->syllable_start_pos[index];
    }

    legal_length = GetLegalPinYinLength(context->input_string);

    if (legal_length < context->input_length)
        ProcessContext(context);
    else if (context->syllable_pos >= context->syllable_count)   //结束！
        MakeResult(context);                //生成结果返回
    else
        ProcessContext(context);            //处理新的候选
}

/*!
 * \brief 处理候选选择
 */
void SelectCandidate(PIMCONTEXT *context, int index)
{
    if (context->syllable_mode)
    {
        //光标之前的音节数
        int syllable_index = GetSyllableIndexByComposeCursor(context, context->compose_cursor_index);

        //光标既不在开头也不在结尾
        if (syllable_index > 0 && syllable_index < context->syllable_count)
        {
            /// do nothing

            int i, selected_len, selected_syllable_len, default_len, cand_len, new_syllable_index, count;
            TCHAR selected_string[MAX_RESULT_LENGTH + 1] = {0};
            TCHAR cand_index_str[MAX_RESULT_LENGTH + 1] = {0};

            _tcscpy_s(selected_string, MAX_RESULT_LENGTH, context->selected_compose_string);

            GetCandidateString(context, &context->candidate_array[index], cand_index_str, _SizeOf(cand_index_str));

            //要特别注意区分SELECT_ITEM的成员syllable_length和GetSelectedItemSyllable
            //的返回值，二者可能不是相同的！！！前者是指该选中项所占的context->syllables
            //中的音节数，例如选中项是小音节词，那么syllable_length==1；后者在内部调用
            //GetCandidateSyllable，返回的是实际的音节数，对于小音节词，通常是2。这里我
            //们意图为光标之前的尚未转化为汉字的音节从default_hz中为其指定默认值。但光
            //标之前到底有几个音节未转化为汉字？GetSyllableIndexByComposeCursor是根据context
            //->compose_string中的分隔符和选中项的syllable_length来计算光标之前的音节数
            //的。但是这样计算出的音节数可能和光标之前的default_hz中的汉字数不匹配，例如
            //输入xianshirenhenhao，移动光标，结果为"xianshi|renhenhao"，default_hz为"西
            //安市人很好"，此时syllable_index为2(xian'shi)，即光标之前有2个音节，而此时
            //没有已经选择的汉字，所以默认的字数是2-2=0，这显然错误。但实际上对于default_hz
            //来说，光标之前的字数应为3("西安市"))，这样3-0=3，才能获得正确的需要默认的
            //汉字数。即我们希望获得的默认汉字数是光标之前的default_hz的字数(或者说光标
            //之前的default_hz_syllables中的音节数)。

            //例： compse_string 你们zai'gan'|shen'me (|为光标位置)
            //候选为 1.在干什么 2.神 3.身 4.深 ...
            //selected_compose_string 你们
            //cand_string 你们
            //syllable_hz 在干什么
            //cand_index_string 身 假设选了3
            //syllable_index 4(指未修正前的，你们zai'gan一共4个音节)
            selected_len = (int)_tcslen(selected_string);
            default_len = (int)_tcslen((TCHAR*)context->default_hz);
            cand_len = (int)_tcslen(cand_index_str);

            //再次注意selected_syllable_len和selected_len不一定相等！！！
            selected_syllable_len = context->syllable_pos;

            new_syllable_index = GetSyllableIndexInDefaultString(context, syllable_index);

            //关于条件"default_len > cand_len"的意义:
            //第2个条件的意义：
            //default_len == cand_len
            //输入ziguanghao，移动光标使其为ziguang|hao，通常第1个候选是所有尚未
            //转化为汉字的拼音的一个默认值(见MakeCandidate的第二个if，且该默认值
            //通常就是default_hz)，这里第1个候选为"紫光号"，从第2个候选开始是拼音
            //为hao的单字，选择第1个候选，此时default_len为3，cand_len也为3，但此
            //时实际上是没有需要默认填充的汉字的，不应该进入下面的if。否则结果为
            //"紫光紫光号"
            //default_len > cand_len
            //输入ziguanghao，移动光标使其为ziguang|hao，第1个候选为"紫光号"，从
            //第2个候选开始是拼音为hao的单字，选择第"号"，此时default_len为3，cand_len
            //为1，需要默认填充"紫光"，应该进入下面的if
            //default_len < cand_len
            //设置超级简拼个数为3，并设置韵母首字母模糊，输入jincha，移动光标使其
            //为jin|cha，此时第1个候选是"金叉"，第2个候选是"重婚案"，选择第2个候选，
            //此时default_len为2，cand_len为3，需要默认填充"金"，应该进入下面的if
            //那么第2个条件是否可以为default_len != cand_len呢？不行！
            //例如输入qixianren，移动光标使其为qi|xianren，此时第1个候选为"七贤人"，
            //第2个候选为"西安人"，default_len == cand_len，且有一个默认填充的的汉
            //字"七"。从文字本身判断，即使不能100%准确，也比从长度判断好得多
            if (new_syllable_index > selected_len && _tcsncmp((TCHAR*)context->default_hz, cand_index_str, default_len))
            {
                //最后一个参数表示光标之前的、尚未转化为汉字的音节数
                _tcsncat_s(selected_string, MAX_RESULT_LENGTH - selected_len,
                           (TCHAR*)context->default_hz, new_syllable_index - selected_len);

                //注意syllable_length不是new_syllable_index - selected_len，原因见上面注释。
                //只需记住candidate里是真实的音节，其余地方是解析出的音节！！！

                //将默认填充的汉字转化为一个智能组词候选
                count = context->selected_item_count;
                context->selected_items[count].left_or_right           = ZFW_NONE;
                context->selected_items[count].syllable_length         = syllable_index - selected_syllable_len;
                context->selected_items[count].syllable_start_pos      = selected_syllable_len;

                context->selected_items[count].candidate.type          = CAND_TYPE_ICW;
                context->selected_items[count].candidate.icw.length    = new_syllable_index - selected_len;
                _tcsncpy_s(context->selected_items[count].candidate.icw.hz,
                           MAX_ICW_LENGTH + 1, context->default_hz, new_syllable_index - selected_len);
                for (i = 0; i < new_syllable_index - selected_len; i++)
                    context->selected_items[count].candidate.icw.syllable[i] = context->default_hz_syllables[i];

                context->syllable_pos += syllable_index - selected_syllable_len;
                context->selected_item_count++;
            }
        }
    }

    SelectCandidateInternal(context, index, 0);
}

/*!
 * \brief 回车上屏
 * \param is_space        是否为按下空格
 */
void SelectInputString(PIMCONTEXT *context, int is_space)
{
    TCHAR fullshape_string[MAX_RESULT_LENGTH] = {0};

    _tcsncpy(context->result_string, context->origin_string, MAX_RESULT_LENGTH);
    context->result_length         = (int)_tcslen(context->result_string);
    context->result_syllable_count = 0;
    context->state                 = STATE_RESULT;

    return;
}

void SelectInputStringWithSelectedItem(PIMCONTEXT *context)
{
    int i, j;
    TCHAR *p;
    TCHAR cand_string[MAX_RESULT_LENGTH + 1];

    p = context->result_string;
    *p = 0;
    for (i = 0; i < context->selected_item_count; i++)
    {
        GetSelectedItemString(context, &context->selected_items[i], cand_string, _SizeOf(cand_string));

        for (j = 0; cand_string[j]; j++)
            *p++ = cand_string[j];
    }

    for (i = context->input_pos; i < context->input_length; i++)
        *p++ = context->origin_string[i];

    *p = 0;

    context->result_length         = (int)_tcslen(context->result_string);
    context->result_syllable_count = 0;
    context->state                 = STATE_RESULT;

    return;
}

