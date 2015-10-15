
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

#include <glib.h>
#ifdef ENABLE_DRM_FEATURE
#include <drm_client.h>
#include <drm_client_types.h>
#endif
/*#include <drm_trusted_client.h>*/
/*#include <drm_trusted_client_types.h>*/

#include "mp-util.h"
#include "mp-video-log.h"
#include "video-player.h"
#include "mp-video-string-define.h"
#include "mp-drm-ctrl.h"
#include "mp-external-ug.h"

/*static DRM_DECRYPT_HANDLE     pDrmConsumeHandle = NULL;*/
/*static int CheckConsumption = VIDEO_DRM_CONSUMPTION_STATE_DISABLE;*/
#define ACCUMULATED_DATE		86400
#define RO_MESSAGE_LEN			1024
#define MOUNTH_COUNT			12
#define DAY_COUNT				30
#define HOUR_COUNT				24

static bool bCheckIntervalConstraint = FALSE;
static bool bCanPlayDrmContents = FALSE;

static drm_file_type_e nDrmFileType = DRM_TYPE_UNDEFINED;
static st_MpConstraintsInfo pMpConstraintsInfo;

static char *g_szMediaUri = NULL;




void mp_drm_check_remain_ro(void *pUserData, char *szTitle);


bool mp_drm_is_itpossible_to_play_drm_content(void)
{
	return bCanPlayDrmContents;
}

bool mp_drm_is_drm_file(char *szPath)
{

	if (!szPath) {
		VideoLogInfo("[ERR] It is not existed file url.");
		return FALSE;
	}

	int nRet = DRM_RETURN_SUCCESS;
	drm_bool_type_e is_drm = DRM_FALSE;

	nRet = drm_is_drm_file(szPath, &is_drm);
	if (nRet != DRM_RETURN_SUCCESS) {
		VideoLogError("[ERR]Fail to get whether drm file or not : %d",
		              nRet);
		return FALSE;
	}

	if (is_drm != DRM_TRUE) {
		VideoLogError("It's not drm file.");
		return FALSE;
	}

	return TRUE;
}

bool mp_drm_is_oma_file_type(char *szPath)
{
	if (!szPath) {
		VideoLogInfo("[ERR] It is not existed file url.");
		return FALSE;
	}

	drm_file_type_e nDrmType = DRM_TYPE_UNDEFINED;

	int nRet = drm_get_file_type(szPath, &nDrmType);
	if (nRet != DRM_RETURN_SUCCESS) {
		VideoLogWarning("[ERR] Fail to get file type : %d", nRet);
		return FALSE;
	}

	if (nDrmType != DRM_TYPE_OMA_V1 && nDrmType != DRM_TYPE_OMA_V2) {
		VideoLogInfo("It's not OMA drm file : %d", nDrmType);
		return FALSE;
	}

	return TRUE;
}

bool mp_drm_check_license(char *szPath)
{
	if (!szPath) {
		VideoLogInfo
		("[ERR] szPath is not existed for checking drm license.");
		return FALSE;
	}

	drm_license_status_e nLicenseStatus = DRM_LICENSE_STATUS_UNDEFINED;
	int nRet =
	        drm_get_license_status(szPath, DRM_PERMISSION_TYPE_PLAY,
	                               &nLicenseStatus);

	if (nRet != DRM_RETURN_SUCCESS) {
		VideoLogWarning("[ERR]Fail to get license info : %d", nRet);
		return FALSE;
	}

	if (nLicenseStatus != DRM_LICENSE_STATUS_VALID) {
		VideoLogInfo("Invalid license status : %d", nLicenseStatus);
		return FALSE;
	}

	return TRUE;
}

