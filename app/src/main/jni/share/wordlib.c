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

/*  词库处理函数组。
 *  系统中的词库可能有多个，因此需要统一管理。
 *      函数列表：
 *      LoadWordLib         装载词库文件到共享区
 *      SaveWordLib         保存词库文件
 *      CloseWordLib        关闭词库文件
 *      BackupWordLib       备份词库
 *      MergeWordLib        合并词库
 *      AddWord             增加一条词汇
 *      DelWord             删除一条词汇
 *      SetFuzzyMode        设定模糊模式
 *      GetWordCANDIDATE    获得词的候选
 *      GetAllItems         获得词库中的全部词条
 *      GetSystemItems      获得全部系统词汇条目
 *      GetUserItems        获得全部用户自定义条目
 */

/*  词库结构说明
 *  采用页表方式进行内存的申请，每次扩展词库都以一个页为单位。
 *  页：1024字节。
 *
 *  词库结构：
 *  1、词库信息
 *  2、词库索引表
 *  3、词库页内容
 */
//#include <tchar.h>
#include <assert.h>
//#include <io.h>
#include <fcntl.h>
#include "wordlib.h"
#include "utility.h"
#include "ci.h"
#include "zi.h"
#include "context.h"
//#include <win32/pim_ime.h>
#include "init_resource.h"

//词库指针数组，等于0的时候，为该ID没有被分配。
static WORDLIB *wordlib_buffer[MAX_WORDLIBS * 2] = { 0, };

//词库文件是否可写
static int wordlib_can_write[MAX_WORDLIBS + MAX_WORDLIBS] = { 0 };
//上面的变量应该为每一个进程分配一个，不能够使用全局的，否则会出毛病（在IE7）中。

typedef void (_stdcall *progress_indicator) (int total_number, int current_number);

/*!
 * \brief 获得用户词库的标识。
 * \param 无
 * \return 用户词库标识，没有装载则返回-1。
 */
int GetUserWordLibId()
{
//    extern int resource_thread_finished;

//此处原意等候资源加载线程，但源码中并未发现此线程，故此处等候是多余的，今后如需如此处理再加回
//    while (!resource_thread_finished)
        /// add by jidan
        //Sleep(0);                     //此处引发了死机问题

    return default_resource->user_wordlib_id;
}

/*!
 * \brief 获得词库的内存大小（用于保存词库数据）
 * \param      wordlib             词库指针
 * \return     词库的大小（头加上页长度）
 */
int GetWordLibSize(WORDLIB *wordlib)
{
    assert(wordlib);

    return sizeof(wordlib->header_data) + wordlib->header.page_count * WORDLIB_PAGE_SIZE;
}

/*!
 * \brief 依据词库标识，获得内存中词库的指针。
 * \param wordlib_id          词库标识
 * \return  找到：词库指针      未找到：0
 */
WORDLIB *GetWordLibrary(int wordlib_id)
{
    if (wordlib_id < 0)             //非法标识
        return 0;

    if (wordlib_buffer[wordlib_id] == 0)        //没有这个词库的指针（可能被释放掉了）
    {
        Log(LOG_ID, L"获取词库指针出错。id=%d", wordlib_id);
        return 0;
    }

    return wordlib_buffer[wordlib_id];
}

/*!
 * \brief 保存词库文件。
 * \param       wordlib_id          词库标识
 * \return       成功：1      失败：0
 */
int SaveWordLibrary(int wordlib_id)
{
    WORDLIB *wordlib;
    int     length;         //词库长度

    if (wordlib_id == default_resource->user_wordlib_id &&
        (!default_resource->can_save_user_wordlib || !default_resource->user_wl_modified))
    {
        Log(LOG_ID, L"用户词库没有改变或禁止改变，不保存词库");
        return 0;
    }

    //词库指针获取
    wordlib = GetWordLibrary(wordlib_id);
    if (!wordlib)
    {
        Log(LOG_ID, L"未找到词库，id=%d", wordlib_id);
        return 0;
    }

    //用户词库，需要做一下备份
    if (GetUserWordLibId() == wordlib_id)
    {
        TCHAR bak_wordlib_name[MAX_PATH];

        _tcscpy_s(bak_wordlib_name, _SizeOf(bak_wordlib_name), default_resource->wordlib_name[wordlib_id]);
        _tcscat_s(bak_wordlib_name, _SizeOf(bak_wordlib_name), TEXT(".bak"));
    }

    length = GetWordLibSize(wordlib);

    if (!SaveToFile(default_resource->wordlib_name[wordlib_id], wordlib, length))
    {
        Log(LOG_ID, L"保存词库失败，id = %d, lib_ptr = %p, length = %d", wordlib_id, wordlib, length);
        return 0;
    }

    if (wordlib_id == default_resource->user_wordlib_id)
        default_resource->user_wl_modified = 0;

    return 1;
}

/*!
 * \brief 获得内存中已经装载的词库文件
 * \param lib_name 词库文件name
 * \return  已经装载 :词库序号  未装载 :-1
 */
int GetWordLibraryLoaded(const TCHAR *lib_name)
{
    int  i;

    //遍历内存中的词库
    for (i = 0; i < MAX_WORDLIBS; i++)
        if (!default_resource->wordlib_deleted[i] && !_tcscmp(default_resource->wordlib_name[i], lib_name))
            break;

    if (i != MAX_WORDLIBS)          //本词库已经在内存中
        return i;

    return -1;
}

