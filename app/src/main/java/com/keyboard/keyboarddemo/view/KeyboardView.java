package com.keyboard.keyboarddemo.view;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Color;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.keyboard.keyboarddemo.R;
import com.keyboard.keyboarddemo.common.BaseKey;
import com.keyboard.keyboarddemo.common.Common;
import com.keyboard.keyboarddemo.common.InputMethodManagent;
import com.keyboard.keyboarddemo.entity.BaseKeyEntity;
import com.keyboard.keyboarddemo.interfaces.IkeyBoardView;
import com.keyboard.keyboarddemo.skbContainer;

/**
 * Created by jidan on 17-7-4.
 */
public class KeyboardView extends LinearLayout implements BaseKey.KeyClick,View.OnClickListener {
    private LinearLayout row2;
    private LinearLayout row1;
    private LinearLayout row3;
    private String[][] keyArrayRow1 = {{"1", "Q"}, {"2", "W"}, {"3", "E"}, {"4", "R"}, {"5", "T"}, {"6", "Y"}, {"7", "U"}, {"8", "I"}, {"9", "O"}, {"0", "P"}};
    private int[] keycodeRow1 = {KeyEvent.KEYCODE_Q, KeyEvent.KEYCODE_W, KeyEvent.KEYCODE_E, KeyEvent.KEYCODE_R, KeyEvent.KEYCODE_R, KeyEvent.KEYCODE_T,
            KeyEvent.KEYCODE_Y, KeyEvent.KEYCODE_U, KeyEvent.KEYCODE_I, KeyEvent.KEYCODE_O, KeyEvent.KEYCODE_P};
    private String[][] keyArrayRow2 = {{"~", "A"}, {"!", "S"}, {"@", "D"}, {"#", "F"}, {"%", "G"}, {"{", "H"}, {"}", "J"}, {"*", "K"}, {"?", "L"}};
    private int[] keycodeRow2 = {KeyEvent.KEYCODE_A, KeyEvent.KEYCODE_S, KeyEvent.KEYCODE_D, KeyEvent.KEYCODE_F, KeyEvent.KEYCODE_G, KeyEvent.KEYCODE_H,
            KeyEvent.KEYCODE_J, KeyEvent.KEYCODE_K, KeyEvent.KEYCODE_L};
    private String[][] keyArrayRow3 = {{"(", "Z"}, {")", "X"}, {".", "C"}, {"_", "V"}, {":", "B"}, {";", "N"}, {"/", "M"}};
    private int[] keycodeRow3 = {KeyEvent.KEYCODE_Z, KeyEvent.KEYCODE_X, KeyEvent.KEYCODE_C, KeyEvent.KEYCODE_V, KeyEvent.KEYCODE_B, KeyEvent.KEYCODE_N, KeyEvent.KEYCODE_M};
    private Context mContext;
    private IkeyBoardView ikeyBoardView;

    public KeyboardView(Context context) {
        super(context);
        mContext = context;
        LayoutInflater.from(context).inflate(R.layout.keyboard_view_layout, this);
        initView();
        initListener();
    }

    private void initListener() {

        BaseKey numKey = (BaseKey) findViewById(R.id.numKey);
        numKey.setListener(this);
        BaseKey commaKey = (BaseKey) findViewById(R.id.commaKey);
        commaKey.setListener(this);
        BaseKey spaceKey = (BaseKey) findViewById(R.id.spaceKey);
        spaceKey.setListener(this);
        BaseKey pointKey = (BaseKey) findViewById(R.id.pointKey);
        pointKey.setListener(this);
        BaseKey baseKey = (BaseKey) findViewById(R.id.enterKey);
        baseKey.setListener(this);
        ((BaseKey) findViewById(R.id.lanKey)).setListener(this);
        ((BaseKey) findViewById(R.id.id_row3_shift_en)).setListener(this);
        ((BaseKey) findViewById(R.id.id_row3_split_zh)).setListener(this);
        ((BaseKey) findViewById(R.id.id_row3_backspace)).setListener(this);
        findViewById(R.id.id_btn_pinyin).setOnClickListener(this);
        findViewById(R.id.id_btn_shouxie).setOnClickListener(this);
        findViewById(R.id.id_btn_en).setOnClickListener(this);
        findViewById(R.id.id_img_hide).setOnClickListener(this);

    }

