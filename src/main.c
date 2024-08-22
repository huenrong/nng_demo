/**
 * @file      main.c
 * @brief     file brief
 * @author    huenrong (huenrong1028@outlook.com)
 * @date      2024-06-16 17:00:55
 *
 * @copyright Copyright (c) 2024 huenrong
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "nng.h"
#include "sub.h"
#include "pub.h"

#define LOG_TAG "main"
#include "user_log.h"

#define PUB_URL "tcp://localhost:3327"
#define SUB_URL "tcp://localhost:3328"

/**
 * @brief 代理线程函数
 *
 * @param[in] arg: 线程参数
 *
 * @return NULL
 */
void *forwarder_thread_func(void *arg)
{
    // 1. 创建socket
    nng_socket pub_socket = NNG_SOCKET_INITIALIZER;
    int ret = nng_pub0_open_raw(&pub_socket);
    if (ret != 0)
    {
        log_e("open pub socket failed: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    nng_socket sub_socket = NNG_SOCKET_INITIALIZER;
    ret = nng_sub0_open_raw(&sub_socket);
    if (ret != 0)
    {
        log_e("open sub socket failed: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 2. 创建listener
    nng_listener pub_listener = NNG_LISTENER_INITIALIZER;
    ret = nng_listener_create(&pub_listener, pub_socket, SUB_URL);
    if (ret != 0)
    {
        log_e("Failed to create back listener: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    nng_listener sub_listener = NNG_LISTENER_INITIALIZER;
    ret = nng_listener_create(&sub_listener, sub_socket, PUB_URL);
    if (ret != 0)
    {
        log_e("Failed to create front listener: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 3. 启动listener
    ret = nng_listener_start(pub_listener, 0);
    if (ret != 0)
    {
        log_e("Failed to start back listener: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    ret = nng_listener_start(sub_listener, 0);
    if (ret != 0)
    {
        log_e("Failed to start front listener: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 4. 启动转发/代理
    ret = nng_device(sub_socket, pub_socket);
    if (ret != 0)
    {
        log_e("nng_device returned %d: %s\n", ret, nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    return NULL;
}

/**
 * @brief 发布线程函数
 *
 * @param[in] arg: 线程参数
 *
 * @return NULL
 */
void *pub_thread_func(void *arg)
{
    // 1. 创建socket
    nng_socket pub_socket;
    int ret = nng_pub0_open(&pub_socket);
    if (ret != 0)
    {
        log_e("open pub socket failed: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 2. 创建dialer
    nng_dialer pub_dialer;
    ret = nng_dialer_create(&pub_dialer, pub_socket, PUB_URL);
    if (ret != 0)
    {
        log_e("unable to create dialer for %s: %s", PUB_URL, nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 3. 启动dialer
    ret = nng_dialer_start(pub_dialer, 0);
    if (ret != 0)
    {
        log_e("unable to start dialer for %s: %s", PUB_URL, nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 待发送的数据，格式为：topic+payload
    // 接收方会将传入的消息与订阅的主题列表进行对比，如果消息以主题中的整个字节集开头，则接收该消息；否则，丢弃该消息。
    // 详见：https://nng.nanomsg.org/man/v1.8.0/nng_sub.7.html
    char data[] = "topic:hello world";
    size_t data_len = strlen(data);

    while (true)
    {
        sleep(2);

        // 4. 构造消息
        nng_msg *msg;
        if ((ret = nng_msg_alloc(&msg, 0)) != 0)
        {
            log_e("nng alloc msg failed: %s", nng_strerror(ret));

            continue;
        }

        // 5. 添加消息内容
        if ((ret = nng_msg_append(msg, data, data_len)) != 0)
        {
            log_e("nng append msg failed: %s", nng_strerror(ret));

            continue;
        }

        // 6. 发送消息
        log_d("send msg[len: %ld]: %s", nng_msg_len(msg), (char *)nng_msg_body(msg));
        if ((ret = nng_sendmsg(pub_socket, msg, 0)) != 0)
        {
            log_e("send error: %s", nng_strerror(ret));
        }
    }

    return NULL;
}

/**
 * @brief 订阅线程函数
 *
 * @param[in] arg: 线程参数
 *
 * @return NULL
 */
void *sub_thread_func(void *arg)
{
    // 1. 创建socket
    nng_socket sub_socket;
    int ret = nng_sub0_open(&sub_socket);
    if (ret != 0)
    {
        log_e("open sub socket failed: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 2. 设置订阅topic, topic为空表示订阅所有topic
    ret = nng_socket_set(sub_socket, NNG_OPT_SUB_SUBSCRIBE, "topic:", strlen("topic:"));
    if (ret != 0)
    {
        log_e("unable to subscribe to topic: %s", nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 3. 创建dialer
    nng_dialer sub_dialer;
    ret = nng_dialer_create(&sub_dialer, sub_socket, SUB_URL);
    if (ret != 0)
    {
        log_e("unable to create dialer for %s: %s", SUB_URL, nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // 4. 启动dialer
    ret = nng_dialer_start(sub_dialer, 0);
    if (ret != 0)
    {
        log_e("unable to start dialer for %s: %s", SUB_URL, nng_strerror(ret));
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        // 5. 接收消息
        nng_msg *msg;
        ret = nng_recvmsg(sub_socket, &msg, 0);
        switch (ret)
        {
        case 0:
        {
            log_d("recv msg[len: %ld]: %s\n", nng_msg_len(msg), (char *)nng_msg_body(msg));
            nng_msg_free(msg);

            continue;
        }

        case NNG_ETIMEDOUT:
        case NNG_ESTATE:
        {
            // Either a regular timeout, or we reached the
            // end of an event like a survey completing.

            return NULL;
        }

        default:
        {
            log_e("recv error: %s", nng_strerror(ret));

            break;
        }
        }
    }

    return NULL;
}

/**
 * @brief 程序入口
 *
 * @param[in] argc: 参数个数
 * @param[in] argv: 参数列表
 *
 * @return 成功: 0
 * @return 失败: 其它
 */
int main(int argc, char *argv[])
{
    log_i("start...");

    // 创建forwarder线程
    pthread_t forwarder_thread;
    pthread_create(&forwarder_thread, NULL, forwarder_thread_func, NULL);
    pthread_detach(forwarder_thread);

    // 确保forwarder线程已执行完毕
    sleep(2);

    // 创建pub线程
    pthread_t pub_thread;
    pthread_create(&pub_thread, NULL, pub_thread_func, NULL);
    pthread_detach(pub_thread);

    // 创建sub线程
    pthread_t sub_thread;
    pthread_create(&sub_thread, NULL, sub_thread_func, NULL);
    pthread_detach(sub_thread);

    while (true)
    {
        sleep(1);
    }

    log_i("end...");

    return 0;
}
