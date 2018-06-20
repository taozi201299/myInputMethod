/*----------------------------------------------------------------
Copyright (c) 2011-2012 Faybee Technologies Co., Ltd.
"fbhwrline.h"	识别引擎头文件 
-----------------------------------------------------------------*/
#ifndef __HWR_LINE_API_H__
#define __HWR_LINE_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

//////////////////////////////////////////////////////////////////////////
//字符集定义
#define	KZIME_CHAR_NUMBER	0x0001
#define KZIME_CHAR_UPPER	0x0002
#define KZIME_CHAR_LOWER	0x0004
#define	KZIME_CHAR_PUNCT1	0x0008
#define KZIME_CHAR_PUNCT2	0x0010

#define KZIME_CHAR_ALPHA    (KZIME_CHAR_UPPER|KZIME_CHAR_LOWER)
#define KZIME_CHAR_PUNCT    (KZIME_CHAR_PUNCT1|KZIME_CHAR_PUNCT2)
#define KZIME_CHAR_ALL      (KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT|KZIME_CHAR_NUMBER)

#define	KZIME_HANZI_GB1		0x0100
#define	KZIME_HANZI_GB2		0x0200
#define KZIME_HANZI_BIG5	0x0400
#define KZIME_HANZI_GBK	    0x0800
#define KZIME_HANZI_RADICAL 0x1000  //部首

#define KZIME_HANZI_GB2312	(KZIME_HANZI_GB1|KZIME_HANZI_GB2)
#define KZIME_HANZI_ALL		(KZIME_HANZI_GB2312|KZIME_HANZI_BIG5|KZIME_HANZI_GBK)

#define KZIME_HANZI_CHAR_ALL	(KZIME_CHAR_ALL |KZIME_HANZI_GBK) //所有字符+符号
#define KZIME_RECO_DEFAUT_GB	(KZIME_CHAR_NUMBER|KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT1|KZIME_HANZI_GB2312)
#define KZIME_RECO_DEFAUT_BIG	(KZIME_CHAR_NUMBER|KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT1|KZIME_HANZI_BIG5)

//////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *用户设定的书写模式
 ******************************************************************************/
//1 单字识别模式
#define HWR_MODE_SINGLECHAR		0x0001
//2 行识别模式
#define HWR_MODE_LINEWRITE		0x0002
//3 叠写模式，目前不支持
#define HWR_MODE_OVERLAP		0x0004
//4 数字串模式，目前不支持
#define HWR_MODE_NUMSTRING		0x0080

//////////////////////////////////////////////////////////////////////////
//Data types
//////////////////////////////////////////////////////////////////////////
//函数接口
/************************************************************************\    
Function:     
	单字识别和语言模型一起初始化，输入时一个目录，仅供开发使用。
Parameters:
	szDictPath		    [I]  kzimehwr.dict和charbi.mod所在目录，不包括最后的'/'
Returns:
	正确返回0，其它情况<0.
\************************************************************************/
int		FBHWRL_Init(char *szDictPath);

/************************************************************************\    
Function:     
    释放单字和语言模型字典。
Parameters:
	空
Returns:
    正确返回0，其它情况<0.
\************************************************************************/
void    FBHWRL_Done();

/************************************************************************\    
Function:     
    行识别主要调用函数，对输入的轨迹进行识别。
Parameters:
    pHwrData			[I]		输入轨迹数据(-1, 0)笔画分割，(-1,-1)笔画结束
	nArrDataSize		[I]		pHwrData数据长度
	pwWords				[O]		词候选返回空间，候选词之间用空格隔开，unicode编码
	nArrWordSize		[I]		存放候选词数组长度
	pbRam				[I]		用于行识别的内存空间
	nRamSize			[I]		用于行识别的内存空间大小byte字节数
	wHwrMode			[I]		识别模式 HWR_MODE_XXX
	wRecFlag			[I]		单字识别字符集选择
Returns:
    正确返回>=0，表示候选词的个数
	其它情况<0.
\************************************************************************/
int		FBHWRL_LineReco(signed short   *pHwrData,
					    signed int      nArrDataSize, //数据short型数据长度
					    unsigned short *pwWords,
					    signed int		nArrWordSize,
					    unsigned char  *pbRam,
					    signed int      nRamSize,
					    unsigned short  wHwrMode,
					    unsigned short  wRecFlag);
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif //__HWR_LINE_API_H__