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

#include <app.h>
#include <glib.h>
#include <stdbool.h>
#include <Elementary.h>
#include <media_content.h>

#include <metadata_extractor.h>
#include <image_util.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-video-value-define.h"
#include "mp-util-media-service.h"
#include "mp-video-string-define.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-type-define.h"
#include "vp-file-util.h"
#include "vp-util.h"

#define CAMERA_FOLDER_PATH	"/opt/usr/media/Camera"


static Eina_List *VideoFolderList = NULL;
static Eina_List *VideoItemList = NULL;
static Eina_List *RecentPlayedItemList = NULL;
static int nWhatFileType = MP_MEDIA_VIDEO_FILE_TYPE;
static int nTotalVideoTypeFiles = 0;
static int nNumberOfVideoItemByType = 0;
//media_content_noti_h g_pCloud_h; 	/* Notify handle for cloud content updating in DB */


bool mp_util_svc_is_3gp_file(char *szFileUri)
{

	int nRet = FALSE;
	char *szTmpUriPath = szFileUri;

	if (szTmpUriPath) {
		char *szExt = NULL;
		szExt = strrchr(szTmpUriPath, '.');

		if ((szExt != NULL) && ((szExt+1) != NULL)) {
			if (!strcmp(szExt+1, "3gp")) {
				nRet = TRUE;
			} else {
				nRet = FALSE;
			}
		} else {
			VideoLogError("No exist extension.");
			nRet = FALSE;
		}
	}
	else
	{
		VideoLogError("No exist file uri.");
		nRet = FALSE;
	}

	return nRet;
}

static bool mp_util_svc_iterate_for_get_video_item_cb(media_info_h pVideoItem, void *pUserData)
{

	media_info_h *pAssignFolderItem = (media_info_h*)pUserData;

	if (pVideoItem != NULL) {
		char *szTmpStr = NULL;
		media_info_clone(pAssignFolderItem, pVideoItem);
		media_info_get_display_name(*pAssignFolderItem, &szTmpStr);
		MP_FREE_STRING(szTmpStr);
	}

	return FALSE;
}

static bool mp_util_svc_iterate_for_video_item_list_view_cb(media_info_h pMediaItem, void *pUserData)
{
	media_info_h pAssignMediaItem = NULL;
	media_content_type_e nMediaType = MEDIA_CONTENT_TYPE_OTHERS;
	if (!pMediaItem) {
		return TRUE;
	}
	media_info_get_media_type(pMediaItem, &nMediaType);

	if (nMediaType == MEDIA_CONTENT_TYPE_VIDEO) {
		char *szTmpItemFilePath = NULL;
		char szItemFilePath[STR_LEN_MAX] = {0,};

		media_info_get_file_path(pMediaItem, &szTmpItemFilePath);

		if (szTmpItemFilePath == NULL) {
			VideoLogError("Fail to get file path of media item.");
			return TRUE;
		}
		strncpy(szItemFilePath, szTmpItemFilePath, STR_LEN_MAX - 1);

		free(szTmpItemFilePath);
		szTmpItemFilePath = NULL;

		nTotalVideoTypeFiles++;

		switch (nWhatFileType) {
		case MP_MEDIA_VIDEO_FILE_TYPE:
			if (mp_util_svc_is_3gp_file(szItemFilePath) == TRUE) {
				return TRUE;
			}
			break;

		case MP_MEDIA_CAMERA_FILE_TYPE:
			if (mp_util_svc_is_3gp_file(szItemFilePath) == FALSE) {
				return TRUE;
			}
			break;

		case MP_MEDIA_ALL_FILE_TYPE:
			break;
		}

		nNumberOfVideoItemByType++;

		media_info_clone(&pAssignMediaItem, pMediaItem);
		Eina_List **pList = (Eina_List **)pUserData;
		*pList = eina_list_append(*pList, pAssignMediaItem);
	}

	return TRUE;
}

static bool mp_util_svc_iterate_cb(media_info_h pMediaItem, void *pUserData)
{
	media_info_h pAssignMediaItem = NULL;
	media_content_type_e nMediaType = MEDIA_CONTENT_TYPE_OTHERS;
	if (!pMediaItem) {
		return FALSE;
	}
	media_info_get_media_type(pMediaItem, &nMediaType);

	if (nMediaType == MEDIA_CONTENT_TYPE_VIDEO) {
		media_info_clone(&pAssignMediaItem, pMediaItem);
		Eina_List **pList = (Eina_List **)pUserData;
		*pList = eina_list_append(*pList, pAssignMediaItem);

		nTotalVideoTypeFiles++;
		nNumberOfVideoItemByType++;
	}

	return TRUE;
}

static bool mp_util_svc_iterate_for_get_folder_cb(media_folder_h pFolderItem, void *pUserData)
{
	media_folder_h *pAssignFolderItem = (media_folder_h*)pUserData;

	if (pFolderItem != NULL) {
		char *szTmpStr = NULL;
		media_folder_clone(pAssignFolderItem, pFolderItem);
		media_folder_get_name(*pAssignFolderItem, &szTmpStr);

		if (szTmpStr) {
			free(szTmpStr);
			szTmpStr = NULL;
		}
	}

	return TRUE;
}

static int mp_util_svc_sort_for_folder_list_cb(const void *pFolder1, const void *pFolder2)
{
	media_folder_h pFolderType1 = (media_folder_h)pFolder1;
	media_folder_h pFolderType2 = (media_folder_h)pFolder2;

	char *folderName1 = NULL;
	char *folderName2 = NULL;

	int nRet = 0;

	media_folder_get_name(pFolderType1, &folderName1);
	media_folder_get_name(pFolderType2, &folderName2);

	if (!folderName1) {
		MP_FREE_STRING(folderName2);
		return (1);
	}

	if (!folderName2) {
		MP_FREE_STRING(folderName1);
		return (-1);
	}

	nRet = strcmp(folderName1, folderName2);

	MP_FREE_STRING(folderName1);
	MP_FREE_STRING(folderName2);

	return nRet;
}

static bool mp_util_svc_iterate_for_folder_list_cb(media_folder_h pFolderItem, void *pUserData)
{
	media_folder_h pAssignFolderItem = NULL;
	Eina_Compare_Cb pCmpFunc = (Eina_Compare_Cb)mp_util_svc_sort_for_folder_list_cb;

	media_folder_clone(&pAssignFolderItem, pFolderItem);
	Eina_List **pList = (Eina_List **)pUserData;
	*pList = eina_list_append(*pList, pAssignFolderItem);
	*pList = eina_list_sort(*pList, eina_list_count(*pList), pCmpFunc);

	return TRUE;
}

static bool mp_util_svc_iterate_for_personal_folder_list_cb(media_folder_h pFolderItem, void *pUserData)
{
	media_folder_h pAssignFolderItem = NULL;
	media_folder_clone(&pAssignFolderItem, pFolderItem);
	Eina_List **pList = (Eina_List **)pUserData;
	Eina_Compare_Cb pCmpFunc = (Eina_Compare_Cb)mp_util_svc_sort_for_folder_list_cb;

	char *szFolderPath = NULL;
	if (pAssignFolderItem) {
		media_folder_get_path(pAssignFolderItem, &szFolderPath);

		if (szFolderPath) {
			if (!strcmp(szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR)) {
				//Skip "/opt/storage/PersonalStorage" folder
				MP_FREE_STRING(szFolderPath);

				media_folder_destroy(pAssignFolderItem);
				pAssignFolderItem	= NULL;

				return TRUE;
			}
			MP_FREE_STRING(szFolderPath);
		}
	}

	*pList = eina_list_append(*pList, pAssignFolderItem);
	*pList = eina_list_sort(*pList, eina_list_count(*pList), pCmpFunc);

	return TRUE;
}

//
// Internal function
//
bool mp_util_svc_get_file_directory_name(char *szFilePath, char *szFileName, char *szDirectoryName, char *szDirectoryPath)
{
	if (!szFilePath) {
		VideoLogError("[ERR]");
		return FALSE;
	}

	int nCount = 0;
	int nLoopCount = 0;

	for (nLoopCount = strlen(szFilePath); nLoopCount >= 0; nLoopCount--) {
		if (szFilePath[nLoopCount] != '\0') {
			nCount++;
		}

		if (szFilePath[nLoopCount] == '/') {
			if (szFileName) {
				memcpy(szFileName, &szFilePath[nLoopCount + 1], --nCount);
				*(szFileName + nCount) = '\0';
			}

			if (szDirectoryPath) {
				memcpy(szDirectoryPath, &szFilePath[0], nLoopCount);
				*(szDirectoryPath + nLoopCount + 1) = '\0';
			}

			if (szDirectoryName) {
				nCount = 0;
				for (--nLoopCount; nLoopCount >= 0; nLoopCount--)
				{
					nCount++;
					if (szFilePath[nLoopCount] == '/') {
						memcpy(szDirectoryName, &szFilePath[nLoopCount + 1], --nCount);
						*(szDirectoryName + nCount) = '\0';

						return TRUE;
					}
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}

//
// External function
//

void mp_util_svc_init_session(void)
{
	if (media_content_connect() != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to connect to media content db.");
	}
	else
	{
		VideoLogInfo("Success to connect to media content db.");
	}
}

void mp_util_svc_finish_session(void)
{
	//mp_util_svc_unset_update_db_cb_func();

	if (media_content_disconnect() != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to disconnect from media content db.");
		return;
	}


	VideoLogInfo("Success to disconnect from media content db.");
	return;
}

bool mp_util_svc_destory_video_item_list(void)
{
	if (!VideoItemList) {
		VideoLogError("VideoItemList is empty already.");
		return FALSE;
	}

	media_info_h pTmpVideoItem = NULL;
	EINA_LIST_FREE(VideoItemList, pTmpVideoItem) {
		if (pTmpVideoItem) {
			media_info_destroy(pTmpVideoItem);
		}
	}
	nNumberOfVideoItemByType = 0;
	VideoItemList = NULL;

	return TRUE;
}

bool mp_util_svc_extract_video_list_from_folder(char *szFolderPath, int nSortType, int nItemType)
{
	if (VideoItemList) {
		VideoLogError("Already exist video list.");
		return FALSE;
	}

	if (!szFolderPath) {
		VideoLogError("[ERR]");
		return FALSE;
	}

	filter_h m_FilterHandle = NULL;
	media_folder_h m_FolderHandle = NULL;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;

	char *szFolderID = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};
	char szOrderObj[STR_LEN_MAX] = {0,};

	nTotalVideoTypeFiles = 0;
	nNumberOfVideoItemByType = 0;

	switch (nSortType) {
	case MP_MEDIA_SORT_BY_NONE:
	case MP_MEDIA_SORT_BY_RECENTLY_VIEWED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_RECENTLY_ADDED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_ADDED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_NAME:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_DISPLAY_NAME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_SIZE:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_SIZE, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_TYPE:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_MIME_TYPE, STR_LEN_MAX - 1);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
	}

	switch (nItemType) {
		case MP_LIST_VIEW_ALL:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and FOLDER_PATH = \"%s\" and FOLDER_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\"", szFolderPath);
			}
		break;

		case MP_LIST_VIEW_PHONE:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and FOLDER_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\"", szFolderPath);
			}
		break;

		case MP_LIST_VIEW_CLOUD:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and MEDIA_STORAGE_TYPE = 101 and FOLDER_PATH = \"%s\" and FOLDER_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_STORAGE_TYPE = 101 and FOLDER_PATH = \"%s\"", szFolderPath);
			}
		break;

		default:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and FOLDER_PATH = \"%s\" and FOLDER_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\"", szFolderPath);
			}
		 break;
	}

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_folder_foreach_folder_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_folder_cb, &m_FolderHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail media_folder_foreach_folder_from_db().");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (!m_FolderHandle) {
		VideoLogError("Fail to get folder handle.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	memset(szTmpStr, 0, STR_LEN_MAX);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101)");
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_set_order(m_FilterHandle, nOrderType, szOrderObj, MEDIA_CONTENT_COLLATE_NOCASE) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);

		return FALSE;
	}

	if (media_folder_get_folder_id(m_FolderHandle, &szFolderID) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get folder id.");
		media_filter_destroy(m_FilterHandle);

		MP_FREE_STRING(szFolderID);
		return FALSE;
	}

	if (media_folder_foreach_media_from_db(szFolderID, m_FilterHandle, mp_util_svc_iterate_cb, &VideoItemList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);

		MP_FREE_STRING(szFolderID);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
	}

	MP_FREE_STRING(szFolderID);

	return TRUE;
}