bool mp_drm_check_expiration(char *szPath, bool bExpired)
{
	if (!szPath) {
		VideoLogError("[ERR] szPath is not existed.");
		return FALSE;
	}

	if (mp_drm_is_drm_file(szPath)) {
		VideoLogError("It's not drm file.");
		return FALSE;
	}

	drm_file_info_s st_DrmFileInfo;
	memset(&st_DrmFileInfo, 0x0, sizeof(drm_file_info_s));

	int nRet = drm_get_file_info(szPath, &st_DrmFileInfo);
	if (nRet == DRM_RETURN_SUCCESS) {
		if (st_DrmFileInfo.oma_info.method ==
		                DRM_METHOD_TYPE_COMBINED_DELIVERY
		                || st_DrmFileInfo.oma_info.method ==
		                DRM_METHOD_TYPE_FORWARD_LOCK) {
			VideoLogInfo
			("st_DrmFileInfo.oma_info.method == DRM_METHOD_TYPE_COMBINED_DELIVERY || st_DrmFileInfo.oma_info.method == DRM_METHOD_TYPE_FORWARD_LOCK");
			return TRUE;
		}
	}

	return FALSE;
}

bool mp_drm_check_foward_lock(char *szPath)
{
	if (!szPath) {
		VideoLogInfo("[ERR] szPath is not existed.");
		return FALSE;
	}

	if (mp_drm_is_drm_file(szPath)) {
		VideoLogInfo("It's not drm file.");
		return FALSE;
	}

	drm_file_info_s st_DrmFileInfo;
	memset(&st_DrmFileInfo, 0x0, sizeof(drm_file_info_s));

	int nRet = drm_get_file_info(szPath, &st_DrmFileInfo);
	if (nRet == DRM_RETURN_SUCCESS) {
		if (st_DrmFileInfo.oma_info.method ==
		                DRM_METHOD_TYPE_FORWARD_LOCK) {
			VideoLogInfo
			("st_DrmFileInfo.oma_info.method == DRM_METHOD_TYPE_FORWARD_LOCK");
			return TRUE;
		}
	}

	return FALSE;
}

bool mp_drm_is_initialized(char *szPath)
{
	if (!szPath) {
		VideoLogError
		("[ERR] szPath is not existed for checking drm license.");
		return FALSE;
	}

	return mp_drm_check_license(szPath);
}

