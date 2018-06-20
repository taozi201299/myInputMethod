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

/*  输入法状态控制函数
 */
#include "config.h"
#include "utility.h"
#include "editor.h"
#include "zi.h"
#include "init_resource.h"

extern PIMCONFIG *pim_config;

/*!
* \brief 处理用户的按键输入(input_length == 0)
* \param key_flag 该参数暂不使用，传0即可
* \param virtual_key 该参数暂不使用，传0即可
* \param ch 按键对应的字符（a-z等）
*/
void ProcessKeyStart(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
    const TCHAR *symbol_string = 0;

    ClearResult(context);
    ResetContext(context);

    //进行正常字符的输入
    if (ch >= 'a' && ch <= 'z')
    {
        if (ch == 'v')
        {
            AddChar(context, ch, 0, V_TO_B);
            context->state = STATE_EDIT;
        }
        else if (ch == 'i')
        {
            AddChar(context, ch, 0, I_TO_Y);
            context->state = STATE_EDIT;
        }
        else if (ch == 'u')
        {
            AddChar(context, ch, 0, U_TO_W);
            context->state = STATE_EDIT;
        }
        else
        {
            AddChar(context, ch, 0, 0);
            context->state = STATE_EDIT;
        }

        return;
    }
}

/*!
* \brief 处理用户的按键输入(input_length > 0)
* \param key_flag 该参数暂不使用，传0即可
* \param virtual_key 该参数暂不使用，传0即可
* \param ch 按键对应的字符（a-z等）
*/
void ProcessKeyEdit(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
    switch(ch)
    {
    case L'8':     //回退键

        /**
         * 处理回退 ，当前包含已经选择的candidate
         */
        if (context->selected_item_count && context->input_length)
        {
            context->syllable_pos = context->selected_items[context->selected_item_count - 1].syllable_start_pos;
            context->selected_item_count--;
            /**
             * 需要开始的拼音串pos
             */
            context->input_pos = GetInputPos(context);

            ProcessContext(context);
        }
            /**
             * 处理回退，当前不包含已经选择的candidate
             */
        else
        {
            BackspaceChar(context);
            if (!context->input_length)
            {
                ResetContext(context);
            }
        }
        break;

    case 0xd:   //回车
        if (key_flag & KEY_SHIFT)
            SelectCandidate(context, context->candidate_selected_index);
        else if (context->selected_item_count)
            SelectInputStringWithSelectedItem(context);
        else
            SelectInputString(context, 0);

        break;

    default:
        //插入字符（在光标位置）
        if (ch >= 0x20 && ch <= 0x80)
        {
            switch (virtual_key & 0xFF)
            {
            case 0x6B:  // +
            case 0x6F:  // /
                AddChar(context, ch, 1, 0);
                break;

            default:
                AddChar(context, ch, 0, 0);
            }
        }
        break;
    }

    if (!context->compose_length)
    {
        context->state            = STATE_START;
    }
}

/*!
 * \brief 处理所有状态下的按键。
 *  如果处理了键，则返回1，否则返回0
 */
void ProcessKey(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
    if (0 == context->input_length)
        ProcessKeyStart(context, key_flag, virtual_key, ch);
    else
        ProcessKeyEdit(context, key_flag, virtual_key, ch);
}
