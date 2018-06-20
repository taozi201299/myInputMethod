package com.keyboard.keyboarddemo.service;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.support.annotation.Nullable;

import com.keyboard.keyboarddemo.ndk.JniUtil;

/**
 * Created by jidan on 17-7-6.
 */
public class PinyinDecodeService extends Service {

    JniUtil util;
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        System.out.println("onBind.....");
        IBinder result = null;
        if ( null == result ) result = new ServiceBinder() ;
        return result;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        util = new JniUtil();
    }

    //此方法是为了可以在Acitity中获得服务的实例

    public  class ServiceBinder extends Binder {
        public PinyinDecodeService getService() {
            return PinyinDecodeService.this;
        }
    }
    public void initPinyinEngine(){
        String path = "/sdcard/";
        util.setAssetsPath(path);
        util.initialize();
    }
    public  String getCLanguageString(){
        return null;
    }
    public  int getCandidateNum(){
        return util.getCandidateNum();
    }
    public  int getCandidateCount(){
        return util.getCandidateCount();
    }
    public  void InputKey(String key){
        util.InputKey(key);
    }
    public  String getCandidate(int iIndex, int iBufLen){
        return util.getCandidate(iIndex,iBufLen);
    }
    public  void clearContext(){
        util.clearContext();
    }
    public  void submitWord(int iIndex){util.submitWord(iIndex);}
    public  String getComposeString(int type){
       return  util.getComposeString(type);
    }
    public  int getState(){
        return util.getState();
    }
    public  int getSelectCountNum(){return  util.getSelectCountNum();}
    public  void setAssetsPath(String name){util.setAssetsPath(name);}

}