char *mp_util_get_folder_thumbnail(int nVideoFolderIndex, int nSortType)
{
	if (!VideoFolderList) {
		VideoLogError("[ERR]");
		return NULL;
	}

	media_folder_h m_FolderHandle = NULL;
	m_FolderHandle = (media_folder_h)eina_list_nth(VideoFolderList, nVideoFolderIndex);
	if (!m_FolderHandle) {
		VideoLogError("Fail to get folder handle.");
		return NULL;
	}

	media_info_h pVideoItem = NULL;
	char *szThumbnailPath = NULL;
	filter_h m_FilterHandle = NULL;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;
	char *szFolderID = NULL;
	char szOrderObj[STR_LEN_MAX] = {0,};
	char szTmpStr[STR_LEN_MAX] = {0,};

	switch (nSortType) {
	case MP_MEDIA_SORT_BY_NONE:
	case MP_MEDIA_SORT_BY_RECENTLY_VIEWED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_RECENTLY_ADDED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_ADDED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_NAME:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_DISPLAY_NAME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_SIZE:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_SIZE, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_TYPE:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_MIME_TYPE, STR_LEN_MAX - 1);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
	}

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return NULL;
	}

	memset(szTmpStr, 0, STR_LEN_MAX);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121)");
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return NULL;
	}

	if (media_filter_set_order(m_FilterHandle, nOrderType, szOrderObj, MEDIA_CONTENT_COLLATE_NOCASE) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);

		return NULL;
	}

	if (media_folder_get_folder_id(m_FolderHandle, &szFolderID) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get folder id.");
		media_filter_destroy(m_FilterHandle);

		MP_FREE_STRING(szFolderID);
		return NULL;
	}

	if (media_folder_foreach_media_from_db(szFolderID, m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);

		MP_FREE_STRING(szFolderID);
		return NULL;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
	}

	if (pVideoItem) {
		media_info_get_thumbnail_path(pVideoItem, &szThumbnailPath);
		VideoSecureLogInfo("Video thumbnail file path : %s", szThumbnailPath);
		media_info_destroy(pVideoItem);
	}

	MP_FREE_STRING(szFolderID);

	return szThumbnailPath;
}

bool mp_util_svc_extract_video_list_by_item_type(int nSortType, int nItemType)
{
	if (VideoItemList) {
		VideoLogError("Already exist video list.");
		return FALSE;
	}

	nWhatFileType = MP_MEDIA_ALL_FILE_TYPE;

	filter_h m_FilterHandle = NULL;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;
	char szTmpStr[STR_LEN_MAX] = {0,};
	char szOrderObj[STR_LEN_MAX] = {0,};

	nNumberOfVideoItemByType = 0;

	switch (nSortType) {
	case MP_MEDIA_SORT_BY_NONE:
	case MP_MEDIA_SORT_BY_RECENTLY_VIEWED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_RECENTLY_ADDED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_ADDED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_NAME:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_DISPLAY_NAME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_SIZE:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_SIZE, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_TYPE:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_MIME_TYPE, STR_LEN_MAX - 1);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
	}

	switch (nItemType) {
	case MP_LIST_VIEW_ALL:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) and MEDIA_PATH not like \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121)");
			}
		break;

	case MP_LIST_VIEW_PHONE:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH not like \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1)");
			}

		break;

	case MP_LIST_VIEW_CLOUD:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and MEDIA_STORAGE_TYPE = 101 and MEDIA_PATH not like \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and MEDIA_STORAGE_TYPE = 101");
			}

		break;

	default:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) and MEDIA_PATH not like \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121)");
			}
		break;
	}
	VideoLogInfo("MEDIA_PATH not like %s", szTmpStr);

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to create media filter handle.");
		return FALSE;
	}
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_set_order(m_FilterHandle, nOrderType, szOrderObj, MEDIA_CONTENT_COLLATE_NOCASE) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_video_item_list_view_cb, &VideoItemList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	return TRUE;
}

bool mp_util_svc_extract_video_list_from_favorite(int nSortType)
{
	if (VideoItemList) {
		VideoLogInfo("Already exist video list.");
		return FALSE;
	}

	filter_h m_FilterHandle = NULL;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;
	char szOrderObj[STR_LEN_MAX] = {0,};
	char szTmpStr[STR_LEN_MAX] = {0,};

	nWhatFileType = MP_MEDIA_ALL_FILE_TYPE;

	nTotalVideoTypeFiles = 0;
	nNumberOfVideoItemByType = 0;

	switch (nSortType) {
	case MP_MEDIA_SORT_BY_NONE:
	case MP_MEDIA_SORT_BY_RECENTLY_VIEWED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_RECENTLY_ADDED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_ADDED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_NAME:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_DISPLAY_NAME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_SIZE:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_SIZE, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_TYPE:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_MIME_TYPE, STR_LEN_MAX - 1);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
	}

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	if (!mp_util_get_personal_status()) {
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and MEDIA_FAVOURITE = 1 and MEDIA_PATH NOT LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
	}
	else
	{
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and MEDIA_FAVOURITE = 1");
	}

	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_set_order(m_FilterHandle, nOrderType, szOrderObj, MEDIA_CONTENT_COLLATE_NOCASE) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_video_item_list_view_cb, &VideoItemList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	return TRUE;
}


bool mp_util_svc_index_list_for_search_view(const char *keyword, void **index_list)
{
	media_info_h pVideoItem = NULL;
	Eina_List *pIterateList = NULL;
	Eina_List *pSearchList = NULL;
	int index = 0;

	if (!VideoItemList) {
		VideoLogError("VideoItemList IS null");
		return FALSE;
	}

	if (!keyword) {
		VideoLogError("keyword IS null");
		return FALSE;
	}

	if (!index_list) {
		VideoLogError("List IS null");
		return FALSE;
	}

	EINA_LIST_FOREACH(VideoItemList, pIterateList, pVideoItem) {
		if (pVideoItem) {
			char *szTitle = NULL;
			media_info_get_display_name(pVideoItem, &szTitle);
			if (szTitle && mp_util_str_str_same_case(szTitle, keyword)) {
				pSearchList = eina_list_append(pSearchList, (const void*)index);
			}
			MP_FREE_STRING(szTitle);
		}
		index ++;
	}
	*index_list = pSearchList;

	return TRUE;
}

bool mp_util_svc_index_list_for_cloud_view(void **index_list, int *nListSize)
{
//	media_info_h pVideoItem = NULL;
//	Eina_List *pIterateList = NULL;
//	int index = 0;
//	int size = 0;
//	if (!VideoItemList) {
//		VideoLogError("VideoItemList IS null");
//		return FALSE;
//	}
//	EINA_LIST_FOREACH(VideoItemList, pIterateList, pVideoItem)
//	{
//		if (pVideoItem)
//		{
//			media_content_storage_ex_e storage_type = MEDIA_CONTENT_STORAGE_INTERNAL_EX;
//			media_info_get_storage_type_ex(pVideoItem, &storage_type);
//			if (storage_type == MEDIA_CONTENT_STORAGE_DROPBOX_EX) {
//				*index_list = eina_list_append(*index_list, (const void*)index);
//				size ++;
//			}
//		}
//		index ++;
//	}
//	*nListSize = size;
//	return TRUE;
	return FALSE;
}


bool mp_util_svc_extract_video_list_for_search_view(const char *keyword, int nItemType)
{
	if (VideoItemList) {
		VideoLogError("Already exist video list.");
		return FALSE;
	}

	filter_h m_FilterHandle = NULL;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;
	char szOrderObj[STR_LEN_MAX] = {0,};
	char szTmpStr[STR_LEN_MAX] = {0,};

	nWhatFileType = MP_MEDIA_ALL_FILE_TYPE;
	nTotalVideoTypeFiles = 0;
	nNumberOfVideoItemByType = 0;

	nOrderType = MEDIA_CONTENT_ORDER_ASC;
	strncpy(szOrderObj, MEDIA_DISPLAY_NAME, STR_LEN_MAX - 1);

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	switch (nItemType) {
		case MP_LIST_VIEW_ALL:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) and MEDIA_DISPLAY_NAME like \"%%%s%%\" and MEDIA_PATH NOT LIKE \"%s%%\"", keyword, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) and MEDIA_DISPLAY_NAME like \"%%%s%%\"", keyword);
			}
			break;

		case MP_LIST_VIEW_PHONE:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1  and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_DISPLAY_NAME like \"%%%s%%\" and MEDIA_PATH NOT LIKE \"%s%%\"", keyword, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_DISPLAY_NAME like \"%%%s%%\"", keyword);
			}
			break;

		case MP_LIST_VIEW_CLOUD:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and MEDIA_STORAGE_TYPE = 101 and MEDIA_DISPLAY_NAME like \"%%%s%%\" and MEDIA_PATH NOT LIKE \"%s%%\"", keyword, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and MEDIA_STORAGE_TYPE = 101 and MEDIA_DISPLAY_NAME like \"%%%s%%\"", keyword);
			}
			break;

		default:
			if (!mp_util_get_personal_status()) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) and MEDIA_DISPLAY_NAME like \"%%%s%%\" and MEDIA_PATH NOT LIKE \"%s%%\"", keyword, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) and MEDIA_DISPLAY_NAME like \"%%%s%%\"", keyword);
			}
			break;
	}

	//snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and MEDIA_TITLE like \"%%%s%%\"", keyword);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_set_order(m_FilterHandle, nOrderType, szOrderObj, MEDIA_CONTENT_COLLATE_NOCASE) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_video_item_list_view_cb, &VideoItemList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	return TRUE;
}


