package com.keyboard.keyboarddemo.view;

import android.content.Context;
import android.content.res.Configuration;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.keyboard.keyboarddemo.R;
import com.keyboard.keyboarddemo.common.BaseKey;
import com.keyboard.keyboarddemo.common.InputMethodManagent;
import com.keyboard.keyboarddemo.interfaces.IHwrBoardView;
import com.keyboard.keyboarddemo.interfaces.IkeyBoardView;

/**
 * Created by jidan on 17-7-12.
 */
public class CanvasView extends LinearLayout implements BaseKey.KeyClick,View.OnClickListener{
    private Context mContext;
    private View view;
    private FrameLayout m_frameLayout;
    private PaintView mPaintView;
    private IkeyBoardView ikeyBoardView;
    private IHwrBoardView iHwrBoardView;


    public CanvasView(Context context) {
        super(context);
        this.mContext = context;
        view = LayoutInflater.from(context).inflate(R.layout.shouxie_layout,this);
        m_frameLayout = (FrameLayout)view.findViewById(R.id.id_canvas_keyboard);
        if(context.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT)
        mPaintView = new PaintView(mContext, 140, 80);
        else{
            mPaintView = new PaintView(mContext,560,80);
        }
        m_frameLayout.addView(mPaintView);
        mPaintView.requestFocus();
        init();
    }
    private void init(){
        findViewById(R.id.id_btn_shouxie_shouxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
        findViewById(R.id.id_btn_pinyin_shoxie).setBackground(null);
        findViewById(R.id.id_btn_en_shouxie).setBackground(null);
        ((BaseKey)view.findViewById(R.id.numKey_shouxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.lanKey_shouxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.spaceKey_shouxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.enterKey_shouxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.commaKey_showxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.pointKey_showxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.symbolKey_showxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.symbolKey1_shouxie)).setListener(this);
        ((BaseKey)view.findViewById(R.id.deleteKey_showxie)).setListener(this);
        findViewById(R.id.id_btn_pinyin_shoxie).setOnClickListener(this);
        findViewById(R.id.id_btn_en_shouxie).setOnClickListener(this);
        findViewById(R.id.id_btn_shouxie_shouxie).setOnClickListener(this);
        findViewById(R.id.id_img_hide_shouxie).setOnClickListener(this);
    }

    @Override
    public void processKeyEvent(BaseKey key, int action) {
        ikeyBoardView.ProcessKey(key, action);

    }
    public void setListener(IkeyBoardView ikeyBoardView,IHwrBoardView iHwrBoardView) {
        this.ikeyBoardView = ikeyBoardView;
        this.iHwrBoardView = iHwrBoardView;
        mPaintView.setListener(iHwrBoardView);
    }
    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.id_btn_pinyin_shoxie:
                findViewById(R.id.id_btn_pinyin_shoxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_en_shouxie).setBackground(null);
                findViewById(R.id.id_btn_shouxie_shouxie).setBackground(null);
                if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH && InputMethodManagent.keyboad == InputMethodManagent.Keyboad.PINYIN){
                    return;
                }else {
                    InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                }
                break;
            case R.id.id_btn_en_shouxie:
                findViewById(R.id.id_btn_en_shouxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_pinyin_shoxie).setBackground(null);
                findViewById(R.id.id_btn_shouxie_shouxie).setBackground(null);
                if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_EN){
                    return;
                }else {
                    InputMethodManagent.lan = InputMethodManagent.Lan.LAN_EN;
                }
                break;
            case  R.id.id_img_hide_shouxie:
                break;
            case R.id.id_btn_shouxie_shouxie:
                findViewById(R.id.id_btn_shouxie_shouxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_pinyin_shoxie).setBackground(null);
                findViewById(R.id.id_btn_en_shouxie).setBackground(null);
                if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH && InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                    return;
                }
                InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;

                break;
        }
        ikeyBoardView.ProcessTitleBtn(v);
    }
    public void initTitle(){
        if(InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH){
            if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.PINYIN){
                findViewById(R.id.id_btn_pinyin_shoxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_shouxie_shouxie).setBackground(null);
                findViewById(R.id.id_btn_en_shouxie).setBackground(null);
            }else  if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                findViewById(R.id.id_btn_shouxie_shouxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
                findViewById(R.id.id_btn_pinyin_shoxie).setBackground(null);
                findViewById(R.id.id_btn_en_shouxie).setBackground(null);
            }
        }else {
            findViewById(R.id.id_btn_en_shouxie).setBackground(getResources().getDrawable(R.mipmap.letter_120x74));
            findViewById(R.id.id_btn_pinyin_shoxie).setBackground(null);
            findViewById(R.id.id_btn_shouxie_shouxie).setBackground(null);
        }
    }
}
