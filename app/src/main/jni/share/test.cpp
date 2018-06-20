//
// Created by jidan on 17-2-6.
//

#include "../include/jniUtil.h"

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

#include "linux.h"
#include "../include/jniUtil.h"
#include "context.h"
#include "config.h"
#include <stdio.h>
#include "init_resource.h"
#include "pim_state.h"
#include "pim_resource.h"
#include "editor.h"

extern PIMCONFIG default_config;

extern DEFAULT_RESOURCE *default_segment;

int wildcard_state = 0;

extern PIMCONFIG *pim_config;

PIMCONTEXT *pim_context;
TCHAR *m_Path;


int resource_thread_finished = 0;

/*!
     * \brief 处理用户的按键输入
     * \param key_flag 该参数暂不使用，传0即可
     * \param virtual_key 该参数暂不使用，传0即可
     * \param ch 按键对应的字符（a-z等）
     * \todo  将不使用的参数去掉（使用的开源输入法，并没有将一些不使用的参数完全去掉）
     */
void InputKey(int key_flag, int virtual_key, TCHAR ch)
{
    pim_config->use_word_suggestion = 0;
    if (1 == wildcard_state) {
        wildcard_state = 0;
        pim_config->use_word_suggestion = 0;
        ResetContext(pim_context);
    }

    if (pim_context->state == STATE_RESULT)
    {
        ResetContext(pim_context);
    }
    ProcessKey(pim_context, key_flag, virtual_key, ch);
}

void setPath(TCHAR *path){
   m_Path = path;
}
/*!
     * \brief 获取candidate
     * \param candBuf  candidate缓冲区
     * \param iIndex   candidate的起始索引
     * \param iBufLen  缓冲区的大小
     * \todo 当前的实现中并不是一次取完全部的candidate，之后需要修改
     */

void getCandidateStr(TCHAR* candBuf, int iIndex, int iBufLen)
{
    if (iIndex >= MAX_CANDIDATES)
        return;
    GetCandidateString(pim_context, pim_context->candidate_array + iIndex, candBuf, iBufLen);
}

/*!
     * \brief 获取当前的写作字符串
     * \param candBuf  compose缓冲区
     * \param iBufLen  缓冲区的大小
     * \param type     枚举类型
     */
void getComposeStr(TCHAR * candBuf, int iBufLen,int type)
{
    switch (type) {
        case pinyinString:
            memcpy(candBuf, pim_context->compose_string, ((iBufLen > MAX_COMPOSE_LENGTH) ? MAX_COMPOSE_LENGTH : iBufLen) * sizeof(TCHAR));
            break;
        case selectString:
            memcpy(candBuf, pim_context->selected_compose_string, ((iBufLen > MAX_COMPOSE_LENGTH) ? MAX_COMPOSE_LENGTH : iBufLen) * sizeof(TCHAR));
            break;
        case resultString:
            memcpy(candBuf,pim_context->result_string,((iBufLen > MAX_RESULT_LENGTH) ? MAX_RESULT_LENGTH : iBufLen) * sizeof(TCHAR));
            break;
        default:
            break;
    }
}

/*!
     * \brief 获取candidate的总个数
     */

int getCandidateCount()
{
      return pim_context->candidate_count;
}

/*!
     * \brief 获取已经选择的candidate个数
     */

int getSelectCountNum()
{
    return pim_context->selected_item_count;
}

/*!
     * \brief 获取当前的编辑状态 （0 开始 1 编辑 4 完成）
     */

int getState()
{
    return pim_context->state;
}
void reset(){
    ResetContext(pim_context);
}
/*!
     * \brief 处理candidate选择
     */
void submitWord(int iIndex)
{
    SelectCandidate(pim_context, iIndex);
}
/*!
     * \brief 保存用户词组
     */
int  saveToFile(){
    SaveWordLibrary(0);
    SaveZiCacheResource();
    SaveCiCacheResource();
    return 1;
}

void initialize()
{
    LoadDefaultResource();

    pim_config = (PIMCONFIG *)malloc(sizeof(PIMCONFIG));
    SaveConfigInternal(&default_config);

    PIM_LoadResources();
    pim_context = (PIMCONTEXT* )malloc(sizeof(PIMCONTEXT));

    FirstTimeResetContext(pim_context);

    //      ToggleChineseMode(context);
    ResetContext(pim_context);
}

void uninitialize()
{
    free(pim_context);

    FreeWordLibraryResource();

    PIM_FreeResources();

    free(pim_config);

    FreeDefaultResource();
}

int getCandidateNum(){
    return -1;
}