/*!
 * \brief 装载词库文件。模块内部使用。
 *  词库文件将装载到内存的共享区。对于同一个用户，所有的应用程序共享相同的
 *  词库（包括多个词库）。
 * \param       lib_name            词库的完整文件名字（包含路径）
 * \param       extra_length        词库所需要的扩展数据长度
 * \param       check_exist         检查是否已经存在（用于词库的更新）
 * \return      成功：词库序号 >= 0      失败：-1
 */
int LoadWordLibraryWithExtraLength(const TCHAR *lib_name, int extra_length, int check_exist)
{

    int length = 0;
    char *buffer = NULL;
    int i,error = 0;
    int empty_id = 0;


    error = 1;

    empty_id = -1;
    for (i = 0; i < MAX_WORDLIBS; i++)
    {
        if (check_exist && !default_resource->wordlib_deleted[i] &&
            !_tcscmp(default_resource->wordlib_name[i], lib_name))
            break;
        if(empty_id == -1 && wordlib_buffer[i] == 0)
            empty_id = i;
    }
    if(i == MAX_WORDLIBS){
        length = GetFileLength(lib_name);               //获得词库的长度
        if (length <= 0)                                    //文件不存在？
            return 0;
        buffer = (char *)malloc(length + extra_length);
        if(!buffer)
            return 0;
        memset(buffer,0,length+extra_length);
        if(!LoadFromFile(lib_name,buffer,length)){
            free(buffer);
            buffer = NULL;
            return 0;
        }

        //对词库相关数据进行记录
        _tcsncpy(default_resource->wordlib_name[empty_id], lib_name, _SizeOf(default_resource->wordlib_name[0]));

        default_resource->wordlib_length[empty_id] = length +extra_length;
        wordlib_buffer[empty_id]    = (WORDLIB*)buffer;
        wordlib_can_write[empty_id] = 1;
        error = 0;
    }
    if(error)
    {
        Log(LOG_ID, L"加载失败");
        return -1;
    }
    return empty_id;
}

/*!
 * \brief 装载词库文件。
 *  词库文件将装载到内存
 *  词库（包括多个词库）。
 * \param       wordlib_name        词库的完整文件名字（包含路径）
 * \return      成功：词库序号 >= 0      失败：-1
 */
int LoadWordLibrary(const TCHAR *wordlib_name)
{
    return LoadWordLibraryWithExtraLength(wordlib_name, WORDLIB_NORMAL_EXTRA_LENGTH, 1);
}

/*!
 * \brief 装载用户词库文件。
 *  词库文件将装载到内存的共享区。对于同一个用户，所有的应用程序共享相同的
 *  词库（包括多个词库）。
 *  由于用户词库将存储自行制造的词汇，因此需要加入扩展的数据长度。
 * \param  wordlib_name        词库的完整文件名字（包含路径）
 * \return   如果成功，返回词库标识；失败返回-1。
 */
int LoadUserWordLibrary(const TCHAR *wordlib_name)
{
    TCHAR new_wordlib_name[MAX_PATH];
    TCHAR bak_wordlib_name[MAX_PATH];

    Log(LOG_ID, L"0");
    Log(LOG_ID, L"加载用户词库<%s>", wordlib_name);

    if (GetFileLength(wordlib_name) <= 0)       //没有词库文件，创建一份
        if (!CreateEmptyWordLibFile(wordlib_name, DEFAULT_USER_WORDLIB_NAME, DEFAULT_USER_WORDLIB_AUTHOR, 1))
            return -1;          //彻底失败

    Log(LOG_ID, L"1");

    //用户词库需要扩充内存
    default_resource->user_wordlib_id = LoadWordLibraryWithExtraLength(wordlib_name, WORDLIB_EXTRA_LENGTH, 1);

    //无法加载，可能是词库文件出错了，备份之前的文件，然后重新创建用户词库
    if (-1 == default_resource->user_wordlib_id)
    {
//      _tcscpy_s(new_wordlib_name, _SizeOf(new_wordlib_name), wordlib_name);
//      _tcscat_s(new_wordlib_name, _SizeOf(new_wordlib_name), TEXT(".bad"));

//      _tcscpy_s(bak_wordlib_name, _SizeOf(bak_wordlib_name), wordlib_name);
//      _tcscat_s(bak_wordlib_name, _SizeOf(bak_wordlib_name), TEXT(".bak"));

//      if (FileExists(new_wordlib_name))
//      {
//          SetFileAttributes(new_wordlib_name, FILE_ATTRIBUTE_NORMAL);
//          DeleteFile(new_wordlib_name);
//      }

        //备份错误的文件
//      MoveFile(wordlib_name, new_wordlib_name);

        //恢复备份文件
//      if (FileExists(bak_wordlib_name))
//          CopyFile(bak_wordlib_name, wordlib_name, FALSE);

        //装载备份文件
//      if (FileExists(wordlib_name))
//          default_resource->user_wordlib_id = LoadWordLibraryWithExtraLength(wordlib_name, WORDLIB_EXTRA_LENGTH, 1);

        //备份文件也不对，创建一个新的用户词库文件
        if (-1 == default_resource->user_wordlib_id)
        {
            CreateEmptyWordLibFile(wordlib_name, DEFAULT_USER_WORDLIB_NAME, DEFAULT_USER_WORDLIB_AUTHOR, 1);
            default_resource->user_wordlib_id = LoadWordLibraryWithExtraLength(wordlib_name, WORDLIB_EXTRA_LENGTH, 1);
        }
    }

    return default_resource->user_wordlib_id;
}

/*!
 * \brief 释放词库数据。
 * \param wordlib_id 词库序号
 * \return 无
 */
