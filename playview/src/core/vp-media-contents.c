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
#include <media_content.h>
#include <metadata_extractor.h>


#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-media-contents.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-file-util.h"


static bool g_bMediaConnect = FALSE;
static MediaUpdateCb g_media_update_cb = NULL;



/* callback functions */
static bool __vp_media_contents_info_cb(media_info_h pMediaHandle,
                                        void *pUserData)
{
	media_info_h *pAssignItem = (media_info_h *) pUserData;

	if (pMediaHandle != NULL) {
		int nRet = MEDIA_CONTENT_ERROR_NONE;

		nRet = media_info_clone(pAssignItem, pMediaHandle);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("media_info_clone is fail : 0x%x", nRet);
			return FALSE;
		}

		char *szTmpStr = NULL;
		nRet = media_info_get_display_name(*pAssignItem, &szTmpStr);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("media_info_get_display_name is fail : 0x%x",
			              nRet);
			return FALSE;
		}

		VideoLogInfo("Display name : %s", szTmpStr);
		VP_FREE(szTmpStr);
	} else {
		VideoLogError("pMeidaHandle is NULL");
		return FALSE;
	}

	return TRUE;
}

static bool __vp_media_contents_bookmark_iter_cb(media_bookmark_h
                                        pBookmark,
                                        void *pUserData)
{
	if (pBookmark == NULL) {
		VideoLogError("pBookmark is NULL");
		return FALSE;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}

	media_bookmark_h pAssignBookmark = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;

	nRet = media_bookmark_clone(&pAssignBookmark, pBookmark);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_bookmark_clone is fail : 0x%x", nRet);
		return FALSE;
	}
	VideoLogWarning("== list append ==");

	GList **pList = (GList **) pUserData;
	*pList = g_list_append(*pList, pAssignBookmark);

	return TRUE;
}


static bool __vp_media_contents_video_iter_cb(media_info_h pMediaInfo,
                                        	void *pUserData)
{
	if (pMediaInfo == NULL) {
		VideoLogError("pMediaInfo is NULL");
		return FALSE;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}

	media_info_h pAssignMedia = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	media_content_type_e nMediaType = MEDIA_CONTENT_TYPE_OTHERS;

	nRet = media_info_get_media_type(pMediaInfo, &nMediaType);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_media_type is fail : 0x%x", nRet);
		return FALSE;
	}

	if (nMediaType == MEDIA_CONTENT_TYPE_VIDEO) {
		nRet = media_info_clone(&pAssignMedia, pMediaInfo);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("media_info_clone is fail : 0x%x", nRet);
			return FALSE;
		}
		GList **pList = (GList **) pUserData;
		*pList = g_list_append(*pList, pAssignMedia);
	}

	return TRUE;
}

static int __vp_media_contents_get_pid(void)
{
	return getpid();
}

static void __vp_media_contents_db_update_cb(media_content_error_e nError,
        		int nPid,
        		media_content_db_update_item_type_e
        		nUpdateItem,
        		media_content_db_update_type_e
        		nUpdateType,
        		media_content_type_e
        		nMediaType, char *szUuid,
        		char *szPath,
        		char *szMimeType,
        		void *pUserData)
{
	if (g_media_update_cb == NULL) {
		VideoLogError("g_media_update_cb is NULL.");
		return;
	}

	if (nError) {
		VideoLogError("error occured! : %d", nError);
		return;
	}

	if (nPid == __vp_media_contents_get_pid()) {
		VideoLogError("It is same processor");
		return;
	}

	g_media_update_cb(pUserData);
}

static bool __vp_media_contents_folder_iter_cb(media_folder_h pFolderItem,
        		void *pUserData)
{
	if (pFolderItem == NULL) {
		VideoLogError("pFolderItem is NULL");
		return FALSE;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}
	media_folder_h *pAssignFolderItem = (media_folder_h *) pUserData;
	if (pFolderItem != NULL) {
		char *szTmpStr = NULL;
		media_folder_clone(pAssignFolderItem, pFolderItem);
		media_folder_get_name(*pAssignFolderItem, &szTmpStr);
		VideoLogInfo("Folder name : %s", szTmpStr);

		if (szTmpStr) {
			free(szTmpStr);
		}
	}

	return TRUE;
}

static bool __vp_media_contents_subtitles_iter_cb(media_info_h pMediaInfo,
        		void *pUserData)
{
	if (pMediaInfo == NULL) {
		VideoLogError("pMediaInfo is NULL");
		return FALSE;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	char *pszFilePath = NULL;

	nRet = media_info_get_file_path(pMediaInfo, &pszFilePath);

	VideoSecureLogInfo("pszFilePath is: %s", pszFilePath);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_file_path is fail : 0x%x", nRet);
		return FALSE;
	}

	GList **pList = (GList **) pUserData;
	*pList = g_list_append(*pList, pszFilePath);

	return TRUE;
}


