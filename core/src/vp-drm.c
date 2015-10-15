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

#include <drm_client.h>
#include <drm_client_types.h>

#include "vp-debug.h"
#include "vp-macro.h"


#include "vp-drm.h"

/* callback functions */


/* internal functions */
static void _vp_drm_print_error_code(drm_result_e nErr)
{
	switch (nErr) {
	case DRM_RETURN_INVALID_ARG:
		vp_dbgE("DRM_RETURN_INVALID_ARG Error");
		break;
	case DRM_RETURN_INSUFFICIENT_MEMORY:
		vp_dbgE("DRM_RETURN_INSUFFICIENT_MEMORY Error");
		break;
	case DRM_RETURN_PARSING_ERROR:
		vp_dbgE("DRM_RETURN_PARSING_ERROR Error");
		break;
	case DRM_RETURN_DB_ERROR:
		vp_dbgE("DRM_RETURN_DB_ERROR Error");
		break;
	case DRM_RETURN_FILE_ERROR:
		vp_dbgE("DRM_RETURN_FILE_ERROR Error");
		break;
	case DRM_RETURN_COMMUNICATION_ERROR:
		vp_dbgE("DRM_RETURN_COMMUNICATION_ERROR Error");
		break;
	case DRM_RETURN_OPL_ERROR:
		vp_dbgE("DRM_RETURN_OPL_ERROR Error");
		break;
	case DRM_RETURN_NO_LICENSE_ERROR:
		vp_dbgE("DRM_RETURN_NO_LICENSE_ERROR Error");
		break;
	case DRM_RETURN_LICENSE_EXPIRED_ERROR:
		vp_dbgE("DRM_RETURN_LICENSE_EXPIRED_ERROR Error");
		break;
	case DRM_RETURN_LICENCE_FUTURE_USE_ERROR:
		vp_dbgE("DRM_RETURN_LICENCE_FUTURE_USE_ERROR Error");
		break;
	case DRM_RETURN_LICENSE_SUSPEND_ERROR:
		vp_dbgE("DRM_RETURN_LICENSE_SUSPEND_ERROR Error");
		break;
	case DRM_RETURN_SERVER_CRASHED:
		vp_dbgE("DRM_RETURN_SERVER_CRASHED Error");
		break;
	case DRM_RETURN_NO_SECURE_CLOCK_ERROR:
		vp_dbgE("DRM_RETURN_NO_SECURE_CLOCK_ERROR Error");
		break;
	case DRM_RETURN_INTERNAL_ERROR:
		vp_dbgE("DRM_RETURN_INTERNAL_ERROR Error");
		break;
	default:
		vp_dbgE("Unknown Error");
		break;
	}

}