void CloseWordLibrary(int wordlib_id)
{
    if (wordlib_id < 0 || wordlib_id >= MAX_WORDLIBS * 2)
        return;

   // FreeSharedMemory(default_resource->wordlib_shared_name[wordlib_id], wordlib_buffer[wordlib_id]);
    if(wordlib_buffer[wordlib_id])
        free(wordlib_buffer[wordlib_id]);
    wordlib_buffer[wordlib_id]                    = 0;      //指针清零
    default_resource->wordlib_length[wordlib_id]     = 0;      //长度清零
    default_resource->wordlib_name[wordlib_id][0]    = 0;      //文件名字清零
    wordlib_can_write[wordlib_id]                 = 0;      //禁止写入
    default_resource->wordlib_deleted[wordlib_id]    = 0;      //被删除标志
}

/*!
 * \brief 释放所有的词库数据。
 * \param 无
 * \return 无
 */
void CloseAllWordLibrary()
{
    int i;

    for (i = 0; i < MAX_WORDLIBS; i++)
        CloseWordLibrary(i);
}

/*!
 * \brief   在词库中建新页。
 * \param       wordlib_id              词库句柄
 * \return      成功创建：页号      失败：-1
 */
/*static*/ int NewWordLibPage(int wordlib_id)
{
    WORDLIB *wordlib = GetWordLibrary(wordlib_id);              //词库指针
    int length = default_resource->wordlib_length[wordlib_id];     //词库的总长度
    int new_length, new_page_no;                                //新的词库长度、新页号

    if (!wordlib)           //没有这个词库
        return -1;

    //计算当前词库的Size是否已经到达词库的边界
    new_length = sizeof(wordlib->header_data) +                         //词库头
                 wordlib->header.page_count * WORDLIB_PAGE_SIZE +       //页数据长度
                 WORDLIB_PAGE_SIZE;                                     //新页数据长度

    if (new_length > length)            //超出内存边界，无法分配
        return -1;

    //对页初始化
    new_page_no = wordlib->header.page_count;

    wordlib->pages[new_page_no].data_length  = 0;
    wordlib->pages[new_page_no].next_page_no = PAGE_END;
    wordlib->pages[new_page_no].page_no      = new_page_no;
    wordlib->pages[new_page_no].length_flag  = 0;

    wordlib->header.page_count++;

    return new_page_no;
}

/*!
 * \brief 判断词是否已经在词库中
 * \param wordlib_id  词库序号
 * \param hz          汉字指针
 * \param hz_length   汉字长度
 * \param syllabe     汉字对应音节指针
 * \param syllable_length 音节长度
 * \return 在词库中：指向词条的指针      不在：0
 */
WORDLIBITEM *GetCiInWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length)
{
    int count, i;
    //CANDIDATE candidates[MAX_CANDIDATES];
    CANDIDATE   *candidates;
    WORDLIBITEM *item = 0;

    candidates = malloc(sizeof(CANDIDATE) * MAX_CANDIDATES);

    count = GetCiCandidates(wordlib_id, syllable, syllable_length, candidates, MAX_CANDIDATES, FUZZY_CI_SYLLABLE_LENGTH);

    for (i = 0; i < count; i++) {
        // if (candidates[i].word.item->ci_length == hz_length && !memcmp(candidates[i].word.hz, hz, hz_length * sizeof(HZ)))
        if (candidates[i].word.item->ci_length == hz_length) {
            {
                for (int j = 0; j < hz_length; j++) {
                    if (candidates[i].word.hz[j] == hz[2 * j]) {
                        free(candidates);
                        return item;
                    }
                }
                item = candidates[i].word.item;         //找到
                break;
            }
        }
    }

    free(candidates);

    return item;
}
/*!
 * \brief 在用户词库中获取词频最小的词
 * \param hz           汉字指针
 * \param hz_length    汉字长度
 * \param syllable     汉字对应音节指针
 * \param syllable_length 音节长度
 * \return 成功: 指向词条的指针     失败：0
 */
USERWORDLIBITEM *GetMiniFreqCiInUserWordLib(HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length)
{
    int i, j, page_no, count = 0;
    int iFound = 0;
    USERWORDLIBITEM* item = 0;
    PAGE *page = 0;
    SYLLABLE syllable_tmp1, syllable_tmp2;
    WORDLIB* wordlib = GetWordLibrary(0);

    if (!wordlib)
        return item;

    for (i = CON_NULL; i < CON_END; i++)
    {
        syllable_tmp1.con = i;
        if (!ContainCon(syllable[0], syllable_tmp1, FUZZY_CI_SYLLABLE_LENGTH))
            continue;

        for (j = CON_NULL; j < CON_END; j++)
        {
            syllable_tmp2.con = j;
            if (!ContainCon(syllable[1], syllable_tmp2, FUZZY_CI_SYLLABLE_LENGTH))
                continue;

            page_no = wordlib->header.index[i][j];
            page = &wordlib->pages[page_no];
            if(page == NULL)
                break;

            while(page_no != PAGE_END)
            {
                for (item = (USERWORDLIBITEM*) page->data; (char*)item < (char*) &page->data + page->data_length; item = GetNextCiItem(item, 0))
                {
                    if (item->freq == USER_BASE_FREQ)
                    {
                        iFound = 1;
                        break;
                    }
                }
                if (iFound) {break;}

                page_no = wordlib->pages[page_no].next_page_no;
            }

            if (iFound) {break;}
        }
        if (iFound) {break;}
    }

    return item;
}

