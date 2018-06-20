package com.keyboard.keyboarddemo.view;

import android.content.Context;
import android.content.res.Configuration;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.HorizontalScrollView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.keyboard.keyboarddemo.R;
import com.keyboard.keyboarddemo.common.Common;

import java.util.ArrayList;

/**
 * Created by jidan on 17-7-4.
 */
public class CandidateView extends RelativeLayout {

    private HorizontalScrollView candidateScrollView;
    private TextView m_Spell;
    public CandidateView(Context context) {
        super(context);
        Log.d("99999999999999999999999999999","11111111111111111111");
        View view  = LayoutInflater.from(context).inflate(R.layout.candidate_view_layout,this);
        candidateScrollView = (HorizontalScrollView)view.findViewById(R.id.id_candidate_word);
        m_Spell = (TextView)findViewById(R.id.id_spell);
    }
}