/* external functions */
bool vp_drm_is_drm_file(const char *szMediaURL, bool *bIsDRM)
{
	if (!szMediaURL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	int nRet = DRM_RETURN_SUCCESS;
	drm_bool_type_e is_drm = DRM_FALSE;

	*bIsDRM = FALSE;

	nRet = drm_is_drm_file(szMediaURL, &is_drm);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_is_drm_file is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (is_drm == DRM_TRUE) {
		*bIsDRM = TRUE;
	}

	return TRUE;
}

bool vp_drm_is_divx_drm_file(const char *szMediaURL, bool *bIsDivXDRM)
{
	if (!szMediaURL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	drm_file_type_e drm_type = DRM_TYPE_UNDEFINED;
	drm_bool_type_e is_drm = DRM_FALSE;
	int nRet = DRM_RETURN_SUCCESS;

	nRet = drm_is_drm_file(szMediaURL, &is_drm);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_is_drm_file is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (is_drm != DRM_TRUE) {
		*bIsDivXDRM = FALSE;
		return TRUE;
	}

	nRet = drm_get_file_type(szMediaURL, &drm_type);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_get_file_type is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (drm_type == DRM_TYPE_DIVX) {
		*bIsDivXDRM = TRUE;
		vp_dbgW("%s file is DIVX", szMediaURL);
	} else {
		*bIsDivXDRM = FALSE;
		vp_dbgW("%s file is %d type", szMediaURL, drm_type);
	}

	return TRUE;
}

bool vp_drm_is_playready_drm_file(const char *szMediaURL,
				  bool *bIsPlayReadyDRM)
{
	if (!szMediaURL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	drm_file_type_e drm_type = DRM_TYPE_UNDEFINED;
	drm_bool_type_e is_drm = DRM_FALSE;
	int nRet = DRM_RETURN_SUCCESS;

	nRet = drm_is_drm_file(szMediaURL, &is_drm);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_is_drm_file is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (is_drm != DRM_TRUE) {
		*bIsPlayReadyDRM = FALSE;
		return TRUE;
	}

	nRet = drm_get_file_type(szMediaURL, &drm_type);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_get_file_type is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (drm_type == DRM_TYPE_PLAYREADY
			|| drm_type == DRM_TYPE_PLAYREADY_ENVELOPE) {
		*bIsPlayReadyDRM = TRUE;
		vp_dbgW("%s file is PlayReady", szMediaURL);
	} else {
		*bIsPlayReadyDRM = FALSE;
		vp_dbgW("%s file is %d type", szMediaURL, drm_type);
	}

	return TRUE;
}


bool vp_drm_is_check_license(const char *szMediaURL, bool *bCheckLicense)
{
	if (!szMediaURL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	*bCheckLicense = TRUE;

	drm_license_status_e nLicenseStatus = DRM_LICENSE_STATUS_UNDEFINED;
	int nRet =
		drm_get_license_status(szMediaURL, DRM_PERMISSION_TYPE_PLAY,
				       &nLicenseStatus);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_get_license_status is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (nLicenseStatus != DRM_LICENSE_STATUS_VALID) {
		vp_dbgE("Invalid license status : %d", nLicenseStatus);
		*bCheckLicense = FALSE;
	}

	return TRUE;
}

bool vp_drm_is_check_forward_lock(const char *szMediaURL,
				  bool *bIsFowardLock)
{
	if (!szMediaURL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	*bIsFowardLock = FALSE;

	drm_bool_type_e is_drm = DRM_FALSE;
	int nRet = DRM_RETURN_SUCCESS;
	drm_file_type_e drm_type = DRM_TYPE_UNDEFINED;

	nRet = drm_is_drm_file(szMediaURL, &is_drm);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgW("drm_is_drm_file is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	nRet = drm_get_file_type(szMediaURL, &drm_type);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_get_file_type is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (drm_type != DRM_TYPE_OMA_V1 && drm_type != DRM_TYPE_OMA_V2) {
		return FALSE;
	}

	drm_file_info_s st_DrmFileInfo;
	memset(&st_DrmFileInfo, 0x0, sizeof(drm_file_info_s));

	nRet = drm_get_file_info(szMediaURL, &st_DrmFileInfo);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_get_file_info is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (st_DrmFileInfo.oma_info.method == DRM_METHOD_TYPE_FORWARD_LOCK) {
		vp_dbgW
		("st_DrmFileInfo.oma_info.method == DRM_METHOD_TYPE_FORWARD_LOCK");
		*bIsFowardLock = TRUE;
	}

	return TRUE;
}

bool vp_drm_get_constarint_info(const char *szMediaURL,
				vp_drm_constraint_info *pConstraintInfo)
{
	if (!szMediaURL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	drm_constraint_info_s sDrmConstraintInfo;

	memset(&sDrmConstraintInfo, 0, sizeof(drm_constraint_info_s));

	int nRet =
		drm_get_constraint_info(szMediaURL, DRM_PERMISSION_TYPE_PLAY,
					&sDrmConstraintInfo);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_get_constraint_info is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}
	switch (sDrmConstraintInfo.const_type.constraint_type) {
	case DRM_CONSTRAINT_TYPE_RENTAL:
		vp_dbgW("VP_DRM_CONSTRAINT_TYPE_RENTAL");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_RENTAL;
		break;
	case DRM_CONSTRAINT_TYPE_RENTAL_UNLIMITED:
		vp_dbgW("DRM_CONSTRAINT_TYPE_RENTAL_UNLIMITED");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_RENTAL_UNLIMITED;
		break;
	case DRM_CONSTRAINT_TYPE_PURCHASE:
		vp_dbgW("DRM_CONSTRAINT_TYPE_PURCHASE");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_PURCHASE;
		break;
	case DRM_CONSTRAINT_TYPE_BASE:
		vp_dbgW("DRM_CONSTRAINT_TYPE_BASE");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_BASE;
		break;
	case DRM_CONSTRAINT_TYPE_EXPIRED:
		vp_dbgW("DRM_CONSTRAINT_TYPE_EXPIRED");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_EXPIRED;
		break;
	case DRM_CONSTRAINT_TYPE_NOT_AUTHORIZED:
		vp_dbgW("DRM_CONSTRAINT_TYPE_NOT_AUTHORIZED");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_NOT_AUTHORIZED;
		break;
	case DRM_CONSTRAINT_TYPE_INVALID_DRM_VERSION:
		vp_dbgW("DRM_CONSTRAINT_TYPE_INVALID_DRM_VERSION");
		pConstraintInfo->status =
			VP_DRM_CONSTRAINT_TYPE_INVALID_DRM_VERSION;
		break;
	case DRM_CONSTRAINT_TYPE_UNKNOWN_TYPE:
		vp_dbgW("DRM_CONSTRAINT_TYPE_UNKNOWN_TYPE");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_UNKNOWN_TYPE;
		break;
	case DRM_CONSTRAINT_TYPE_GENERAL_ERROR:
		vp_dbgW("DRM_CONSTRAINT_TYPE_GENERAL_ERROR");
		pConstraintInfo->status = VP_DRM_CONSTRAINT_TYPE_GENERAL_ERROR;
		break;
	}

	if (sDrmConstraintInfo.const_type.is_unlimited) {
		vp_dbgW("UNLIMITED");
		pConstraintInfo->constraints = VP_DRM_CONSTRAINT_UNLIMITED;
		return TRUE;
	}

	if (sDrmConstraintInfo.const_type.is_count) {
		vp_dbgW("DRM_COUNT [%d]", sDrmConstraintInfo.remaining_count);
		pConstraintInfo->constraints |= VP_DRM_CONSTRAINT_COUNT;
		pConstraintInfo->original_count =
			sDrmConstraintInfo.original_count;
		pConstraintInfo->remaining_count =
			sDrmConstraintInfo.remaining_count;
	}

	if (sDrmConstraintInfo.const_type.is_datetime) {
		vp_dbgW("DRM_DATETIME");
		pConstraintInfo->constraints |= VP_DRM_CONSTRAINT_DATE_TIME;
		vp_dbgW("%d.%d.%d %d:%d~%d.%d.%d %d:%d",
			sDrmConstraintInfo.start_time.tm_mday,
			sDrmConstraintInfo.start_time.tm_mon,
			sDrmConstraintInfo.start_time.tm_year,
			sDrmConstraintInfo.start_time.tm_hour,
			sDrmConstraintInfo.start_time.tm_min,
			sDrmConstraintInfo.end_time.tm_mday,
			sDrmConstraintInfo.end_time.tm_mon,
			sDrmConstraintInfo.end_time.tm_year,
			sDrmConstraintInfo.end_time.tm_hour,
			sDrmConstraintInfo.end_time.tm_min);

		struct timeval tv;
		gettimeofday(&tv, NULL);
		struct tm *ptm = NULL;
		ptm = localtime(&tv.tv_sec);
		ptm->tm_mon += 1;
		ptm->tm_year += 1900;

		time_t local_t = mktime(ptm);
		time_t start_t = mktime(&(sDrmConstraintInfo.start_time));
		time_t end_t = mktime(&(sDrmConstraintInfo.end_time));


		vp_dbgW("%d.%d.%d %d:%d",
			ptm->tm_mday, ptm->tm_mon, ptm->tm_year,
			ptm->tm_hour, ptm->tm_min);

		if (local_t >= start_t &&local_t <= end_t) {
			pConstraintInfo->date_time_expired = FALSE;
		} else {
			pConstraintInfo->date_time_expired = TRUE;
		}

	}

	if (sDrmConstraintInfo.const_type.is_interval) {
		vp_dbgW("DRM_INTERVAL");
		pConstraintInfo->constraints |= VP_DRM_CONSTRAINT_INTERVAL;
		vp_dbgW("Remain... %d.%d.%d %d:%d",
			sDrmConstraintInfo.interval_time.tm_mon,
			sDrmConstraintInfo.interval_time.tm_mday,
			sDrmConstraintInfo.interval_time.tm_year,
			sDrmConstraintInfo.interval_time.tm_hour,
			sDrmConstraintInfo.interval_time.tm_min);

		pConstraintInfo->remaining_interval_sec =
			sDrmConstraintInfo.interval_time.tm_sec +
			sDrmConstraintInfo.interval_time.tm_min * 60 +
			sDrmConstraintInfo.interval_time.tm_hour * 3600;
		pConstraintInfo->remaining_interval_sec +=
			(sDrmConstraintInfo.interval_time.tm_mday +
			 sDrmConstraintInfo.interval_time.tm_mon * 30 +
			 sDrmConstraintInfo.interval_time.tm_year * 365) * (3600 *
					 24);
	}

	if (sDrmConstraintInfo.const_type.is_timedcount) {
		vp_dbgW("DRM_TIMED_COUNT");
		vp_dbgW("%d left (%d sec)",
			sDrmConstraintInfo.timed_remaining_count,
			sDrmConstraintInfo.timed_count_timer);
		pConstraintInfo->constraints |= VP_DRM_CONSTRAINT_TIMED_COUNT;
		pConstraintInfo->remaining_timed_count =
			sDrmConstraintInfo.timed_remaining_count;
	}

	if (sDrmConstraintInfo.const_type.is_accumulated) {
		vp_dbgW("DRM_ACCUMULATED [%d]",
			sDrmConstraintInfo.accumulated_remaining_seconds);
		pConstraintInfo->constraints |= VP_DRM_CONSTRAINT_ACCUMLATED_TIME;
		pConstraintInfo->remaining_acc_sec =
			sDrmConstraintInfo.accumulated_remaining_seconds;
	}

	if (sDrmConstraintInfo.const_type.is_individual) {
		vp_sdbg("DRM_INDIVISUAL_ID [%s]",
			sDrmConstraintInfo.individual_id);
	}

	if (sDrmConstraintInfo.const_type.is_system) {
		vp_sdbg("DRM_SYSTEM [ID:%s, type:%d]",
			sDrmConstraintInfo.system_id,
			sDrmConstraintInfo.system_identity_type);
	}

	return TRUE;
}

bool vp_drm_get_file_mime_type(const char *szMediaURL, char **szMime)
{
	if (!szMediaURL) {
		vp_dbgE("[ERR]");
		return FALSE;
	}


	drm_bool_type_e is_drm = DRM_FALSE;
	int nRet = DRM_RETURN_SUCCESS;
	drm_file_type_e drm_type = DRM_TYPE_UNDEFINED;

	nRet = drm_is_drm_file(szMediaURL, &is_drm);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgW("drm_is_drm_file is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	nRet = drm_get_file_type(szMediaURL, &drm_type);
	if (nRet != DRM_RETURN_SUCCESS) {
		vp_dbgE("drm_get_file_type is fail");
		_vp_drm_print_error_code(nRet);
		return FALSE;
	}

	if (drm_type == DRM_TYPE_PLAYREADY
			|| drm_type == DRM_TYPE_PLAYREADY_ENVELOPE) {
		vp_sdbg("contentType: video/vnd.ms-playready.media.pyv");
		return FALSE;
	} else if (drm_type == DRM_TYPE_OMA_V1 || drm_type == DRM_TYPE_OMA_V2) {
		drm_content_info_s content_info;

		int nRet = DRM_RETURN_SUCCESS;
		memset(&content_info, 0, sizeof(drm_content_info_s));

		nRet = drm_get_content_info(szMediaURL, &content_info);
		if (nRet != DRM_RETURN_SUCCESS) {
			vp_dbgE("drm_get_constraint_info is fail");
			_vp_drm_print_error_code(nRet);
			return FALSE;
		}

		vp_sdbg("contentType: %s", content_info.mime_type);
		VP_STRDUP(*szMime, content_info.mime_type);
	} else {
		return FALSE;
	}

	return TRUE;
}


bool
vp_drm_get_store_item_license(char *pPath, char *pStoreAppId,
			      char *pUserId, char *pImei, char *pOrderId,
			      char *pMvId, char *pServerId)
{
	int nRet = -1;

	// for domain hardening, license aquisition
	drm_initiator_info_s initiator_info = { "", 0, };
	drm_web_server_resp_data_s ws_resp_data = { "", 0, };

	if (!pPath) {
		vp_dbgE("[ERR]");
		return FALSE;
	}

	if (!pStoreAppId) {
		vp_dbgE("[ERR]");
		return FALSE;
	}

	if (!pUserId) {
		vp_dbgE("[ERR]");
		return FALSE;
	}

	if (!pImei) {
		vp_dbgE("[ERR]");
		return FALSE;
	}

	if (!pOrderId) {
		vp_dbgE("[ERR]");
		return FALSE;
	}

	if (!pMvId) {
		vp_dbgE("[ERR]");
		return FALSE;
	}

	if (!pServerId) {
		vp_dbgE("[ERR]");
		return FALSE;
	}

	vp_sdbg
	("pPath: [%s], pStoreAppId: [%s], pUserId: [%s], pImei: [%s], pOrderId: [%s], pMvId: [%s], pServerId: [%s]",
	 pPath, pStoreAppId, pUserId, pImei, pOrderId, pMvId, pServerId);

	initiator_info.init_type = DRM_INITIATOR_TYPE_LICENSE_ACQ;
	initiator_info.initiator_url_len = strlen(pPath);
	snprintf(initiator_info.initiator_url, DRM_MAX_LEN_INITIATOR_URL,
		 "%s", pPath);
	snprintf(initiator_info.custom_data.app_id, DRM_MAX_LEN_APP_ID + 1,
		 "%s", pStoreAppId);
	snprintf(initiator_info.custom_data.user_guid,
		 DRM_MAX_LEN_USER_GUID + 1, "%s", pUserId);
	snprintf(initiator_info.custom_data.device_id,
		 DRM_MAX_LEN_DEVICE_ID + 1, "%s", pImei);
	snprintf(initiator_info.custom_data.order_id,
		 DRM_MAX_LEN_ORDER_ID + 1, "%s", pOrderId);
	snprintf(initiator_info.custom_data.mv_id, DRM_MAX_LEN_MV_ID + 1,
		 "%s", pMvId);
	snprintf(initiator_info.custom_data.svr_id, DRM_MAX_LEN_SVR_ID + 1,
		 "%s", pServerId);

	nRet =
		drm_process_request(DRM_REQUEST_TYPE_SUBMIT_INITIATOR_URL,
				    (void *)(&initiator_info),
				    (void *)(&ws_resp_data));

	if (nRet == DRM_RETURN_SUCCESS) {
		vp_dbg("Server Error Code = %d", ws_resp_data.result_code);
		vp_sdbg("Content URL = %s", ws_resp_data.content_url);
	} else {
		vp_dbgE("drm_process_request failed!!!");
		_vp_drm_print_error_code((drm_result_e) nRet);
		return FALSE;
	}

	return TRUE;
}
#endif
