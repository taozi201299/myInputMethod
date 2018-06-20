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

/*  智能组词头文件
 */

#ifndef _ICW_H_
#define _ICW_H_

#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "linux.h"

#define     BIGRAM_FILE_NAME        TEXT("unispim6//wordlib//bigram.dat")

static char triangleMark[8] = {0xa1, 0xf7, 0,0,0,0,0,0};

#define ICW_MAX_ITEMS           1024                    //每一个ICW项的最大候选数目
#define ICW_MAX_CI_ITEMS        256                     //每项最大的词数目
#define ICW_MAX_PART_SYLLABLES  5                       //最多5个非全音节

extern int GetIcwCandidates(SYLLABLE *syllable, int syllable_count, CANDIDATE *candidate);
extern int LoadBigramData(const TCHAR *name);
extern int FreeBigramData();
extern int MakeBigramFaster();

#ifdef __cplusplus
}
#endif

#endif
