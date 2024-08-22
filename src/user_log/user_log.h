/**
 * @file      : user_log.h
 * @brief     : 用户日志库头文件(参考easy_logger实现)
 * @author    : huenrong (huenrong1028@outlook.com)
 * @date      : 2022-08-15 10:01:48
 *
 * @copyright : Copyright (c) 2022 huenrong
 *
 */

#ifndef __USER_LOG_H
#define __USER_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define USE_USER_LOG

/**
 * CSI(Control Sequence Introducer/Initiator) sign
 * more information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define CSI_START "\033["
#define CSI_END   "\033[0m"

/* output log front color */
#define F_BLACK   "30;"
#define F_RED     "31;"
#define F_GREEN   "32;"
#define F_YELLOW  "33;"
#define F_BLUE    "34;"
#define F_MAGENTA "35;"
#define F_CYAN    "36;"
#define F_WHITE   "37;"

/* output log background color */
#define B_NULL
#define B_BLACK   "40;"
#define B_RED     "41;"
#define B_GREEN   "42;"
#define B_YELLOW  "43;"
#define B_BLUE    "44;"
#define B_MAGENTA "45;"
#define B_CYAN    "46;"
#define B_WHITE   "47;"

/* output log fonts style */
#define S_BOLD      "1m"
#define S_UNDERLINE "4m"
#define S_BLINK     "5m"
#define S_NORMAL    "22m"

/* output log default color definition: [front color] + [background color] + [show style] */
#ifndef ELOG_COLOR_ASSERT
#define ELOG_COLOR_ASSERT (F_MAGENTA B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_ERROR
#define ELOG_COLOR_ERROR (F_RED B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_WARN
#define ELOG_COLOR_WARN (F_YELLOW B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_INFO
#define ELOG_COLOR_INFO (F_CYAN B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_DEBUG
#define ELOG_COLOR_DEBUG (F_GREEN B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_VERBOSE
#define ELOG_COLOR_VERBOSE (F_BLUE B_NULL S_NORMAL)
#endif

/* output log's level */
#define ELOG_LVL_ASSERT  0
#define ELOG_LVL_ERROR   1
#define ELOG_LVL_WARN    2
#define ELOG_LVL_INFO    3
#define ELOG_LVL_DEBUG   4
#define ELOG_LVL_VERBOSE 5

#if !defined(LOG_TAG)
#define LOG_TAG "NO_TAG"
#endif

#ifdef USE_USER_LOG
/**
 * @brief  输出日志函数
 * @param  level : 输入参数, 日志等级
 * @param  tag   : 输入参数, 日志tag
 * @param  func  : 输入参数, 调用函数
 * @param  line  : 输入参数, 调用行数
 * @param  format: 输入参数, 输出格式
 * @param  ...   : 输入参数, args
 */
void user_log_output(const uint8_t level, const char *tag, const char *func, const long line, const char *format, ...);

#define log_a(...) user_log_output(ELOG_LVL_ASSERT, LOG_TAG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_e(...) user_log_output(ELOG_LVL_ERROR, LOG_TAG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_w(...) user_log_output(ELOG_LVL_WARN, LOG_TAG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_i(...) user_log_output(ELOG_LVL_INFO, LOG_TAG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_d(...) user_log_output(ELOG_LVL_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_v(...) user_log_output(ELOG_LVL_VERBOSE, LOG_TAG, __FUNCTION__, __LINE__, __VA_ARGS__)

#else

#define log_a(...)
#define log_e(...)
#define log_w(...)
#define log_i(...)
#define log_d(...)
#define log_v(...)

#endif // USE_USER_LOG

#ifdef __cplusplus
}
#endif

#endif // __USER_LOG_H