bool mp_drm_get_left_ro_info(char *szPath,
                             st_MpConstraintsInfo *pLeftRoInfo,
                             bool *pbHasValidRo)
{
	if (!szPath) {
		VideoLogError("[ERR]szPath is not existed.");
		return FALSE;
	}

	if (!pLeftRoInfo) {
		VideoLogError("[ERR]pLeftRoInfo is not existed.");
		return FALSE;
	}

	if (!mp_drm_check_license(szPath)) {
		VideoLogInfo("[ERR]Invalid drm file.");
		*pbHasValidRo = FALSE;
		return FALSE;
	} else {
		VideoLogInfo("Valid drm file.");
		*pbHasValidRo = TRUE;
	}

	static drm_constraint_info_s st_pDrmConstraintInfo;

	memset(pLeftRoInfo, 0x0, sizeof(st_MpConstraintsInfo));
	memset(&st_pDrmConstraintInfo, 0x0, sizeof(drm_constraint_info_s));

	int nRes =
	        drm_get_constraint_info(szPath, DRM_PERMISSION_TYPE_PLAY,
	                                &st_pDrmConstraintInfo);
	if (nRes == DRM_RETURN_SUCCESS) {
		if (st_pDrmConstraintInfo.const_type.is_unlimited) {
			VideoLogInfo("UNLIMITED");
			pLeftRoInfo->constraints = MP_DRM_CONSTRAINT_UNLIMITED;
			return true;
		}

		if (st_pDrmConstraintInfo.const_type.is_count) {
			VideoLogInfo("DRM_COUNT [%d]",
			             st_pDrmConstraintInfo.remaining_count);
			pLeftRoInfo->constraints |= MP_DRM_CONSTRAINT_COUNT;
			pLeftRoInfo->remaining_count =
			        st_pDrmConstraintInfo.remaining_count;
		}

		if (st_pDrmConstraintInfo.const_type.is_datetime) {
			pLeftRoInfo->constraints |= MP_DRM_CONSTRAINT_DATE_TIME;
			VideoLogInfo("%d.%d.%d %d:%d~%d.%d.%d %d:%d",
			             st_pDrmConstraintInfo.start_time.tm_mday,
			             st_pDrmConstraintInfo.start_time.tm_mon + 1,
			             st_pDrmConstraintInfo.start_time.tm_year + 1900,
			             st_pDrmConstraintInfo.start_time.tm_hour,
			             st_pDrmConstraintInfo.start_time.tm_min,
			             st_pDrmConstraintInfo.end_time.tm_mday,
			             st_pDrmConstraintInfo.end_time.tm_mon + 1,
			             st_pDrmConstraintInfo.end_time.tm_year + 1900,
			             st_pDrmConstraintInfo.end_time.tm_hour,
			             st_pDrmConstraintInfo.end_time.tm_min);

			struct timeval tv;
			gettimeofday(&tv, NULL);
			struct tm *ptm = NULL;
			ptm = localtime(&tv.tv_sec);

			if (ptm->tm_year >= st_pDrmConstraintInfo.start_time.tm_year
			                && ptm->tm_mon >= st_pDrmConstraintInfo.start_time.tm_mon
			                && ptm->tm_mday >=
			                st_pDrmConstraintInfo.start_time.tm_mday
			                && ptm->tm_hour >=
			                st_pDrmConstraintInfo.start_time.tm_hour
			                && ptm->tm_min >= st_pDrmConstraintInfo.start_time.tm_min
			                && ptm->tm_mon <= st_pDrmConstraintInfo.end_time.tm_mon
			                && ptm->tm_mday <= st_pDrmConstraintInfo.end_time.tm_mday
			                && ptm->tm_hour <= st_pDrmConstraintInfo.end_time.tm_hour
			                && ptm->tm_min <= st_pDrmConstraintInfo.end_time.tm_min) {
				/* not expired*/
				pLeftRoInfo->date_time_expired = false;
			} else {
				pLeftRoInfo->date_time_expired = true;
			}
		}

		if (st_pDrmConstraintInfo.const_type.is_interval) {
			pLeftRoInfo->constraints |= MP_DRM_CONSTRAINT_INTERVAL;
			VideoLogInfo("Remain... %d.%d.%d %d:%d",
			             st_pDrmConstraintInfo.interval_time.tm_mon,
			             st_pDrmConstraintInfo.interval_time.tm_mday,
			             st_pDrmConstraintInfo.interval_time.tm_year,
			             st_pDrmConstraintInfo.interval_time.tm_hour,
			             st_pDrmConstraintInfo.interval_time.tm_min);

			pLeftRoInfo->remaining_interval_sec =
			        st_pDrmConstraintInfo.interval_time.tm_sec +
			        st_pDrmConstraintInfo.interval_time.tm_min * 60 +
			        st_pDrmConstraintInfo.interval_time.tm_hour * 3600;
			pLeftRoInfo->remaining_interval_sec +=
			        (st_pDrmConstraintInfo.interval_time.tm_mday +
			         st_pDrmConstraintInfo.interval_time.tm_mon * 30 +
			         st_pDrmConstraintInfo.interval_time.tm_year * 365) *
			        (3600 * 24);
		}

		if (st_pDrmConstraintInfo.const_type.is_timedcount) {
			VideoLogInfo("%d left (%d sec)",
			             st_pDrmConstraintInfo.timed_remaining_count,
			             st_pDrmConstraintInfo.timed_count_timer);
			pLeftRoInfo->constraints |= MP_DRM_CONSTRAINT_TIMED_COUNT;
			pLeftRoInfo->remaining_timed_count =
			        st_pDrmConstraintInfo.timed_remaining_count;
		}

		if (st_pDrmConstraintInfo.const_type.is_accumulated) {
			VideoLogInfo("DRM_ACCUMULATED [%d]",
			             st_pDrmConstraintInfo.
			             accumulated_remaining_seconds);
			pLeftRoInfo->constraints |= MP_DRM_CONSTRAINT_ACCUMLATED_TIME;
			pLeftRoInfo->remaining_acc_sec =
			        st_pDrmConstraintInfo.accumulated_remaining_seconds;
		}

		if (st_pDrmConstraintInfo.const_type.is_individual) {
			VideoLogInfo("DRM_INDIVISUAL_ID [%s]",
			             st_pDrmConstraintInfo.individual_id);
		}

		if (st_pDrmConstraintInfo.const_type.is_system) {
			VideoLogInfo("DRM_SYSTEM [ID:%s, type:%d]",
			             st_pDrmConstraintInfo.system_id,
			             st_pDrmConstraintInfo.system_identity_type);
		}
	} else {
		VideoLogWarning("drm_get_constraint_info().. 0x%x", nRes);
		return FALSE;
	}

	return TRUE;
}

