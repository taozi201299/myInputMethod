package com.keyboard.keyboarddemo.interfaces;

import android.view.View;

import com.keyboard.keyboarddemo.common.BaseKey;

/**
 * Created by jidan on 17-7-4.
 */
public interface IkeyBoardView {
    void ProcessKey(BaseKey key,int action);
    void ProcessTitleBtn(View v);
}
