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

#ifndef __MP_UTIL_COPY_CTRL__
#define __MP_UTIL_COPY_CTRL__

#include <Eina.h>
#include <Ecore_Evas.h>
#include <Elementary.h>


typedef enum {
	MP_VIDEO_UPDATE,
	MP_VIDEO_UPDATE_CANCEL,
	MP_VIDEO_UPDATE_FINISH,
	MP_VIDEO_UPDATE_FAILED,
	MP_VIDEO_UPDATE_ERROR,
} euThreadFeedback;


typedef void (*mp_util_copy_cb) (euThreadFeedback msg_type, unsigned long long size, int nErrorCode, void *userData);

int mp_util_move_push(const char *szSrcPath, const char *szDstPath, mp_util_copy_cb cbFun, void *userData, unsigned long long nTotalSize);
bool mp_util_move_cancel_check(void);

void mp_util_move_set_cancel_state(bool bCancel);
bool mp_util_move_get_total_dir_size(const char *szDir, unsigned long long *size);

int mp_util_move_delete_video_folder(const char *szFolderPath, void *userData);

#endif

