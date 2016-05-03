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


#ifndef _VIDEO_LOG_H_
#define _VIDEO_LOG_H_

#include <dlog.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <stdbool.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif


#define LOG_TAG "VIDEO_PLAYER"

#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#ifndef LOGD_IF
#define LOGD_IF(fmt, arg...) dlog_print(DLOG_DEBUG, LOG_TAG, ##arg)
#endif
#ifndef LOGI_IF
#define LOGI_IF(fmt, arg...) dlog_print(DLOG_INFO, LOG_TAG, ##arg)
#endif
#ifndef LOGW_IF
#define LOGW_IF(fmt, arg...) dlog_print(DLOG_ERROR, LOG_TAG, ##arg)
#endif
#ifndef LOGE_IF
#define LOGE_IF(fmt, arg...) dlog_print(DLOG_ERROR, LOG_TAG, ##arg)
#endif
#ifndef SECURE_LOGD
#define SECURE_LOGD(fmt, arg...) dlog_print(DLOG_DEBUG, LOG_TAG, ##arg)
#endif
#ifndef SECURE_LOGI
#define SECURE_LOGI(fmt, arg...) dlog_print(DLOG_DEBUG, LOG_TAG, ##arg)
#endif
#ifndef SECURE_LOGE
#define SECURE_LOGE(fmt, arg...) dlog_print(DLOG_DEBUG, LOG_TAG, ##arg)
#endif

#define VideoLogInfoWithTid(fmt,arg...)			LOGI_IF(true, FONT_COLOR_BLUE fmt FONT_COLOR_RESET", syscall(__NR_gettid), ##arg)
#define VideoLogInfo(fmt,arg...)			LOGI_IF(true, FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##arg)
#define VideoLogDebug(fmt,arg...)			LOGD_IF(true, FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##arg)
#define VideoLogWarning(fmt,arg...)			LOGW_IF(true, FONT_COLOR_CYAN fmt FONT_COLOR_RESET, ##arg)
#define VideoLogError( fmt,arg...)			LOGE_IF(true, FONT_COLOR_RED fmt FONT_COLOR_RESET, ##arg)

#define VideoLogPerfS( fmt,arg...)			LOGI_IF(true, "\033[0;31m [PERF] ---> Start " fmt "\033[0m\n", ##arg)
#define VideoLogPerfE( fmt,arg...)			LOGI_IF(true, "\033[0;31m [PERF] End <--- " fmt "\033[0m\n",  ##arg)


#define VideoSecureLogInfoWithTid(fmt,arg...)		SECURE_LOGI(FONT_COLOR_BLUE fmt FONT_COLOR_RESET, syscall(__NR_gettid), ##arg)
#define VideoSecureLogDebug(fmt,arg...)			SECURE_LOGD(true, FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##arg)
#define VideoSecureLogInfo(fmt,arg...)			SECURE_LOGI(true, FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##arg)
#define VideoSecureLogError( fmt,arg...)		SECURE_LOGE(true, FONT_COLOR_RED fmt FONT_COLOR_RESET, ##arg)


//#define video_info(fmt,arg...)					printf( "[%s %s : %d] " fmt "\n",__FILE__,  __FUNCTION__,__LINE__, ##arg)
//#define video_debug(fmt,arg...)					printf( "[%s %s : %d] " fmt "\n",__FILE__, __FUNCTION__,__LINE__, ##arg)
//#define video_warning(fmt,arg...)				printf( "[%s %s : %d] " fmt "\n",__FILE__, __FUNCTION__,__LINE__, ##arg)
//#define video_error( fmt,arg...)				printf( "[%s %s : %d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__, ##arg)


#endif /*_MPVIDEO_LOG_H_*/
