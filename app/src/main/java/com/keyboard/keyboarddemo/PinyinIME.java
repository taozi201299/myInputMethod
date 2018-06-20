package com.keyboard.keyboarddemo;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.inputmethodservice.InputMethodService;
import android.os.Handler;
import android.os.IBinder;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.style.ImageSpan;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

import com.keyboard.keyboarddemo.common.BaseFunctionKey;
import com.keyboard.keyboarddemo.common.BaseKey;
import com.keyboard.keyboarddemo.common.Common;
import com.keyboard.keyboarddemo.common.InputMethodManagent;
import com.keyboard.keyboarddemo.service.PinyinDecodeService;


import java.util.ArrayList;


/**
 * Created by jidan on 17-7-4.
 */
public class PinyinIME extends InputMethodService {

    private LinearLayout m_keyboardContainer;
    private skbContainer skbContainer;
    PopupWindow tipWin;
    private PinyinDecodeService connService;
    private View m_CandidateView;
    private TextView m_Spell;
    private HorizontalScrollView m_CandidateScorll;
    private ImageView imageView;
    ArrayList<String>m_Candidates = new ArrayList<>();
    private boolean bExtendCandidate = false;
    private Handler m_handler ;
    @Override
    public void onCreate() {
        super.onCreate();
        skbContainer = new skbContainer(this);
        skbContainer.setService(this);
        Intent intent = new Intent(this, PinyinDecodeService.class);
        /** 进入Activity开始服务 */
        bindService(intent, conn, Context.BIND_AUTO_CREATE);
        m_handler = new Handler();
        Log.d("1111111111111111111111111", "oncreate");
    }
    private ServiceConnection conn = new ServiceConnection() {
        /** 获取服务对象时的操作 */
        public void onServiceConnected(ComponentName name, IBinder service) {
            connService = ((PinyinDecodeService.ServiceBinder) service).getService();
            connService.initPinyinEngine();

        }

        /** 无法获取到服务对象时的操作 */
        public void onServiceDisconnected(ComponentName name) {
            // TODO Auto-generated method stub
            connService = null;
        }
    };
    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public View onCreateInputView() {
        Log.d("1111111111111111111111111","onCreateInputView");
        LayoutInflater inflater = getLayoutInflater();
        View view  = inflater.inflate(R.layout.keyboard_container, null);
        m_keyboardContainer = (LinearLayout)view.findViewById(R.id.id_keyboard_container);
        return m_keyboardContainer;
    }

    // TODO: 17-7-13 candidateView 和keyboard 合并在一起