void mp_util_svc_destory_video_list(void)
{

	if (VideoFolderList) {
		mp_util_svc_destory_video_folder_list();
		VideoFolderList = NULL;
	}

	if (VideoItemList) {
		mp_util_svc_destory_video_item_list();
		VideoItemList = NULL;
	}

	if (RecentPlayedItemList) {
		mp_util_svc_destory_last_played_item_list();
		RecentPlayedItemList = NULL;
	}

	nTotalVideoTypeFiles = 0;
	nNumberOfVideoItemByType = 0;
}

bool mp_util_svc_remove_item_by_url_with_list(char *szFilePath)
{
	if (!szFilePath) {
		VideoLogError("[ERR]");
		return FALSE;
	}

	if (!VideoItemList) {
		VideoLogError("pList is empty already.");
		return FALSE;
	}

	VideoSecureLogInfo("szFilePath : %s", szFilePath);

	char *szGetFilePath = NULL;
	media_info_h pItem = NULL;
	Eina_List *pIterateList = NULL;
	EINA_LIST_FOREACH(VideoItemList, pIterateList, pItem) {
		if (pItem) {
			if (media_info_get_file_path(pItem, &szGetFilePath) == MEDIA_CONTENT_ERROR_NONE) {
				if (szGetFilePath) {
					if (!strcmp(szGetFilePath, szFilePath)) {
						char *szMediaID = NULL;
						media_info_get_media_id(pItem, &szMediaID);
						media_info_delete_from_db(szMediaID);
						mp_util_delete_wall_file(szMediaID);
						VideoItemList = eina_list_remove(VideoItemList, pItem);
						media_info_destroy(pItem);

						nNumberOfVideoItemByType--;

						if (szMediaID) {
							free(szMediaID);
						}

						if (szGetFilePath) {
							free(szGetFilePath);
						}

						return TRUE;
					}
				} else {
					VideoLogError("No exist %s in media info Database module.", szFilePath);
					return FALSE;
				}
			}
		}
	}

	if (szGetFilePath) {
		free(szGetFilePath);
	}
	return FALSE;
}

bool mp_util_svc_remove_item_by_list_index(int nVideoItemIndex)
{

	if (!VideoItemList) {
		VideoLogError("pList is empty already.");
		return FALSE;
	}

	media_info_h pItem = eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pItem) {
		char *szMediaID = NULL;
		media_info_get_media_id(pItem, &szMediaID);
		if (szMediaID) {
			media_info_delete_from_db(szMediaID);
			mp_util_delete_wall_file(szMediaID);
			VideoItemList = eina_list_remove(VideoItemList, pItem);
			free(szMediaID);
			media_info_destroy(pItem);
			nNumberOfVideoItemByType--;
		} else {
			VideoLogError("No exist media ID in index %d of list.", nVideoItemIndex);
			return FALSE;
		}
	} else {
		VideoLogError("Fail to remove item in media svc and list.");
		return FALSE;
	}

	return TRUE;
}

bool mp_util_svc_remove_item_from_db_by_list_index(int nVideoItemIndex)
{

	if (!VideoItemList) {
		VideoLogError("pList is empty already.");
		return FALSE;
	}

	media_info_h pItem = eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pItem) {
		char *szMediaID = NULL;
		media_info_get_media_id(pItem, &szMediaID);
		if (szMediaID) {
			mp_util_delete_wall_file(szMediaID);
			media_info_delete_from_db(szMediaID);
			free(szMediaID);
		} else {
			VideoLogError("No exist media ID in index %d of list.", nVideoItemIndex);
			return FALSE;
		}
	} else {
		VideoLogError("Fail to remove item in media svc and list.");
		return FALSE;
	}

	return TRUE;
}

bool mp_util_svc_get_video_id_by_video_url(const char *szPath, char **szVideoID)
{
	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};
	bool bRet = FALSE;

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to create media filter handle.");
		bRet = FALSE;
	}

	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_PATH = \"%s\"", szPath);
	if (media_filter_set_condition(m_FilterHandle,
		szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) !=
			MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		bRet = FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle,
		mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) !=
			MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		bRet = FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		bRet = FALSE;
	}

	if (pVideoItem) {
		char *szMediaID = NULL;
		media_info_get_media_id(pVideoItem, &szMediaID);
		*szVideoID = szMediaID;
		media_info_destroy(pVideoItem);
		bRet = TRUE;
	} else {
		bRet = FALSE;
	}

	return bRet;
}

unsigned int mp_util_svc_get_video_last_play_time_by_video_url(const char *szPath)
{
	if (!szPath) {
		VideoLogError("No exist video path.");
		return 0;
	}

	VideoLogInfo("szPath : %s", szPath);

	int nLastPlayedTime = 0;
	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	//snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_PATH = \"%s\"", szPath);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szPath);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	if (pVideoItem) {
		video_meta_h pVideoMetaHandle = NULL;
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) ==
			MEDIA_CONTENT_ERROR_NONE) {
			video_meta_get_played_position(pVideoMetaHandle, &nLastPlayedTime);
			VideoLogInfo("Last Played Time : %d", nLastPlayedTime);
			video_meta_destroy(pVideoMetaHandle);
		}
		media_info_destroy(pVideoItem);
	}

	return (unsigned int)nLastPlayedTime;
}

unsigned int mp_util_svc_get_video_duration_by_video_url(const char *szPath)
{
	if (!szPath) {
		VideoLogError("No exist video path.");
		return 0;
	}

	VideoSecureLogInfo("szPath : %s", szPath);

	int nDurationTime = 0;
	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szPath);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	if (pVideoItem) {
		video_meta_h pVideoMetaHandle = NULL;
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) == MEDIA_CONTENT_ERROR_NONE) {
			video_meta_get_duration(pVideoMetaHandle, &nDurationTime);
			video_meta_destroy(pVideoMetaHandle);
		}
		media_info_destroy(pVideoItem);
	}

	return (unsigned int)nDurationTime;
}

int mp_util_svc_get_video_file_size_by_video_url(const char *szPath)
{
	if (!szPath) {
		VideoLogError("No exist video path.");
		return 0;
	}

	VideoSecureLogInfo("szPath : %s", szPath);

	unsigned long long nSize = 0;
	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szPath);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	if (pVideoItem) {
		media_info_get_size(pVideoItem, &nSize);
		media_info_destroy(pVideoItem);
	}

	return nSize;
}

char *mp_util_svc_get_video_Thumbnail_by_video_url(const char *szMediaUri)
{
	if (!szMediaUri) {
		VideoLogError("No exist video path.");
		return NULL;
	}

	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char *szThumbnailPath = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	memset(szTmpStr, 0, STR_LEN_MAX);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szMediaUri);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	if (pVideoItem) {
		media_info_get_thumbnail_path(pVideoItem, &szThumbnailPath);
		VideoSecureLogInfo("Video thumbnail file path : %s", szThumbnailPath);
		media_info_destroy(pVideoItem);
	}

	return szThumbnailPath;
}

int mp_util_svc_get_video_width_by_video_url(const char *szMediaUri)
{
	if (!szMediaUri) {
		VideoLogError("No exist video path.");
		return 0;
	}

	VideoSecureLogInfo("szPath : %s", szMediaUri);

	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};
	int nWidth = 0;

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	memset(szTmpStr, 0, STR_LEN_MAX);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szMediaUri);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	if (pVideoItem) {
		video_meta_h pVideoMetaHandle = NULL;

		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("Fail to get pVideoMetaHandle.");
		} else {
			if (pVideoMetaHandle) {
				video_meta_get_width(pVideoMetaHandle, &nWidth);
				video_meta_destroy(pVideoMetaHandle);
			} else {
				VideoLogError("No exist pVideoMetaHandle");
			}
		}
	}

	return nWidth;
}

int mp_util_svc_get_video_height_by_video_url(const char *szMediaUri)
{
	if (!szMediaUri) {
		VideoLogError("No exist video path.");
		return 0;
	}

	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};
	int nHeight = 0;

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	memset(szTmpStr, 0, STR_LEN_MAX);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szMediaUri);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	if (pVideoItem) {
		video_meta_h pVideoMetaHandle = NULL;

		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("Fail to get pVideoMetaHandle.");
		} else {
			if (pVideoMetaHandle) {
				video_meta_get_height(pVideoMetaHandle, &nHeight);
				video_meta_destroy(pVideoMetaHandle);
			} else {
				VideoLogError("No exist pVideoMetaHandle");
			}
		}
	}

	return nHeight;
}

void mp_util_svc_set_video_last_played_time_by_url(char *szMediaUri, unsigned int nPosition)
{
	if (!szMediaUri) {
		VideoLogError("No exist media uri.");
		return;
	}

	VideoSecureLogInfo("szMediaUri : %s, nLastPlayedTime : %d", szMediaUri, nPosition);

	int nLastPlayedTime = (int)nPosition;
	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return;
	}

	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szMediaUri);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return;
	}

	video_meta_h pVideoMetaHandle = NULL;

	if (pVideoItem) {
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("Fail to get pVideoMetaHandle.");
		} else {
			if (pVideoMetaHandle) {
				video_meta_set_played_position(pVideoMetaHandle, nLastPlayedTime);
				video_meta_update_to_db(pVideoMetaHandle);
				video_meta_destroy(pVideoMetaHandle);
			} else {
				VideoLogError("No exist pVideoMetaHandle");
			}
		}
		media_info_destroy(pVideoItem);
	}
}

bool mp_util_svc_get_video_current_url_pre_next_item(char *szCurrMediaUri, char *szPreMediaUri, char *szNextMediaUri, bool bLoop)
{

	if (!szCurrMediaUri) {
		VideoLogError("[ERR] No exist current file path.");
		return FALSE;
	}

	if (!VideoItemList) {
		VideoLogError("No exist VideoItemList.");
		if (bLoop) {
			strncpy(szNextMediaUri, szCurrMediaUri, (sizeof(char) * STR_LEN_MAX) - 1);
		}
		return FALSE;
	}

	int nCount = 0;
	int nListSize = 0;
	media_info_h pVideoItem = NULL;
	Eina_List *pIterateList = NULL;

	memset(szPreMediaUri, 0, sizeof(char) * STR_LEN_MAX);
	memset(szNextMediaUri, 0, sizeof(char) * STR_LEN_MAX);

	nListSize = eina_list_count(VideoItemList);

	if (nListSize <= 1) {
		VideoLogError("Video list size == 1.");
		return FALSE;
	}

	EINA_LIST_FOREACH(VideoItemList, pIterateList, pVideoItem) {
		if (pVideoItem == NULL) {
			continue;
		}

		char *szVideoItemFilePath = NULL;
		media_info_get_file_path(pVideoItem, &szVideoItemFilePath);
		if (szVideoItemFilePath) {
			VideoSecureLogInfo("URL of the File item - %s (%d / %d)", szVideoItemFilePath, nCount, nListSize);
			if (!strcmp(szVideoItemFilePath, szCurrMediaUri)) {
				free(szVideoItemFilePath);
				szVideoItemFilePath = NULL;

				pVideoItem = NULL;
				pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nCount - 1);
				if (pVideoItem) {
					media_info_get_file_path(pVideoItem, &szVideoItemFilePath);
					strncpy(szPreMediaUri, szVideoItemFilePath, (sizeof(char) * STR_LEN_MAX) - 1);

					free(szVideoItemFilePath);
					szVideoItemFilePath = NULL;
				} else if (bLoop) {
					media_info_h m_LastItem = (media_info_h)eina_list_nth(VideoItemList, nListSize - 1);
					VideoLogError("%p", m_LastItem);
					if (m_LastItem) {
						media_info_get_file_path(m_LastItem, &szVideoItemFilePath);
						strncpy(szPreMediaUri, szVideoItemFilePath, (sizeof(char) * STR_LEN_MAX) - 1);

						free(szVideoItemFilePath);
						szVideoItemFilePath = NULL;
					}
					m_LastItem = NULL;
				}

				pVideoItem = NULL;
				pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nCount + 1);
				if (pVideoItem) {
					media_info_get_file_path(pVideoItem, &szVideoItemFilePath);
					strncpy(szNextMediaUri, szVideoItemFilePath, (sizeof(char) * STR_LEN_MAX) - 1);

					free(szVideoItemFilePath);
					szVideoItemFilePath = NULL;
				} else if (bLoop) {
					media_info_h m_FirstItem = (media_info_h)eina_list_nth(VideoItemList, 0);
					if (m_FirstItem) {
						media_info_get_file_path(m_FirstItem, &szVideoItemFilePath);
						strncpy(szNextMediaUri, szVideoItemFilePath, (sizeof(char) * STR_LEN_MAX) - 1);

						free(szVideoItemFilePath);
						szVideoItemFilePath = NULL;
					}
					m_FirstItem = NULL;
				}

				VideoSecureLogInfo("PreMediaUri-NextMediaUri:%s-%s", szPreMediaUri, szNextMediaUri);

				return TRUE;
			}
			free(szVideoItemFilePath);
			szVideoItemFilePath = NULL;
		}

		++nCount;
	}

	return FALSE;
}

