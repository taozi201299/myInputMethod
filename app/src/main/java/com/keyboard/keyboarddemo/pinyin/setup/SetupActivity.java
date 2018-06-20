package com.keyboard.keyboarddemo.pinyin.setup;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewDebug;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;
import android.widget.Toast;

import com.keyboard.keyboarddemo.R;
import com.keyboard.keyboarddemo.pinyin.setup.SettingsActivity;


/**
 * Created by jidan on 17-7-27.
 */
public class SetupActivity extends Activity implements View.OnClickListener{
    private TextView btn_choose;
    private TextView btn_switch;
    private TextView btn_finish;
    private Handler mHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.switch_setting_layout);
        btn_choose = (TextView)findViewById(R.id.id_btn_choose);
        btn_switch = (TextView)findViewById(R.id.id_btn_switch);
        btn_finish = (TextView)findViewById(R.id.id_btn_finish);
        btn_choose.setOnClickListener(this);
        btn_switch.setOnClickListener(this);
        btn_finish.setOnClickListener(this);
        initUI();
        mHandler = new Handler();


    }
    Runnable modifyUI = new Runnable() {
        @Override
        public void run() {
             initUI();
        }
    };
    public void initUI(){
        String strEnableMethods= Settings.Secure.getString(SetupActivity.this.getContentResolver(),
                Settings.Secure.ENABLED_INPUT_METHODS);
        String strCurrentMethod = Settings.Secure.getString(SetupActivity.this.getContentResolver(),
                Settings.Secure.DEFAULT_INPUT_METHOD);
        Log.d("11111111111111111111111", strEnableMethods);
        Log.d("22222222222222222222222",strCurrentMethod);
        if(strEnableMethods.contains(SettingsActivity.INPUT_METHOD)){
            if(strCurrentMethod.equals(SettingsActivity.INPUT_METHOD)){
                btn_choose.setEnabled(false);
                btn_finish.setEnabled(true);
                btn_switch.setEnabled(false);
                btn_choose.setAlpha(Float.valueOf("0.5"));
                btn_switch.setAlpha(Float.valueOf("0.5"));
                btn_finish.setAlpha(Float.valueOf("1.0"));
                btn_choose.setTextColor(R.color.black);
                btn_switch.setTextColor(R.color.black);
            }else {
                btn_choose.setEnabled(false);
                btn_switch.setEnabled(true);
                btn_finish.setEnabled(false);
                btn_choose.setAlpha(Float.valueOf("0.5"));
                btn_finish.setAlpha(Float.valueOf("0.5"));
                btn_switch.setAlpha(Float.valueOf("1.0"));
                btn_choose.setTextColor(R.color.black);
                btn_switch.setTextColor(R.color.white);
            }
        }else {
            btn_choose.setEnabled(true);
            btn_switch.setEnabled(false);
            btn_finish.setEnabled(false);
            btn_choose.setAlpha(Float.valueOf("1.0"));
            btn_finish.setAlpha(Float.valueOf("0.5"));
            btn_switch.setAlpha(Float.valueOf("0.5"));
            btn_choose.setTextColor(R.color.white);
            btn_switch.setTextColor(R.color.black);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {

          mHandler.postDelayed(modifyUI,100);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d("==================","pause");
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.id_btn_choose:
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.showInputMethodPicker();
                break;
            case R.id.id_btn_switch:
                InputMethodManager imm1 = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm1.showInputMethodPicker();
                break;
            case R.id.id_btn_finish:
                finish();
                break;
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        Log.d("000000000000","touch");
        switch (event.getAction()){
            case MotionEvent.ACTION_UP:
                String ss= Settings.Secure.getString(SetupActivity.this.getContentResolver(),
                        Settings.Secure.DEFAULT_INPUT_METHOD);
                if(ss.equals(SettingsActivity.INPUT_METHOD)){
                    btn_choose.setEnabled(false);
                    btn_finish.setEnabled(true);
                    btn_choose.setAlpha(Float.valueOf("0.5"));
                    btn_finish.setAlpha(Float.valueOf("1.0"));
                    btn_switch.setEnabled(false);
                    btn_switch.setAlpha(Float.valueOf("0.5"));
                }else {
                    Toast.makeText(this,"请选择我的输入法",Toast.LENGTH_LONG);
                }
        }
        return true;
    }
}