    public void initView() {

        row1 = (LinearLayout) findViewById(R.id.row1);
        row2 = (LinearLayout) findViewById(R.id.row2);
        row3 = (LinearLayout) findViewById(R.id.row3_letter);
        int size0 = keyArrayRow1.length;
        int size = keyArrayRow2.length;
        int size1 = keyArrayRow3.length;
        if (row1.getChildCount() != 0) {
            row1.removeAllViews();
        }
        for (int i = 0; i < size0; i++) {
            BaseKeyEntity entity = new BaseKeyEntity();
            entity.setKeyTitle(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH || InputMethodManagent.isLock || InputMethodManagent.isShift ? keyArrayRow1[i][1] : keyArrayRow1[i][1].toLowerCase());
            entity.setTopTitle(keyArrayRow1[i][0]);
            entity.setKeycode(keycodeRow1[i]);
            BaseKey baseKey = new BaseKey(mContext, entity);
            baseKey.setListener(this);
            LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
            if(Common.orientation == Configuration.ORIENTATION_PORTRAIT)
            layoutParams.setMargins(3, 0, 0, 0);
            else if(Common.orientation == Configuration.ORIENTATION_LANDSCAPE){
                layoutParams.setMargins(4,0,0,0);
            }
            baseKey.setLayoutParams(layoutParams);
            row1.addView(baseKey);

        }
        if (row2.getChildCount() != 0) {
            row2.removeAllViews();
        }
        for (int i = 0; i < size; i++) {
            BaseKeyEntity entity = new BaseKeyEntity();
            entity.setKeyTitle(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH || InputMethodManagent.isLock || InputMethodManagent.isShift ? keyArrayRow2[i][1] : keyArrayRow2[i][1].toLowerCase());
            entity.setTopTitle(keyArrayRow2[i][0]);
            entity.setKeycode(keycodeRow2[i]);
            BaseKey baseKey = new BaseKey(mContext, entity);
            LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
            if(Common.orientation == Configuration.ORIENTATION_PORTRAIT)
            layoutParams.setMargins(3, 0, 0, 0);
            else  if(Common.orientation == Configuration.ORIENTATION_LANDSCAPE){
                layoutParams.setMargins(4,0,0,0);
            }
            baseKey.setLayoutParams(layoutParams);
            baseKey.setListener(this);

            row2.addView(baseKey);
        }
        if (row3.getChildCount() != 0) {
            row3.removeAllViews();
        }
        for (int i = 0; i < size1; i++) {
            BaseKeyEntity entity = new BaseKeyEntity();
            entity.setKeyTitle(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH || InputMethodManagent.isLock || InputMethodManagent.isShift ? keyArrayRow3[i][1] : keyArrayRow3[i][1].toLowerCase());
            entity.setTopTitle(keyArrayRow3[i][0]);
            entity.setKeycode(keycodeRow3[i]);
            BaseKey baseKey = new BaseKey(mContext, entity);

            LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
           if(Common.orientation == Configuration.ORIENTATION_PORTRAIT)
            layoutParams.setMargins(3, 0, 0, 0);
            else  if(Common.orientation == Configuration.ORIENTATION_LANDSCAPE)
           {
               layoutParams.setMargins(4,0,0,0);
           }
            baseKey.setLayoutParams(layoutParams);
            baseKey.setListener(this);

            row3.addView(baseKey);
        }
        if (InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH) {
            findViewById(R.id.id_row3_split_zh).setVisibility(VISIBLE);
            findViewById(R.id.id_row3_shift_en).setVisibility(GONE);
            ((BaseKey) findViewById(R.id.lanKey)).getKey_title().setText("中");
            ((BaseKey)findViewById(R.id.commaKey)).getKey_title().setText("，");
            ((BaseKey)findViewById(R.id.pointKey)).getKey_title().setText("。");
        } else {
            findViewById(R.id.id_row3_split_zh).setVisibility(GONE);
            findViewById(R.id.id_row3_shift_en).setVisibility(VISIBLE);
            ((BaseKey) findViewById(R.id.lanKey)).getKey_title().setText("En");
            ((BaseKey)findViewById(R.id.commaKey)).getKey_title().setText(",");
            ((BaseKey)findViewById(R.id.pointKey)).getKey_title().setText(".");
            if (InputMethodManagent.isShift) {
                if (InputMethodManagent.isLock) {
                    ((BaseKey) findViewById(R.id.id_row3_shift_en)).getKey_img().setImageResource(R.mipmap.shift_daxie_suoding1);
                } else {
                    ((BaseKey) findViewById(R.id.id_row3_shift_en)).getKey_img().setImageResource(R.mipmap.shift_daxie_1);
                }

            } else {
                ((BaseKey) findViewById(R.id.id_row3_shift_en)).getKey_img().setImageResource(R.mipmap.shift_xiaoxie1);
            }
        }

        if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH){
            findViewById(R.id.id_btn_pinyin).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
            findViewById(R.id.id_btn_en).setBackground(null);
            findViewById(R.id.id_btn_shouxie).setBackground(null);
        }else {
            findViewById(R.id.id_btn_en).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
            findViewById(R.id.id_btn_pinyin).setBackground(null);
            findViewById(R.id.id_btn_shouxie).setBackground(null);
        }

    }
    public void setListener(IkeyBoardView ikeyBoardView) {
        this.ikeyBoardView = ikeyBoardView;
    }

    @Override
    public void processKeyEvent(BaseKey key, int action) {
        ikeyBoardView.ProcessKey(key, action);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.id_btn_pinyin:
                findViewById(R.id.id_btn_pinyin).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_en).setBackground(null);
                findViewById(R.id.id_btn_shouxie).setBackground(null);
                if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH && InputMethodManagent.keyboad == InputMethodManagent.Keyboad.PINYIN){
                    return;
                }else {
                    InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                }
                break;
            case R.id.id_btn_en:
                findViewById(R.id.id_btn_en).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_pinyin).setBackground(null);
                findViewById(R.id.id_btn_shouxie).setBackground(null);
                if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_EN){
                    return;
                }else {
                    InputMethodManagent.lan = InputMethodManagent.Lan.LAN_EN;

                }
                break;
            case  R.id.id_img_hide:
                break;
            case R.id.id_btn_shouxie:
                findViewById(R.id.id_btn_shouxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_pinyin).setBackground(null);
                findViewById(R.id.id_btn_en).setBackground(null);
                if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH && InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                    return;
                }
                InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                break;
        }
        ikeyBoardView.ProcessTitleBtn(v);
    }

    public void updateEnterIcon(EditorInfo editorInfo){
        int action = editorInfo.imeOptions
                & (EditorInfo.IME_MASK_ACTION | EditorInfo.IME_FLAG_NO_ENTER_ACTION);
        ((BaseKey)findViewById(R.id.enterKey)).key_img.setImageResource(R.mipmap.enter);
        ((BaseKey)findViewById(R.id.enterKey)).key_title.setText("");
        if(action == editorInfo.IME_ACTION_GO) {
            ((BaseKey) findViewById(R.id.enterKey)).key_img.setImageResource(0);
            ((BaseKey) findViewById(R.id.enterKey)).key_title.setText("去往");
        }else if(action == editorInfo.IME_ACTION_NEXT){
            ((BaseKey)findViewById(R.id.enterKey)).key_img.setImageResource(0);
            ((BaseKey)findViewById(R.id.enterKey)).key_title.setText("下一步");
            ((BaseKey)findViewById(R.id.enterKey)).key_title.setTextSize(11);

        }else if(action == editorInfo.IME_ACTION_DONE){
            ((BaseKey)findViewById(R.id.enterKey)).key_img.setImageResource(0);
            ((BaseKey)findViewById(R.id.enterKey)).key_title.setText("完成");

        }else if(action == editorInfo.IME_ACTION_SEARCH){
            ((BaseKey)findViewById(R.id.enterKey)).key_img.setImageResource(0);
            ((BaseKey)findViewById(R.id.enterKey)).key_title.setText("搜索");

        }else if(action == editorInfo.IME_ACTION_SEND){
            ((BaseKey)findViewById(R.id.enterKey)).key_img.setImageResource(0);
            ((BaseKey)findViewById(R.id.enterKey)).key_title.setText("发送");

        }

    }
    public void  setTitleShown (boolean bShown) {
        if (bShown) {
            findViewById(R.id.title).setVisibility(VISIBLE);
        } else {
            findViewById(R.id.title).setVisibility(GONE);
        }
    }

}
