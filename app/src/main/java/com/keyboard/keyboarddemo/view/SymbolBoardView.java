package com.keyboard.keyboarddemo.view;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.keyboard.keyboarddemo.R;
import com.keyboard.keyboarddemo.common.BaseFunctionKey;
import com.keyboard.keyboarddemo.interfaces.ISymbolBoardView;
import com.keyboard.keyboarddemo.interfaces.IkeyBoardView;
import com.keyboard.keyboarddemo.skbContainer;

import java.util.ArrayList;

/**
 * Created by jidan on 17-7-4.
 */
public class SymbolBoardView extends RelativeLayout implements View.OnClickListener{

    private View view = null;
    private ListView m_ListView;
    private GridView m_GridView;


    private Context m_Context;
    private TitleAdapter m_listAdapter;
    private GridValueAdapter m_gridAdatper;
    private int m_titleIndex = 0;
    private int m_oldTitleIndex = 0;
    private ArrayList<String>values = new ArrayList<>();

    private ArrayList<String>titles = new ArrayList<>();
    private ISymbolBoardView iSymbolBoardView;

    private final String numArray[] = {"1","2","3","+","4","5","6","-","7","8","9",".","*","0","#",","};
    private final String zhArray[] ={"！","@","#","￥","%","^","*","（","）","”","“","=","……","|","、","：","；","？","。","+","——","/","'\\","【","】","『","』","《","》"};
    private final String EnArray[] ={"!","@","#","$","%","^","*","(",")","'","","=","_","|","'",":",";","?",".","+","_","/","","[","]","{","}","<",">"};
    private final String emoji[] ={":)",";-)",":-D",":p",":-(",":')",":-)",":-*",":-$",":-/",":-!",":S",":o","B-)",":()"};
    private final String web[] ={".com",".cn",".net",".org",".gov","www.","http://",".中国","news.","blog.","google","gmail","baidu","sina","qq","3g."};
    private final String emoji1[] ={String.valueOf(R.mipmap.emoji_1),String.valueOf(R.mipmap.emoji_2),String.valueOf(R.mipmap.emoji_3),
    String.valueOf(R.mipmap.emoji_4),String.valueOf(R.mipmap.emoji_5),String.valueOf(R.mipmap.emoji_6),String.valueOf(R.mipmap.emoji_7),
    String.valueOf(R.mipmap.emoji_8),String.valueOf(R.mipmap.emoji_9),String.valueOf(R.mipmap.emoji_10),String.valueOf(R.mipmap.emoji_11),String.valueOf(R.mipmap.emoji_12)
    ,String.valueOf(R.mipmap.emoji_13),String.valueOf(R.mipmap.emoji_14),String.valueOf(R.mipmap.emoji_15),
    String.valueOf(R.mipmap.emoji_16),String.valueOf(R.mipmap.emoji_17),String.valueOf(R.mipmap.emoji_18),String.valueOf(R.mipmap.emoji_19),
    String.valueOf(R.mipmap.emoji_20)};

