//
// Created by jidan on 17-7-6.
//

#ifndef KEYBOARDDEMO_JNIUTIL_H
#define KEYBOARDDEMO_JNIUTIL_H


#include "../share/linux.h"
#include "../share/context.h"
#include "../share/config.h"
#include <stdio.h>
#include "../share/init_resource.h"
#include "../share/pim_state.h"
#include "../share/pim_resource.h"
#include "../share/editor.h"
int getCandidateNum();
void InputKey(int key_flag, int virtual_key, TCHAR ch);

/*!
     * \brief 获取candidate
     * \param candBuf  candidate缓冲区
     * \param iIndex   candidate的起始索引
     * \param iBufLen  缓冲区的大小
     * \todo 当前的实现中并不是一次取完全部的candidate，之后需要修改
     */

void getCandidateStr(TCHAR* candBuf, int iIndex, int iBufLen);

/*!
     * \brief 获取当前的写作字符串
     * \param candBuf  compose缓冲区
     * \param iBufLen  缓冲区的大小
     * \param type     枚举类型
     */
void getComposeStr(TCHAR * candBuf, int iBufLen,int type);

/*!
     * \brief 获取candidate的总个数
     */

int getCandidateCount();

/*!
     * \brief 获取已经选择的candidate个数
     */

int getSelectCountNum();

/*!
     * \brief 获取当前的编辑状态 （0 开始 1 编辑 4 完成）
     */

int getState();
void reset();
/*!
     * \brief 处理candidate选择
     */
void submitWord(int iIndex);
/*!
     * \brief 保存用户词组
     */
int  saveToFile();

void initialize();


void uninitialize();
void setPath(TCHAR *path);


#endif //KEYBOARDDEMO_JNIUTIL_H