char *mp_util_svc_get_video_url(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	char *szVideoItemFilePath = NULL;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pVideoItem) {
		media_info_get_file_path(pVideoItem, &szVideoItemFilePath);
		VideoSecureLogInfo("szVideoItemFilePath : %s", szVideoItemFilePath);
	}

	return szVideoItemFilePath;
}

char *mp_util_svc_get_video_thumbnail(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	char *szVideoItemThumbnailPath = NULL;

	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pVideoItem) {
		media_info_get_thumbnail_path(pVideoItem, &szVideoItemThumbnailPath);
	}

	VideoSecureLogInfo("pThumbIconUri = %s", szVideoItemThumbnailPath);

	return szVideoItemThumbnailPath;
}

char *mp_util_svc_get_video_title(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	char *szTitle = NULL;

	if (!szTitle) {
		media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

		if (pVideoItem) {
			media_info_get_display_name(pVideoItem, &szTitle);
		}
	}

	return szTitle;
}

int mp_util_svc_get_video_duration_time(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return 0;
	}

	int nDurationTime = 0;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pVideoItem) {
		video_meta_h pVideoMetaHandle = NULL;
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) == MEDIA_CONTENT_ERROR_NONE) {
			if (video_meta_get_duration(pVideoMetaHandle, &nDurationTime) != MEDIA_CONTENT_ERROR_NONE) {
				VideoLogError("fail to get duration of video metadata");
			}

			video_meta_destroy(pVideoMetaHandle);
		}
	}

	return nDurationTime;
}

int mp_util_svc_get_video_last_played_pos(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	int nLastPlayedTime = 0;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pVideoItem) {
		video_meta_h pVideoMetaHandle = NULL;
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) == MEDIA_CONTENT_ERROR_NONE) {
			video_meta_get_played_position(pVideoMetaHandle, &nLastPlayedTime);

			VideoLogInfo("nIndexGenlist : %d==%d", nLastPlayedTime, nVideoItemIndex);
			video_meta_destroy(pVideoMetaHandle);
		}
	}

	return (int)nLastPlayedTime;
}

unsigned long long mp_util_svc_get_video_item_size(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	unsigned long long nSize = 0;

	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
	if (pVideoItem) {
		media_info_get_size(pVideoItem, &nSize);
	}

	return nSize;

}

int mp_util_svc_get_video_item_width(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	int nWidth = 0;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
	video_meta_h pVideoMetaHandle = NULL;

	if (pVideoItem) {
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("Fail to get pVideoMetaHandle.");
		} else {
			if (pVideoMetaHandle) {

				video_meta_get_width(pVideoMetaHandle, &nWidth);
				video_meta_destroy(pVideoMetaHandle);
			} else {
				VideoLogError("No exist pVideoMetaHandle");
			}
		}
	}

	return nWidth;
}

int mp_util_svc_get_video_item_height(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	int nHeight = 0;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
	video_meta_h pVideoMetaHandle = NULL;

	if (pVideoItem) {
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("Fail to get pVideoMetaHandle.");
		} else {
			if (pVideoMetaHandle) {

				video_meta_get_height(pVideoMetaHandle, &nHeight);
				video_meta_destroy(pVideoMetaHandle);
			} else {
				VideoLogError("No exist pVideoMetaHandle");
			}
		}
	}

	return nHeight;
}

time_t mp_util_svc_get_video_item_data_time(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	time_t nDateTime = 0;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
	if (pVideoItem) {
		if (media_info_get_modified_time(pVideoItem, &nDateTime) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("media_info_get_modified_time fail");
			return 0;
		}
		VideoLogError("Video date time : %d", nDateTime);
	}

	return nDateTime;
}

char *mp_util_svc_get_video_item_extension(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	return NULL;
}

double mp_util_svc_get_video_item_longitude(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	double nLongitude = 0;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
	if (pVideoItem) {
		int	nRet	= MEDIA_CONTENT_ERROR_NONE;
		nRet		= media_info_get_longitude(pVideoItem, &nLongitude);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("media_info_get_longitude failed : [%d]!!!", nRet);
			return 0;
		}
	}

	return nLongitude;
}

double mp_util_svc_get_video_item_latitude(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	double dLatitude = 0;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
	if (pVideoItem) {
		int	nRet	= MEDIA_CONTENT_ERROR_NONE;
		nRet = media_info_get_latitude(pVideoItem, &dLatitude);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("media_info_get_latitude failed : [%d]!!!", nRet);
			return 0;
		}
	}

	return dLatitude;
}

char *mp_util_svc_get_video_item_modified_time(const int nVideoItemIndex)
{
	if (!VideoItemList){
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	VideoLogInfo("");

	time_t pModifiedTime;
	struct tm sModifiedTime;
	char szModifiedTime[STR_LEN_MAX] = {0,};
	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
	if (pVideoItem) {
		int nRet = MEDIA_CONTENT_ERROR_NONE;
		nRet = media_info_get_modified_time(pVideoItem, &pModifiedTime);
		if (nRet != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("media_info_get_modified_time failed : [%d]!!!", nRet);
			return 0;
		}
		gmtime_r(&pModifiedTime, &sModifiedTime);
		strftime(szModifiedTime, STR_LEN_MAX-1, "%d %b. %Y. %H:%M", &sModifiedTime);

		return g_strdup(szModifiedTime);
	}
	return NULL;
}

int mp_util_svc_get_video_list_size_for_checking_index(void)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	return (int)eina_list_count(VideoItemList);
}

int mp_util_svc_get_number_of_video_item_by_type(void)
{

	return nNumberOfVideoItemByType;
}

int mp_util_svc_get_total_video_type_files(void)
{

	return (int)nTotalVideoTypeFiles;
}

void *mp_util_svc_get_video_item(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	return (void*)eina_list_nth(VideoItemList, nVideoItemIndex);
}

void mp_util_svc_print_list_info(void)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return;
	}

	VideoLogInfo("List size : %d", mp_util_svc_get_number_of_video_item_by_type());

	media_info_h pVideoItem = NULL;
	Eina_List *pIterateList = NULL;
	int nCounter = 0;
	EINA_LIST_FOREACH(VideoItemList, pIterateList, pVideoItem) {
		if (pVideoItem) {
			char *szTitle = NULL;
			media_info_get_display_name(pVideoItem, &szTitle);
			if (szTitle) {
				VideoLogInfo("%d - Print title of all list items : %s", nCounter, szTitle);
				free(szTitle);
			} else {
				VideoLogInfo("%d - Print title of all list items : NULL.", nCounter);
			}
			nCounter++;
		}
	}
}

int mp_util_svc_find_video_item_index_by_url(const char *szPath)
{
	if (!VideoItemList || !szPath) {
		VideoLogError("[ERR] No exist VideoItemList or szPath.");
		return 99999;
	}

	unsigned int nCounter = 0;

	Eina_List *pCurList = NULL;
	media_info_h pVideoItem = NULL;

	EINA_LIST_FOREACH(VideoItemList, pCurList, pVideoItem) {
		if (pVideoItem) {
			char *szUrl = NULL;
			//media_info_get_display_name(pVideoItem, &szTitle);
			media_info_get_file_path(pVideoItem, &szUrl);
			if (szUrl && szPath) {
				if (!g_strcmp0(szUrl , szPath))
				{
					VideoSecureLogDebug("Find same file url. index : %d", nCounter);
					free(szUrl);
					return nCounter;
				}
			}
			MP_FREE_STRING(szUrl);
		}

		nCounter++;
	}

	return 99999;
}

bool mp_util_svc_extract_video_folder_list_by_type(int nSortType, int nItemType)
{
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	switch (nItemType) {
	case MP_LIST_VIEW_ALL:
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121)");
		break;

	case MP_LIST_VIEW_PHONE:
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1)");

		break;

	case MP_LIST_VIEW_CLOUD:
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101) and MEDIA_STORAGE_TYPE = 101");

		break;

	default:
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121)");

		break;
	}

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_folder_foreach_folder_from_db(m_FilterHandle, mp_util_svc_iterate_for_folder_list_cb, &VideoFolderList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail media_folder_foreach_folder_from_db().");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
	}

	return TRUE;
}

bool mp_util_svc_extract_personal_video_folder_list(int nSortType, bool bPersonal)
{
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	if (bPersonal == FALSE) {
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH NOT LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
	}
	else
	{
		snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
	}
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_folder_foreach_folder_from_db(m_FilterHandle, mp_util_svc_iterate_for_personal_folder_list_cb, &VideoFolderList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail media_folder_foreach_folder_from_db().");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
	}

	return TRUE;
}


bool mp_util_svc_destory_video_folder_list(void)
{
	if (!VideoFolderList) {
		VideoLogError("VideoFolderList is empty already.");
		return FALSE;
	}

	media_folder_h pTmpFolderItem = NULL;
	EINA_LIST_FREE(VideoFolderList, pTmpFolderItem) {
		if (pTmpFolderItem) {
			media_folder_destroy(pTmpFolderItem);
		}
	}

	VideoFolderList = NULL;

	return TRUE;
}

int mp_util_svc_get_video_folder_size(void)
{
	if (!VideoFolderList) {
		VideoLogError("Not exist video item list handle.");
		return 0;
	}

	if (VideoFolderList) {
		return (int)eina_list_count(VideoFolderList);
	}

	return 0;
}

