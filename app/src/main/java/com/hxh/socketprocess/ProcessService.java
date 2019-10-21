package com.hxh.socketprocess;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Process;
import android.support.annotation.Nullable;
import android.util.Log;

import java.util.Timer;
import java.util.TimerTask;

/**
 * Created by HXH at 2019/10/21
 * 推送服务
 */
public class ProcessService extends Service {

    private static final String TAG = "HXH";

    int i = 0;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Watcher watcher = new Watcher();
        watcher.createWatcher(String.valueOf(Process.myPid()),
                BuildConfig.APPLICATION_ID + "/" + this.getClass().getName());
        watcher.connectMonitor();
        Timer timer = new Timer();
        // 定时器
        timer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                Log.i(TAG, "服务开启中 " + i);
                i++;
            }
        }, 0, 1000 * 3);
    }
}
