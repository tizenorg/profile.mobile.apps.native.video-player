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

#define VideoLogInfoWithTid(fmt,arg...)			dlog_print(DLOG_DEBUG, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, syscall(__NR_gettid), ##arg)
#define VideoLogInfo(fmt,arg...)				dlog_print(DLOG_DEBUG, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define VideoLogDebug(fmt,arg...)				dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define VideoLogWarning(fmt,arg...)				dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define VideoLogError( fmt,arg...)				dlog_print(DLOG_ERROR, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)

#define VideoLogPerfS( fmt,arg...)				dlog_print(DLOG_INFO, LOG_TAG, "\033[0;31m [PERF] ---> Start " fmt "\033[0m\n", ##arg)
#define VideoLogPerfE( fmt,arg...)				dlog_print(DLOG_INFO, LOG_TAG, "\033[0;31m [PERF] End <--- " fmt "\033[0m\n",  ##arg)


#define VideoSecureLogInfoWithTid(fmt,arg...)	dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, syscall(__NR_gettid), ##arg)
#define VideoSecureLogDebug(fmt,arg...)			dlog_print(DLOG_DEBUG, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define VideoSecureLogInfo(fmt,arg...)			dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define VideoSecureLogError( fmt,arg...)		dlog_print(DLOG_ERROR, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)


//#define video_info(fmt,arg...)				printf( "[%s %s : %d] " fmt "\n",__FILE__,  __FUNCTION__,__LINE__, ##arg)
//#define video_debug(fmt,arg...)				printf( "[%s %s : %d] " fmt "\n",__FILE__, __FUNCTION__,__LINE__, ##arg)
//#define video_warning(fmt,arg...)				printf( "[%s %s : %d] " fmt "\n",__FILE__, __FUNCTION__,__LINE__, ##arg)
//#define video_error( fmt,arg...)				printf( "[%s %s : %d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__, ##arg)


#endif /*_MPVIDEO_LOG_H_*/
