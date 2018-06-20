package com.keyboard.keyboarddemo.view;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.keyboard.keyboarddemo.R;

/**
 * Created by jidan on 17-7-11.
 */
public class NumberBoardView extends LinearLayout {

    private Context m_Context;
    private View view;
    private GridView m_gridView;
    private GridValueAdapter adapter;
    private String numArray[][] = {{"1","0"},{"2","0"},{"3","0"},{"+","1"},{"4","0"},{"5","0"},{"6","0"},{"-","1"},
            {"7","0"},{"8","0"},{"9","0"},{"10","1"},{"11","0"},{"0","0"},{"12","0"},{"13","1"}};
    public NumberBoardView(Context context) {
        super(context);
        m_Context = context;
        view = LayoutInflater.from(context).inflate(R.layout.number_keyboard_layout,this);
        m_gridView = (GridView)view.findViewById(R.id.id_number_keyboard);
        adapter = new GridValueAdapter();
        m_gridView.setAdapter(adapter);
    }
    class ViewHolder{
        public TextView textView;
        private TextView topTextView;
        private ImageView imageView;
    }
    class GridValueAdapter extends BaseAdapter {


        GridValueAdapter(){

        }
        @Override
        public int getCount() {
            return numArray.length;
        }

        @Override
        public Object getItem(int position) {
            return numArray[position];
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if(convertView == null){
                convertView = LayoutInflater.from(m_Context).inflate(R.layout.number_item_layout,null);
                holder = new ViewHolder();
                holder.textView = (TextView)convertView.findViewById(R.id.id_title);
                holder.topTextView = (TextView)convertView.findViewById(R.id.id_top_title);
                holder.imageView = (ImageView)convertView.findViewById(R.id.id_title_img);
                convertView.setTag(holder);
            }
            holder =(ViewHolder)convertView.getTag();
            convertView.setBackgroundColor(getResources().getColor(R.color.white));
            int num = 0 ;
            try {
                num = Integer.valueOf(numArray[position][0]);
            }catch (NumberFormatException e){
                  e.printStackTrace();
            }
            String type = numArray[position][1];
            if((num >=0 && num <=9) || numArray[position][0].equals("+") || numArray[position][0].equals("-")){
                holder.imageView.setVisibility(GONE);
                holder.topTextView.setVisibility(GONE);
                if(type.equals("0")){
                    convertView.setBackgroundColor(getResources().getColor(R.color.white));
                }else {
                    convertView.setBackgroundColor(getResources().getColor(R.color.topTextColor));
                }
                holder.textView.setText(numArray[position][0]);
            }
            if(num == 10){
                holder.topTextView.setVisibility(GONE);
                holder.textView.setVisibility(GONE);
                holder.imageView.setImageResource(R.mipmap.space);
                convertView.setBackgroundColor(getResources().getColor(R.color.topTextColor));
            }else  if(num == 11){
                holder.imageView.setVisibility(GONE);
                holder.topTextView.setText("p");
                holder.textView.setText("*");
            }else if(num == 12){
                holder.imageView.setVisibility(GONE);
                holder.topTextView.setText("w");
                holder.textView.setText("#");
            }else if(num == 13){
                holder.topTextView.setVisibility(GONE);
                holder.textView.setVisibility(GONE);
                holder.imageView.setImageResource(R.mipmap.delete);
                convertView.setBackgroundColor(getResources().getColor(R.color.topTextColor));
            }



            return convertView;
        }
    }
}