/* external functions */
bool vp_media_contents_connect()
{
	int nRet = MEDIA_CONTENT_ERROR_NONE;

	if (g_bMediaConnect) {
//              VideoLogDebug("Already Media content connect");
		return TRUE;
	}

	nRet = media_content_connect();
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_content_connect is fail : 0x%x", nRet);
		return FALSE;
	}

	g_bMediaConnect = TRUE;

	return TRUE;
}

bool vp_media_contents_disconnect()
{
	int nRet = MEDIA_CONTENT_ERROR_NONE;

	if (g_bMediaConnect == FALSE) {
		VideoLogWarning("Already Media content disconnect");
		return TRUE;
	}

	nRet = media_content_disconnect();
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_content_connect is fail : 0x%x", nRet);
		return FALSE;
	}

	g_bMediaConnect = FALSE;

	return TRUE;
}


bool vp_media_contents_get_video_id(const char *szFilePath,
                                    char **szVideoID)
{
	if (szVideoID == NULL) {
		VideoLogError("szVideoID is NULL");
		return FALSE;
	}

	/* Initialize output value */
	*szVideoID = NULL;

	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	char *szMediaID = NULL;

	char szTmpStr[4096] = {0,};
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_media_id(pMediaHandle, &szMediaID);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_media_id is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	VP_STRDUP(*szVideoID, szMediaID);
	VP_FREE(szMediaID);

	return TRUE;

Exception:

	VP_FREE(szMediaID);
	*szVideoID = NULL;

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

bool vp_media_contents_get_video_thumbnail_path(const char *szFilePath,
        char **szThumbnailPath)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	char *szThumbnail = NULL;

	char szTmpStr[4096] = {0,};
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_thumbnail_path(pMediaHandle, &szThumbnail);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_thumbnail_path is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	VP_STRDUP(*szThumbnailPath, szThumbnail);
	VP_FREE(szThumbnail);

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

	VP_FREE(szThumbnail);

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

bool vp_media_contents_scan_file(const char *szFilePath)
{
	if (!szFilePath) {
		VideoLogError("szFilePath is NULL.");
		return FALSE;
	}
	media_content_scan_file(szFilePath);
	return TRUE;
}

bool vp_media_contents_update_db(const char *szMediaURL)
{
	if (!szMediaURL) {
		VideoLogError("No exist szMediaURL.");
		return FALSE;
	}

	media_info_h info = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	nRet = media_info_insert_to_db(szMediaURL, &info);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_insert_to_db fail : %d", nRet);
		if (info) {
			media_info_destroy(info);
			info = NULL;
		}
		return FALSE;
	}

	if (info) {
		media_info_destroy(info);
		info = NULL;
	}

	return TRUE;
}

bool vp_media_contents_refresh_db(const char *szMediaID)
{
	if (!szMediaID) {
		VideoLogError("No exist szMediaID.");
		return FALSE;
	}

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	nRet = media_info_refresh_metadata_to_db(szMediaID);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_refresh_metadata_to_db fail : %d",
		              nRet);
		return FALSE;
	}
	return TRUE;
}

bool vp_media_contents_set_played_time(const char *szFilePath)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;
	video_meta_h pVideoHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_video(pMediaHandle, &pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_video is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pVideoHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	time_t tNowTime = 0;
	time(&tNowTime);

	nRet = video_meta_set_played_time(pVideoHandle, tNowTime);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError(" video_meta_set_played_time is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = video_meta_update_to_db(pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_update_to_db is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = video_meta_destroy(pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	if (pMediaHandle) {
		media_info_destroy(pMediaHandle);
		pMediaHandle = NULL;
	}

	if (pVideoHandle) {
		video_meta_destroy(pVideoHandle);
		pVideoHandle = NULL;
	}
	return FALSE;
}

bool vp_media_contents_set_played_position(const char *szFilePath,
        int nPosition)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;
	video_meta_h pVideoHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_video(pMediaHandle, &pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_video is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pVideoHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = video_meta_set_played_position(pVideoHandle, nPosition);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError(" video_meta_set_played_position is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	nRet = video_meta_update_to_db(pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_update_to_db is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = video_meta_destroy(pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	if (pMediaHandle) {
		media_info_destroy(pMediaHandle);
		pMediaHandle = NULL;
	}

	if (pVideoHandle) {
		video_meta_destroy(pVideoHandle);
		pVideoHandle = NULL;
	}
	return FALSE;
}

