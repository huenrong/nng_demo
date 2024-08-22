/**
 * @file      : user_log.c
 * @brief     : 用户日志库源文件
 * @author    : huenrong (huenrong1028@outlook.com)
 * @date      : 2022-08-15 10:34:51
 *
 * @copyright : Copyright (c) 2022 huenrong
 *
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

#include "./user_log.h"

// clang-format off
// color output info
static const char *color_output_info[] = {
    [ELOG_LVL_ASSERT]  = ELOG_COLOR_ASSERT,
    [ELOG_LVL_ERROR]   = ELOG_COLOR_ERROR,
    [ELOG_LVL_WARN]    = ELOG_COLOR_WARN,
    [ELOG_LVL_INFO]    = ELOG_COLOR_INFO,
    [ELOG_LVL_DEBUG]   = ELOG_COLOR_DEBUG,
    [ELOG_LVL_VERBOSE] = ELOG_COLOR_VERBOSE,
};

// level output info
static const char *level_output_info[] = {
        [ELOG_LVL_ASSERT]  = "A/",
        [ELOG_LVL_ERROR]   = "E/",
        [ELOG_LVL_WARN]    = "W/",
        [ELOG_LVL_INFO]    = "I/",
        [ELOG_LVL_DEBUG]   = "D/",
        [ELOG_LVL_VERBOSE] = "V/",
};
// clang-format on

static pthread_mutex_t s_user_log_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief  获取时间
 * @param  time_str: 输出参数, 时间字符串(格式: yyyy-mm-dd hh:MM:ss.us)
 */
static void user_log_get_time(char *time_str)
{
    time_t current_time = time(NULL);

    struct tm sys_time = {0};
    localtime_r(&current_time, &sys_time);

    struct timeval time_value = {0};
    gettimeofday(&time_value, NULL);

    sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d.%06ld", (sys_time.tm_year + 1900), (sys_time.tm_mon + 1),
            sys_time.tm_mday, sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec, time_value.tv_usec);
}

/**
 * @brief  输出日志函数
 * @param  level : 输入参数, 日志等级
 * @param  tag   : 输入参数, 日志tag
 * @param  func  : 输入参数, 调用函数
 * @param  line  : 输入参数, 调用行数
 * @param  format: 输入参数, 输出格式
 * @param  ...   : 输入参数, args
 */
void user_log_output(const uint8_t level, const char *tag, const char *func, const long line, const char *format, ...)
{
    va_list args_list = {0};

    pthread_mutex_lock(&s_user_log_mutex);

    char time_str[64] = {0};
    user_log_get_time(time_str);
    printf("%s%s%s%-16s[%s] (%s:%ld)", CSI_START, color_output_info[level], level_output_info[level], tag, time_str,
           func, line);

    va_start(args_list, format);
    vprintf(format, args_list);
    va_end(args_list);

    printf("%s\n", CSI_END);
    fflush(stdout);

    pthread_mutex_unlock(&s_user_log_mutex);
}
