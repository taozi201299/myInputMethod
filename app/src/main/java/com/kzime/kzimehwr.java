package com.kzime;

public class kzimehwr
{
    public static final char KZIME_CHAR_NUMBER =0x0001;
    public static final char KZIME_CHAR_UPPER  =0x0002;
    public static final char KZIME_CHAR_LOWER  =0x0004;
    public static final char KZIME_CHAR_PUNCT1 =0x0008;
    public static final char KZIME_CHAR_PUNCT2 =0x0010;

    public static final char KZIME_CHAR_ALPHA  =  (KZIME_CHAR_UPPER|KZIME_CHAR_LOWER);
    public static final char KZIME_CHAR_PUNCT  =   (KZIME_CHAR_PUNCT1|KZIME_CHAR_PUNCT2);
    public static final char KZIME_CHAR_ALL    =  (KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT|KZIME_CHAR_NUMBER);

    public static final char KZIME_HANZI_GB1   =0x0100;
    public static final char KZIME_HANZI_GB2   =0x0200;
    public static final char KZIME_HANZI_BIG5  =0x0400;
    public static final char KZIME_HANZI_GBK   =0x0800;
    public static final char KZIME_HANZI_RADICAL =0x1000;  //部首

    public static final char KZIME_HANZI_GB2312	=(KZIME_HANZI_GB1|KZIME_HANZI_GB2);
    public static final char KZIME_HANZI_ALL	=(KZIME_HANZI_GB2312|KZIME_HANZI_BIG5|KZIME_HANZI_GBK);

    public static final char KZIME_HANZI_CHAR_ALL =(KZIME_CHAR_ALL |KZIME_HANZI_GBK); //所有字符+符号
    public static final char KZIME_RECO_DEFAUT_GB =(KZIME_CHAR_NUMBER|KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT1|KZIME_HANZI_GB2312);
    public static final char KZIME_RECO_DEFAUT_BIG=(KZIME_CHAR_NUMBER|KZIME_CHAR_ALPHA|KZIME_CHAR_PUNCT1|KZIME_HANZI_BIG5);


    //////////////////////////////////////////////////////////////////////////
    /******************************************************************************
     *用户设定的书写模式
     ******************************************************************************/
    //1 单字识别模式
    public static final char HWR_MODE_SINGLECHAR =0x0001;
    //2 行识别模式
    public static final char HWR_MODE_LINEWRITE	 =0x0002;
    //3 叠写模式，目前不支持
    public static final char HWR_MODE_OVERLAP 	 = 0x0004;
    //4 数字串模式，目前不支持
    public static final char  HWR_MODE_NUMSTRING =0x0080;

    //初始化，对于识别核心so，已经包含了字典，所以fileName，可以是任意路径，但是不能为空。

    public native int    FBLInitDict(byte fileName[]);
    //卸载识别核心，释放内存
    public native void   FBLUnInitDict();
    //行识别
    public native int	 FBLRecognize( short []lpTrace, int lDataSize,
                                        char wHwrMode, //输入模式
                                        char wRecFlag, //单字符识别标记，HWR_MODE_SINGLECHAR模式有效
                                        char []lpResult); //返回的识别结果


    static {
        System.loadLibrary("fbhwrlineo");
    }
}