bool vp_media_contents_get_played_position(const char *szFilePath,
        int *nPosition)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;
	video_meta_h pVideoHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_video(pMediaHandle, &pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_video is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pVideoHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = video_meta_get_played_position(pVideoHandle, nPosition);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_get_played_position is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	nRet = video_meta_destroy(pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	if (pMediaHandle) {
		media_info_destroy(pMediaHandle);
		pMediaHandle = NULL;
	}

	if (pVideoHandle) {
		video_meta_destroy(pVideoHandle);
		pVideoHandle = NULL;
	}
	return FALSE;


}

int vp_media_contents_get_modified_time(media_info_h pMediaHandle,
                                        char **szLastModified)
{
	if (!pMediaHandle) {
		VideoLogInfo("pMediaHandle is NULL.");
		return MEDIA_CONTENT_ERROR_INVALID_PARAMETER;
	}

	VideoLogInfo("");

	media_info_h pVideoItem = (media_info_h) pMediaHandle;
	int nRet = MEDIA_CONTENT_ERROR_NONE;

	time_t pModifiedTime;
	struct tm sModifiedTime;
	char szModifiedTime[BYTE_SIZE] = {0,};

	nRet = media_info_get_modified_time(pVideoItem, &pModifiedTime);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_modified_time failed : [%d]!!!",
		              nRet);
		return nRet;
	}

	gmtime_r(&pModifiedTime, &sModifiedTime);
	strftime(szModifiedTime, BYTE_SIZE - 1, "%d %b. %Y. %H:%M",
	         &sModifiedTime);
	*szLastModified = g_strdup(szModifiedTime);

	return nRet;
}

bool vp_media_contents_get_content_info(const char *szFilePath,
                                        char **szTitle, int *nWidth,
                                        int *nHeight, int *nDuration,
                                        double *dLatitude,
                                        double *dLongitude,
                                        char **szTmpLastModified)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;
	video_meta_h pVideoHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = {0,};
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_display_name(pMediaHandle, szTitle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_display_name is fail : 0x%x", nRet);
	}

	nRet = media_info_get_video(pMediaHandle, &pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_video is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    vp_media_contents_get_modified_time(pMediaHandle,
	                                        szTmpLastModified);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_modified_time is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	nRet = media_info_get_latitude(pMediaHandle, dLatitude);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_latitude is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_get_longitude(pMediaHandle, dLongitude);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_longitude is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pVideoHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = video_meta_get_width(pVideoHandle, nWidth);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_get_width is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = video_meta_get_height(pVideoHandle, nHeight);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_get_height is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = video_meta_get_duration(pVideoHandle, nDuration);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_get_duration is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = video_meta_destroy(pVideoHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("video_meta_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	if (pMediaHandle) {
		media_info_destroy(pMediaHandle);
		pMediaHandle = NULL;
	}

	if (pVideoHandle) {
		video_meta_destroy(pVideoHandle);
		pVideoHandle = NULL;
	}
	return FALSE;
}

bool vp_media_metadata_get_gps_info(const char *szFilePath,
                                    double *dLatitude, double *dLongitude)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	metadata_extractor_h pMetadata;
	char *szVal = NULL;

	int nRet = METADATA_EXTRACTOR_ERROR_NONE;

	nRet = metadata_extractor_create(&pMetadata);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_create is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = metadata_extractor_set_path(pMetadata, szFilePath);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_set_path is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    metadata_extractor_get_metadata(pMetadata, METADATA_LATITUDE,
	                                    &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_get_metadata is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	if (szVal) {
		*dLatitude = atof(szVal);
	} else {
		*dLatitude = 0.0;
	}
	VP_FREE(szVal);

	nRet =
	    metadata_extractor_get_metadata(pMetadata, METADATA_LONGITUDE,
	                                    &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_get_metadata is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	if (szVal) {
		*dLongitude = atof(szVal);
	} else {
		*dLongitude = 0.0;
	}

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return TRUE;
Exception:

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return FALSE;
}

bool vp_media_metadata_get_duration(const char *szFilePath,
                                    int *nDuration)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	metadata_extractor_h pMetadata;
	char *szVal = NULL;

	int nRet = METADATA_EXTRACTOR_ERROR_NONE;

	nRet = metadata_extractor_create(&pMetadata);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_create is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = metadata_extractor_set_path(pMetadata, szFilePath);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_set_path is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    metadata_extractor_get_metadata(pMetadata, METADATA_DURATION,
	                                    &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_get_metadata is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	if (szVal) {
		*nDuration = atoi(szVal);
	} else {
		*nDuration = 0.0;
	}

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return TRUE;
Exception:

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return FALSE;

}

bool vp_media_metadata_get_width(const char *szFilePath, int *nWidth)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	metadata_extractor_h pMetadata;
	char *szVal = NULL;

	int nRet = METADATA_EXTRACTOR_ERROR_NONE;

	nRet = metadata_extractor_create(&pMetadata);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_create is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = metadata_extractor_set_path(pMetadata, szFilePath);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_set_path is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    metadata_extractor_get_metadata(pMetadata, METADATA_VIDEO_WIDTH,
	                                    &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_get_metadata is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	if (szVal) {
		*nWidth = atoi(szVal);
	} else {
		*nWidth = 0;
	}

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return TRUE;
Exception:

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return FALSE;

}

bool vp_media_metadata_get_height(const char *szFilePath, int *nHeight)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	metadata_extractor_h pMetadata;
	char *szVal = NULL;

	int nRet = METADATA_EXTRACTOR_ERROR_NONE;

	nRet = metadata_extractor_create(&pMetadata);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_create is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = metadata_extractor_set_path(pMetadata, szFilePath);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_set_path is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    metadata_extractor_get_metadata(pMetadata, METADATA_VIDEO_HEIGHT,
	                                    &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_get_metadata is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	if (szVal) {
		*nHeight = atoi(szVal);
	} else {
		*nHeight = 0;
	}

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return TRUE;
Exception:

	VP_FREE(szVal);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return FALSE;

}


bool vp_media_contents_bookmark_list_get(const char *szMediaID,
        GList **list)
{
	if (!szMediaID) {
		VideoLogError("No exist szMediaID.");
		return FALSE;
	}

	if (!list) {
		VideoLogError("No exist list.");
		return FALSE;
	}

	vp_media_contents_connect();

	filter_h pFilterHandle = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("pFilterHandle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_order(pFilterHandle, MEDIA_CONTENT_ORDER_ASC,
	                           BOOKMARK_MARKED_TIME,
	                           MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_order is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_bookmark_from_db(szMediaID, pFilterHandle,
	                                        __vp_media_contents_bookmark_iter_cb,
	                                        list);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError
		("media_info_foreach_bookmark_from_db is fail : 0x%x", nRet);
		goto Exception;
	}


	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		return FALSE;
	}

	return TRUE;

Exception:

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	return FALSE;
}


bool vp_media_contents_bookmark_list_clear(GList *pList)
{
	VideoLogInfo("");

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	int nCount = 0;
	int i = 0;

	nCount = g_list_length(pList);
	for (i = 0; i < nCount; i++) {
		media_bookmark_h pBookmarkItem = NULL;
		pBookmarkItem = (media_bookmark_h)g_list_nth_data(pList, i);
		if (pBookmarkItem) {
			media_bookmark_destroy(pBookmarkItem);
			pBookmarkItem = NULL;
		}
	}

	g_list_free(pList);
	pList = NULL;

	return TRUE;
}

bool vp_media_contents_bookmark_insert(const char *szMediaID,
                                       int nPosition,
                                       const char *szFilePath)
{

	if (szMediaID == NULL) {
		VideoLogError("szMediaID is NULL");
		return FALSE;
	}

	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	if (nPosition < 0) {
		VideoLogError("nPosition is Invalid");
		return FALSE;
	}

	vp_media_contents_connect();

	if (!vp_file_exists(szFilePath)) {
		VideoLogError("(%s) file doesnot exist", szFilePath);
		return FALSE;
	}

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	nRet = media_bookmark_insert_to_db(szMediaID, nPosition, szFilePath);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_bookmark_insert_to_db is fail : 0x%x", nRet);
		return FALSE;
	}

	return TRUE;
}



