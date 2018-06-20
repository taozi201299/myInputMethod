package com.keyboard.keyboarddemo.common;

/**
 * Created by jidan on 17-7-4.
 */
public class InputMethodManagent {

    public enum Lan {
        LAN_ZH,
        LAN_EN
    }

        /**
         * 标记中文状态下，是拼音模式还是手写模式
         */
    public enum Keyboad {
        PINYIN,
        SHOUXIE,
        PINYIN_9,
        EN,
        EN_9
    }
    public static boolean isShift = false;
    public static boolean isLock = false;
    public static Lan lan = Lan.LAN_ZH;
    public static Keyboad keyboad = Keyboad.PINYIN;
}
