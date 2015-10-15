#ifdef ENABLE_DRM_FEATURE
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


#ifndef _MP_DRM_CTRL_H_
#define _MP_DRM_CTRL_H_

#define	DRM_STR_LEN_MAX				2048
#define DIVX_DRM_UNLIMIT_VALUE		-999

#define DIVX_DRM_AUTHORIZATION_ERROR_MSG		"This phone is not authorized to play this DivX protected video."
#define DIVX_DRM_RENTAL_EXPIRED_MSG				"This DivX rental has used %d of %d views.<br>This DivX rental has expired."
#define DIVX_DRM_RENTAL_CONTINUE_MSG			"This DivX rental has used %d of %d views.<br>Continue?"


typedef enum {
	DRM_CONTENT_INFO_NULL			= -1,
	DRM_CONTENT_INFO_AUTHOR,
	DRM_CONTENT_INFO_RIGHTS_URL,
	DRM_CONTENT_INFO_DESCRIPTION,
} MpVideoDrmContentInfoType;


typedef enum
{
	MP_VIDEO_DRM_ERROR_NONE = 0,
	MP_VIDEO_DRM_ERROR_INTERNAL,
	MP_VIDEO_DRM_ERROR_TIME_BASED,
	MP_VIDEO_DRM_ERROR_INTERVAL,
	MP_VIDEO_DRM_ERROR_INTERVAL_FIRST_USE,
	MP_VIDEO_DRM_ERROR_ACCUMULATED,
	MP_VIDEO_DRM_ERROR_COUNT,
	MP_VIDEO_DRM_ERROR_COUNT_SMALL,
	MP_VIDEO_DRM_ERROR_TIME_COUNT_BASED,
	MP_VIDEO_DRM_ERROR_TIME_INDIVIDULAL,
	MP_VIDEO_DRM_ERROR_TIME_SYSTEM,
	MP_VIDEO_DRM_ERROR_MAX,
}MpVideoDrmErrorType;


typedef struct
{
	int constraints;

	int remaining_count;
	bool date_time_expired;
	int remaining_interval_sec;
	int remaining_timed_count;
	int remaining_acc_sec;
}st_MpConstraintsInfo;

typedef enum {
	MP_DRM_CONSTRAINT_UNLIMITED			= 0x0,
	MP_DRM_CONSTRAINT_COUNT				= 0x01,
	MP_DRM_CONSTRAINT_DATE_TIME			= 0x02,
	MP_DRM_CONSTRAINT_INTERVAL			= 0x04,
	MP_DRM_CONSTRAINT_TIMED_COUNT		= 0x08,
	MP_DRM_CONSTRAINT_ACCUMLATED_TIME	= 0x10,
}MpDrmConstraintInfoType;

bool mp_drm_is_drm_file(char* szPath);
bool mp_drm_check_foward_lock(char *szPath);

bool mp_drm_is_divx(char *szPath);
bool mp_drm_check_divx_rental_expired(char *szPath,int *use_count, int *total_count);
void mp_drm_get_file_extension(char *szPath, char *szContentType, int nContentTypeStrSize);

#endif //_MP_DRM_CTRL_H_
#endif