bool mp_drm_get_content_info(char *szPath,
                             MpVideoDrmContentInfoType nFirstInfoType,
                             ...)
{
	/* return info shoud be free */
	VideoLogInfo("");

	drm_content_info_s st_pDrmContenInfo;
	memset(&st_pDrmContenInfo, 0x0, sizeof(drm_content_info_s));

	int nRes = drm_get_content_info(szPath, &st_pDrmContenInfo);
	if (nRes != DRM_RETURN_SUCCESS) {
		VideoLogWarning("[Err] drm_get_content_info().. [0x%x]", nRes);
		return FALSE;
	}

	va_list var_args;
	MpVideoDrmContentInfoType nAttrInfoType;

	nAttrInfoType = nFirstInfoType;
	va_start(var_args, nFirstInfoType);
	char **ret_val = NULL;
	char *value = NULL;

	while (nAttrInfoType > DRM_CONTENT_INFO_NULL) {
		ret_val = va_arg((var_args), char **);

		switch (nAttrInfoType) {
		case DRM_CONTENT_INFO_AUTHOR:
			value = st_pDrmContenInfo.author;
			break;

		case DRM_CONTENT_INFO_RIGHTS_URL:
			value = st_pDrmContenInfo.rights_url;
			break;

		case DRM_CONTENT_INFO_DESCRIPTION:
			value = st_pDrmContenInfo.description;
			break;

		default:
			VideoLogInfo("Not defined [%d]", nAttrInfoType);
			value = NULL;
			break;
		}

		/* output */
		*ret_val = g_strdup(value);

		nAttrInfoType = va_arg(var_args, MpVideoDrmContentInfoType);
	}

	va_end(var_args);

	return TRUE;
}

static void mp_drm_popup_unlock_drm_yes_button_cb(void *pUserData,
                Evas_Object *pObject,
                void *pEventInfo)
{
	if (!pUserData) {
		VideoLogInfo("[ERR] No exist pUserData");
		return;
	}

	mp_util_delete_popup_handle();

	if (bCanPlayDrmContents) {
		mp_drm_check_remain_ro(pUserData, NULL);
	} else {
		char *szRightsUrl = NULL;
		bool bRet =
		        mp_drm_get_content_info(g_szMediaUri,
		                                DRM_CONTENT_INFO_RIGHTS_URL,
		                                &szRightsUrl, -1);

		if (bRet && szRightsUrl) {
			VideoSecureLogInfo("szRightsUrl : %s", szRightsUrl);

			if (!mp_external_ug_launch_browser(szRightsUrl)) {
				VideoLogInfo("[ERR] Fail to launch browser!!!");
			}

			free(szRightsUrl);
			szRightsUrl = NULL;
		} else {
			VideoLogInfo("[ERR] Fail to get right url.");
		}
	}
}

static void mp_drm_popup_unlock_drm_no_button_cb(void *pUserData,
                Evas_Object *pObject,
                void *pEventInfo)
{
	if (!pUserData) {
		VideoLogInfo("[ERR] No exist pUserData");
		return;
	}

	mp_util_delete_popup_handle();
}