char *mp_util_svc_get_video_folder_name(int nVideoFolderIndex)
{
	if (!VideoFolderList) {
		VideoLogError("Not exist video folder list handle.");
		return 0;
	}

	if (nVideoFolderIndex < 0 || nVideoFolderIndex >= mp_util_svc_get_video_folder_size()) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	char *szFolderName = NULL;
	media_folder_h pFolderItem = (media_folder_h)eina_list_nth(VideoFolderList, nVideoFolderIndex);
	if (pFolderItem) {
		media_folder_get_name(pFolderItem, &szFolderName);

		if (szFolderName) {
			VideoLogError("szFolderName : %s", szFolderName);
			return szFolderName;
		}
	}

	return NULL;
}

char *mp_util_svc_get_video_folder_url(int nVideoFolderIndex)
{
	if (!VideoFolderList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	if (nVideoFolderIndex < 0 || nVideoFolderIndex >= mp_util_svc_get_video_folder_size()) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	char *szFolderPath = NULL;
	media_folder_h pFolderItem = (media_folder_h)eina_list_nth(VideoFolderList, nVideoFolderIndex);
	if (pFolderItem) {
		media_folder_get_path(pFolderItem, &szFolderPath);

		if (szFolderPath) {
			VideoSecureLogInfo("szFolderPath : %s", szFolderPath);
			return szFolderPath;
		}
	}

	return NULL;
}

char *mp_util_svc_get_video_folder_id(int nVideoFolderIndex)
{
	if (!VideoFolderList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	if (nVideoFolderIndex < 0 || nVideoFolderIndex >= mp_util_svc_get_video_folder_size()) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	char *szFolderId = NULL;
	media_folder_h pFolderItem = (media_folder_h)eina_list_nth(VideoFolderList, nVideoFolderIndex);
	if (pFolderItem) {
		media_folder_get_folder_id(pFolderItem, &szFolderId);

		if (szFolderId) {
			VideoLogInfo("szFolderId : %s", szFolderId);
			return szFolderId;
		}
	}

	return NULL;
}

void mp_util_svc_register_thumbnail_update_func(int nVideoItemIndex, void *pUserCallbackFunc, void *pUserData)
{
	if (!pUserCallbackFunc) {
		VideoLogError("[ERR]");
		return;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return;
	}

	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	if (media_info_create_thumbnail(pVideoItem, pUserCallbackFunc, pUserData) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogWarning("media_info_create_thumbnail is Fail");
	}
}

void mp_util_svc_cancel_thumbnail_update(int nVideoItemIndex)
{
	//VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return;
	}

	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pVideoItem) {
		if (media_info_cancel_thumbnail(pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogWarning("media_info_cancel_thumbnail is Fail");
			return;
		}
	}
	else
	{
		VideoLogError("[ERR] pVideoItem is not existed.");
	}
}

void mp_util_svc_update_thumbnail_info(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return;
	}

	char *szMediaUri = mp_util_svc_get_video_url(nVideoItemIndex);

	if (!szMediaUri) {
		VideoLogError("No exist video path.");
		return;
	}

	VideoSecureLogInfo("szMediaUri : %s", szMediaUri);

	media_info_h pUpdatedVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to create media filter handle.");
		return;
	}

	memset(szTmpStr, 0, STR_LEN_MAX);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szMediaUri);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		if (szMediaUri) {
			free(szMediaUri);
			szMediaUri = NULL;
		}
		return;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pUpdatedVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		if (szMediaUri) {
			free(szMediaUri);
			szMediaUri = NULL;
		}
		return;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		if (szMediaUri) {
			free(szMediaUri);
			szMediaUri = NULL;
		}
		return;
	}

	if (pUpdatedVideoItem) {
		char *szTmpStr = NULL;
		media_info_get_thumbnail_path(pUpdatedVideoItem, &szTmpStr);
		VideoSecureLogDebug("pUpdatedVideoItem - thumbnail path : %s", szTmpStr);
		if (szTmpStr) {
			free(szTmpStr);
			szTmpStr = NULL;
		}

		media_info_get_display_name(pUpdatedVideoItem, &szTmpStr);
		VideoSecureLogDebug("pUpdatedVideoItem -  title : %s", szTmpStr);
		if (szTmpStr) {
			free(szTmpStr);
			szTmpStr = NULL;
		}

		int nCounter = 0;
		Eina_List *pCurList = NULL;
		media_info_h pTmpVideoItem = NULL;
		EINA_LIST_FOREACH(VideoItemList, pCurList, pTmpVideoItem) {
			if (nCounter == nVideoItemIndex) {
				VideoLogError("nCounter : %d", nCounter);
				if (pTmpVideoItem)
				{
					media_info_destroy(pTmpVideoItem);
				}
				pCurList->data = pUpdatedVideoItem;
				break;
			}
			++nCounter;
		}
	}
	else
	{
		VideoLogError("Fail to get video item from db..");
	}

	if (szMediaUri) {
		free(szMediaUri);
		szMediaUri = NULL;
	}
}

char *mp_util_svc_get_video_id_by_index(const int nVideoItemIndex)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	char *szMediaId = NULL;
	if (pVideoItem) {
		int ret = media_info_get_media_id(pVideoItem, &szMediaId);
		VideoLogInfo("VideoID : %s, %d", szMediaId, ret);
	}

	return szMediaId;
}

MpMediaStorageType mp_util_svc_get_folder_storage(const int nVideoFolderIndex)
{

	MpMediaStorageType storage_local = MP_MEDIA_TYPE_STORAGE_UNKNOWN;

	if (!VideoFolderList) {
		VideoLogError("Not exist video folder list handle.");
		return storage_local;
	}

	if (nVideoFolderIndex < 0 || nVideoFolderIndex >= mp_util_svc_get_video_folder_size()) {
		VideoLogError("Error number of list item.");
		return storage_local;
	}

	media_content_storage_e storage_type = MEDIA_CONTENT_STORAGE_INTERNAL;
	media_folder_h pFolderItem = (media_folder_h)eina_list_nth(VideoFolderList, nVideoFolderIndex);
	if (media_folder_get_storage_type(pFolderItem, &storage_type) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("fail to get the folder storage type");
	}
	if (storage_type == MEDIA_CONTENT_STORAGE_INTERNAL)
	{
		storage_local = MP_MEDIA_TYPE_STORAGE_INTERNAL;
	} else if (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL)
	{
		storage_local = MP_MEDIA_TYPE_STORAGE_EXTERNAL;
	}

	//char *szFolderName = NULL;
//	media_folder_h pFolderItem = (media_folder_h)eina_list_nth(VideoFolderList, nVideoFolderIndex);
//	media_content_storage_ex_e storage_type = MEDIA_CONTENT_STORAGE_INTERNAL_EX;
//	if (pFolderItem)
//	{
//		if (media_folder_get_storage_type_ex(pFolderItem, &storage_type) != MEDIA_CONTENT_ERROR_NONE) {
//			VideoLogError("fail to get the folder storage type");
//		}
//
//		VideoSecureLogInfo("storage_type : %d", storage_type);
//		if (storage_type == MEDIA_CONTENT_STORAGE_INTERNAL_EX)
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_INTERNAL;
//		}
//		else if (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL_EX)
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_EXTERNAL;
//		}
//		else if (storage_type == MEDIA_CONTENT_STORAGE_DROPBOX_EX)
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_DROPBOX;
//		}
//		else
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_UNKNOWN;
//		}
//	}

	return storage_local;
}


MpMediaStorageType mp_util_svc_get_video_storage(const int nVideoItemIndex)
{
	MpMediaStorageType storage_local = MP_MEDIA_TYPE_STORAGE_UNKNOWN;

	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return storage_local;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return storage_local;
	}

	//media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);
//	media_content_storage_ex_e storage_type = MEDIA_CONTENT_STORAGE_DROPBOX_EX;

//	if (pVideoItem)
//	{
//		if (media_info_get_storage_type_ex(pVideoItem, &storage_type) != MEDIA_CONTENT_ERROR_NONE)	{
//			VideoLogError("fail to get the content storage type");
//		}
//
//		VideoSecureLogInfo("storage_type : %d", storage_type);
//		if (storage_type == MEDIA_CONTENT_STORAGE_INTERNAL_EX)
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_INTERNAL;
//		}
//		else if (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL_EX)
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_EXTERNAL;
//		}
//		else if (storage_type == MEDIA_CONTENT_STORAGE_DROPBOX_EX)
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_DROPBOX;
//		}
//		else
//		{
//			storage_local = MP_MEDIA_TYPE_STORAGE_UNKNOWN;
//		}
//	}

	return storage_local;
}

bool mp_util_svc_is_cloud_storage(const int nVideoItemIndex)
{
	VideoLogInfo("disable cloud module.");
	return FALSE;
}

bool mp_util_folder_is_cloud_storage(const int nFolderIndex)
{
	VideoLogInfo("disable cloud module.");
	return FALSE;
}


int mp_util_svc_get_cloud_total_from_video_list(void)
{
	VideoLogInfo("disable cloud module.");
	return 0;
}

