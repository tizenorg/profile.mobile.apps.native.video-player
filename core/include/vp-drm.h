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

#ifdef ENABLE_DRM_FEATURE
#pragma once

#include <stdbool.h>
#include <Elementary.h>


typedef enum {
	VP_DRM_CONSTRAINT_TYPE_RENTAL = 0,
	VP_DRM_CONSTRAINT_TYPE_RENTAL_UNLIMITED,
	VP_DRM_CONSTRAINT_TYPE_PURCHASE,
	VP_DRM_CONSTRAINT_TYPE_BASE,
	VP_DRM_CONSTRAINT_TYPE_EXPIRED,
	VP_DRM_CONSTRAINT_TYPE_NOT_AUTHORIZED,
	VP_DRM_CONSTRAINT_TYPE_INVALID_DRM_VERSION,
	VP_DRM_CONSTRAINT_TYPE_UNKNOWN_TYPE,
	VP_DRM_CONSTRAINT_TYPE_GENERAL_ERROR
}vp_drm_constraint_status_t;


typedef struct _vp_drm_constraint_info
{
	vp_drm_constraint_status_t status;
	int constraints;

	int original_count;
	int remaining_count;
	bool date_time_expired;
	int remaining_interval_sec;
	int remaining_timed_count;
	int remaining_acc_sec;
}vp_drm_constraint_info;

typedef enum {
	VP_DRM_CONSTRAINT_UNLIMITED		= 0x0,
	VP_DRM_CONSTRAINT_COUNT			= 0x01,
	VP_DRM_CONSTRAINT_DATE_TIME		= 0x02,
	VP_DRM_CONSTRAINT_INTERVAL		= 0x04,
	VP_DRM_CONSTRAINT_TIMED_COUNT		= 0x08,
	VP_DRM_CONSTRAINT_ACCUMLATED_TIME	= 0x10,
}vp_drm_constraint_type_t;


bool vp_drm_is_drm_file(const char *szMediaURL, bool *bIsDRM);
bool vp_drm_is_divx_drm_file(const char *szMediaURL, bool *bIsDivXDRM);
bool vp_drm_is_playready_drm_file(const char *szMediaURL, bool *bIsPlayReadyDRM);
bool vp_drm_is_check_license(const char *szMediaURL, bool *bCheckLicense);
bool vp_drm_is_check_forward_lock(const char *szMediaURL, bool *bIsFowardLock);
bool vp_drm_get_constarint_info(const char *szMediaURL, vp_drm_constraint_info *pConstraintInfo);
bool vp_drm_get_file_mime_type(const char *szMediaURL, char **szMime);
bool vp_drm_get_store_item_license( char *pPath, char *pStoreAppId, char *pUserId, char *pImei, char *pOrderId, char *pMvId, char *pServerId );

#endif //ENABLE_DRM_FEATURE