void mp_drm_check_remain_ro(void *pUserData, char *szTitle)
{
	if (szTitle) {
		VideoLogWarning("[ERR] No exist szTitle");
		szTitle = "NO TITLE";
	}

	if (!pUserData) {
		VideoLogInfo("[ERR] No exist pUserData");
		return;
	}

	char *szFormat = NULL;
	char *szMessage = NULL;

	if (pMpConstraintsInfo.constraints & MP_DRM_CONSTRAINT_COUNT
	                && pMpConstraintsInfo.remaining_count <= 2) {
		int nRemain = pMpConstraintsInfo.remaining_count - 1;
		if (nRemain == 1) {
			szFormat =
			        MP_VPL_DRM_PS_CAN_USE_1_MORE_TIME_GET_ANOTHER_LICENCE_Q;
			szMessage = g_strdup_printf(szFormat, szTitle);
		} else if (nRemain == 0) {
			szFormat =
			        MP_VPL_DRM_PS_CANNOT_USE_ANY_MORE_TIMES_GET_ANOTHER_LICENCE_Q;
			szMessage = g_strdup_printf(szFormat, szTitle);
		}
	} else if (pMpConstraintsInfo.
	                constraints & MP_DRM_CONSTRAINT_ACCUMLATED_TIME) {
		if (pMpConstraintsInfo.remaining_acc_sec / ACCUMULATED_DATE <= 1) {
			szFormat =
			        MP_VPL_DRM_PS_IS_ABOUT_TO_EXPIRE_GET_ANOTHER_LICENCE_Q;
			szMessage = g_strdup_printf(szFormat, szTitle);
		}
	}

	if (szMessage) {
		bCanPlayDrmContents = FALSE;
		mp_util_yes_no_popup(pUserData, NULL, szMessage,
		                     mp_drm_popup_unlock_drm_yes_button_cb,
		                     mp_drm_popup_unlock_drm_no_button_cb);
		free(szMessage);
	}
}

