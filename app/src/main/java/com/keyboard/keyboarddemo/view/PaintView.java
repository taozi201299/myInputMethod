package com.keyboard.keyboarddemo.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.View;

import com.keyboard.keyboarddemo.interfaces.IHwrBoardView;
import com.keyboard.keyboarddemo.interfaces.IkeyBoardView;
import com.kzime.kzimehwr;


/**
 * Created by jidan on 17-7-12.
 */
public class PaintView extends View  implements Runnable{
    private Paint mPaint;
    private Path mPath;
    private Bitmap mBitmap;
    private Canvas mCanvas;

    private int screenWidth,screenHeight;
    private float currentX,currentY;


    private int 	nPointNum;
    private short[] mTracePoints;
    //Clear Trace values
    public Handler mTraceRunner;
    private boolean bTraceReady;
    private final static int MAX_TRACE_POINT_NUM = 1024;

    //Recognizer
    private kzimehwr mfbRecognizer;
    private String 	strWordsResults ="";
    private char 	wCharSet;
    private byte 	hwrdictpath[];
    private int 	nResHWR;

    IHwrBoardView iHwrBoardView;

    public  PaintView(Context context,int screenWidth,int screenHeight){
        super(context);
        this.screenWidth = screenWidth;
        this.screenHeight = screenHeight;
        init();
    }
    private void init(){
        mPaint = new Paint();
        mPaint.setAntiAlias(true);
        mPaint.setStrokeWidth(5);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setColor(Color.BLACK);

        mPath = new Path();
        mBitmap = Bitmap.createBitmap(screenWidth,screenHeight, Bitmap.Config.ARGB_8888);
        mCanvas = new Canvas(mBitmap);

        mTracePoints = new short[MAX_TRACE_POINT_NUM*2];
        nPointNum = 0;
        bTraceReady = false;
        mfbRecognizer = new kzimehwr();
        mTraceRunner= new Handler();

        //init the recognizer!
        hwrdictpath = new byte[512];
        hwrdictpath[0] = 0;
        nResHWR = mfbRecognizer.FBLInitDict(hwrdictpath);

        if (nResHWR < 0) { 	//errors goes there!
        }
        //Caller Can change the charset if you want!
        wCharSet = kzimehwr.KZIME_HANZI_CHAR_ALL;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.drawBitmap(mBitmap,0,0,null);
        canvas.drawPath(mPath, mPaint);
    }
    public void onDestroy()	{
        //free resource
        mfbRecognizer.FBLUnInitDict();
    }
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int x = (int)event.getX();
        int  y = (int) event.getY();
        switch (event.getAction()){
            case MotionEvent.ACTION_DOWN:
                removeCallbacks(clearRunnable);
                AddPoint(x, y);
                currentX = x;
                currentY = y;
                mPath.moveTo(currentX,currentY);
                invalidate();
                break;
            case MotionEvent.ACTION_MOVE:
                removeCallbacks(clearRunnable);
                currentX = x;
                currentY = y;
                AddPoint(x, y);
                if(mPath.isEmpty()){
                    mPath.moveTo(currentX,currentY);
                }else {
                    mPath.lineTo(currentX, currentY);
                }
                invalidate();
                break;
            case MotionEvent.ACTION_UP:
                currentY = 0;
                currentX = 0;
                AddPoint(x, y);
                AddPoint(-1, 0);
                mCanvas.drawPath(mPath,mPaint);
                postDelayed(clearRunnable,1000);
                break;
        }
        return true;
    }
    public Path getPath(){
        return mPath;
    }
    public void clear(){
        if(mCanvas != null){
            mPath.reset();
            mCanvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
            invalidate();
        }
        bTraceReady = true;
        mTraceRunner.postDelayed(this, 0);//1s
    }
    Runnable clearRunnable = new Runnable() {
        @Override
        public void run() {
            clear();
        }
    };

    private final void AddPoint(int x, int y)
    {
        int offset = nPointNum*2;
        if(nPointNum+1<MAX_TRACE_POINT_NUM)
        {
            mTracePoints[offset+0] = (short)x;
            mTracePoints[offset+1] = (short)y;
            nPointNum++;
        }
    }

    public final void run()
    {
        if (bTraceReady)
        {
            // Add Charcter trace end flag
            this.AddPoint(-1, -1);

            //////////////////////////////////
            char[] wRecoResults = new char[512];
            int candnb = 0;
            //NOTE!
            //wRecoResults[0] = 1 for high frequent character adjust
            wRecoResults[0] = 1;
            candnb = mfbRecognizer.FBLRecognize(mTracePoints, nPointNum*2, kzimehwr.HWR_MODE_LINEWRITE, kzimehwr.KZIME_HANZI_CHAR_ALL, wRecoResults);

            //empty the results! and than conver the recognition result to it!
            //wRecoResults[candnb] = 0;
            strWordsResults = "";
            for(int i = 0 ; i< wRecoResults.length ; i ++) {
                if (wRecoResults[i] != 0) {
                    strWordsResults += wRecoResults[i];
                }
            }
            iHwrBoardView.candidateFinish(strWordsResults);
            //empty the trace data
            nPointNum = 0;
            bTraceReady = false;
        }
    }
    public void setListener(IHwrBoardView iHwrBoardView){
       this.iHwrBoardView = iHwrBoardView;
    }
}