/*!
 * \brief 在词库中删除词汇。
 * \param  wordlib_id              词库标识
 * \param  syllable                音节数组
 * \param  hz                      汉字数组
 * \param  length                  长度
 * \return
 *      1                       删除成功
 *      0                       未找到
 *     -1                       失败
 */
int DeleteCiFromWordLib(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int clear_syllable)
{
    WORDLIBITEM *item;
    WORDLIB     *wordlib;

    wordlib = GetWordLibrary(wordlib_id);
    if (!wordlib)       //没有这个词库
        return -1;

    //在词库中查找词
    item = GetCiInWordLibrary(wordlib_id, hz, hz_length, syllable, syllable_length);
    if (!item)              //没有找到
        return 0;

    if (item->effective)
        wordlib->header.word_count--;

    item->effective = 0;

    if (clear_syllable)
    {
        item->syllable[0].con = CON_NULL;
        item->syllable[0].vow = VOW_NULL;
    }

    if (wordlib_id == default_resource->user_wordlib_id)
        default_resource->user_wl_modified = 1;

    return 1;
}

/*!
 * \brief 向词库中增加词汇（用户自造词）。
 *  要求：词的音节必须与字一一对应，不能出现“分隔符”以及“通配符”。
 * \param       wordlib_id  词库标识
 * \param       hz          词
 * \param       length      词长度
 * \param       freq        词频
 * \return
 *      成功加入：1
 *      失败：0
 */
int AddCiToWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int freq)
{
    WORDLIBITEM *item;                          //词项指针
    WORDLIB     *wordlib;                       //词库指针
    PAGE        *page;                          //页指针
    int         page_no, new_page_no;           //页号
    int         item_length;                    //词项长度
    int         i;

    //检查词频，避免越界
    if (freq > WORDLIB_MAX_FREQ)
        freq = WORDLIB_MAX_FREQ;

    if (wordlib_id < 0 || wordlib_id >= MAX_WORDLIBS * 2)
        return 0;

    //不能进行写的词库文件
    if (!wordlib_can_write[wordlib_id])
        return 0;

    if (syllable_length < 2 || syllable_length > MAX_WORD_LENGTH || hz_length < 2 || hz_length > MAX_WORD_LENGTH)
    {
        Log(LOG_ID, L"增加词汇长度错误。syllable_length = %d, ci_length = %d", syllable_length, hz_length);
        return 0;               //音节过少或者过大
    }

    //判断是否都是汉字
    if (!IsAllCanInLibrary(hz, hz_length))
        return 0;

    //进行插入
    wordlib = GetWordLibrary(wordlib_id);
    if (!wordlib)               //没有这个词库
        return 0;

    //判断该词是否在词库中存在，如果存在不做插入，但将有效置为1，并且增加一次词频
    if ((item = GetCiInWordLibrary(wordlib_id, hz, hz_length, syllable, syllable_length)) != 0)
    {
        if (!item->effective)
        {
            item->effective = 1;
            wordlib->header.word_count++;
        }

        if (wordlib_id == GetUserWordLibId())
        {
            if (freq > (int)item->freq)
                item->freq = (unsigned int)freq;
            else
                item->freq++;

            default_resource->user_wl_modified = 1;
            return 1;
        }

        if (freq > (int)item->freq)
            item->freq = (unsigned int)freq;

        return 1;
    }

    item_length = GetItemLength(hz_length, syllable_length);

    //找出音节序列的词库页索引
    page_no = wordlib->header.index[syllable[0].con][syllable[1].con];
    if (page_no == PAGE_END)                            //索引没有指向页
    {
        new_page_no = NewWordLibPage(wordlib_id);       //分配新页
        if (new_page_no == -1)                          //未能分配成功，只好返回
            return 0;

        wordlib->header.index[syllable[0].con][syllable[1].con] = new_page_no;      //索引联接
        page_no = new_page_no;
    }

    //遍历页表找出最后一页。
    //不进行已删除词汇空洞的填补工作，省力（好编）并且省心（程序健壮）。
    while(wordlib->pages[page_no].next_page_no != PAGE_END)
        page_no = wordlib->pages[page_no].next_page_no;

    //获得页
    page = &wordlib->pages[page_no];

    //如果本页的数据不能满足加入要求
    if (page->data_length + item_length > WORDLIB_PAGE_DATA_LENGTH)
    {//需要分配新页
        new_page_no = NewWordLibPage(wordlib_id);
        if (new_page_no == -1)      //未能分配成功，只好返回
            return 0;

        //分配成功，维护页链表
        page->next_page_no  = new_page_no;
        page_no             = new_page_no;
        page                = &wordlib->pages[page_no];
    }

    assert(page->data_length + item_length <= WORDLIB_PAGE_DATA_LENGTH);

    //词汇长度
    page->length_flag |= (1 << syllable_length);

    //在本页中插入输入
    item = (WORDLIBITEM*)&page->data[page->data_length];

    item->effective       = 1;                  //有效
    item->ci_length       = hz_length;          //词长度
    item->syllable_length = syllable_length;    //音节长度
    item->freq            = freq;               //词频

    for (i = 0; i < syllable_length; i++)
        item->syllable[i] = syllable[i];        //音节

    for (i = 0; i < hz_length; i++)
        GetItemHZPtr(item)[i] = hz[i];          //汉字

    //增加页的数据长度
    page->data_length += item_length;

    //增加了一条记录
    wordlib->header.word_count++;

    if (wordlib_id == default_resource->user_wordlib_id)
        default_resource->user_wl_modified = 1;

    //成功插入
    return 1;
}

