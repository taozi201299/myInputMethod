package com.keyboard.keyboarddemo.common;

/**
 * Created by jidan on 17-7-4.
 */

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.keyboard.keyboarddemo.R;
import com.keyboard.keyboarddemo.entity.BaseKeyEntity;


/**
 * Created by jidan on 17-2-6.
 */
public class BaseKey extends RelativeLayout  {
    public TextView key_top_title;

    public TextView getKey_top_title() {
        return key_top_title;
    }

    public KeyClick getKeyClick() {
        return keyClick;
    }

    public int getKeycode() {
        return keycode;
    }

    public ImageView getKey_img() {
        return key_img;
    }

    public TextView getKey_title() {
        return key_title;
    }



    public void setKey_img(ImageView key_img) {
        this.key_img = key_img;
    }

    public void setKey_title(TextView key_title) {
        this.key_title = key_title;
    }

    public void setKey_top_title(TextView key_top_title) {
        this.key_top_title = key_top_title;
    }

    public TextView key_title;
    public ImageView key_img;
    public int keycode;
    private KeyClick  keyClick;
    public boolean bLongPress;

    public boolean bKeyUp = false;
    private float m_lastX = 0;
    private float m_lastY = 0;

    private Runnable mLongPressRunnable = new Runnable() {
        @Override
        public void run() {
            bLongPress = true;
            if(!bKeyUp) {
                keyClick.processKeyEvent(BaseKey.this, MotionEvent.ACTION_DOWN);
                postDelayed(mLongPressRunnable,200);
            }

        }
    };

    public  interface KeyClick {
        void processKeyEvent(BaseKey key,int action);
    }
    public BaseKey(Context context,BaseKeyEntity entity){
        super(context);
        LayoutInflater.from(context).inflate(R.layout.base_key_layout, this);
        key_top_title = (TextView)findViewById(R.id.key_top_title);
        key_title =(TextView)findViewById(R.id.key_title);
        String strTopTitle = entity.getTopTitle();
        String strKeyTitle = entity.getKeyTitle();
        key_top_title.setText(strTopTitle);
        key_title.setText(strKeyTitle);
        keycode = entity.getKeycode();
        setBackgroundResource(R.drawable.base_key_drawable);
    }
    public BaseKey(Context context, AttributeSet attrs) {
        super(context, attrs);
        View view =  LayoutInflater.from(context).inflate(R.layout.base_key_layout, this);
        key_top_title = (TextView)findViewById(R.id.key_top_title);
        key_title =(TextView)findViewById(R.id.key_title);
        key_img = (ImageView)findViewById(R.id.key_img);
        TypedArray array = context.obtainStyledAttributes(attrs, R.styleable.BaseKey);
        String strTopTitle = array.getString(R.styleable.BaseKey_top_title);
        String strKeyTitle = array.getString(R.styleable.BaseKey_key_title);
        Integer resource =  array.getResourceId(R.styleable.BaseKey_key_back_ground, R.drawable.base_key_drawable);
        setBackgroundResource(resource);
        Integer color = array.getColor(R.styleable.BaseKey_text_color, getResources().getColor(R.color.black));
        key_top_title.setText(strTopTitle);
        key_title.setText(strKeyTitle);
        key_title.setTextColor(color);
        key_title.setTextSize(array.getInteger(R.styleable.BaseKey_text_size, 15));
        Integer imgSrc = array.getResourceId(R.styleable.BaseKey_key_img, -1);
        if(imgSrc != -1) {
            key_img.setVisibility(VISIBLE);
            key_img.setImageResource(imgSrc);
        }
        array.recycle();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        Log.d("==================","touch");
        switch (event.getAction()){
            case MotionEvent.ACTION_DOWN:
                bLongPress = false;
                bKeyUp = false;
                m_lastX = event.getX();
                m_lastY = event.getY();
                postDelayed(mLongPressRunnable,500);
                break;
            case MotionEvent.ACTION_UP :
                m_lastX = 0;
                m_lastY = 0;
                bKeyUp = true;
                break;
            case MotionEvent.ACTION_MOVE:
                if(Math.abs(event.getX() - m_lastX) > 10 || Math.abs(event.getY() - m_lastY) >10){
                    Log.d("----------------------","pppppppppppppppppppppp");

                }else {
                    return true;
                }
                break;

        }
        keyClick.processKeyEvent(this,event.getAction());
        return true;
    }

    public void setListener(KeyClick listener){
        this.keyClick = listener;
    }



}
