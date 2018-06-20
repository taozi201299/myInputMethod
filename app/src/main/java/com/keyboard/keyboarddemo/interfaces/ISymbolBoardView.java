package com.keyboard.keyboarddemo.interfaces;

import com.keyboard.keyboarddemo.common.BaseKey;
import com.keyboard.keyboarddemo.skbContainer;

/**
 * Created by jidan on 17-7-10.
 */
public interface ISymbolBoardView{
    void ProcessKey(int id,int code,skbContainer.KeyboardModel keyboardModel);
    void ProcessKey(int id,String text,skbContainer.KeyboardModel keyboardModel);
}