/*!
 * \brief 向词库中增加词汇（用户自造词）。
 *  要求：词的音节必须与字一一对应，不能出现“分隔符”以及“通配符”。
 * \param       wordlib_id  词库标识
 * \param       hz          词
 * \param       length      词长度
 * \param       freq        词频
 * \return
 *      成功加入：1
 *      失败：0
 */
int AddCiToUserWordLibrary(HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int freq)
{
    USERWORDLIBITEM *item;                      //词项指针
    WORDLIB     *wordlib;                       //词库指针
    PAGE        *page;                          //页指针
    int         page_no, new_page_no;           //页号
    int         item_length;                    //词项长度
    int         i;

    //检查词频，避免越界
    freq = (freq > WORDLIB_MAX_FREQ) ? WORDLIB_MAX_FREQ : freq;

    if (syllable_length < 2 || syllable_length > USER_MAX_SYLLABLES ||
         hz_length < 2 || hz_length > USER_MAX_SYLLABLES)
        return 0;

    //判断是否都是汉字
    if (!IsAllCanInLibrary(hz, hz_length))
        return 0;

    //进行插入
    wordlib = GetWordLibrary(0);
    if (!wordlib)
        return 0;

    //判断该词是否在词库中存在，如果存在不做插入，但将有效置为1，并且增加一次词频
    if ((item = GetCiInWordLibrary(0, hz, hz_length, syllable, syllable_length)) != 0)
    {
        if (!item->effective)
        {
            item->effective = 1;
            wordlib->header.word_count++;
        }

        if (freq > (int)item->freq)
            item->freq = (unsigned int)freq;
        else
            item->freq++;

        default_resource->user_wl_modified = 1;

        return 1;
    }

    item_length = sizeof(USERWORDLIBITEM);

    if (wordlib->header.word_count < MAX_USER_WORD)
    {
        //找出音节序列的词库页索引
        page_no = wordlib->header.index[syllable[0].con][syllable[1].con];
        if (page_no == PAGE_END)
        {
            new_page_no = NewWordLibPage(0);

            if (new_page_no != -1)
            {
                wordlib->header.index[syllable[0].con][syllable[1].con] = new_page_no;
                page_no = new_page_no;

                while(wordlib->pages[page_no].next_page_no != PAGE_END)
                    page_no = wordlib->pages[page_no].next_page_no;
            }
            else
                return 0;
        }

        page = &wordlib->pages[page_no];

        //如果本页的数据不能满足加入要求
        if (page->data_length + item_length > WORDLIB_PAGE_DATA_LENGTH)
        {
            new_page_no = NewWordLibPage(0);
            if (new_page_no != -1)
            {
                //分配成功，维护页链表
                page->next_page_no  = new_page_no;
                page_no             = new_page_no;
                page                = &wordlib->pages[page_no];
            }
        }

        //词汇长度
        page->length_flag |= (1 << syllable_length);

        //在本页中插入输入
        item = (USERWORDLIBITEM*)&page->data[page->data_length];
    }

    if (!item)
        item = GetMiniFreqCiInUserWordLib(hz, hz_length, syllable, syllable_length);

    if (!item)
        return 0;

    item->effective       = 1;                  //有效
    item->ci_length       = hz_length;          //词长度
    item->syllable_length = syllable_length;    //音节长度
    item->freq            = freq;               //词频

    for (i = 0; i < syllable_length; i++)
        item->syllable[i] = syllable[i];        //音节

    for (i = 0; i < hz_length; i++)
        item->hz[i] = hz[2*i];            //汉字

    //增加页的数据长度
    page->data_length += item_length;
    wordlib->header.word_count++;
    default_resource->user_wl_modified = 1;

    //成功插入
    return 1;
}

/*!
 * \brief 创建新的空的词库文件。
 * \param       wordlib_file_name       词库文件名字（全路径）
 * \param       name                    词库名字（放在词库内部）
 * \param       author                  作者
 * \param       can_be_edit             是否允许编辑
 * \return
 *      成功创建：1
 *      失败：0
 */
int CreateEmptyWordLibFile(const TCHAR *wordlib_file_name, const TCHAR *name, const TCHAR *author, int can_be_edit)
{
    WORDLIB     wordlib;
    int i, j;

    Log(LOG_ID, L"创建新词库<%s>, name:%s, author:%s, can_be_edit:%d",
        wordlib_file_name, name, author, can_be_edit);

    //清零
    memset(&wordlib, 0, sizeof(wordlib));

    //作者名字
    utf16ncpy(wordlib.header.author_name, author, _SizeOf(wordlib.header.author_name));

    //词库名字
    utf16ncpy(wordlib.header.name, name, _SizeOf(wordlib.header.name));

    wordlib.header.can_be_edit = can_be_edit;

    //将Index置为没有页
    for (i = CON_NULL; i < CON_END; i++)
        for (j = CON_NULL; j < CON_END; j++)
            wordlib.header.index[i][j] = PAGE_END;

    //没有已分配的页
    wordlib.header.page_count   = 0;
    wordlib.header.pim_version  = HYPIM_VERSION;
    wordlib.header.signature    = HYPIM_WORDLIB_V66_SIGNATURE;
    wordlib.header.word_count   = 0;

    if (!SaveToFile(wordlib_file_name, &wordlib, sizeof(wordlib)))
        return 0;

    return 1;
}

/*!
 * \brief 检查是否词库的指针都已经是正确的。
 */