bool mp_drm_check_left_ro(char *szPath, char *szTitle, void *pUserData)
{
	if (!szPath) {
		VideoLogWarning
		("[ERR]szPath is not existed for checking drm license.");
		return FALSE;
	}

	if (!szTitle) {
		VideoLogWarning("szTitle is not existed.");
		return FALSE;
	}

	if (!pUserData) {
		VideoLogWarning("pUserData is not existed.");
		return FALSE;
	}

	VideoLogInfo("");

	bool bHasValidRo = FALSE;
	bool bWapLaunch = FALSE;
	bool bExpired = FALSE;

	char *szFormat = NULL;
	char *szMessage = NULL;

	bCanPlayDrmContents = FALSE;

	memset(&pMpConstraintsInfo, 0, sizeof(st_MpConstraintsInfo));

	if (!mp_drm_get_left_ro_info
	                (szPath, &pMpConstraintsInfo, &bHasValidRo)) {
		VideoLogWarning("Fail to get left ro info.");
		return FALSE;
	}

	if (bHasValidRo) {
		while (pMpConstraintsInfo.constraints) {
			if (pMpConstraintsInfo.constraints & MP_DRM_CONSTRAINT_COUNT) {
				if (pMpConstraintsInfo.remaining_count == 2) {
					szFormat = MP_VPL_DRM_PS_2_MORE_TIME_START_NOW_Q;
					szMessage = g_strdup_printf(szFormat, szTitle);
				} else if (pMpConstraintsInfo.remaining_count == 1) {
					szFormat = MP_VPL_DRM_PS_1_MORE_TIME_START_NOW_Q;
					szMessage = g_strdup_printf(szFormat, szTitle);
				} else if (pMpConstraintsInfo.remaining_count == 0) {
					bWapLaunch = TRUE;
					break;
				}
			}

			if (pMpConstraintsInfo.
			                constraints & MP_DRM_CONSTRAINT_DATE_TIME) {
				if (pMpConstraintsInfo.date_time_expired) {
					bWapLaunch = TRUE;
					break;
				}
			}

			if (pMpConstraintsInfo.
			                constraints & MP_DRM_CONSTRAINT_INTERVAL) {
				if (pMpConstraintsInfo.remaining_interval_sec == 0) {
					bWapLaunch = TRUE;
					break;
				} else if (!bCheckIntervalConstraint) {
					bCheckIntervalConstraint = TRUE;
					int nDays =
					        pMpConstraintsInfo.remaining_interval_sec /
					        ACCUMULATED_DATE + 1;
					szFormat = MP_VPL_DRM_PS_FOR_PD_DAYS_START_NOW_Q;
					szMessage = g_strdup_printf(szFormat, szTitle, nDays);
				}
			}

			if (pMpConstraintsInfo.
			                constraints & MP_DRM_CONSTRAINT_TIMED_COUNT) {
				if (pMpConstraintsInfo.remaining_timed_count == 2) {
					szFormat = MP_VPL_DRM_PS_2_MORE_TIME_START_NOW_Q;
					szMessage = g_strdup_printf(szFormat, szTitle);
				} else if (pMpConstraintsInfo.remaining_timed_count == 1) {
					szFormat = MP_VPL_DRM_PS_1_MORE_TIME_START_NOW_Q;
					szMessage = g_strdup_printf(szFormat, szTitle);
				} else if (pMpConstraintsInfo.remaining_timed_count == 0) {
					bWapLaunch = TRUE;
					break;
				}
			}

			if (pMpConstraintsInfo.
			                constraints & MP_DRM_CONSTRAINT_ACCUMLATED_TIME) {
				if (pMpConstraintsInfo.remaining_acc_sec == 0) {
					bWapLaunch = TRUE;
					break;
				}
			}

			bCanPlayDrmContents = TRUE;
			break;
		}

		if (pMpConstraintsInfo.constraints == MP_DRM_CONSTRAINT_UNLIMITED) {
			bCanPlayDrmContents = TRUE;
		}
	}

	if (!bHasValidRo
	                || (bWapLaunch && !mp_drm_check_expiration(szPath, bExpired))) {
		VideoLogWarning("have no valid ro");

		szFormat = MP_VPL_DRM_PS_CURRENTLY_LOCKED_UNLOCK_Q;
		szMessage = g_strdup_printf(szFormat, szTitle);
		mp_util_yes_no_popup(pUserData, NULL, szMessage,
		                     mp_drm_popup_unlock_drm_yes_button_cb,
		                     mp_drm_popup_unlock_drm_no_button_cb);

		if (szMessage) {
			free(szMessage);
			szMessage = NULL;
		}

		return FALSE;
	} else if (szMessage) {
		VideoLogWarning("warning popup=[%s]", szMessage);

		mp_util_yes_no_popup(pUserData, NULL, szMessage,
		                     mp_drm_popup_unlock_drm_yes_button_cb,
		                     mp_drm_popup_unlock_drm_no_button_cb);

		if (szMessage) {
			free(szMessage);
			szMessage = NULL;
		}

		return FALSE;
	}

	return TRUE;
}

/* DivX DRM*/

bool mp_drm_is_divx(char *szPath)
{

	if (!szPath) {
		VideoLogInfo("[ERR]");
		return FALSE;
	}


	int nRet = DRM_RETURN_SUCCESS;
	drm_bool_type_e is_drm = DRM_FALSE;

	nRet = drm_is_drm_file(szPath, &is_drm);
	if (nRet != DRM_RETURN_SUCCESS) {
		VideoLogError("[ERR]drm_is_drm_file : %d", nRet);
		return FALSE;
	}

	if (is_drm != DRM_TRUE) {
		VideoLogError("It's not drm file.");
		return FALSE;
	}

	drm_file_type_e drm_type = DRM_TYPE_UNDEFINED;

	nRet = drm_get_file_type(szPath, &drm_type);
	if (nRet != DRM_RETURN_SUCCESS) {
		VideoLogError("[ERR]drm_get_file_type : %d", nRet);
		return FALSE;
	}

	nDrmFileType = drm_type;

	if (drm_type != DRM_TYPE_DIVX) {
		VideoLogError("It's not DivX drm file : %d", drm_type);
		return FALSE;
	}

	return TRUE;

}

