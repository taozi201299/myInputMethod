package com.keyboard.keyboarddemo;

import android.content.Context;
import android.inputmethodservice.InputMethodService;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.LinearLayout;

import com.keyboard.keyboarddemo.common.BaseKey;
import com.keyboard.keyboarddemo.common.Common;
import com.keyboard.keyboarddemo.common.InputMethodManagent;
import com.keyboard.keyboarddemo.interfaces.IHwrBoardView;
import com.keyboard.keyboarddemo.interfaces.ISymbolBoardView;
import com.keyboard.keyboarddemo.interfaces.IkeyBoardView;
import com.keyboard.keyboarddemo.view.CanvasView;
import com.keyboard.keyboarddemo.view.KeyboardView;
import com.keyboard.keyboarddemo.view.NumberBoardView;
import com.keyboard.keyboarddemo.view.SymbolBoardView;

/**
 * Created by jidan on 17-7-4.
 */
public class skbContainer extends LinearLayout implements IkeyBoardView ,ISymbolBoardView,IHwrBoardView {

    private KeyboardView keyboardView;
    private SymbolBoardView symbolboardView;
    private NumberBoardView numberBoardView;
    private CanvasView canvasBoardView;

    public static KeyboardModel model = null;
    private EditorInfo m_EditorInfo;
    private InputMethodService service;

    private boolean bNeedChange = false;
    private Context m_Context;

    public static InputMethodManagent.Lan lan = InputMethodManagent.Lan.LAN_ZH;

    @Override
    public void ProcessKey(int id, int code, KeyboardModel keyboardModel) {
        ((PinyinIME)service).ProcessEmoji(id, code, keyboardModel);
    }

    @Override
    public void ProcessKey(int id, String text, KeyboardModel keyboardModel) {
        ((PinyinIME)service).ProcessKey(id, text, keyboardModel);
    }

    @Override
    public void candidateFinish(String results) {
        ((PinyinIME)service).hwrCandidate(results);
    }

    public  enum KeyboardModel {
        PinyinKeyboard("0"),
        SymbolKeyboard("1"),
        SymbolNumKeyboard("2"),
        NumberKeyboard("3"),
        CanvasKeyboard("4");

        private String result;
        KeyboardModel(String result){this.result = result;}
    }



    public skbContainer(Context context) {
        super(context);
        m_Context = context;
        keyboardView = new KeyboardView(context);
        symbolboardView = new SymbolBoardView(context);
        numberBoardView = new NumberBoardView(context);
        canvasBoardView = new CanvasView(context);

        keyboardView.setListener(this);
        symbolboardView.setListener(this);
        canvasBoardView.setListener(this, this);

    }

    @Override
    public void ProcessKey(BaseKey key, int action) {
        ((PinyinIME)service).ProcessKey(key, action);
    }

    @Override
    public void ProcessTitleBtn(View v) {
        ((PinyinIME)service).ProcessTitleKey(v);
    }

    public View updateView(){
        View view = null;
        switch (model){
            case PinyinKeyboard:
                keyboardView =  new KeyboardView(m_Context);
                keyboardView.setListener(this);
                view = keyboardView;
                break;
            case SymbolKeyboard:
            case SymbolNumKeyboard:
                symbolboardView = new SymbolBoardView(m_Context);
                symbolboardView.setListener(this);
                view = symbolboardView;
                break;
            case NumberKeyboard:
                numberBoardView = new NumberBoardView(m_Context);
                view = numberBoardView;
                break;
            case CanvasKeyboard:
                canvasBoardView = new CanvasView(m_Context);
                canvasBoardView.setListener(this,this);
                if (bNeedChange) {
                        canvasBoardView.initTitle();
                }
                view = canvasBoardView;
                break;
        }
        bNeedChange = false;
        return  view;
    }

    public View getView(){
        return  symbolboardView;
    }
    public void setKeyboardChange(boolean change){
        this.bNeedChange = change;

    }
    /**
     * 修改enter按钮的icon
     */
    public void updateEnterIcon(){
        keyboardView.updateEnterIcon(m_EditorInfo);

    }
    public void updateIcon(){

    }
    public void setModel(KeyboardModel keyboardModel){
        if(keyboardModel == null) {
            switch (m_EditorInfo.inputType & EditorInfo.TYPE_MASK_CLASS) {
                case EditorInfo.TYPE_CLASS_NUMBER:
                case EditorInfo.TYPE_CLASS_DATETIME:
                    InputMethodManagent.lan = InputMethodManagent.Lan.LAN_EN;
                    model = KeyboardModel.SymbolKeyboard;
                    InputMethodManagent.keyboad = InputMethodManagent.Keyboad.EN;
                    break;
                case EditorInfo.TYPE_CLASS_PHONE:
                    InputMethodManagent.lan = InputMethodManagent.Lan.LAN_EN;
                    model = KeyboardModel.NumberKeyboard;
                    InputMethodManagent.keyboad = InputMethodManagent.Keyboad.EN;
                    break;
                case EditorInfo.TYPE_CLASS_TEXT:
                    int v = m_EditorInfo.inputType & EditorInfo.TYPE_MASK_VARIATION;
                    if (v == EditorInfo.TYPE_TEXT_VARIATION_EMAIL_ADDRESS
                            || v == EditorInfo.TYPE_TEXT_VARIATION_PASSWORD
                            || v == EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD
                            || v == EditorInfo.TYPE_TEXT_VARIATION_URI) {
                        InputMethodManagent.lan = InputMethodManagent.Lan.LAN_EN;
                        InputMethodManagent.keyboad = InputMethodManagent.Keyboad.EN;
                    }else {
                        InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                        model = KeyboardModel.PinyinKeyboard;
                        if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.PINYIN){
                            model = KeyboardModel.PinyinKeyboard;
                        }else if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                            model = KeyboardModel.CanvasKeyboard;
                        }else {
                            InputMethodManagent.keyboad = InputMethodManagent.Keyboad.PINYIN;
                        }
                    }

                    break;
                default:
                    InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                    model = KeyboardModel.PinyinKeyboard;
                    if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.PINYIN){
                        model = KeyboardModel.PinyinKeyboard;
                    }else if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                        model = KeyboardModel.CanvasKeyboard;
                    }else {
                        InputMethodManagent.keyboad = InputMethodManagent.Keyboad.PINYIN;
                    }
                    break;
            }
        } else {
            this.model = keyboardModel;
        }
    }

    public void setTitleShown(boolean bShow){keyboardView.setTitleShown(bShow);}
    public void setLan(InputMethodManagent.Lan lan){
        this.lan = lan;
    }
    public void setService(InputMethodService service){
          this.service = service;
    }
    public void setEditorInfo(EditorInfo editorInfo){
        m_EditorInfo = editorInfo;
    }
    public KeyboardModel getModel(){
        return this.model;
    }

}
