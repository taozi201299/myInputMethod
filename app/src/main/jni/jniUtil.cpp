//
// Created by jidan on 17-7-6.
//
#include "com_keyboard_keyboarddemo_ndk_JniUtil.h"
#include "./include/jniUtil.h"
#include "./share/hwrlineapi.h"
#include <stdlib.h>
#include <fcntl.h>
#include <android/log.h>
#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>

void *filehandle = NULL;
int (*hwrInitialize)(char *szDictPath);
void(*hwrDone)();
int(*hwrLineDo)(
        signed short   *pHwrData,
        signed int      nArrDataSize, //数据short型数据长度
        unsigned short *pwWords,
        signed int		nArrWordSize,
        unsigned char  *pbRam,
        signed int      nRamSize,
        unsigned short  wHwrMode,
        unsigned short  wRecFlag);

#ifdef __cplusplus
extern "C"
{
#endif
/*
 * Class:     io_github_yanbober_ndkapplication_NdkJniUtils
 * Method:    getCLanguageString
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_getCLanguageString
        (JNIEnv *env, jobject obj){

    jstring strRet = env->NewStringUTF("HelloWorld from JNI !");
    char p[10] = "jni test";
    jstring  jstring1 = env->NewStringUTF(p);
    return jstring1;
}

JNIEXPORT jint  JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_getCandidateNum
        (JNIEnv *env, jobject obj){

    int num =  getCandidateNum();
    return num;

}
/**
 * jstring 和 wchat_t 的转换
 */
JNIEXPORT void JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_InputKey
(JNIEnv *env, jobject obj, jstring jstring1){
int len = env->GetStringLength(jstring1);
wchar_t *w_buffer = new wchar_t[len + 1];
memset(w_buffer,0,sizeof(wchar_t) * (len+1));
const jchar  * pjstr = env->GetStringChars(jstring1,0);
for(int i = 0; i <len;i++){
memcpy(&w_buffer[i],&pjstr[i],2);
}
InputKey(0,0,*w_buffer);
}

/*
 * Class:     example_my_preferencescreen_NdkJniUtils
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_initialize
(JNIEnv * env, jobject obj){
initialize();


}
JNIEXPORT jint JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_getCandidateCount
        (JNIEnv * env, jobject obj){
    int count = getCandidateCount();
    return count;

}

/**
 * wchat_t 和jstring的转换
 */
JNIEXPORT jstring JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_getCandidate
        (JNIEnv * env, jobject obj, jint index , jint iBufLen){
    wchar_t *w_buffer = new wchar_t[iBufLen];
    memset(w_buffer,0,sizeof(wchar_t) *iBufLen);
    getCandidateStr(w_buffer,index,iBufLen);
    size_t len = wcslen(w_buffer);
    jchar* str2 = (jchar*)malloc(sizeof(jchar)*(len+1));
    int i;
    for (i = 0; i < len; i++)
        str2[i] = w_buffer[i];
    str2[len] = 0;
    jstring js = env->NewString(str2, len);
    free(str2);
    return js;
}

JNIEXPORT void JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_clearContext
(JNIEnv * env, jobject obj){
reset();
}
JNIEXPORT void JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_submitWord
(JNIEnv * env, jobject obj,  jint index){
submitWord(index);
}
/*
 * Class:     example_my_preferencescreen_NdkJniUtils
 * Method:    getComposeString
 * Signature: (I)V
 */
JNIEXPORT jstring JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_getComposeString
        (JNIEnv * env, jobject obj, jint iType){
    int iBufLen = 40;
    wchar_t *w_buffer = new wchar_t[iBufLen];
    memset(w_buffer,0,iBufLen);
    getComposeStr(w_buffer,iBufLen,iType);
    size_t len = wcslen(w_buffer);
    jchar* str2 = (jchar*)malloc(sizeof(jchar)*(len+1));
    int i;
    for (i = 0; i < len; i++)
        str2[i] = w_buffer[i];
    str2[len] = 0;
    jstring js = env->NewString(str2, len);
    free(str2);
    return js;
}

/*
 * Class:     example_my_preferencescreen_NdkJniUtils
 * Method:    getState
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_getState
        (JNIEnv * env, jobject obj){
    return  getState();
}
/*
 * Class:     example_my_preferencescreen_NdkJniUtils
 * Method:    getSelectCountNum
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_getSelectCountNum
        (JNIEnv * env, jobject obj){
    getSelectCountNum();
}
JNIEXPORT void JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_setAssetsPath
(JNIEnv * env, jobject obj, jstring jstring1){
int len = env->GetStringLength(jstring1);
TCHAR *w_buffer = new TCHAR[len + 1];
memset(w_buffer,0,sizeof(TCHAR) * (len+1));
const jchar  * pjstr = env->GetStringChars(jstring1,0);
for(int i = 0; i <len;i++){
memcpy(&w_buffer[i],&pjstr[i],2);
}
 setPath(w_buffer);
}

/*
 * Class:     com_keyboard_keyboarddemo_ndk_JniUtil
 * Method:    hwrengine_init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_hwrengine_1init
        (JNIEnv *, jobject){
}

/*
 * Class:     com_keyboard_keyboarddemo_ndk_JniUtil
 * Method:    hwrGetCandidate
 * Signature: (Ljava/util/ArrayList;)Ljava/util/ArrayList;
 */
JNIEXPORT jobject JNICALL Java_com_keyboard_keyboarddemo_ndk_JniUtil_hwrGetCandidate
        (JNIEnv *, jobject, jobject){
    return 0;
}
#ifdef __cplusplus
}
#endif
