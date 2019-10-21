package com.hxh.socketprocess;

/**
 * Created by HXH at 2019/10/21
 * NDK双进程守护（单工机制）
 */
public class Watcher {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    /**
     * 守护进程 监听
     *
     * @param userId          父进程id
     * @param serviceFullInfo 要启动的服务的包名 + "/" +要复活启动的服务的全类名
     */
    public native void createWatcher(String userId, String serviceFullInfo);

    /**
     * 父进程连接
     */
    public native void connectMonitor();
}