bool mp_util_svc_insert_file_to_media_content_db(const char *szFilePath)
{
	if (!szFilePath) {
		VideoLogError("No existed file path.");
		return FALSE;
	}

	media_info_h info = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	nRet = media_info_insert_to_db(szFilePath, &info);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to insert a download file into media-content database. : %d", nRet);
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

void mp_util_svc_update_database_cb(media_content_error_e nError, int nPid,
									media_content_db_update_item_type_e nUpdateItem,
									media_content_db_update_type_e nUpdateType,
									media_content_type_e nMediaType,
									char *szUuid, char *szPath, char *szMimeType, void *pUserData)
{
	if (nError) {
		VideoLogError("error occured! : %d", nError);
		return;
	}

	if (nPid == mp_util_get_pid()) {
		VideoLogError("It is same processor");
		return;
	}

	UpdateDatabaseCbFunc pUpdateDbCbFunc = pUserData;

	if (nUpdateItem == MEDIA_ITEM_FILE) {

		if (nMediaType != MEDIA_CONTENT_TYPE_VIDEO) {
			VideoLogError("Updated contents are not video items.");
			return;
		}
	}
	else if (nUpdateItem == MEDIA_ITEM_DIRECTORY) {
		VideoLogInfo("MEDIA_ITEM_DIRECTORY");
		///////////////////////////////////////////////////////////////////
		// UPDATE ALL because It is not possible to extract video items. //
		///////////////////////////////////////////////////////////////////
	}
	else
	{
		VideoLogInfo("Nothing item type.");
	}

	if (nUpdateType == MEDIA_CONTENT_INSERT) {
		VideoLogInfo("MEDIA_CONTENT_INSERT");
	}
	else if (nUpdateType == MEDIA_CONTENT_DELETE) {
		VideoLogInfo("MEDIA_CONTENT_DELETE");
	}
	else if (nUpdateType == MEDIA_CONTENT_UPDATE) {
		VideoLogInfo("MEDIA_CONTENT_UPDATE");
	}
	else
	{
		VideoLogInfo("Nothing update type.");
	}

	if (pUpdateDbCbFunc) {
		VideoLogInfo("Update List View. = %d,%d", nUpdateItem, nUpdateType);
		pUpdateDbCbFunc((void *)nUpdateType, 1.0, nUpdateType);
	}
}


bool mp_util_svc_set_update_db_cb_func(UpdateDatabaseCbFunc pUpdateDbCbFunc)
{
	if (!pUpdateDbCbFunc) {
		VideoLogError("No exsited pUpdateDbCbFunc.");
		return FALSE;
	}
	bool bRet = TRUE;
	mp_util_svc_unset_update_db_cb_func();

	if (media_content_set_db_updated_cb((void*)mp_util_svc_update_database_cb, pUpdateDbCbFunc) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to registe update database callback function.");
		bRet = FALSE;
	}

//	if (media_content_set_db_updated_cloud_cb(&g_pCloud_h, (void*)mp_util_svc_update_database_cb, pUpdateDbCbFunc) != MEDIA_CONTENT_ERROR_NONE)
//	{
//		VideoLogInfo("Fail to registe update database callback function.");
//		bRet = FALSE;
//	}

	return bRet;
}


void mp_util_svc_unset_update_db_cb_func(void)
{

	if (media_content_unset_db_updated_cb() != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to unregiste media_content_set_db_updated_cb.");
	}
//	if (media_content_unset_db_updated_cloud_cb(g_pCloud_h) != MEDIA_CONTENT_ERROR_NONE)
//	{
//		VideoLogInfo("Fail to unregiste media_content_set_db_updated_cb.");
//	}
	//g_pCloud_h = NULL;
}

///////////////////////////////////////////////////////
// For livebox list

static bool mp_util_svc_iterate_for_last_played_video_item_cb(media_info_h pMediaItem, void *pUserData)
{
	media_info_h pAssignMediaItem = NULL;
	media_content_type_e nMediaType = MEDIA_CONTENT_TYPE_OTHERS;

	if (!pMediaItem) {
		return TRUE;
	}

	media_info_get_media_type(pMediaItem, &nMediaType);

	if (nMediaType == MEDIA_CONTENT_TYPE_VIDEO) {
		char *szTmpItemFilePath = NULL;
		char szItemFilePath[STR_LEN_MAX] = {0,};

		media_info_get_file_path(pMediaItem, &szTmpItemFilePath);

		if (szTmpItemFilePath == NULL) {
			VideoLogError("Fail to get file path of media item.");
			return TRUE;
		}
		strncpy(szItemFilePath, szTmpItemFilePath, STR_LEN_MAX - 1);

		free(szTmpItemFilePath);
		szTmpItemFilePath = NULL;

		media_info_clone(&pAssignMediaItem, pMediaItem);
		Eina_List **pList = (Eina_List **)pUserData;
		*pList = eina_list_append(*pList, pAssignMediaItem);

		if (eina_list_count(*pList) > RECENTLY_PLAYED_VIDEO_ITEM_MAX) {
			VideoLogError("Break extract video item about recentl played item.");
			return FALSE;
		}
	}

	return TRUE;
}

bool mp_util_svc_extract_last_played_item(void)
{
	if (RecentPlayedItemList) {
		mp_util_svc_destory_last_played_item_list();
	}

	filter_h m_FilterHandle = NULL;

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	if (media_filter_set_condition(m_FilterHandle, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121)", MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_set_order(m_FilterHandle, MEDIA_CONTENT_ORDER_DESC, MEDIA_LAST_PLAYED_TIME, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_last_played_video_item_cb, &RecentPlayedItemList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	return TRUE;
}

bool mp_util_svc_destory_last_played_item_list(void)
{
	if (!RecentPlayedItemList) {
		VideoLogError("VideoItemList is empty already.");
		return FALSE;
	}

	media_info_h pTmpRecentPlayedItem = NULL;
	EINA_LIST_FREE(RecentPlayedItemList, pTmpRecentPlayedItem) {
		if (pTmpRecentPlayedItem) {
			media_info_destroy(pTmpRecentPlayedItem);
		}
	}

	RecentPlayedItemList = NULL;

	return TRUE;
}

char *mp_util_svc_get_last_played_video_item_thumbnail(const int nVideoItemIndex)
{
	if (!RecentPlayedItemList) {
		VideoLogError("Not exist video item list handle.");
		return NULL;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex > (int)eina_list_count(RecentPlayedItemList)) {
		VideoLogError("Error number of list item.");
		return NULL;
	}

	char *szVideoItemThumbnailPath = NULL;
	media_info_h pVideoItem = (media_info_h)eina_list_nth(RecentPlayedItemList, nVideoItemIndex);

	if (pVideoItem) {
		media_info_get_thumbnail_path(pVideoItem, &szVideoItemThumbnailPath);
		VideoSecureLogInfo("szVideoItemThumbnailFilePath : %s", szVideoItemThumbnailPath);
	}

	return szVideoItemThumbnailPath;
}


char *mp_util_svc_get_duration_str_time(int nVideoItemIndex)
{
	char szTmpStr[STR_LEN_MAX] = {0, };
	char szTmpStrDurationTime[STR_LEN_MAX] = {0, };
	char szTmpStrLastPlayedPos[STR_LEN_MAX] = {0, };

	unsigned int nVideoInfoDuration = mp_util_svc_get_video_duration_time(nVideoItemIndex);
	unsigned int nVideoInfoLastTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);

	if (nVideoInfoDuration > 0) {

		mp_util_convert_time(nVideoInfoDuration, szTmpStrDurationTime, FALSE);

		if (nVideoInfoLastTime > 0) {
			mp_util_convert_time(nVideoInfoLastTime, szTmpStrLastPlayedPos, TRUE);
			snprintf(szTmpStr, STR_LEN_MAX, "<match>%s</match>/%s", szTmpStrLastPlayedPos, szTmpStrDurationTime);
		} else {
			snprintf(szTmpStr, STR_LEN_MAX, "%s", szTmpStrDurationTime);
		}
	}
	else
	{
		snprintf(szTmpStr, STR_LEN_MAX, "--:--:--");
	}

	return strdup(szTmpStr);
}

bool mp_util_svc_get_video_detail_by_video_url(const char *szPath,
	int *nWidth, int *nHeight, char **szTitle, unsigned int *nDuration, unsigned int *nPlayedTime)
{
	if (!szPath) {
		VideoLogError("No exist video path.");
		return FALSE;
	}

	VideoSecureLogDebug("szPath : %s", szPath);

	media_info_h pVideoItem = NULL;
	filter_h m_FilterHandle = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	//snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_PATH = \"%s\"", szPath);
	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121) AND MEDIA_PATH = \"%s\"", szPath);
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_video_item_cb, &pVideoItem) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	if (pVideoItem) {
		video_meta_h pVideoMetaHandle = NULL;
		if (media_info_get_video(pVideoItem, &pVideoMetaHandle) == MEDIA_CONTENT_ERROR_NONE) {
			//last played time
			int nLastPlayedTime = 0;
			video_meta_get_played_position(pVideoMetaHandle, &nLastPlayedTime);
			*nPlayedTime = (unsigned int)nLastPlayedTime;

			//Duratiom time
			int duration = 0;
			video_meta_get_duration(pVideoMetaHandle, &duration);
			*nDuration = (unsigned int)duration;

			//get width
			int width = 0;
			video_meta_get_width(pVideoMetaHandle, &width);
			*nWidth = width;

			//get height
			int height = 0;
			video_meta_get_height(pVideoMetaHandle, &height);
			*nHeight = height;

			video_meta_destroy(pVideoMetaHandle);
		}

		//display name
		char *szName = NULL;
		media_info_get_display_name(pVideoItem, &szName);
		*szTitle = szName;
		media_info_destroy(pVideoItem);
	}
	VideoSecureLogDebug("Last Played Time : %s,%d:%d,%d,%d", *szTitle, *nPlayedTime, *nDuration, *nWidth, *nHeight);

	return TRUE;
}


bool mp_util_svc_extract_video_list_from_folder_by_personal(char *szFolderPath, int nSortType, int nItemType, int nViewType)
{
	if (VideoItemList) {
		VideoLogError("Already exist video list.");
		return FALSE;
	}

	if (!szFolderPath) {
		VideoLogError("[ERR]");
		return FALSE;
	}

	filter_h m_FilterHandle = NULL;
	media_folder_h m_FolderHandle = NULL;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;

	char *szFolderID = NULL;
	char szTmpStr[STR_LEN_MAX] = {0,};
	char szOrderObj[STR_LEN_MAX] = {0,};

	nTotalVideoTypeFiles = 0;
	nNumberOfVideoItemByType = 0;

	switch (nSortType) {
	case MP_MEDIA_SORT_BY_NONE:
	case MP_MEDIA_SORT_BY_RECENTLY_VIEWED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_RECENTLY_ADDED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_ADDED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_NAME:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_DISPLAY_NAME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_SIZE:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_SIZE, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_TYPE:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_MIME_TYPE, STR_LEN_MAX - 1);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
	}

	switch (nItemType) {
		case MP_LIST_VIEW_ALL:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			}
		break;

		case MP_LIST_VIEW_PHONE:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			}
		break;

		case MP_LIST_VIEW_CLOUD:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			}
		break;

		default:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH NOT LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and FOLDER_PATH = \"%s\" and MEDIA_PATH LIKE \"%s%%\"", szFolderPath, VIDEO_UTIL_PERSONAL_HEAD_STR);
			}
		 break;
	}

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to create media filter handle.");
		return FALSE;
	}

	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_folder_foreach_folder_from_db(m_FilterHandle, mp_util_svc_iterate_for_get_folder_cb, &m_FolderHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail media_folder_foreach_folder_from_db().");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (!m_FolderHandle) {
		VideoLogError("Fail to get folder handle.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1");
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_set_order(m_FilterHandle, nOrderType, szOrderObj, MEDIA_CONTENT_COLLATE_NOCASE) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_folder_get_folder_id(m_FolderHandle, &szFolderID) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get folder id.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_folder_foreach_media_from_db(szFolderID, m_FilterHandle, mp_util_svc_iterate_cb, &VideoItemList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);

		if (szFolderID) {
			free(szFolderID);
			szFolderID = NULL;
		}
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
	}

	if (szFolderID) {
		free(szFolderID);
		szFolderID = NULL;
	}

	return TRUE;
}

