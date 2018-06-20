package com.keyboard.keyboarddemo.pinyin.setup;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.View;

import com.keyboard.keyboarddemo.R;

/**
 * Created by jidan on 17-7-6.
 */
public class SettingsActivity extends Activity implements View.OnClickListener {

    public static   String INPUT_METHOD = "com.keyboard.keyboarddemo/.PinyinIME";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String ss= Settings.Secure.getString(SettingsActivity.this.getContentResolver(),
                Settings.Secure.DEFAULT_INPUT_METHOD);
        Log.d("11111111111111111111111",ss);
        if(!ss.equals(INPUT_METHOD)){
            gotoActivity();
            return;
        }


        setContentView(R.layout.keyboard_setting_layout);
        findViewById(R.id.id_uninstall).setOnClickListener(this);
    }

    public void unInstallApp(){
        Intent uninstall_intent = new Intent();
        uninstall_intent.setAction(Intent.ACTION_DELETE);
        uninstall_intent.setData(Uri.parse("package:" + getPackageName()));
        startActivity(uninstall_intent);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.id_uninstall:
                unInstallApp();
                break;
            default:
                break;
        }
    }
    private void gotoActivity(){
        Intent intent = new Intent();
        intent.setClass(SettingsActivity.this,SetupActivity.class);
        startActivity(intent);
        finish();
    }
}
