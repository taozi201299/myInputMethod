package com.keyboard.keyboarddemo.entity;

import java.io.Serializable;

/**
 * Created by jidan on 17-7-4.
 */
public class BaseKeyEntity implements Serializable {
    String topTitle;
    String keyTitle;
    String img;

    public int getKeycode() {
        return keycode;
    }

    public void setKeycode(int keycode) {
        this.keycode = keycode;
    }

    int keycode;

    public String getTopTitle() {
        return topTitle;
    }

    public void setTopTitle(String topTitle) {
        this.topTitle = topTitle;
    }

    public String getKeyTitle() {
        return keyTitle;
    }

    public void setKeyTitle(String keyTitle) {
        this.keyTitle = keyTitle;
    }

    public String getImg() {
        return img;
    }

    public void setImg(String img) {
        this.img = img;
    }
}