bool mp_util_svc_extract_video_list_by_personal(int nSortType, int nItemType, int nViewType)
{
	if (VideoItemList) {
		VideoLogError("Already exist video list.");
		return FALSE;
	}

	nWhatFileType = MP_MEDIA_ALL_FILE_TYPE;

	filter_h m_FilterHandle = NULL;
	media_content_order_e nOrderType = MEDIA_CONTENT_ORDER_ASC;
	char szTmpStr[STR_LEN_MAX] = {0,};
	char szOrderObj[STR_LEN_MAX] = {0,};

	nTotalVideoTypeFiles = 0;
	nNumberOfVideoItemByType = 0;

	switch (nSortType) {
	case MP_MEDIA_SORT_BY_NONE:
	case MP_MEDIA_SORT_BY_RECENTLY_VIEWED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_RECENTLY_ADDED:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_ADDED_TIME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_NAME:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_DISPLAY_NAME, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_SIZE:
		nOrderType = MEDIA_CONTENT_ORDER_DESC;
		strncpy(szOrderObj, MEDIA_SIZE, STR_LEN_MAX - 1);
		break;

	case MP_MEDIA_SORT_BY_TYPE:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_MIME_TYPE, STR_LEN_MAX - 1);
		break;

	default:
		nOrderType = MEDIA_CONTENT_ORDER_ASC;
		strncpy(szOrderObj, MEDIA_LAST_PLAYED_TIME, STR_LEN_MAX - 1);
	}

	switch (nItemType) {
	case MP_LIST_VIEW_ALL:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH NOT LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			}
		break;

	case MP_LIST_VIEW_PHONE:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH NOT LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 and (MEDIA_STORAGE_TYPE = 0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			}

		break;

	case MP_LIST_VIEW_CLOUD:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH NOT LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			}

		break;

	default:
			if (nViewType == 0) {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH NOT LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			} else {
				snprintf(szTmpStr, STR_LEN_MAX, "MEDIA_TYPE = 1 AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1) and MEDIA_PATH LIKE \"%s%%\"", VIDEO_UTIL_PERSONAL_HEAD_STR);
			}
		break;
	}
	VideoLogInfo("MEDIA_PATH not like %s", szTmpStr);

	if (media_filter_create(&m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("Fail to create media filter handle.");
		return FALSE;
	}
	if (media_filter_set_condition(m_FilterHandle, szTmpStr, MEDIA_CONTENT_COLLATE_DEFAULT) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_set_order(m_FilterHandle, nOrderType, szOrderObj, MEDIA_CONTENT_COLLATE_NOCASE) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to set order.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_info_foreach_media_from_db(m_FilterHandle, mp_util_svc_iterate_for_video_item_list_view_cb, &VideoItemList) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to get video item list with filter condition.");
		media_filter_destroy(m_FilterHandle);
		return FALSE;
	}

	if (media_filter_destroy(m_FilterHandle) != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("Fail to destroy media filter handle.");
		return FALSE;
	}

	return TRUE;
}

bool mp_util_svc_check_valid_personal(int nViewType)
{
	bool bRet = FALSE;
	bool bDeviceVideo = FALSE;
	char *pVideoFile = NULL;

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	if ((nVideoListSize < 1) || (mp_util_get_personal_status() == FALSE)) {
		return FALSE;
	}

	int nIndex = 0;
	for (nIndex = 0; nIndex < nVideoListSize; nIndex++) {

		bDeviceVideo =	mp_util_svc_is_device_storage(nIndex);
		if (!bDeviceVideo) {
			continue;
		}
		pVideoFile = mp_util_svc_get_video_url(nIndex);
		bRet = mp_util_check_video_personal_status((const char*)pVideoFile);
		MP_FREE_STRING(pVideoFile);
		if (nViewType == 0) {
			if (!bRet) {
				return TRUE;
			}
		} else {
			if (bRet) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

int mp_util_svc_move_from_db(const char *path)
{
	if (!path) {
		return -1;
	}
	int res = MEDIA_CONTENT_ERROR_NONE;
	res = media_content_scan_file(path);
	return res;
}


void mp_util_svc_move_to_db(const int nVideoItemIndex, const char *szVideoPath)
{
	if (!VideoItemList) {
		VideoLogError("Not exist video item list handle.");
		return;
	}

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("Error number of list item.");
		return;
	}

	media_info_h pVideoItem = (media_info_h)eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pVideoItem) {
		if (media_info_move_to_db(pVideoItem, szVideoPath) != MEDIA_CONTENT_ERROR_NONE) {
			VideoLogError("Failed move to db");
		}
	}

}

bool mp_util_svc_remove_list_item_from_list(int nVideoItemIndex)
{

	if (!VideoItemList) {
		VideoLogError("pList is empty already.");
		return FALSE;
	}

	media_info_h pItem = eina_list_nth(VideoItemList, nVideoItemIndex);

	if (pItem) {
		VideoItemList = eina_list_remove(VideoItemList, pItem);
		media_info_destroy(pItem);
		nNumberOfVideoItemByType--;
	}
	else
	{
		VideoLogError("Fail to remove item in media svc and list.");
		return FALSE;
	}

	return TRUE;
}

bool mp_util_svc_remove_folder_list_item_from_list(int nVideoItemIndex)
{
	if (!VideoFolderList) {
		VideoLogError("pList is empty already.");
		return FALSE;
	}

	media_folder_h pItem = eina_list_nth(VideoFolderList, nVideoItemIndex);

	if (pItem) {
		VideoFolderList = eina_list_remove(VideoFolderList, pItem);
		media_folder_destroy(pItem);
	}
	else
	{
		VideoLogError("Fail to remove item in media svc and list.");
		return FALSE;
	}

	return TRUE;
}

#ifdef ENABLE_DRM_FEATURE
bool mp_util_svc_filter_drm_video_item_list(void)
{
	if (!VideoItemList) {
		VideoLogError("VideoItemList is empty already.");
		return FALSE;
	}

	char *szVideoItemFilePath = NULL;
	int nVideoIndex = 0;
	for (nVideoIndex = 0; nVideoIndex < nNumberOfVideoItemByType; nVideoIndex++) {

		szVideoItemFilePath = mp_util_svc_get_video_url(nVideoIndex);
		bool bDRMLock = FALSE;
		mp_util_check_drm_lock((const char*)szVideoItemFilePath, &bDRMLock);

		if (bDRMLock) {
			mp_util_svc_remove_list_item_from_list(nVideoIndex);

			nVideoIndex --;
		}

		if (szVideoItemFilePath) {
			free(szVideoItemFilePath);
			szVideoItemFilePath = NULL;
		}


	}

	return TRUE;
}
#endif

#ifdef ENABLE_DRM_FEATURE
bool mp_util_svc_check_no_drm_video(void)
{
	if (!VideoItemList) {
		VideoLogError("VideoItemList is empty already.");
		return FALSE;
	}
	char *szVideoItemFilePath = NULL;
	media_info_h pVideoItem = NULL;
	Eina_List *pIterateList = NULL;
	bool bDRMLock = FALSE;


	EINA_LIST_FOREACH(VideoItemList, pIterateList, pVideoItem) {
		if (pVideoItem) {
			media_info_get_file_path(pVideoItem, &szVideoItemFilePath);
			if (szVideoItemFilePath) {
				mp_util_check_drm_lock((const char*)szVideoItemFilePath, &bDRMLock);
				free(szVideoItemFilePath);
				szVideoItemFilePath = NULL;
			}
			if (!bDRMLock) {
				return TRUE;
			}
		}
	}

	return FALSE;
}
#endif

bool mp_util_svc_is_device_storage(const int nVideoItemIndex)
{
	MpMediaStorageType storage_type = mp_util_svc_get_video_storage(nVideoItemIndex);
	if ((storage_type == MP_MEDIA_TYPE_STORAGE_INTERNAL) || (storage_type == MP_MEDIA_TYPE_STORAGE_EXTERNAL)) {
		return TRUE;
	}

	return FALSE;
}

bool mp_util_svc_is_personal_data(const int nVideoItemIndex)
{
	bool bPersonalVideo = FALSE;
	char *szPath = mp_util_svc_get_video_url(nVideoItemIndex);
	if (szPath) {
		bPersonalVideo = mp_util_check_video_personal_status(szPath);
		free(szPath);
		szPath = NULL;
	}

	return bPersonalVideo;
}

bool mp_util_svc_scan_file(const char *szFilePath)
{
	if (!szFilePath) {
		VideoLogError("szFilePath is NULL.");
		return FALSE;
	}
	media_content_scan_file(szFilePath);
	return TRUE;
}


/*----------------sort video list-----------------------------*/

static int __mp_util_svc_sort_by_type(const void *d1, const void *d2)
{
	if (d1 == NULL) {
		return (1);
	}
	if (d2 == NULL) {
		return (-1);
	}

	char *szMime_1 = NULL;
	char *szMime_2 = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	int result = 0;

	media_info_h pVideoItem_1 = (media_info_h)d1;
	media_info_h pVideoItem_2 = (media_info_h)d2;

	nRet = media_info_get_mime_type(pVideoItem_1, &szMime_1);
	if (nRet != MEDIA_CONTENT_ERROR_NONE || szMime_1 == NULL) {
		VideoLogError("[ERR] Fail to get item title. - 0x%x", nRet);
		MP_FREE_STRING(szMime_1);
		return (1);
	}
	nRet = media_info_get_mime_type(pVideoItem_2, &szMime_2);
	if (nRet != MEDIA_CONTENT_ERROR_NONE || szMime_2 == NULL) {
		VideoLogError("[ERR] Fail to get item title. - 0x%x", nRet);
		MP_FREE_STRING(szMime_1);
		MP_FREE_STRING(szMime_2);
		return (-1);
	}

	result = strcasecmp(szMime_1, szMime_2);

	MP_FREE_STRING(szMime_1);
	MP_FREE_STRING(szMime_2);

	if (result > 0) {
		return (1);
	} else {
		return (-1);
	}

	return result;
}

static int __mp_util_svc_sort_by_name(const void *d1, const void *d2)
{
	if (d1 == NULL) {
		return (1);
	}
	if (d2 == NULL) {
		return (-1);
	}

	char *szTitle_1 = NULL;
	char *szTitle_2 = NULL;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	int result = 0;

	media_info_h pVideoItem_1 = (media_info_h)d1;
	media_info_h pVideoItem_2 = (media_info_h)d2;
	nRet = media_info_get_display_name(pVideoItem_1, &szTitle_1);
	if (nRet != MEDIA_CONTENT_ERROR_NONE || szTitle_1 == NULL) {
		VideoLogError("[ERR] Fail to get item title. - 0x%x", nRet);
		MP_FREE_STRING(szTitle_1);
		return (1);
	}

	nRet = media_info_get_display_name(pVideoItem_2, &szTitle_2);
	if (nRet != MEDIA_CONTENT_ERROR_NONE || szTitle_2 == NULL) {
		MP_FREE_STRING(szTitle_1);
		MP_FREE_STRING(szTitle_2);
		VideoLogError("[ERR] Fail to get item title. - 0x%x", nRet);
		return (-1);
	}

	result = strcasecmp(szTitle_1, szTitle_2);

	MP_FREE_STRING(szTitle_1);
	MP_FREE_STRING(szTitle_2);

	if (result > 0) {
		return (1);
	} else {
		return (-1);
	}
	return result;
}

static int __mp_util_svc_sort_by_size(const void *d1, const void *d2)
{
	if (d1 == NULL) {
		return (1);
	}
	if (d2 == NULL) {
		return (-1);
	}

	unsigned long long nFileSize1 = 0;
	unsigned long long nFileSize2 = 0;
	int nRet = MEDIA_CONTENT_ERROR_NONE;
	int result = 0;

	media_info_h pVideoItem_1 = (media_info_h)d1;
	media_info_h pVideoItem_2 = (media_info_h)d2;


	nRet = media_info_get_size(pVideoItem_1, &nFileSize1);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("[ERR] Fail to get item size. - 0x%x", nRet);
		return (1);
	}


	nRet = media_info_get_size(pVideoItem_2, &nFileSize2);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogInfo("[ERR] Fail to get item size. - 0x%x", nRet);
		return (-1);
	}

	if (nFileSize1 < nFileSize2) {
		result = 1;
	} else if (nFileSize1 > nFileSize2) {
		result = -1;
	} else {
		result = -1;
	}

	return result;
}
/*
static int __mp_util_svc_sort_by_date(const void *d1, const void *d2)
{
	if (d1 == NULL) {
		return (1);
	}
	if (d2 == NULL) {
		return (-1);
	}

	time_t nTime_1 = 0;
	time_t nTime_2 = 0;

	int nRet = MEDIA_CONTENT_ERROR_NONE;
	int result = 0;

	media_info_h pVideoItem_1 = (media_info_h)d1;
	media_info_h pVideoItem_2 = (media_info_h)d2;


	nRet = media_info_get_modified_time(pVideoItem_1, &nTime_1);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("[ERR] Fail to get item date. - 0x%x", nRet);
		return (1);
	}

	nRet = media_info_get_modified_time(pVideoItem_2, &nTime_2);
	if (nRet != MEDIA_CONTENT_ERROR_NONE) {
		VideoLogError("[ERR] Fail to get item date. - 0x%x", nRet);
		return (-1);
	}

	if (nTime_1 < nTime_2) {
		result = -1;
	} else if (nTime_1 > nTime_2) {
		result = 1;
	} else {
		result = -1;
	}

	return result;
}
*/
void mp_util_svc_ctrl_sort_list(int SortType)
{
	if (!VideoItemList) {
		VideoLogWarning("VideoItemList is NULL.");
		return;
	}

	Eina_Compare_Cb sort_func = NULL;

	switch (SortType) {
	case MP_MEDIA_SORT_BY_RECENTLY_VIEWED:
//		sort_func = __mp_util_svc_sort_by_recently_viewed;
//		break;
	case MP_MEDIA_SORT_BY_RECENTLY_ADDED:
//		sort_func = __mp_util_svc_sort_by_recently_added;
//		break;
	case MP_MEDIA_SORT_BY_NAME:
		sort_func = __mp_util_svc_sort_by_name;
		break;
	case MP_MEDIA_SORT_BY_SIZE:
		sort_func = __mp_util_svc_sort_by_size;
		break;
	case MP_MEDIA_SORT_BY_TYPE:
		sort_func = __mp_util_svc_sort_by_type;
		break;
	default:
		sort_func = __mp_util_svc_sort_by_name;
		break;
	}
	int nVideoTotal = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogWarning("nVideoTotal is %d.", nVideoTotal);
	VideoItemList = eina_list_sort(VideoItemList, nVideoTotal, sort_func);
}