void MaintainWordLibPointer()
{
    int i;

    for (i = 0; i < MAX_WORDLIBS; i++)
    {
        //词库不在内存中 || 是被删除的词库 || 指针已经被设置
        if (!default_resource->wordlib_name[i][0] || default_resource->wordlib_deleted[i] || wordlib_buffer[i])
            continue;

        //获取词库的指针
       // wordlib_buffer[i] = GetSharedMemory(default_resource->wordlib_shared_name[i]);
    }
}

/*!
 * \brief 获得下一个词库标识，用于词库的遍历。
 * \param cur_id              当前的ID（第一次的时候输入-1）
 * \return
 *      下一个词库标识：>0
 *      没有下一个词库的标识：-1
 */
int GetNextWordLibId(int cur_id)
{
    int i;

    if (cur_id < 0)
        cur_id = -1;

    for (i = cur_id + 1; i < MAX_WORDLIBS; i++)
        if (!default_resource->wordlib_deleted[i] && wordlib_buffer[i])
            return i;

    return -1;
}

/*!
 * \brief 获得词库中被删除的词条
 * \param  wordlib_id      词库标识
 * \param  item_array      词项指针数组
 * \param  length          数组长度
 * \return
 *      被删除词条数目
 */
int GetDeleteItems(int wordlib_id, WORDLIBITEM **item_array, int length)
{
    extern WORDLIBITEM *NextCiItem(WORDLIBITEM *item);
    WORDLIBITEM *item;
    WORDLIB *wl;
    PAGE *page;
    int i;
    int count = 0;

    if (!item_array)
        return 0;

    wl = GetWordLibrary(wordlib_id);
    if (!wl)
        return 0;

    for (i = 0; i < wl->header.page_count; i++)
    {   //遍历页表
        for (page = &wl->pages[i], item = (WORDLIBITEM*) page->data;
             count < length && (char*)item < (char*) &page->data + page->data_length;
             item = GetNextCiItem(item, wordlib_id))
        {
            if (item->effective) //有效的词跳过
                continue;

            item_array[count++] = item;
        }
    }

    return count;
}

/*!
 * \brief 基于词库ID获得词库的文件名称
 * \param  wl_id           词库标识
 * \return 词库名称
 */
const TCHAR *GetWordLibFileName(int wl_id)
{
    if (wl_id < 0 || wl_id >= MAX_WORDLIBS)
        return TEXT("");

    return default_resource->wordlib_name[wl_id];
}

/*!
 * \brief 清除用户词库的内容
 * \param id 词库标识
 */
void ClearWordLib(int id)
{
    WORDLIB *wordlib;                       //词库指针
    int i, j;

    if (id != GetUserWordLibId())
        return;

    wordlib = GetWordLibrary(id);
    if (!wordlib)
        return;

    wordlib->header.page_count = 0;
    wordlib->header.word_count = 0;

    //将Index置为没有页
    for (i = CON_NULL; i < CON_END; i++)
        for (j = CON_NULL; j < CON_END; j++)
            wordlib->header.index[i][j] = PAGE_END;

    default_resource->user_wl_modified = 1;
}

/*!
 * \brief 通知词库被删除，由配置程序发出。
 *  将共享内存置为废弃，但释放的时候，还是要进行删除的
 * \todo 接口暂不使用，后续整理
 */
void WINAPI NotifyDeleteWordLibrary(const TCHAR *wordlib_file_name, int flag)
{
    int i, id;

    if (flag & WLF_DELETE_WORDLIB)
    {
        //检查配置中是否包含本词库，有则删掉
        for (i = 0; i < MAX_WORDLIBS; i++)
            if (!_tcscmp(pim_config->wordlib_name[i], wordlib_file_name))
                break;

        if (i != MAX_WORDLIBS)
        {
            pim_config->wordlib_name[i][0] = 0;
            for (; i < MAX_WORDLIBS - 1; i++)
                _tcscpy_s(pim_config->wordlib_name[i], _SizeOf(pim_config->wordlib_name[i]), pim_config->wordlib_name[i + 1]);

            pim_config->wordlib_count--;
        }

        //检查内存中是否已经存入了词库的数据
        for (i = 0; i < MAX_WORDLIBS; i++)
        {
            if (!default_resource->wordlib_deleted[i] && !_tcscmp(default_resource->wordlib_name[i], wordlib_file_name))
                break;
        }
        if (i == MAX_WORDLIBS)
            return;

        //置为已经删除
        default_resource->wordlib_deleted[i] = 1;
        return;
    }

    if (flag & WLF_CLEAR_WORDLIB)
    {
        id = GetWordLibraryLoaded(wordlib_file_name);
        if (id == -1)
            return;

        //清空用户词库优先级为高，将之强制写出
        ClearWordLib(id);
        default_resource->can_save_user_wordlib = 1;
        SaveWordLibrary(id);

        return;
    }

    return;
}

/*!
 * \brief  批量更新词库
 *  首先看系统内的词库是否存在，如果存在，则在这个词库中添加，如果满，
 *  则再加载新的词库，最后释放。词库的多余容量将存储一些数据，这些数据
 *  应能够立刻起到作用。
 *  \param    wl_name         词库名称（词库内部的名称，非文件名称）
 *  \param    update_items    更新条目数组
 *  \param    item_count      条目数目
 *  \param    flag            处理标志：WLF_CONTINUE_ON_ERROR, WLF_HALT_ON_ERROR
 *  \return
 *      1：成功
 *      <0：失败
 *      -1: WLE_CI      词错误
 *      -2: WLE_YIN     音错误
 *      -3: WLE_FREQ    频错误
 *      -4: WLE_NAME    词库名字错误
 *      -5: WLE_ITEMS   项指针
 *      -6: WLE_NOCI    没有该词
 *      -10: WLE_OTHER  其他错误
 */
