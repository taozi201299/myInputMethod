/*----------------------------------------------------------------
Copyright (c) 2011-2012 Faybee Technologies Co., Ltd.
"fbhwrline.h"	ʶ������ͷ�ļ� 
-----------------------------------------------------------------*/
#ifndef __HWR_LINE_API_H__
#define __HWR_LINE_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

//////////////////////////////////////////////////////////////////////////
//�ַ�������
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
#define KZIME_HANZI_RADICAL 0x1000  //����

#define KZIME_HANZI_GB2312	(KZIME_HANZI_GB1|KZIME_HANZI_GB2)
#define KZIME_HANZI_ALL		(KZIME_HANZI_GB2312|KZIME_HANZI_BIG5|KZIME_HANZI_GBK)

#define KZIME_HANZI_CHAR_ALL	(KZIME_CHAR_ALL |KZIME_HANZI_GBK) //�����ַ�+����
#define KZIME_RECO_DEFAUT_GB	(KZIME_CHAR_NUMBER|KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT1|KZIME_HANZI_GB2312)
#define KZIME_RECO_DEFAUT_BIG	(KZIME_CHAR_NUMBER|KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT1|KZIME_HANZI_BIG5)

//////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *�û��趨����дģʽ
 ******************************************************************************/
//1 ����ʶ��ģʽ
#define HWR_MODE_SINGLECHAR		0x0001
//2 ��ʶ��ģʽ
#define HWR_MODE_LINEWRITE		0x0002
//3 ��дģʽ��Ŀǰ��֧��
#define HWR_MODE_OVERLAP		0x0004
//4 ���ִ�ģʽ��Ŀǰ��֧��
#define HWR_MODE_NUMSTRING		0x0080

//////////////////////////////////////////////////////////////////////////
//Data types
//////////////////////////////////////////////////////////////////////////
//�����ӿ�
/************************************************************************\    
Function:     
	����ʶ�������ģ��һ���ʼ��������ʱһ��Ŀ¼����������ʹ�á�
Parameters:
	szDictPath		    [I]  kzimehwr.dict��charbi.mod����Ŀ¼������������'/'
Returns:
	��ȷ����0���������<0.
\************************************************************************/
int		FBHWRL_Init(char *szDictPath);

/************************************************************************\    
Function:     
    �ͷŵ��ֺ�����ģ���ֵ䡣
Parameters:
	��
Returns:
    ��ȷ����0���������<0.
\************************************************************************/
void    FBHWRL_Done();

/************************************************************************\    
Function:     
    ��ʶ����Ҫ���ú�����������Ĺ켣����ʶ��
Parameters:
    pHwrData			[I]		����켣����(-1, 0)�ʻ��ָ(-1,-1)�ʻ�����
	nArrDataSize		[I]		pHwrData���ݳ���
	pwWords				[O]		�ʺ�ѡ���ؿռ䣬��ѡ��֮���ÿո������unicode����
	nArrWordSize		[I]		��ź�ѡ�����鳤��
	pbRam				[I]		������ʶ����ڴ�ռ�
	nRamSize			[I]		������ʶ����ڴ�ռ��Сbyte�ֽ���
	wHwrMode			[I]		ʶ��ģʽ HWR_MODE_XXX
	wRecFlag			[I]		����ʶ���ַ���ѡ��
Returns:
    ��ȷ����>=0����ʾ��ѡ�ʵĸ���
	�������<0.
\************************************************************************/
int		FBHWRL_LineReco(signed short   *pHwrData,
					    signed int      nArrDataSize, //����short�����ݳ���
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