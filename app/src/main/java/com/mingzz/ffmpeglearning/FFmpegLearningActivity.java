package com.mingzz.ffmpeglearning;

/**
 * @File FFmpegLearningActivity.java
 * @Author Mingzz__/zemingzeng@126.com
 * @Date 2023/11/21 14:26.
 * @Brief
 * @Version 1.0
 * @Copyright Copyright (c) 2023 Zengzeming All rights reserved.
 */

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class FFmpegLearningActivity extends AppCompatActivity {
    private final String TAG = "mingzz__FFmpegLearning";

    static {
        System.loadLibrary("ffmpeglearning");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ffmpeglearning_activity_layout);
        init();
    }

    private void init() {
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI(11, 22, 33));
        findViewById(R.id.bt).setOnClickListener(v -> {
            startActivity(new Intent(this, AVPlayActivity.class));
        });
    }

    public native String stringFromJNI(int a, int b, int c);
}