bool vp_media_contents_bookmark_delete(const char *szMediaID,
                                       int nPosition)
{
	if (szMediaID == NULL) {
		VideoLogError("szMediaID is NULL");
		return FALSE;
	}

	if (nPosition < 0) {
		VideoLogError("nPosition is Invalid");
		return FALSE;
	}

	GList *list = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	int nCount = 0;
	int idx = 0;
	int nDeleteBookmarkID = -1;
	char *szDeletePath = NULL;

	if (!vp_media_contents_bookmark_list_get(szMediaID, &list)) {
		VideoLogError("vp_media_contents_bookmark_list_get is fail");
		return FALSE;
	}

	nCount = g_list_length(list);
	for (idx = 0; idx < nCount; idx++) {
		media_bookmark_h pBookmarkItem = NULL;
		pBookmarkItem = (media_bookmark_h)g_list_nth_data(list, idx);
		if (pBookmarkItem) {
			time_t nMarkedTime;
			nRet =
			    media_bookmark_get_marked_time(pBookmarkItem,
			                                   &nMarkedTime);
			if (nRet != MEDIA_CONTENT_ERROR_NONE) {
				VideoLogWarning
				("media_bookmark_get_marked_time is fail : 0x%x",
				 nRet);
				continue;
			}

			if (nPosition == nMarkedTime) {
				nRet =
				    media_bookmark_get_bookmark_id(pBookmarkItem,
				                                   &nDeleteBookmarkID);
				if (nRet != MEDIA_CONTENT_ERROR_NONE) {
					VideoLogWarning
					("media_bookmark_get_bookmark_id is fail : 0x%x",
					 nRet);
					continue;
				}

				nRet =
				    media_bookmark_get_thumbnail_path(pBookmarkItem,
				                                      &szDeletePath);
				if (nRet != MEDIA_CONTENT_ERROR_NONE) {
					VideoLogWarning
					("media_bookmark_get_thumbnail_path is fail : 0x%x",
					 nRet);
					continue;
				}
				break;
			}
		}
	}

	vp_media_contents_bookmark_list_clear(list);
	list = NULL;

	if (nDeleteBookmarkID == -1 || szDeletePath == NULL) {
		VideoLogError("Not match item : %d , %s", nDeleteBookmarkID,
		              szDeletePath);
		VP_FREE(szDeletePath);
		return FALSE;
	}

	nRet = media_bookmark_delete_from_db(nDeleteBookmarkID);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_bookmark_delete_from_db is fail : 0x%x",
		              nRet);
		VP_FREE(szDeletePath);
		return FALSE;
	}


	if (vp_file_exists(szDeletePath)) {
		if (!vp_file_unlink(szDeletePath)) {
			VideoLogWarning("Remove Failed :%s", szDeletePath);
		}
	} else {
		VideoLogWarning("%s doesnot exist", szDeletePath);
	}

	VP_FREE(szDeletePath);

	return TRUE;
}



