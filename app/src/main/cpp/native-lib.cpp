#include <jni.h>
#include <string>
#include "native-lib.h"

int m_child;
const char *id;
const char *userId;
const char *serviceFullInfo;
const char *PATH = "data/data/com.hxh.socketprocess/my.sock";

/**
 * 用于开启socket
 */
void child_do_work() {
    // 开启socket 服务端
    // 把服务端分成两部分，child_create_channel用来连接，child_listen_msg用来读取数据
    if (child_create_channel()) {
        child_listen_msg();
    }
}

/**
 * 服创建服务端sockte
 * @return 1;
 */
int child_create_channel() {
    // socket可以跨进程，文件端口读写  linux文件系统  ip+端口 实际上指明文件
    // ip 端口 文件
    int listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    unlink(PATH);
    // addr --> 内存区域
    struct sockaddr_un addr;
    // 清空刚刚建立的结构体，全部赋值为零
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_LOCAL;
    // addr.sun_data = PATH; 不能够直接赋值,所以使用内存拷贝的方式赋值
    strcpy(addr.sun_path, PATH);

    // 相当于绑定端口号
    if (bind(listenfd, (const sockaddr *) &addr, sizeof(sockaddr_un)) < 0) {
        LOGE("绑定错误");
        return 0;
    }
    int connfd = 0;
    //能够同时连接5个客户端,最大为10
    listen(listenfd, 5);

    // 用死循环保证连接能成功
    // 保证宿主进程连接成功 百度全家桶
    while (1) {
        // 返回客户端地址 accept是阻塞式函数,返回有两种情况，一种成功，一种失败
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno == EINTR) {
                // 成功的情况下continue继续往后的步骤
                continue;
            } else {
                LOGE("读取错误");
                return 0;
            }
        }
        m_child = connfd;
        LOGE("apk 父进程连接上了 %d", m_child);
        break;
    }
    return 1;
}


/**
 * 创建服务端的socket
 */
void child_listen_msg() {
    fd_set fdSet;
    struct timeval timeout = {3, 0};
    while (1) {
        // 清空内容
        FD_ZERO(&fdSet);
        FD_SET(m_child, &fdSet);
        // 如果是两个客户端就在原来的基础上+1以此类推，最后一个参数是找到他的时间超过3秒就是超时
        // select会先执行，会找到m_child对应的文件如果找到就返回大于0的值，进程就会阻塞没找到就不会
        int r = select(m_child + 1, &fdSet, NULL, NULL, &timeout);
        //LOGE("读取消息前 %d  ", r);
        if (r > 0) {
            // 缓冲区
            char byte[256] = {0};
            //阻塞式函数
            LOGE("读取消息后 %d", r);
            read(m_child, byte, sizeof(byte));
            LOGE("在这里===%s", id);
            // 不在阻塞，开启服务
            // 新进程与原进程有相同的PID。
            execlp("am", "am", "startservice", "--user", id, serviceFullInfo,
                   (char *) NULL);
            break;

            // 保证所读到的信息是指定apk客户端的
            /*if (FD_ISSET(m_child, &fdSet)) {
                // 阻塞式函数
                int result = read(m_child, byte, sizeof(byte));
                LOGE("userId %s ", userId);
                // 开启服务
                execlp("am", "am", "startservice", "--user", userId,
                       "com.hxh.socketprocess/com.hxh.socketprocess.ProcessService", (char *) NULL);
                break;
            }*/
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hxh_socketprocess_Watcher_createWatcher(JNIEnv *env, jobject instance, jstring userId_,
                                                 jstring serviceFullInfo_) {
    userId = env->GetStringUTFChars(userId_, 0);
    serviceFullInfo = env->GetStringUTFChars(serviceFullInfo_, 0);

    // 开双进程 并且有两个返回值0和1，0代表子进程，1代表父进程
    // 这里我们只用到子进程，父进程不会用到
    pid_t pid = fork();
    if (pid < 0) {
        // 这里如果pid小于0则代表开双进程失败
    } else if (pid == 0) {
        // 子进程 守护进程
        child_do_work();
    } else {
        // 父进程
    }

    env->ReleaseStringUTFChars(userId_, userId);
    env->ReleaseStringUTFChars(serviceFullInfo_, serviceFullInfo);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hxh_socketprocess_Watcher_connectMonitor(JNIEnv *env, jobject instance) {
    // 客户端进程调用
    // 主进程
    int socked;
    // addr --> 内存区域
    struct sockaddr_un addr;
    while (1) {
        LOGE("客户端父进程开始连接");
        socked = socket(AF_LOCAL, SOCK_STREAM, 0);
        if (socked < 0) {
            LOGE("连接失败");
            return;
        }

        // 清空内存
        memset(&addr, 0, sizeof(sockaddr_un));
        addr.sun_family = AF_LOCAL;
        strcpy(addr.sun_path, PATH);

        if (connect(socked, (const sockaddr *) &addr, sizeof(sockaddr_un)) < 0) {
            LOGE("连接失败");
            // 如果连接失败了就关闭当前socked，休眠一秒重新开始连接
            close(socked);
            sleep(1);
            // 再来下一次尝试连接
            continue;
        }
        LOGE("连接成功");
        break;
    }

}