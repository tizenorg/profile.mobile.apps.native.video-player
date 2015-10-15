/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/


#pragma once

#include <dlog.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <stdbool.h>

#ifdef _cplusplus
extern "C"
{
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif


#define LOG_TAG "VIDEO_PLAYER"

#ifndef _USE_DLOG_
#define _USE_DLOG_
#endif

/* anci c color type */
#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#ifdef _USE_LOG_FILE_
FILE *g_log_fp;
#endif // _USE_LOG_FILE_

#ifdef _USE_LOG_FILE_


#elif defined _USE_DLOG_

#define vp_dbg_tid(fmt,arg...)		LOGI_IF(true, FONT_COLOR_YELLOW"[TID:%d]" fmt FONT_COLOR_RESET"\n", syscall(__NR_gettid), ##arg)
#define vp_dbg(fmt, arg...)		LOGD_IF(true, "" fmt "\n", ##arg)
#define vp_dbgW(fmt, arg...)		LOGW_IF(true, FONT_COLOR_GREEN" " fmt FONT_COLOR_RESET"\n", ##arg)
#define vp_dbgE(fmt, arg...)		LOGE_IF(true, FONT_COLOR_RED" " fmt FONT_COLOR_RESET"\n", ##arg)

#define vp_sdbg(fmt,arg...)			SECURE_LOGD( " " fmt "\n", ##arg)
#define vp_sdbg_tid(fmt,arg...)			SECURE_LOGI( fmt "\n", ##arg)
#define vp_sdbgE( fmt,arg...)		SECURE_LOGE( "\033[0;31m [ERR] " fmt FONT_COLOR_RESET"\n", ##arg)


#elif defined _USE_LOG_CONSOLE_


#else

#define vp_dbg(fmt,arg...)
#define vp_dbgW(fmt, arg...)
#define vp_dbgE(fmt, arg...)
#endif


#define vp_retm_if(expr, fmt, arg...) do { \
	if(expr) { \
		vp_dbgE(fmt, ##arg); \
		vp_dbgE("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define vp_retvm_if(expr, val, fmt, arg...) do { \
	if(expr) { \
		vp_dbgE(fmt, ##arg); \
		vp_dbgE("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define CHECK_EXCEP(expr) do { \
	if(!(expr)) { \
		vp_dbgE("Critical ERROR ########################################## Check below item.");\
		goto EXCEPTION;\
	} \
} while (0)

#define VP_CHECK_VAL(expr, val) 		vp_retvm_if(!(expr), val, "Invalid parameter, return ERROR code!")
#define VP_CHECK_NULL(expr) 			vp_retvm_if(!(expr), NULL, "Invalid parameter, return NULL!")
#define VP_CHECK_FALSE(expr) 			vp_retvm_if(!(expr), false, "Invalid parameter, return FALSE!")
#define VP_CHECK_CANCEL(expr) 			vp_retvm_if(!(expr), ECORE_CALLBACK_CANCEL, "Invalid parameter, return ECORE_CALLBACK_CANCEL!")
#define VP_CHECK(expr) 				vp_retm_if(!(expr), "Invalid parameter, return!")

#define VP_START_FUNC()              	LOGD_IF(true, "*********************** [Func: %s] enter!! ********************* \n", __func__)
#define VP_END_FUNC()              	LOGD_IF(true, "*********************** [Func: %s] leave!! ********************* \n", __func__)

#ifdef _cplusplus
}
#endif