    @Override
    public View onCreateCandidatesView() {
        Log.d("555555555555","onCreateCandidatesView");
        LayoutInflater inflater = getLayoutInflater();
        m_CandidateView= inflater.inflate(R.layout.candidate_view_layout,null);
        m_Spell = (TextView)m_CandidateView.findViewById(R.id.id_spell);
        m_CandidateScorll = (HorizontalScrollView)m_CandidateView.findViewById(R.id.id_candidate_word);
        imageView = (ImageView)m_CandidateView.findViewById(R.id.id_extend_candidate);
        imageView.setVisibility(View.GONE);
        imageView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!bExtendCandidate) {
                    extendCandidate();
                    imageView.setImageResource(R.mipmap.up_arrow);
                } else {
                    hideCandidate();
                    imageView.setImageResource(R.mipmap.down_arrow);
                }
            }
        });
        setCandidatesViewShown(false);
       return m_CandidateView;
    }

    public void onConfigurationChanged(Configuration newConfig) {
        if(skbContainer.getVisibility() == View.INVISIBLE){
            return;
        }
        skbContainer.removeAllViews();

        Log.d("22222222222222222", "configchange");
        Common.configurationChange = true;
        String message=newConfig.orientation==Configuration.ORIENTATION_LANDSCAPE ? "屏幕设置为：横屏" : "屏幕设置为：竖屏";
        Common.orientation = newConfig.orientation;

        Toast.makeText(this, message, Toast.LENGTH_LONG).show();

        m_handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                updateView();
                clearContext();
            }
        },800);


    }

    /**
     * 先于onStartInputView 调用
     * @param attribute
     * @param restarting
     */
    @Override
    public void onStartInput(EditorInfo attribute, boolean restarting) {
        skbContainer.setEditorInfo(attribute);
        skbContainer.setModel(null);
        skbContainer.updateIcon();
        skbContainer.updateEnterIcon();
    }

    @Override
    public void onFinishInput() {
        super.onFinishInput();
    }

    @Override
    public void onStartInputView(EditorInfo info, boolean restarting) {
        skbContainer.setEditorInfo(info);
        /**
         * 根据输入框的设置 决定哪种类型的键盘
         */
        skbContainer.setModel(null);
        skbContainer.updateIcon();
        /**
         * 根据输入框的设置,修改enterkey的text显示
         */
        skbContainer.updateEnterIcon();
        /**
         * 10.16 根据输入框的设置 需要重新刷新键盘
         */
        skbContainer.setKeyboardChange(true);
        updateView();
    }
    @Override
    public void onFinishInputView(boolean finishingInput) {
        super.onFinishInputView(finishingInput);
    }
    private void  updateView(){
        View view =  skbContainer.updateView();
        m_keyboardContainer.removeAllViews();
        m_keyboardContainer.addView(view);

    }
    public void ProcessKey(BaseKey key, int action) {
        popUpTips(key, action);
        if(action == MotionEvent.ACTION_UP) {
            switch (key.getId()) {
                case R.id.numKey:
                case R.id.numKey_shouxie:
                    clearContext();
                    skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.SymbolKeyboard);
                    updateView();
                    break;
                case R.id.commaKey:
                case R.id.commaKey_showxie:
                    InputConnection ic0 = getCurrentInputConnection();
                    ic0.commitText(key.getKey_title().getText().toString(), 0);
                    break;
                case R.id.spaceKey:
                case R.id.spaceKey_shouxie:
                    if(m_Spell.getText().toString().length() == 0){
                        getCurrentInputConnection().sendKeyEvent(
                                new KeyEvent(KeyEvent.ACTION_UP,
                                        KeyEvent.KEYCODE_SPACE));
                        getCurrentInputConnection().sendKeyEvent(
                                new KeyEvent(KeyEvent.ACTION_DOWN,
                                        KeyEvent.KEYCODE_SPACE));
                    }else {
                        InputConnection ic = getCurrentInputConnection();
                        ic.commitText(m_Candidates.get(0), 0);
                        clearContext();
                    }
                    break;
                case R.id.id_row3_backspace:
                case R.id.deleteKey_showxie:
                    if(m_Spell.getText().toString().length() == 0){
                        getCurrentInputConnection().sendKeyEvent(
                                new KeyEvent(KeyEvent.ACTION_UP,
                                        KeyEvent.KEYCODE_DEL));
                        getCurrentInputConnection().sendKeyEvent(
                                new KeyEvent(KeyEvent.ACTION_DOWN,
                                        KeyEvent.KEYCODE_DEL));

                    }else {
                        connService.InputKey("8");
                        if (connService.getComposeString(0).length() != 0)
                            updateCandidate();
                        else {
                            clearContext();
                        }
                    }
                    break;
                case R.id.pointKey:
//                    commitCandidate(key.getKey_title().getText().toString());
//                    break;
                case R.id.pointKey_showxie:
                case R.id.symbolKey1_shouxie:
                case R.id.symbolKey_showxie:
                    InputConnection ic = getCurrentInputConnection();
                    ic.commitText(key.getKey_title().getText().toString(),0);
                    break;
                case R.id.enterKey:
                case R.id.enterKey_shouxie:
                    if(m_Spell.getText().toString().length() == 0){
                        sendKeyChar('\n');
                    }else {
                        InputConnection ic1 = getCurrentInputConnection();
                        ic1.commitText(m_Spell.getText().toString(), 0);
                        clearContext();
                    }
                    break;
                case R.id.lanKey:
                case R.id.lanKey_shouxie:
                    InputMethodManagent.isLock = false;
                    InputMethodManagent.isShift = false;
                    clearContext();
                    if (InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH) {
                        InputMethodManagent.lan = InputMethodManagent.Lan.LAN_EN;
                        skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.PinyinKeyboard);
                        skbContainer.setKeyboardChange(true);
                    } else {
                        InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                        if (InputMethodManagent.keyboad == InputMethodManagent.Keyboad.PINYIN) {
                            skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.PinyinKeyboard);
                            skbContainer.setKeyboardChange(true);
                        } else if (InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE) {
                            skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.CanvasKeyboard);
                            skbContainer.setKeyboardChange(true);
                        }
                    }
                    updateView();
                    clearContext();
                    break;
                case R.id.id_row3_shift_en:
                    if (InputMethodManagent.isShift) {
                        if (InputMethodManagent.isLock) {
                            InputMethodManagent.isLock = false;
                            InputMethodManagent.isShift = false;
                        } else {
                            InputMethodManagent.isLock = true;
                        }
                    } else {
                        InputMethodManagent.isShift = true;
                        InputMethodManagent.isLock = false;
                    }
                    skbContainer.setKeyboardChange(true);
                    updateView();
                    break;
                case R.id.id_row3_split_zh:
                    processKey(key);
                    break;
                default:
                    processKey(key);
                    break;
            }
        }else if((action == MotionEvent.ACTION_DOWN  || action == MotionEvent.ACTION_MOVE)&& key.bLongPress == true){
            switch (key.getId()){
                case R.id.id_row3_backspace:
                case R.id.deleteKey_showxie:
                    if(m_Spell.getText().toString().length() == 0){
                        getCurrentInputConnection().sendKeyEvent(
                                new KeyEvent(KeyEvent.ACTION_UP,
                                        KeyEvent.KEYCODE_DEL));
                        getCurrentInputConnection().sendKeyEvent(
                                new KeyEvent(KeyEvent.ACTION_DOWN,
                                        KeyEvent.KEYCODE_DEL));

                    }else {
                        connService.InputKey("8");
                        if (connService.getComposeString(0).length() != 0)
                            updateCandidate();
                        else {
                            clearContext();
                        }
                    }
                    break;
            }
        }

    }
    public void ProcessKey(int id, String text,com.keyboard.keyboarddemo.skbContainer.KeyboardModel keyboardModel){
        if(keyboardModel == com.keyboard.keyboarddemo.skbContainer.KeyboardModel.SymbolKeyboard){
            switch (id){
                case R.id.id_symbol_return:
                    if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.PINYIN || InputMethodManagent.keyboad == InputMethodManagent.Keyboad.EN) {
                        skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.PinyinKeyboard);
                    }if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                        skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.CanvasKeyboard);
                    }
                    updateView();
                    break;
                case R.id.id_symbol_enter:
                    sendKeyChar('\n');
                    break;
                case R.id.id_symbol_space:
                    getCurrentInputConnection().sendKeyEvent(
                            new KeyEvent(KeyEvent.ACTION_UP,
                                    KeyEvent.KEYCODE_SPACE));
                    getCurrentInputConnection().sendKeyEvent(
                            new KeyEvent(KeyEvent.ACTION_DOWN,
                                    KeyEvent.KEYCODE_SPACE));
                    break;
                case R.id.id_symbol_del:
                    getCurrentInputConnection().sendKeyEvent(
                            new KeyEvent(KeyEvent.ACTION_UP,
                                    KeyEvent.KEYCODE_DEL));
                    getCurrentInputConnection().sendKeyEvent(
                            new KeyEvent(KeyEvent.ACTION_DOWN,
                                    KeyEvent.KEYCODE_DEL));
                    break;
                case BaseFunctionKey.KEY_COMMIT:
                    InputConnection ic = getCurrentInputConnection();
                    ic.commitText(text, 0);
                    break;
            }
        }
    }

    public void ProcessEmoji(int id, int code, com.keyboard.keyboarddemo.skbContainer.KeyboardModel keyboardModel){
        InputConnection ic = getCurrentInputConnection();
        ic.commitText(getEmojiStringByUnicode(code), 0);

    }
    private String getEmojiStringByUnicode(int unicode) {
        return new String(Character.toChars(unicode));
    }

    private void processKey(BaseKey key){
        int keycode = key.keycode;
        if(key.bLongPress && (keycode >= KeyEvent.KEYCODE_A && keycode <= KeyEvent.KEYCODE_Z)){
            InputConnection ic = getCurrentInputConnection();
            ic.commitText(key.key_top_title.getText().toString(), 0);
        }else {
            if (((keycode >= KeyEvent.KEYCODE_A && keycode <= KeyEvent.KEYCODE_Z) && InputMethodManagent.lan == InputMethodManagent.Lan.LAN_ZH)
                    || key.getId() == R.id.id_row3_split_zh) {
                connService.InputKey(key.key_title.getText().toString().toLowerCase());
                updateCandidate();

            } else if ((keycode >= KeyEvent.KEYCODE_A && keycode <= KeyEvent.KEYCODE_Z) && InputMethodManagent.lan == InputMethodManagent.Lan.LAN_EN) {
                InputConnection ic = getCurrentInputConnection();
                ic.commitText(key.getKey_title().getText().toString(), 0);
                if (!InputMethodManagent.isLock) {
                    InputMethodManagent.isShift = false;
                    skbContainer.setKeyboardChange(true);
                    skbContainer.updateView();
                }

            } else {
            }
        }
    }
    public void ProcessTitleKey(View v){
        switch (v.getId()){
            case R.id.id_btn_pinyin:
            case R.id.id_btn_pinyin_shoxie:
                clearContext();
                InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                InputMethodManagent.keyboad = InputMethodManagent.Keyboad.PINYIN;
                skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.PinyinKeyboard);
                skbContainer.setKeyboardChange(true);
                updateView();
                break;
            case R.id.id_btn_en:
            case R.id.id_btn_en_shouxie:
                clearContext();
                InputMethodManagent.lan = InputMethodManagent.Lan.LAN_EN;
                InputMethodManagent.keyboad = InputMethodManagent.Keyboad.EN;
                skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.PinyinKeyboard);
                skbContainer.setKeyboardChange(true);
                updateView();
                break;
            case R.id.id_btn_shouxie:
            case R.id.id_btn_shouxie_shouxie:
                clearContext();
                InputMethodManagent.lan = InputMethodManagent.Lan.LAN_ZH;
                InputMethodManagent.keyboad = InputMethodManagent.Keyboad.SHOUXIE;
                skbContainer.setModel(com.keyboard.keyboarddemo.skbContainer.KeyboardModel.CanvasKeyboard);
                skbContainer.setKeyboardChange(true);
                updateView();
                break;
            case R.id.id_img_hide:
            case R.id.id_img_hide_shouxie:
                clearContext();
                 hideWindow();
                break;
        }
    }
    private void updateCandidate(){
        setCandidatesViewShown(true);
        skbContainer.setTitleShown(true);

        imageView.setVisibility(View.VISIBLE);
        int count = connService.getCandidateCount();
        m_Candidates.clear();

        for(int i = 0; i < count ;i ++ ) {
            String str = connService.getCandidate(i,80);
            m_Candidates.add(str);

        }
        setCandidates(m_Candidates);
        updateSpell(0);
    }

    public void setCandidates(ArrayList<String> datas){
        m_CandidateScorll.removeAllViews();
        final LinearLayout layout=new LinearLayout(this);
        layout.setOrientation(LinearLayout.HORIZONTAL);
        layout.setPadding(25, 0, 0, 0);
        for(final  String word : datas) {
            final TextView textView = new TextView(this);
            int width = word.length() < 3? 100 : ViewGroup.LayoutParams.WRAP_CONTENT;
            textView.setLayoutParams(new ViewGroup.LayoutParams(width, 100));
            textView.setPadding(15, 25,5, 5);
            textView.setText(word);
            textView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                        InputConnection ic = getCurrentInputConnection();
                        ic.commitText(word, 0);
                        clearContext();
                    }else {
                        commitCandidate(((TextView) v).getText().toString());
                    }
                }
            });
            layout.addView(textView);

        }
        m_CandidateScorll.addView(layout);
        m_CandidateScorll.fullScroll(View.FOCUS_LEFT);
    }
    public  void commitCandidate(String word){
        int count = m_Candidates.size();
        int position = 0;
        for( position = 0 ; position <count ;position ++){
            if(m_Candidates.get(position).equals(word)){
                break;
            }
        }
        if(connService.getState() == 4){
            InputConnection ic = getCurrentInputConnection();
            ic.commitText(connService.getComposeString(1) + word, 0);
            clearContext();
        }else {
            connService.submitWord(position);
            updateSpell(0);
            updateCandidate();
        }
        if(connService.getState() == 4) {
            InputConnection ic = getCurrentInputConnection();
            ic.commitText(connService.getComposeString(1) + word, 0);
            clearContext();
        }
    }
    private  void updateSpell(int type){
        String spell = connService.getComposeString(type);
        m_Spell.setText(spell);
    }
    public void clearContext(){
        setCandidatesViewShown(false);
        if(bExtendCandidate){
            hideCandidate();
        }
        setCandidatesViewShown(false);
        connService.clearContext();
        if(m_Spell != null) m_Spell.setText("");
        if(m_Candidates != null)m_Candidates.clear();
        if(imageView != null)imageView.setVisibility(View.GONE);
        skbContainer.setTitleShown(true);
        if(m_CandidateScorll != null)m_CandidateScorll.removeAllViews();
    }
    private void popUpTips(View view,int action){
        int keycode = ((BaseKey)view).keycode;
        if(keycode < KeyEvent.KEYCODE_A || keycode > KeyEvent.KEYCODE_Z) return;
        String tipTitle = ((BaseKey)view).getKey_title().getText().toString();
        String tipOption = ((BaseKey)view).getKey_top_title().getText().toString();
        View tip ;
        TextView textView;
        if(tipWin == null) {
             tip = LayoutInflater.from(this).inflate(R.layout.key_tip_popwindow, null);
             textView = (TextView) tip.findViewById(R.id.id_tip_title);
        }else {
            tip = tipWin.getContentView();
            textView = (TextView)tip.findViewById(R.id.id_tip_title);

        }
        if(((BaseKey)view).bLongPress){
            textView.setText(tipOption);
        }else {
            textView.setText(tipTitle);
        }
        if(tipWin == null)
            tipWin = new PopupWindow(tip, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        tip.measure(View.MeasureSpec.UNSPECIFIED, View.MeasureSpec.UNSPECIFIED);
        int[] location = new int[2];
        int popupHeight = tip.getMeasuredHeight();
        view.getLocationInWindow(location);
        if(action == MotionEvent.ACTION_DOWN)
            tipWin.showAtLocation(view, Gravity.NO_GRAVITY, location[0],location[1] - popupHeight);
        else if(action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_MOVE){
            if(tipWin != null && tipWin.isShowing()) {
                new Handler().postDelayed(new Runnable(){
                    public void run() {
                        //execute the task
                        if(tipWin != null) {
                            tipWin.dismiss();
                            tipWin = null;
                        }
                    }
                }, 90);

            }
        }
    }

    // TODO: 17-10-11 shuiqingyuduyue 的问题
    private void extendCandidate(){
       bExtendCandidate = true;
        LayoutInflater inflater = getLayoutInflater();
        View view  = inflater.inflate(R.layout.candidate_extend_layout, null);
        LinearLayout layout = (LinearLayout)view.findViewById(R.id.id_extend_candidate_container);
        m_keyboardContainer.removeAllViews();
        m_keyboardContainer.addView(view);
        int index = 0;
        int count = m_Candidates.size();
        int len = 0;
        int listIndex = 0;

        ArrayList<LinearLayout>linearLayouts = new ArrayList<>();
        LinearLayout linearLayout = null ;
        for(int i = index; i < count; i ++){
            if(len == 0) {
                linearLayout = new LinearLayout(this);
                linearLayout.setOrientation(LinearLayout.HORIZONTAL);
                linearLayouts.add(linearLayout);
                listIndex ++;
            }
            final  String word = m_Candidates.get(i);
            int max ;
            if (Common.orientation == Configuration.ORIENTATION_PORTRAIT ) max = 6 ;
            else max = 4;
            if(m_Candidates.get(i).length() > max){
                final TextView textView = new TextView(this);
                int width = LinearLayout.LayoutParams.MATCH_PARENT;
                textView.setLayoutParams(new ViewGroup.LayoutParams(width, 100));
                textView.setPadding(5, 0, 5, 5);
                textView.setText(word);
                textView.setGravity(Gravity.CENTER);
                textView.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                            InputConnection ic = getCurrentInputConnection();
                            ic.commitText(word, 0);
                            clearContext();
                        }else {
                            commitCandidate(((TextView) v).getText().toString());
                            if (m_Candidates.size() != 0) {
                                extendCandidate();
                            }
                        }
                    }
                });
                linearLayout.addView(textView);
                layout.addView(linearLayouts.get(listIndex -1));
                index ++;
            }
            else {
                final TextView textView = new TextView(this);
                int width ;
                if(Common.orientation == Configuration.ORIENTATION_PORTRAIT)width = 180 ;
                else width = 220;
                textView.setLayoutParams(new ViewGroup.LayoutParams(width, 100));
                textView.setPadding(5, 0, 5, 5);
                textView.setText(word);
                textView.setGravity(Gravity.CENTER);
                textView.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if(InputMethodManagent.keyboad == InputMethodManagent.Keyboad.SHOUXIE){
                            InputConnection ic = getCurrentInputConnection();
                            ic.commitText(word, 0);
                            clearContext();
                        }else {
                            commitCandidate(((TextView) v).getText().toString());
                            if (m_Candidates.size() != 0) {
                                extendCandidate();
                            }
                        }
                      //  hideCandidate();
                    }
                });
                if(len <= max -1) {
                    linearLayout.addView(textView);
                    index++;
                    len++;
                }else {
                    len = 0;
                    layout.addView(linearLayouts.get(listIndex -1));
                }if(len == max){
                    len = 0;
                    layout.addView(linearLayouts.get(listIndex -1));
                }
            }

        }
        if(len!=0){
            layout.addView(linearLayouts.get(listIndex-1));
        }
    }
    private void hideCandidate(){
        bExtendCandidate = false;
        m_CandidateScorll.setVisibility(View.VISIBLE);
        imageView.setImageResource(R.mipmap.down_arrow);
        updateView();
    }
    void hwrCandidate(String results){
        m_Candidates.clear();
        setCandidatesViewShown(true);
        skbContainer.setTitleShown(true);
        String [] arrayWords = results.split(" ");
        for(String word :arrayWords){
            if(arrayWords != null){
                m_Candidates.add(word);
            }

        }
        imageView.setVisibility(View.VISIBLE);
        setCandidates(m_Candidates);
    }
}