bool vp_media_contents_bookmark_delete_all(const char *szMediaID)
{
	if (szMediaID == NULL) {
		VideoLogError("szMediaID is NULL");
		return FALSE;
	}

	GList *list = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	int nCount = 0;
	int idx = 0;

	if (!vp_media_contents_bookmark_list_get(szMediaID, &list)) {
		VideoLogError("vp_media_contents_bookmark_list_get is fail");
		return FALSE;
	}

	nCount = g_list_length(list);
	for (idx = 0; idx < nCount; idx++) {
		media_bookmark_h pBookmarkItem = NULL;
		int nDeleteBookmarkID = -1;
		char *szDeletePath = NULL;

		pBookmarkItem = (media_bookmark_h) g_list_nth_data(list, idx);
		if (pBookmarkItem) {
			nRet =
			    media_bookmark_get_bookmark_id(pBookmarkItem,
			                                   &nDeleteBookmarkID);
			if (nRet != MEDIA_CONTENT_ERROR_NONE) {
				VideoLogWarning
				("media_bookmark_get_bookmark_id is fail : 0x%x",
				 nRet);
			}

			nRet =
			    media_bookmark_get_thumbnail_path(pBookmarkItem,
			                                      &szDeletePath);
			if (nRet != MEDIA_CONTENT_ERROR_NONE) {
				VideoLogWarning
				("media_bookmark_get_thumbnail_path is fail : 0x%x",
				 nRet);
			}

			nRet = media_bookmark_delete_from_db(nDeleteBookmarkID);
			if (nRet != MEDIA_CONTENT_ERROR_NONE) {
				VideoLogError
				("media_bookmark_delete_from_db is fail : 0x%x",
				 nRet);
			}


			if (vp_file_exists(szDeletePath)) {
				if (!vp_file_unlink(szDeletePath)) {
					VideoLogWarning("Remove Failed :%s", szDeletePath);
				}
			} else {
				VideoLogWarning("The file not eixst :%s", szDeletePath);
			}

			VP_FREE(szDeletePath);
		}
	}

	vp_media_contents_bookmark_list_clear(list);
	list = NULL;

	return TRUE;
}


