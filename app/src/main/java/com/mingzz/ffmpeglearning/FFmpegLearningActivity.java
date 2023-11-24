package com.mingzz.ffmpeglearning;

/**
 * @File FFmpegLearningActivity.java
 * @Author Mingzz__/zemingzeng@126.com
 * @Date 2023/11/21 14:26.
 * @Brief
 * @Version 1.0
 * @Copyright Copyright (c) 2023 Zengzeming All rights reserved.
 */

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.File;

public class FFmpegLearningActivity extends AppCompatActivity {

    private final String TAG = "mingzz__FFmpegLearning";
    private  String videoName = "fox.mp4";
    private  String videoPath = "../../../.mp4";

    static {
        System.loadLibrary("ffmpeglearning");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI(11, 22, 33));

        videoPath=getCacheDir().getAbsolutePath()+ File.separator+"fox.mp4";
        Log.i(TAG, "onCreate video url->"+videoPath);

        ffmpegLearningStart(videoPath);

    }

    public native String stringFromJNI(int a, int b, int c);
    public native void ffmpegLearningStart(String url);
}