bool mp_util_svc_get_frame_by_position(const char *szFilePath, const char *szDestImage, int nPosition, int nWidth, int nHeight)
{
	if (!szFilePath || !szDestImage) {
		VideoLogInfo("[ERR] No exist szFilePath or szDestImage.");
		return FALSE;
	}

	metadata_extractor_h pMetadata;
	int nRet = 0;
	void *pFrame = NULL;
	int nFrameSize = 0;
	char *szVal = NULL;
	int nRotate = 0;
	unsigned char *pRotBuf = NULL;

	if (metadata_extractor_create(&pMetadata) != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogInfo("[ERR] - metadata_extractor_create()");
		goto Execption;
	}

	if (metadata_extractor_set_path(pMetadata, szFilePath) != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogInfo("[ERR] - metadata_extractor_set_path()");
		goto Execption;
	}

	if (metadata_extractor_get_metadata(pMetadata, METADATA_ROTATE, &szVal) != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogInfo("[ERR] - metadata_extractor_get_metadata()");
		goto Execption;
	}

	if (szVal == NULL) {
		nRotate = 0;
	}
	else {
		nRotate = atoi(szVal);
		MP_FREE_STRING(szVal);
	}

	if (metadata_extractor_get_frame_at_time(pMetadata, nPosition, TRUE, &pFrame, &nFrameSize) != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogInfo("[ERR] - metadata_extractor_get_frame_at_time()");
		goto Execption;
  	}

	if (pFrame == NULL) {
		VideoLogInfo("pFrame is NULL");
		goto Execption;
	}

	if (nRotate != 0) {
		image_util_rotation_e nRot = IMAGE_UTIL_ROTATION_NONE;
		if (nRotate == 90) {
			nRot = IMAGE_UTIL_ROTATION_90;
		}
		else if (nRotate == 180) {
			nRot = IMAGE_UTIL_ROTATION_180;
		}
		else if (nRotate == 270) {
			nRot = IMAGE_UTIL_ROTATION_270;
		}

		if (nRot != IMAGE_UTIL_ROTATION_NONE) {
			unsigned int nResizBufSize = 0;
			int nSrcW = nWidth;
			int nSrcH = nHeight;
			if (image_util_calculate_buffer_size(nWidth, nHeight, IMAGE_UTIL_COLORSPACE_RGB888, &nResizBufSize) != IMAGE_UTIL_ERROR_NONE) {
				VideoLogInfo("image_util_calculate_buffer_size is fail : [0x%x]", nRet);
				goto Execption;
			}

			pRotBuf = calloc(1,sizeof(unsigned char) * nResizBufSize);
			if (pRotBuf == NULL) {
				VideoLogInfo("pRotBuf alloc fail");
				goto Execption;
			}
			else {
				VideoLogInfo("ResizeBuf Size : %d", nResizBufSize);
				if (vp_util_image_rotate(pRotBuf, &nWidth, &nHeight, nRot, pFrame, nSrcW, nSrcH, IMAGE_UTIL_COLORSPACE_RGB888) != IMAGE_UTIL_ERROR_NONE) {
					VideoLogInfo("vp_util_image_rotate is fail : [0x%x]", nRet);
					goto Execption;
				}
			}
		}
	}
	if (pRotBuf) {
		if (image_util_encode_jpeg(pRotBuf, nWidth, nHeight, IMAGE_UTIL_COLORSPACE_RGB888, 100, szDestImage)!= IMAGE_UTIL_ERROR_NONE) {
			VideoLogInfo("image_util_encode_jpeg is fail : [0x%x]", nRet);
			goto Execption;
		}
	} else {
		if (image_util_encode_jpeg(pFrame, nWidth, nHeight, IMAGE_UTIL_COLORSPACE_RGB888, 100, szDestImage)!= IMAGE_UTIL_ERROR_NONE) {
			VideoLogInfo("image_util_encode_jpeg is fail : [0x%x]", nRet);
			goto Execption;
		}
	}

	if (pMetadata) {
	  metadata_extractor_destroy(pMetadata);
	  pMetadata = NULL;
	}

	MP_FREE_STRING(pFrame);
	MP_FREE_STRING(pRotBuf);

	return TRUE;
Execption:

	MP_FREE_STRING(pFrame);
	MP_FREE_STRING(pRotBuf);

	if (pMetadata) {
	  metadata_extractor_destroy(pMetadata);
	  pMetadata = NULL;
	}
	return FALSE;
}

//static bool __mp_util_svc_check_invalid_video_id(void *pInvalidList, char *pVideoId)
//{
//	if (!pInvalidList || !pVideoId)
//	{
//		return FALSE;
//	}
//	char *pVideoTmpId = NULL;
//	Eina_List *pIterateList = NULL;
//
//	EINA_LIST_FOREACH(pInvalidList, pIterateList, pVideoTmpId)
//	{
//		if (pVideoId)
//		{
//			if (g_strcmp0(pVideoTmpId, pVideoId) == 0)
//			{
//				return TRUE;
//			}
//		}
//	}
//
//	return FALSE;
//}

int mp_util_svc_get_wall_video_index(void *pInvalidList)
{
//	media_info_h pVideoItem = NULL;
//	Eina_List *pIterateList = NULL;
//	int nVideoIndex = 0;
//	if (!VideoItemList) {
//		VideoLogError("VideoItemList IS null");
//		return -1;
//	}
//	char *szWallDir = NULL;
//	char *szWallFile = NULL;
//	char *szMediaId = NULL;

//	EINA_LIST_FOREACH(VideoItemList, pIterateList, pVideoItem)
//	{
//		if (pVideoItem)
//		{
//			media_content_storage_ex_e storage_type = MEDIA_CONTENT_STORAGE_INTERNAL_EX;
//			media_info_get_storage_type_ex(pVideoItem, &storage_type);
//			if ((storage_type == MEDIA_CONTENT_STORAGE_INTERNAL_EX) || (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL_EX)) {
//				media_info_get_media_id(pVideoItem, &szMediaId);
//				if (szMediaId)
//				{
//					szWallDir = g_strdup_printf("%s/%s", MP_VIDEO_WALL_DIR, szMediaId);
//					szWallFile = g_strdup_printf(MP_VIDEO_WALL_FRAME_FILE_PREFIX, szWallDir);
//					if (!__mp_util_svc_check_invalid_video_id(pInvalidList, szMediaId))
//					{
//						if (szWallFile && !vp_file_exists(szWallFile))
//						{
//							MP_FREE_STRING(szWallFile);
//							MP_FREE_STRING(szMediaId);
//							return nVideoIndex;
//						}
//					}
//					MP_FREE_STRING(szMediaId);
//				}
//				MP_FREE_STRING(szWallFile);
//			}
//			++nVideoIndex;
//		}
//	}
	return -1;
}

bool mp_util_svc_check_valid_media_id(char *pMediaId, int style, int *nMediaIndex)
{
	if (!pMediaId) {
		return FALSE;
	}

	Eina_List *pIterateList = NULL;
	char *szMediaId = NULL;
	int nIndex = 0;

	if (style == MEDIA_TYPE_FILE) {
		if (!VideoItemList) {
			return FALSE;
		}
		media_info_h pVideoItem = NULL;
		nIndex = 0;

		EINA_LIST_FOREACH(VideoItemList, pIterateList, pVideoItem) {
			if (pVideoItem) {
				media_info_get_media_id(pVideoItem, &szMediaId);

				if (szMediaId && (g_strcmp0(szMediaId, pMediaId) == 0))
				{
					MP_FREE_STRING(szMediaId);
					*nMediaIndex = nIndex;
					return TRUE;
				}
				nIndex++;
				MP_FREE_STRING(szMediaId);
			}
		}
	}
	else if (style == MEDIA_TYPE_DIRECTORY) {
		if (!VideoFolderList) {
			return FALSE;
		}
		nIndex = 0;
		media_folder_h pFolderItem = NULL;

		EINA_LIST_FOREACH(VideoFolderList, pIterateList, pFolderItem) {
			if (pFolderItem) {
				media_folder_get_folder_id(pFolderItem, &szMediaId);
				if (g_strcmp0(szMediaId, pMediaId) == 0)
				{
					MP_FREE_STRING(szMediaId);
					*nMediaIndex = nIndex;

					return TRUE;
				}
				MP_FREE_STRING(szMediaId);
				nIndex++;
			}
		}

	}

	return FALSE;
}