bool mp_drm_check_divx_rental_expired(char *szPath, int *use_count,
                                      int *total_count)
{
	VideoLogInfo("");

	if (!szPath) {
		VideoLogError("[ERR]");
		return FALSE;
	}

	int nRet = DRM_RETURN_SUCCESS;
	drm_constraint_info_s constraint_info;
	memset(&constraint_info, 0, sizeof(drm_constraint_info_s));
	nRet =
	        drm_get_constraint_info(szPath, DRM_PERMISSION_TYPE_PLAY,
	                                &constraint_info);
	if (nRet != DRM_RETURN_SUCCESS) {
		VideoLogError("[ERR]drm_get_file_info : %d", nRet);
		return FALSE;
	}

	VideoLogError("[TYPE]\n"
	              "unlimied : %d\n"
	              "is_count : %d\n"
	              "is_datetime : %d\n"
	              "is_interval : %d\n"
	              "is_timedcount : %d\n"
	              "is_accumulated : %d\n"
	              "is_individual : %d\n"
	              "is_system : %d\n"
	              "constraint_type : %d",
	              constraint_info.const_type.is_unlimited,
	              constraint_info.const_type.is_count,
	              constraint_info.const_type.is_datetime,
	              constraint_info.const_type.is_interval,
	              constraint_info.const_type.is_timedcount,
	              constraint_info.const_type.is_accumulated,
	              constraint_info.const_type.is_individual,
	              constraint_info.const_type.is_system,
	              constraint_info.const_type.constraint_type);

	if (constraint_info.const_type.is_unlimited) {
		VideoLogError("unlimited");
		*use_count = DIVX_DRM_UNLIMIT_VALUE;
		*total_count = DIVX_DRM_UNLIMIT_VALUE;
	} else if (constraint_info.const_type.is_count) {

		*use_count =
		        constraint_info.original_count -
		        constraint_info.remaining_count;
		*total_count = constraint_info.original_count;

		if (constraint_info.remaining_count <= 0) {
			VideoLogError("[ERR]over");
			return FALSE;
		}
	} else if (constraint_info.const_type.is_datetime) {
		VideoLogError("[ERR]is_datetime");
	} else if (constraint_info.const_type.is_interval) {
		VideoLogError("[ERR]is_interval");
	} else if (constraint_info.const_type.is_timedcount) {
		VideoLogError("[ERR]is_timedcount");
	} else if (constraint_info.const_type.is_accumulated) {
		VideoLogError("[ERR]is_accumulated");
	} else if (constraint_info.const_type.is_individual) {
		VideoLogError("[ERR]is_individual");
	} else if (constraint_info.const_type.is_system) {
		VideoLogError("[ERR]is_system");
	}

	return TRUE;
}

void mp_drm_get_file_extension(char *szPath, char *szContentType,
                               int nContentTypeStrSize)
{
	if (!szContentType) {
		VideoLogError("[ERR]");
		return;
	}

	if (!szPath) {
		VideoLogError("[ERR]");
		return;
	}

	memset(szContentType, 0, nContentTypeStrSize);

	drm_content_info_s content_info;
	int nRet = DRM_RETURN_SUCCESS;
	memset(&content_info, 0, sizeof(drm_content_info_s));

	nRet = drm_get_content_info(szPath, &content_info);


	VideoLogInfo("nRet : [%d] contentType: %s", nRet,
	             content_info.mime_type);

	strncpy(szContentType, content_info.mime_type,
	        nContentTypeStrSize - 1);
}
#endif