    private final int emojiCode[] = {0x1F600,0x1F601,0x1F602,0x1F923,0x1F603,0x1F604,0x1F605,
    0x1F606,0x1F609,0x1F60A,0x1F60B,0x1F60E,0x1F60D,0x1F618,0x1F617,0x1F619,0x1F61A,
    0x263A,0x1F642,0x1F917};
    public SymbolBoardView(Context context) {
        super(context);
        m_Context = context;
        initData();
        view = LayoutInflater.from(context).inflate(R.layout.symbol_keyboard_layout,this);
        m_ListView = (ListView)view.findViewById(R.id.id_symbol_title);
        m_GridView = (GridView)view.findViewById(R.id.id_symbol_value);
        m_listAdapter = new TitleAdapter();
        m_ListView.setAdapter(m_listAdapter);
        m_ListView.setSelector(R.drawable.symbol_list_item_drawable);
        m_gridAdatper = new GridValueAdapter();
        m_GridView.setAdapter(m_gridAdatper);
        setValues();
        notifyTextColor(0);
        m_ListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (m_oldTitleIndex == position) {
                    return;
                }
                notifyTextColor(position);

                m_titleIndex = position;
                m_oldTitleIndex = m_titleIndex;
                setValues();
            }
        });
        m_GridView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if(m_titleIndex == 5){
                    iSymbolBoardView.ProcessKey(BaseFunctionKey.KEY_COMMIT,emojiCode[position],skbContainer.KeyboardModel.SymbolKeyboard);
                }else {
                    iSymbolBoardView.ProcessKey(BaseFunctionKey.KEY_COMMIT, values.get(position), skbContainer.KeyboardModel.SymbolKeyboard);
                }
            }
        });

        findViewById(R.id.id_symbol_return).setOnClickListener(this);
        findViewById(R.id.id_symbol_space).setOnClickListener(this);
        findViewById(R.id.id_symbol_enter).setOnClickListener(this);
        findViewById(R.id.id_symbol_del).setOnClickListener(this);


    }
    private void  initData(){
        titles.clear();
        String array[] = {"数字","中文","英文","表情","Web","Emoji"};
        int length = array.length;
        for(int i  = 0 ; i < length; i++){
            titles.add(array[i]);
        }

    }
    private void setValues(){
        int length = 0 ;
        values.clear();
        String array[] ={};

        switch (m_titleIndex){
            case 0:
                array = numArray;
                break;
            case 1:
                array = zhArray;
                break;
            case 2:
                array = EnArray;
                break;
            case 3:
                array = emoji;
                break;
            case 4:
                array = web;
                break;
            case 5:
                array = emoji1;
                break;
        }
        length = array.length;
        for(int i = 0 ; i < length; i++){
            values.add(array[i]);
        }
        m_gridAdatper.notifyDataSetChanged();

    }
    private void setValue(){


    }
    public void reset(){
        notifyTextColor(0);
        m_ListView.setSelectionAfterHeaderView();
        m_ListView.smoothScrollToPosition(0);
        m_titleIndex = 0;
        setValues();
    }
    public void notifyTextColor(int position) {
        m_listAdapter.setSelectPosition(position);
        m_listAdapter.notifyDataSetChanged();
    }
    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.id_symbol_return) notifyTextColor(0);
     iSymbolBoardView.ProcessKey(v.getId(),"", skbContainer.KeyboardModel.SymbolKeyboard);
    }
    public void setListener(ISymbolBoardView iSymbolBoardView) {
        this.iSymbolBoardView = iSymbolBoardView;
    }

    class ViewHolder{
        public TextView textView;
        private ImageView emojiImg;
    }

    class TitleAdapter extends BaseAdapter{

        private int mPosition;

        TitleAdapter(){

        }
        @Override
        public int getCount() {
            return titles.size();
        }

        @Override
        public Object getItem(int position) {
            return titles.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if(convertView == null){
                convertView = LayoutInflater.from(m_Context).inflate(R.layout.symbol_listview_item,null);
                holder = new ViewHolder();
                holder.textView = (TextView)convertView.findViewById(R.id.id_symbol_title_item);
                convertView.setTag(holder);
            }
            holder =(ViewHolder)convertView.getTag();
            holder.textView.setText(titles.get(position));
            if (position == mPosition) {
                convertView.setBackgroundColor(getResources().getColor(R.color.topTextColor));
            }
            else {
                convertView.setBackgroundColor(getResources().getColor(R.color.backGround));
            }
            return convertView;
        }
        public void setSelectPosition(int position){
            this.mPosition = position;
        }
    }
    class GridValueAdapter extends BaseAdapter{


        GridValueAdapter(){

        }
        @Override
        public int getCount() {
            return values.size();
        }

        @Override
        public Object getItem(int position) {
            return values.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if(convertView == null){
                convertView = LayoutInflater.from(m_Context).inflate(R.layout.symbol_gridview_item,null);
                holder = new ViewHolder();
                holder.textView = (TextView)convertView.findViewById(R.id.id_symbol_value_item);
                holder.emojiImg = (ImageView)convertView.findViewById(R.id.id_emoji_item);
                convertView.setTag(holder);
            }
            holder =(ViewHolder)convertView.getTag();
            holder.textView.setText(values.get(position));
            if(m_titleIndex == 5){
                 holder.emojiImg.setVisibility(VISIBLE);
                holder.textView.setVisibility(GONE);
                holder.emojiImg.setImageResource(Integer.valueOf(values.get(position)));
            }else {
                holder.textView.setVisibility(VISIBLE);
                holder.emojiImg.setVisibility(GONE);
            }
            if(m_titleIndex == 4){
                holder.textView.setTextSize(12);
            }else {
                holder.textView.setTextSize(20);
            }
            return convertView;
        }
    }
}
