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

#include "vp-db-common.h"

#include "vp-media-content-util.h"



/* callback functions */
static bool __vp_media_content_info_cb(media_info_h pMediaHandle,
                                       void *pUserData)
{
	media_info_h *pAssignItem = (media_info_h *) pUserData;

	if (pMediaHandle != NULL) {
		int nRet = MEDIA_CONTENT_ERROR_NONE;

		nRet = media_info_clone(pAssignItem, pMediaHandle);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			vp_dbgE("media_info_clone is fail : 0x%x", nRet);
			return FALSE;
		}

		char *szTmpStr = NULL;
		nRet = media_info_get_display_name(*pAssignItem, &szTmpStr);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			vp_dbgE("media_info_get_display_name is fail : 0x%x", nRet);
			return FALSE;
		}

		vp_dbg("Display name : %s", szTmpStr);
		VP_FREE(szTmpStr);
	} else {
		vp_dbgE("pMeidaHandle is NULL");
		return FALSE;
	}

	return TRUE;
}

/* internal functions */


/* external functions */
bool vp_media_content_util_get_video_id(const char *szFilePath,
                                        char **szVideoID)
{
	if (szFilePath == NULL) {
		vp_dbgE("szFilePath is NULL");
		return FALSE;
	}

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;
	char *szMediaID = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096, "MEDIA_PATH = \"%s\"", szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_create is fail : 0x%x", nRet);
		goto Execption;
	}

	if (pFilterHandle == NULL) {
		vp_dbgE("Filter handle is NULL");
		goto Execption;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_set_condition is fail : 0x%x", nRet);
		goto Execption;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_content_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_set_condition is fail : 0x%x", nRet);
		goto Execption;
	}

	if (pMediaHandle == NULL) {
		vp_dbgE("media info handle is NULL");
		goto Execption;
	}

	nRet = media_info_get_media_id(pMediaHandle, &szMediaID);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_info_get_media_id is fail : 0x%x", nRet);
		goto Execption;
	}

	VP_STRDUP(*szVideoID, szMediaID);
	VP_FREE(szMediaID);

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_destroy is fail : 0x%x", nRet);
		goto Execption;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_info_destroy is fail : 0x%x", nRet);
		goto Execption;
	}

	return TRUE;

Execption:
	VP_FREE(szMediaID);

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	if (pMediaHandle) {
		media_info_destroy(pMediaHandle);
		pMediaHandle = NULL;
	}
	return FALSE;
}

bool vp_media_content_util_get_first_video(char **szFilePath)
{
	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;
	char *szMediaID = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	nRet = media_content_connect();
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_content_connect is fail : 0x%x", nRet);
		return FALSE;
	}

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096, "MEDIA_TYPE = 1");

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_create is fail : 0x%x", nRet);
		goto Execption;
	}

	if (pFilterHandle == NULL) {
		vp_dbgE("Filter handle is NULL");
		goto Execption;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_set_condition is fail : 0x%x", nRet);
		goto Execption;
	}

	nRet =
	    media_filter_set_order(pFilterHandle, MEDIA_CONTENT_ORDER_DESC,
	                           MEDIA_MODIFIED_TIME,
	                           MEDIA_CONTENT_COLLATE_NOCASE);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_set_order is fail : 0x%x", nRet);
		goto Execption;
	}

	nRet = media_filter_set_offset(pFilterHandle, 0, 1);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_set_offset is fail : 0x%x", nRet);
		goto Execption;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_content_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Execption;
	}

	if (pMediaHandle == NULL) {
		vp_dbgE("media info handle is NULL");
		goto Execption;
	}

	nRet = media_info_get_file_path(pMediaHandle, szFilePath);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_info_get_media_id is fail : 0x%x", nRet);
		goto Execption;
	}


	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_filter_destroy is fail : 0x%x", nRet);
		goto Execption;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_info_destroy is fail : 0x%x", nRet);
		goto Execption;
	}
	nRet = media_content_disconnect();
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		vp_dbgE("media_content_connect is fail : 0x%x", nRet);
		goto Execption;
	}

	return TRUE;

Execption:
	VP_FREE(szMediaID);

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	if (pMediaHandle) {
		media_info_destroy(pMediaHandle);
		pMediaHandle = NULL;
	}
	return FALSE;
}