bool vp_media_contents_bookmark_get_item_info(void *pItem,
        char **szFilePath,
        int *nPosition)
{
	if (pItem == NULL) {
		VideoLogError("szMediaID is NULL");
		return FALSE;
	}

	media_bookmark_h bookmark_h = (media_bookmark_h) pItem;
	int nRet = MEDIA_CONTENT_ERROR_NONE;

	nRet =
	    media_bookmark_get_marked_time(bookmark_h, (time_t *) nPosition);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_bookmark_get_marked_time is fail : 0x%x",
		              nRet);
		return FALSE;
	}

	nRet = media_bookmark_get_thumbnail_path(bookmark_h, szFilePath);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_bookmark_get_thumbnail_path is fail : 0x%x",
		              nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_media_contents_get_video_items_to_folder(int nSortType,
        char *szFolder,
        GList **pList)
{
	if (szFolder == NULL) {
		VideoLogError("szFolder is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	filter_h pFilterHandle = NULL;
	media_folder_h pFolderHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	video_play_sort_type_t nType = (video_play_sort_type_t) nSortType;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;

	char *szOrder = NULL;
	char *szCondition = NULL;
	char *szFolderID = NULL;

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("pFilterHandle is NULL");
		goto Exception;
	}


	switch (nType) {
	case VIDEO_SORT_BY_NONE:
	case VIDEO_SORT_BY_NAME_A_Z:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		VP_STRDUP(szOrder, MEDIA_TITLE);
		break;

	case VIDEO_SORT_BY_NAME_Z_A:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		VP_STRDUP(szOrder, MEDIA_TITLE);
		break;

	case VIDEO_SORT_BY_DATE_MOST_RECENT:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		VP_STRDUP(szOrder, MEDIA_MODIFIED_TIME);
		break;

	case VIDEO_SORT_BY_OLDEST:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		VP_STRDUP(szOrder, MEDIA_MODIFIED_TIME);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		VP_STRDUP(szOrder, MEDIA_TITLE);
	}

	szCondition =
	    g_strdup_printf
	    ("MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE IS NOT 101) and FOLDER_PATH = \"%s\"",
	     szFolder);
	nRet =
	    media_filter_set_condition(pFilterHandle, szCondition,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_folder_foreach_folder_from_db(pFilterHandle,
	                                        __vp_media_contents_folder_iter_cb,
	                                        &pFolderHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError
		("media_folder_foreach_folder_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFolderHandle == NULL) {
		VideoLogError("pFolderHandle is NULL");
		goto Exception;
	}

	nRet = media_folder_get_folder_id(pFolderHandle, &szFolderID);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_folder_get_folder_id is fail : 0x%x", nRet);
		goto Exception;
	}

	VP_FREE(szCondition);

	szCondition = g_strdup_printf("MEDIA_TYPE = 1");
	nRet =
	    media_filter_set_condition(pFilterHandle, szCondition,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_filter_set_order(pFilterHandle, nOrderType, szOrder,
	                           MEDIA_CONTENT_COLLATE_NOCASE);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_order is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_folder_foreach_media_from_db(szFolderID, pFilterHandle,
	                                       __vp_media_contents_video_iter_cb,
	                                       pList);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	VP_FREE(szFolderID);
	VP_FREE(szCondition);
	VP_FREE(szOrder);

	return TRUE;

Exception:

	VP_FREE(szFolderID);
	VP_FREE(szCondition);
	VP_FREE(szOrder);

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	return FALSE;
}

bool vp_media_contents_get_video_items(int nSortType, GList **pList)
{
	vp_media_contents_connect();

	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	video_play_sort_type_t nType = (video_play_sort_type_t) nSortType;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;
	char *szOrder = NULL;
	char *szCondition = NULL;

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("pFilterHandle is NULL");
		goto Exception;
	}
	switch (nType) {
	case VIDEO_SORT_BY_NONE:
	case VIDEO_SORT_BY_NAME_A_Z:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		VP_STRDUP(szOrder, MEDIA_TITLE);
		break;

	case VIDEO_SORT_BY_NAME_Z_A:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		VP_STRDUP(szOrder, MEDIA_TITLE);
		break;

	case VIDEO_SORT_BY_DATE_MOST_RECENT:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		VP_STRDUP(szOrder, MEDIA_MODIFIED_TIME);
		break;

	case VIDEO_SORT_BY_OLDEST:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		VP_STRDUP(szOrder, MEDIA_MODIFIED_TIME);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		VP_STRDUP(szOrder, MEDIA_TITLE);
	}

	szCondition =
	    g_strdup_printf
	    ("MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE IS NOT 101)");

	nRet =
	    media_filter_set_condition(pFilterHandle, szCondition,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_filter_set_order(pFilterHandle, nOrderType, szOrder,
	                           MEDIA_CONTENT_COLLATE_NOCASE);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_order is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_video_iter_cb,
	                                     pList);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		VP_FREE(szCondition);
		VP_FREE(szOrder);
		return FALSE;
	}

	VP_FREE(szCondition);
	VP_FREE(szOrder);

	return TRUE;

Exception:
	VP_FREE(szCondition);
	VP_FREE(szOrder);

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	return FALSE;
}

bool vp_media_contents_free_video_items(GList *pList)
{
	VideoLogInfo("");

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	int nCount = 0;
	int i = 0;

	nCount = g_list_length(pList);
	for (i = 0; i < nCount; i++) {
		media_info_h pMediaItem = NULL;
		pMediaItem = (media_info_h) g_list_nth_data(pList, i);
		if (pMediaItem) {
			media_info_destroy(pMediaItem);
			pMediaItem = NULL;
		}
	}

	g_list_free(pList);
	pList = NULL;

	return TRUE;
}

bool vp_media_contents_get_next_file_path(const char *szMediaURL,
        char **szNextURL, bool bLoop,
        GList *pList)
{
	if (!szMediaURL) {
		VideoLogError("No exist szMediaID.");
		return FALSE;
	}

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	int nCount = 0;
	int i = 0;

	media_info_h pFirstMediaItem = NULL;
	bool bCheck = FALSE;

	nCount = g_list_length(pList);
	for (i = 0; i < nCount; i++) {
		media_info_h pMediaItem = NULL;
		pMediaItem = (media_info_h) g_list_nth_data(pList, i);
		if (pMediaItem == NULL) {
			continue;
		}

		if (pFirstMediaItem == NULL) {
			pFirstMediaItem = pMediaItem;
		}

		char *szPath = NULL;
		int nRet = MEDIA_CONTENT_ERROR_NONE;
		nRet = media_info_get_file_path(pMediaItem, &szPath);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogWarning("media_info_get_file_path is fail : 0x%x",
			                nRet);
			continue;
		}

		if (szPath) {

			if (bCheck) {
				VP_STRDUP(*szNextURL, szPath);
				VP_FREE(szPath);
				return TRUE;
			}

			if (!strcmp(szMediaURL, szPath)) {
				bCheck = TRUE;
			}

			VP_FREE(szPath);
		}
	}

	if (bLoop) {
		if (pFirstMediaItem) {
			char *szPath = NULL;
			media_info_get_file_path(pFirstMediaItem, &szPath);
			if (szPath) {
				VP_STRDUP(*szNextURL, szPath);
				VP_FREE(szPath);
			}
		}
	}

	return TRUE;
}