int __stdcall UpdateWordLibrary(const TCHAR *wl_name, UPDATEITEM *update_items, int item_count, int flag)
{
    SYLLABLE syllables[MAX_WORD_LENGTH + 1];
    TCHAR full_wl_name[MAX_PATH];
    TCHAR *ci_str, *py_str;
    int ci_length, syllable_count;
    int freq, oper;
    int wl_id, wl_reload;
    int i, ret_code;

    if (!update_items)
        return WLE_ITEMS;

    if (!wl_name)
        return WLE_NAME;

    //如果所更新的词库为用户词库，则需要在UserApp目录中提取
    if (!_tcscmp(wl_name, WORDLIB_USER_SHORT_NAME))
        GetFileFullName(TYPE_USERAPP, WORDLIB_NAME_PREFIX, full_wl_name);
    else
        GetFileFullName(TYPE_ALLAPP, WORDLIB_NAME_PREFIX, full_wl_name);

    if (_tcslen(full_wl_name) + _tcslen(wl_name) > _SizeOf(full_wl_name) - 1)
        return WLE_NAME;        //长度不足

    _tcscat_s(full_wl_name, _SizeOf(full_wl_name), wl_name);

    wl_reload = 0;

    //TODO:需要考虑动态更新词库时，增加新的词库
    //首先加载内存中的词库文件
    wl_id = LoadWordLibrary(full_wl_name);
    if (wl_id == -1)        //无法装载
    {
        wl_id = LoadWordLibraryWithExtraLength(full_wl_name, WORDLIB_NORMAL_EXTRA_LENGTH, 0);
        if (wl_id == -1)
            return WLE_NAME;

        wl_reload = 1;
    }

    ret_code = 1;
    for (i = 0; i < item_count; i++)
    {
        freq   = update_items[i].freq;
        oper   = update_items[i].operation;
        ci_str = update_items[i].ci_string;
        py_str = update_items[i].py_string;

        if (freq < 0)
        {
            if (flag & WLF_HALT_ON_ERROR)
            {
                ret_code = WLE_FREQ;
                break;
            }

            continue;
        }

        if (!freq && oper != WLUP_OPER_UPDATE)
            freq = USER_BASE_FREQ;

        syllable_count = ParsePinYinStringReverse(py_str, syllables, MAX_WORD_LENGTH, 0, PINYIN_QUANPIN/*, 0, 0*/);

        if (!syllable_count)        //音节错误
        {
            if (flag & WLF_HALT_ON_ERROR)
            {
                ret_code = WLE_YIN;
                break;
            }
            continue;
        }

        ci_length = (int)_tcslen(ci_str);

        //判断词是否都是汉字
        if (!IsAllCanInLibrary((HZ*)ci_str, ci_length))
        {
            if (flag & WLF_HALT_ON_ERROR)
            {
                ret_code = WLE_CI;
                break;
            }

            continue;
        }

        if (ci_length > MAX_WORD_LENGTH)
        {
            if (flag & WLF_HALT_ON_ERROR)
            {
                ret_code = WLE_CI;
                break;
            }

            continue;
        }

        if (syllable_count > MAX_WORD_LENGTH)
        {
            if (flag & WLF_HALT_ON_ERROR)
            {
                ret_code = WLE_YIN;
                break;
            }

            continue;
        }

        if (oper == WLUP_OPER_ADD)
        {
            if (GetCiInWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count))
                continue; //已经存在

            if (AddCiToWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count, freq))
                continue;

            //可能词库满，重新装载词库
            SaveWordLibrary(wl_id);                 //保存词库数据
            CloseWordLibrary(wl_id);                //关闭词库
            wl_id = LoadWordLibraryWithExtraLength(full_wl_name, WORDLIB_NORMAL_EXTRA_LENGTH, 0);
            if (wl_id == -1)                        //词库出错，无法更新
                return WLE_OTHER;

            wl_reload = 1;
            if (AddCiToWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count, freq))
                continue;

            ret_code = WLE_OTHER;
            break;                                  //还是出错，无法继续
        }
        else if (oper == WLUP_OPER_DEL)
        {
            //被删除的词在用户词库中存储，所以要在用户词库中添加以及删除
            AddCiToWordLibrary(GetUserWordLibId(), (HZ*)ci_str, ci_length, syllables, syllable_count, freq);
            DeleteCiFromWordLib(GetUserWordLibId(), (HZ*)ci_str, ci_length, syllables, syllable_count, 0);

            continue;
        }
        else if (oper == WLUP_OPER_UPDATE)
        {   //更新词频
            WORDLIBITEM *item = GetCiInWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count);
            if (!item)  //没有找到
            {
                if (flag & WLF_HALT_ON_ERROR)
                {
                    ret_code = WLE_NOCI;
                    break;
                }

                continue;
            }
            item->freq = freq;

            if (wl_id == default_resource->user_wordlib_id)
                default_resource->user_wl_modified = 1;

            continue;
        }
    }

    SaveWordLibrary(wl_id);                 //保存词库数据

    if (wl_reload)                          //只有加载了新的词库才需要释放
        CloseWordLibrary(wl_id);            //关闭词库

    return ret_code;
}

/*!
 * \brief 装载全部词库文件。
 *  其中，用户词库与系统词库为默认加载；外挂词库基于config中的设定进行加载
 * \return
 *      0：失败，一般为系统词库或者用户词库设置错误
 *      1：成功，但可能存在某些外挂词库错误的情况
 */
