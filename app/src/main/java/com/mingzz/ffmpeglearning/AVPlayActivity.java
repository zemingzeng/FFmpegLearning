package com.mingzz.ffmpeglearning;

import android.os.Bundle;
import android.os.SystemClock;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import java.io.File;

/**
 * @File AVPlayActivity
 * @Author Mingzz__/zemingzeng@126.com
 * @Date 2023/12/25 8:53.
 * @Brief
 * @Version 1.0
 * @Copyright Copyright (c) 2023 Zengzeming All rights reserved.
 */
public class AVPlayActivity extends AppCompatActivity implements
        View.OnClickListener, SurfaceHolder.Callback {
    private static final String TAG = "mingzz__AVPlayActivity";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.avplay_activity_layout);
        init();
    }

    private String videoName = "fox.mp4";
    private String videoPath = "../../../.mp4";

    SurfaceView surfaceView;
    boolean surfaceViewCreated = false;

    Surface surface;

    private void init() {
        surfaceView = findViewById(R.id.sv);
        surfaceView.getHolder().addCallback(this);

        findViewById(R.id.bt_start).setOnClickListener(this);
        findViewById(R.id.bt_start).setEnabled(true);
        findViewById(R.id.bt_stop).setOnClickListener(this);
        findViewById(R.id.bt_stop).setEnabled(false);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.i(TAG, "surfaceCreated: ");
        surfaceViewCreated = true;
        surface = holder.getSurface();
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.i(TAG, "surfaceChanged: ");
    }


    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.i(TAG, "surfaceDestroyed: ");
    }

    private void startToPlay() {
        videoPath = getCacheDir().getAbsolutePath() + File.separator + videoName;
        Log.d(TAG, "onCreate video url->" + videoPath + " surface->" + surface);
        avStartToPlay(videoPath, surface);
    }

    @Override
    public void onClick(View v) {
        if (R.id.bt_start == v.getId()) {
            if (!surfaceViewCreated) {
                SystemClock.sleep(10);
                Log.d(TAG, "onClick: after sleep surfaceViewCreated->" +
                        surfaceViewCreated);
                if (!surfaceViewCreated) {
                    Log.e(TAG, "onClick: surface not created!");
                    return;
                }
            }
            startToPlay();
            findViewById(R.id.bt_stop).setEnabled(true);
            findViewById(R.id.bt_start).setEnabled(false);
        } else if (R.id.bt_stop == v.getId()) {
            avStop();
            findViewById(R.id.bt_stop).setEnabled(false);
            findViewById(R.id.bt_start).setEnabled(true);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        avStop();
    }

    //native method
    public native void avStartToPlay(String url, Surface surface);

    public native void avStop();
}