bool vp_media_contents_get_prev_file_path(const char *szMediaURL,
        char **szPrevURL, bool bLoop,
        GList *pList)
{
	if (!szMediaURL) {
		VideoLogError("No exist szMediaID.");
		return FALSE;
	}

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	int nCount = 0;
	int i = 0;

	media_info_h pLastMediaItem = NULL;
	bool bCheck = FALSE;

	nCount = g_list_length(pList);
	for (i = nCount - 1; i >= 0; i--) {
		VideoLogWarning("-- %d --", i);
		media_info_h pMediaItem = NULL;
		pMediaItem = (media_info_h) g_list_nth_data(pList, i);
		if (pMediaItem == NULL) {
			continue;
		}

		if (pLastMediaItem == NULL) {
			pLastMediaItem = pMediaItem;
		}

		char *szPath = NULL;
		int nRet = MEDIA_CONTENT_ERROR_NONE;
		nRet = media_info_get_file_path(pMediaItem, &szPath);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogWarning("media_info_get_file_path is fail : 0x%x",
			                nRet);
			continue;
		}
		if (szPath) {

			if (bCheck) {
				VP_STRDUP(*szPrevURL, szPath);
				VP_FREE(szPath);
				return TRUE;
			}

			if (!strcmp(szMediaURL, szPath)) {
				bCheck = TRUE;
			}

			VP_FREE(szPath);
		}
	}

	if (bLoop) {
		if (pLastMediaItem) {
			char *szPath = NULL;
			media_info_get_file_path(pLastMediaItem, &szPath);
			if (szPath) {
				VP_STRDUP(*szPrevURL, szPath);
				VP_FREE(szPath);
			}
		}
	}

	return TRUE;
}


bool vp_media_contents_set_update_cb(MediaUpdateCb pFunc, void *pUserData)
{
	if (pFunc == NULL) {
		return FALSE;
	}

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	g_media_update_cb = pFunc;

	nRet =
	    media_content_set_db_updated_cb((void *)
	                                    __vp_media_contents_db_update_cb,
	                                    pUserData);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogWarning("media_content_set_db_updated_cb is fail : 0x%x", nRet);
		return FALSE;
	}

	return TRUE;
}

void vp_media_contents_unset_update_cb()
{
	g_media_update_cb = NULL;
	if (media_content_unset_db_updated_cb() != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to media_content_unset_db_updated_cb.");
	}
}

bool vp_media_contents_get_cloud_attribute(const char *szFilePath,
        bool *bCloud)
{

	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}
//      media_content_storage_ex_e storage_type = MEDIA_CONTENT_STORAGE_INTERNAL_EX;
//      media_info_get_storage_type_ex(pMediaHandle, &storage_type);
//      if (storage_type == MEDIA_CONTENT_STORAGE_DROPBOX_EX) {
//              *bCloud = TRUE;
//      }
//      else
//      {
//              *bCloud = FALSE;
//      }

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

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

