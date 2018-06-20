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

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "kernel.h"
#include "context.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ZFW_NONE        0
#define ZFW_LEFT        1
#define ZFW_RIGHT       2
typedef enum IUV_FUZZY_MAP
    {
           NO_FUZZY = 0,
           I_TO_Y,
           U_TO_W,
           V_TO_B,
    }IUV_FUZZY;

//extern void AddChar(PIMCONTEXT *context, TCHAR ch, int is_numpad,bool bCandidate,int word_suggestion);
extern void AddChar(PIMCONTEXT *context, TCHAR ch, int is_numpad, IUV_FUZZY fuzzyState);
extern void BackspaceChar(PIMCONTEXT *context);
extern void ProcessContext(PIMCONTEXT *context);
extern void SelectCandidate(PIMCONTEXT *context, int index);
extern void SelectInputString(PIMCONTEXT *context, int is_space);
extern void SelectInputStringWithSelectedItem(PIMCONTEXT *context);
extern void MakeCandidate(PIMCONTEXT *context);
extern int GetCandidateSyllable(CANDIDATE *candidate, SYLLABLE *syllables, int length);
extern int GetCandidateSyllableCount(CANDIDATE *candidate);
extern int GetCandidateDisplayString(PIMCONTEXT *context, CANDIDATE *candidate, TCHAR *buffer, int length, int first_candidate);
extern int GetInputPos(PIMCONTEXT *context);
extern int GetSyllableIndexByComposeCursor(PIMCONTEXT *context, int pos);
extern int GetSyllableIndexInDefaultString(PIMCONTEXT *context, int syllable_index);
int GetCandidateString(PIMCONTEXT *context, CANDIDATE *candidate, TCHAR *buffer, int length);

#ifdef __cplusplus
}
#endif

#endif
