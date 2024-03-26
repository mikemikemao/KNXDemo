package com.hikvision.knxdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import com.hikvision.jni.IKNX;


public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    IKNX iknx;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        iknx = new IKNX();
        initView();
    }
    private void initView(){
        findViewById(R.id.tv_openLight).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.tv_openLight:
                iknx.native_openLight();
                break;
            default:
                break;
        }
    }
}