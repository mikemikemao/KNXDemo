package com.hikvision.knxdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import com.hikvision.jni.IKNX;


public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    IKNX iknx;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        iknx = new IKNX();
        initView();
        iknx.native_test();
    }
    private void initView(){
        findViewById(R.id.tv_openLight).setOnClickListener(this);
        findViewById(R.id.tv_closeLight).setOnClickListener(this);
        findViewById(R.id.tv_closeLight_State).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        int ret = 0;
        switch (v.getId()) {
            case R.id.tv_openLight:
                iknx.native_openLight(1);
                break;
            case R.id.tv_closeLight:
                iknx.native_openLight(0);
                break;
            case R.id.tv_closeLight_State:
                ret = iknx.native_openLightState();
                if(ret ==0){
                    Toast.makeText(getApplicationContext(),"已关灯",Toast.LENGTH_LONG).show();
                }else if(ret == 1){
                    Toast.makeText(getApplicationContext(),"已开灯",Toast.LENGTH_LONG).show();
                }

                break;
            default:
                break;
        }
    }
}