bool vp_media_contents_set_location_tag(const char *szFilePath,
                                        const char *pLocation)
{
	if (szFilePath == NULL || pLocation == NULL) {
		VideoLogError("szFilePath or city is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_set_location_tag(pMediaHandle, pLocation);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_set_location_tag is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	nRet = media_info_update_to_db(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Failed to update media db!");
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

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

bool vp_media_contents_get_location_tag(const char *szFilePath,
                                        char **szLocationTag)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	char *szLocation = NULL;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_dbmedia_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_location_tag(pMediaHandle, &szLocation);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_location_tag is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	VP_STRDUP(*szLocationTag, szLocation);
	VP_FREE(szLocation);

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

	VP_FREE(szLocation);

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

bool vp_media_contents_set_weahter_tag(const char *szFilePath,
                                       const char *pWeather)
{
	if (szFilePath == NULL || pWeather == NULL) {
		VideoLogError("szFilePath or pWeather is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	//char *szWeather = NULL;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_set_weather(pMediaHandle, pWeather);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_set_weather is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_update_to_db(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Failed to update media db!");
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

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

bool vp_media_contents_get_weahter_tag(const char *szFilePath,
                                       char **szWeatherTag)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	media_info_h pMediaHandle = NULL;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	char *szWeather = NULL;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto Exception;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_info_cb,
	                                     &pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pMediaHandle == NULL) {
		VideoLogError("media info handle is NULL");
		goto Exception;
	}

	nRet = media_info_get_weather(pMediaHandle, &szWeather);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_get_weather is fail : 0x%x", nRet);
		goto Exception;
	}

	VP_STRDUP(*szWeatherTag, szWeather);
	VP_FREE(szWeather);

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = media_info_destroy(pMediaHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_destroy is fail : 0x%x", nRet);
		goto Exception;
	}

	return TRUE;

Exception:

	VP_FREE(szWeather);

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

bool vp_media_contents_subtitle_list_get(GList **pList)
{
	char *szCondition = NULL;

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	filter_h pFilterHandle = NULL;
	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto Exception;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("pFilterHandle is NULL");
		goto Exception;
	}
	// media_type 4 : other file
	szCondition =
	    (char *)
	    g_strdup_printf
	    ("MEDIA_TYPE = 4 AND (MEDIA_DISPLAY_NAME like \"%%%s\" OR MEDIA_DISPLAY_NAME like \"%%%s\" OR MEDIA_DISPLAY_NAME like \"%%%s\")",
	     ".smi", ".sub", ".srt");
	if (szCondition == NULL) {
		VideoLogError("szCondition NULL");
		goto Exception;
	}

	VideoSecureLogInfo("szCondition is szCondition: %s", szCondition);

	nRet =
	    media_filter_set_condition(pFilterHandle, szCondition,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_filter_set_order(pFilterHandle, MEDIA_CONTENT_ORDER_ASC,
	                           MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_order is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_subtitles_iter_cb,
	                                     pList);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x",
		              nRet);
		goto Exception;
	}

	nRet = media_filter_destroy(pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_destroy is fail : 0x%x", nRet);
		VP_FREE(szCondition);
		return FALSE;
	}

	VP_FREE(szCondition);

	return TRUE;

Exception:
	VP_FREE(szCondition);

	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	return FALSE;
}

/* callback functions */
static bool __vp_media_contents_file_is_exist_cb(media_info_h
        pMediaHandle,
        void *pUserData)
{
	bool *bExist = (bool *) pUserData;

	if (pMediaHandle != NULL) {
		VideoLogInfo("pMeidaHandle is %p", pMediaHandle);
		*bExist = TRUE;
	} else {
		VideoLogError("pMeidaHandle is NULL");
		return FALSE;
	}

	return TRUE;
}

bool vp_media_contents_file_is_exist(const char *szFilePath)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	vp_media_contents_connect();

	bool bExist = FALSE;
	filter_h pFilterHandle = NULL;

	int nRet = MEDIA_CONTENT_ERROR_NONE;

	char szTmpStr[4096] = { 0, };
	snprintf(szTmpStr, 4096,
	         "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) AND MEDIA_PATH = \"%s\"",
	         szFilePath);

	nRet = media_filter_create(&pFilterHandle);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_create is fail : 0x%x", nRet);
		goto End;
	}

	if (pFilterHandle == NULL) {
		VideoLogError("Filter handle is NULL");
		goto End;
	}

	nRet =
	    media_filter_set_condition(pFilterHandle, szTmpStr,
	                               MEDIA_CONTENT_COLLATE_DEFAULT);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_filter_set_condition is fail : 0x%x", nRet);
		goto End;
	}

	nRet =
	    media_info_foreach_media_from_db(pFilterHandle,
	                                     __vp_media_contents_file_is_exist_cb,
	                                     &bExist);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("media_info_foreach_media_from_db is fail : 0x%x", nRet);
		goto End;
	}

End:
	if (pFilterHandle) {
		media_filter_destroy(pFilterHandle);
		pFilterHandle = NULL;
	}

	return bExist;
}

bool vp_media_contents_del_video(const char *szFilePath)
{

	if (!szFilePath) {
		VideoLogError("szFilePath is NULL.");
		return FALSE;
	}

	vp_file_unlink(szFilePath);

	char *szVideoId = NULL;
	vp_media_contents_get_video_id(szFilePath, &szVideoId);

	if (!szVideoId) {
		VideoLogError("szVideoId is NULL.");
		return FALSE;
	}

	media_info_delete_from_db(szVideoId);
	VideoSecureLogInfo("szFilePath:=%s,%s", szVideoId, szFilePath);
	VP_FREE(szVideoId);

	return TRUE;
}