int LoadAllWordLibraries()
{
    int i, has_syslib = 0;
    TCHAR name[MAX_PATH];
    int used_wordlib[MAX_WORDLIBS + 1];

    Log(LOG_ID, L"-1, pim_config=%p, wc=%d", pim_config, pim_config->wordlib_count);
    Log(LOG_ID, L"装载词库资源文件");

    if (!pim_config->wordlib_count)
        return 0;

    Log(LOG_ID, L"0");
    memset(used_wordlib, 0, sizeof(used_wordlib));

    //装载用户词库文件，一般不会失败，如果没有则创建一个用户词库文件
    GetFileFullName(TYPE_USERAPP, pim_config->wordlib_name[0], name);
    Log(LOG_ID, L"0.5");

    if (0 > LoadUserWordLibrary(name))      //失败
        ;//     return 0;                   //如果失败的话，也要进行下面的装载，否则可能发生丢失词库现象。

    used_wordlib[0] = 1;

    //检查系统词库是否被加载
    for (i = 1; i < pim_config->wordlib_count; i++)
    {
        if (!_tcscmp(pim_config->wordlib_name[i], WORDLIB_SYS_FILE_NAME))
        {
            has_syslib = 1;
            break;
        }
    }

    //如果系统词库没有被加载，就加载之，避免出现异常导致系统词库没有加载，而无法输入常用词
    if ((!has_syslib) && (pim_config->wordlib_count < MAX_WORDLIBS))
    {
        _tcscpy_s(pim_config->wordlib_name[pim_config->wordlib_count], MAX_FILE_NAME_LENGTH, WORDLIB_SYS_FILE_NAME);
        pim_config->wordlib_count++;
    }

    for (i = 1; i < pim_config->wordlib_count; i++)
    {
        int id;
        GetFileFullName(TYPE_USERAPP, pim_config->wordlib_name[i], name);
        if ((id = LoadWordLibrary(name)) >= 0)
            used_wordlib[id] = 1;
    }

    //此时要处理不再需要加载的词库文件
    for (i = 0; i < MAX_WORDLIBS; i++)
        if (default_resource->wordlib_name[i][0])
            default_resource->wordlib_deleted[i] = !used_wordlib[i];

    return 1;
}

static int wordlib_modified[MAX_WORDLIBS * 2];

int DeleteACiFromAllWordlib(TCHAR *ci_str, int ci_length, SYLLABLE *syllables, int syllable_length, int exclude_userlib)
{
    int wordlib_id, count = 0;

    //在内存中的全部词库中查找候选
    wordlib_id = GetNextWordLibId(-1);
    while (wordlib_id != -1)
    {
        if (exclude_userlib && wordlib_id == default_resource->user_wordlib_id)
        {
            wordlib_id = GetNextWordLibId(wordlib_id);
            continue;
        }

        if (DeleteCiFromWordLib(wordlib_id, ci_str, ci_length, syllables, syllable_length, 1) > 0)
        {
            wordlib_modified[wordlib_id] = 1;

            count++;
        }

        wordlib_id = GetNextWordLibId(wordlib_id);
    };

    return count;
}

/*!
 * \brief 从词库中删除词，不可恢复
 * \param  ci      词条
 * \param  length  长度
 * \return
 *      成功：删除的词条数
 *      失败：-1
 */
int WINAPI __stdcall DeleteCiFromAllWordLib(TCHAR *ci_str, int ci_length, TCHAR *py_str, int py_length)
{
    int i, userlib_id, count = 0;
    SYLLABLE syllables[MAX_WORD_LENGTH + 1];

    //初始化词库修改标记数组
    for (i = 0; i < MAX_WORDLIBS; i++)
        wordlib_modified[i] = 0;

    //删除单个词
    if (ci_str && ci_length && py_str && py_length)
    {
        int syllable_length = ParsePinYinStringReverse(py_str, syllables, MAX_WORD_LENGTH, 0, PINYIN_QUANPIN/*, 0, 0*/);
        if (!syllable_length)
            return -1;

        count = DeleteACiFromAllWordlib(ci_str, ci_length, syllables, syllable_length, 0);
    }
    //删除被标记删除的词
    else
    {
        WORDLIB *wordlib;
        WORDLIBITEM *item;
        PAGE *page;

        //找到用户词库
        userlib_id = GetUserWordLibId();
        if (userlib_id == -1)
            return -1;

        wordlib = GetWordLibrary(userlib_id);
        if (!wordlib)
            return -1;

        for (i = 0; i < wordlib->header.page_count; i++)
        {
            //遍历页表
            for (page = &wordlib->pages[i], item = (WORDLIBITEM*)page->data;
                 (char*)item < (char*)&page->data + page->data_length;
                 item = GetNextCiItem(item, userlib_id))
            {
                if (!item->effective && (item->syllable[0].con != CON_NULL || item->syllable[0].vow != VOW_NULL))
                {
                    //从其他词库里删除无效的词
                    count += DeleteACiFromAllWordlib(GetItemHZPtr(item), item->ci_length, item->syllable, item->syllable_length, 1);

                    //删除用户词库里的词
                    item->syllable[0].con           = CON_NULL;
                    item->syllable[0].vow           = VOW_NULL;
                    wordlib_modified[userlib_id]    = 1;
                    default_resource->user_wl_modified = 1;
                }
            }
        }
    }

    //保存修改过的词库
    for (i = 0; i < MAX_WORDLIBS; i++)
    {
        if (!wordlib_modified[i])
            continue;

        SaveWordLibrary(i);
        wordlib_modified[i] = 0;
    }

    return count;
}

