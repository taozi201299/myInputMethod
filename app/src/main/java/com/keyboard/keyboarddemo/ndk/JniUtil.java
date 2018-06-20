package com.keyboard.keyboarddemo.ndk;

import android.graphics.Point;

import java.util.ArrayList;

/**
 * Created by jidan on 17-7-6.
 */

public class JniUtil  {
    public native String getCLanguageString();
    public native int getCandidateNum();
    public native int getCandidateCount();
    public native void InputKey(String key);
    public native void initialize();
    public native String getCandidate(int iIndex, int iBufLen);
    public native void clearContext();
    public native void submitWord(int iIndex);
    public native String getComposeString(int type);
    public native int getState();
    public native int getSelectCountNum();
    public native void setAssetsPath(String name);
    public  native  void hwrengine_init();
    public  native ArrayList<String> hwrGetCandidate(ArrayList<Point> points);

    static {
        System.loadLibrary("JniLibName");   //defaultConfig.ndk.moduleName
    }
}