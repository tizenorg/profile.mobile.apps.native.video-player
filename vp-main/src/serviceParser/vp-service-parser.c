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


#include <string.h>
#include <app_preference.h>
#include <Ecore_File.h>
#include <tzplatform_config.h>

#include "vp-media-content-util.h"

#include "vp-service-parser.h"
#include "vp-play-log.h"
#include "vp-file-util.h"


#define VP_VIDEO_MULTI_WIN_TYPE_KEY		"http://tizen.org/appcontrol/data/miniapp"
#define VP_VIDEO_MULTI_WIN_ON_VALUE		"on"
#define VP_VIDEO_EDIT_MODE_KEY			"edit_mode"
#define VP_VIDEO_LAUNCHED_BY_APP_KEY		"launching_application"
#define VP_VIDEO_SORT_TYPE_KEY			"order_type"
#define VP_VIDEO_LIST_TYPE_KEY			"video_list_type"
#define VP_VIDEO_DEVICE_ID_KEY			"Device id"
#define VP_VIDEO_DMR_ID_KEY				"DMR_ID"

#define VP_VIDEO_COOKIE_KEY			"cookie"
#define VP_VIDEO_PROXY_KEY			"proxy"

#define VP_VIDEO_URI_PATH_KEY			"path"
#define VP_VIDEO_EXIT_ON_BACK			"ExitOnBack"

#define VP_VIDEO_STORE_DOWNLOAD			"StoreDownload"
#define VP_VIDEO_STORE_DOWNLOAD_ON_VALUE	"on"
#define VP_VIDEO_STORE_DOWNLOAD_PLAYED_TIME	"StoreItemLastPlayPosition"
#define VP_VIDEO_STORE_DOWNLOAD_FILE_SIZE	"StoreDL_filesize"

#define VP_VIDEO_APP_CONTROL_REPEAT_MODE		"repeat_mode"
#define VP_VIDEO_APP_CONTROL_REPEAT_VALUE_STOP_AFTER_ALL	"stop_after_all"

#define VP_VIDEO_STORE_ORDER_ID				"store_order_id"
#define VP_VIDEO_STORE_APP_ID				"store_app_id"
#define VP_VIDEO_STORE_MV_ID				"store_mv_id"
#define VP_VIDEO_STORE_SERVER_ID			"store_server_id"


#define VP_VIDEO_TITLE_KEY			"title"

#define VP_VIDEO_DATA_SUBTITLE 			"slink/subtitle"
#define VP_VIDEO_DATA_POSITION 			"slink/position"
#define VP_VIDEO_DATA_TITLE 			"slink/title"
#define VP_VIDEO_DATA_DURATION 			"slink/duration"

static inline char* full_path(char *str1, char *str2) {
	char path[1024] = {};
	snprintf(path, 1024, "%s%s", str1, str2);
	return path;
}

#define PHONE_FOLDER 					tzplatform_getenv(TZ_USER_CONTENT)

#define VP_DEFAULT_VIDEO_ITEM_URL_STR 			full_path(PHONE_FOLDER, "/Videos/Color.mp4")

#define VP_VIDEO_FILE_PREFIX		"file://"

#define PREF_VP_PREVIEW_URL_VIDEOS "preference/org.tizen.videos/preview_url_videos"
#define PREF_VP_PREVIEW_URL_RECORDS "preferencepreference/org.tizen.videos/preview_url_records"


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
static char *__VpServiceParserGetMediaUri(app_control_h pAppSvcHandle);

static bool __VpServiceParserGetMultiAppControl(app_control_h pAppSvcHandle, const char *key, char ***szArrayVal, int *nLength);

static VpLaunchType __VpServiceParserGetLaunchType(app_control_h pAppSvcHandle);
static Eina_Bool __VpServiceParserGetLaunchMultiWindow(app_control_h pAppSvcHandle);
static Eina_Bool __VpServiceParserGetStoreDownloadMode(app_control_h pAppSvcHandle);
static double __VpServiceParserGetStoreDownloadFileSize(app_control_h pAppSvcHandle);
static int __VpServiceParserGetStoreDownloadPlayedTime(app_control_h pAppSvcHandle);
static Eina_Bool __VpServiceParserGetRepeatMode(app_control_h pAppSvcHandle);
static void __VpServiceParserGetStoreOrderInfo(app_control_h pAppSvcHandle, VpServiceData *pReceiveData);

static Eina_Bool __VpServiceParserGetStreamingLaunch(char *pUri);
static char *__VpServiceParserGetDeviceID(app_control_h pAppSvcHandle);
static char *__VpServiceParserGetDMRID(app_control_h pAppSvcHandle);

static char *__VpServiceParserGetCookie(app_control_h pAppSvcHandle);
static char *__VpServiceParserGetProxy(app_control_h pAppSvcHandle);
static char *__VpServiceParserGetMediaTitle(app_control_h pAppSvcHandle);
static Eina_Bool __VpServiceParserGetAUL(app_control_h pAppSvcHandle, char **dpUri);
static Eina_Bool __VpServiceParserGetAppControl(app_control_h pAppSvcHandle, char **dpUri);
static Eina_Bool __VpServiceParserGetWindowID(app_control_h pAppSvcHandle, unsigned int *nXID);
static VpVideoSortType __VpServiceParserGetSortType(app_control_h pAppSvcHandle);
static VpVideoPlayListType __VpServiceParserGetListType(app_control_h pAppSvcHandle);
static Eina_Bool __VpServiceParserCheckURL(char *szURL);
static Eina_Bool __VpServiceParserCheckFileIsExist(char *szFilePath);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


Eina_Bool VpServiceParserGetServiceData(app_control_h pAppSvcHandle, VpServiceData *pReceiveData)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}

	if (pReceiveData == NULL) {
		VideoLogError("pReceiveData == NULL!!!");
		return EINA_FALSE;
	}

	pReceiveData->pMediaUri			= __VpServiceParserGetMediaUri(pAppSvcHandle);
	pReceiveData->szMediaTitle 		= __VpServiceParserGetMediaTitle(pAppSvcHandle);
	pReceiveData->eLaunchType 		= __VpServiceParserGetLaunchType(pAppSvcHandle);
	pReceiveData->bStoreDownload	= __VpServiceParserGetStoreDownloadMode(pAppSvcHandle);
	pReceiveData->dStoreDL_FileSize	= __VpServiceParserGetStoreDownloadFileSize(pAppSvcHandle);
	pReceiveData->nStorePlayedTime 	= 0;
	pReceiveData->bStopAfterRepeat	= __VpServiceParserGetRepeatMode(pAppSvcHandle);

	if (pReceiveData->bStoreDownload) {
		pReceiveData->nStorePlayedTime = __VpServiceParserGetStoreDownloadPlayedTime(pAppSvcHandle);
		__VpServiceParserGetStoreOrderInfo(pAppSvcHandle, pReceiveData);
	}

	if (__VpServiceParserCheckURL(pReceiveData->pMediaUri) == EINA_FALSE) {
		if (__VpServiceParserGetMultiAppControl(pAppSvcHandle, APP_CONTROL_DATA_PATH, &pReceiveData->szMultiPath, &(pReceiveData->nMultiPath))) {
			pReceiveData->eLaunchType = VP_LAUNCH_TYPE_MULTI_PATH;
			__VpServiceParserGetMultiAppControl(pAppSvcHandle, VP_VIDEO_DATA_TITLE, &pReceiveData->szMultiTitle, &(pReceiveData->nMultiTitle));
			__VpServiceParserGetMultiAppControl(pAppSvcHandle, VP_VIDEO_DATA_SUBTITLE, &pReceiveData->szMultiSubTitle, &(pReceiveData->nMultiSubTitle));
			__VpServiceParserGetMultiAppControl(pAppSvcHandle, VP_VIDEO_DATA_POSITION, &pReceiveData->szMultiPosition, &(pReceiveData->nMultiPosition));
			__VpServiceParserGetMultiAppControl(pAppSvcHandle, VP_VIDEO_DATA_DURATION, &pReceiveData->szMultiDuration, &(pReceiveData->nMultiDuration));
		} else {
			preference_get_string(PREF_VP_PREVIEW_URL_VIDEOS, &(pReceiveData->pMediaUri));
			if (__VpServiceParserCheckFileIsExist(pReceiveData->pMediaUri) == EINA_FALSE) {
				preference_set_string(PREF_VP_PREVIEW_URL_VIDEOS, "");
				VideoSecureLogError(" == %s ==", pReceiveData->pMediaUri);
				if (pReceiveData->pMediaUri) {
					free(pReceiveData->pMediaUri);
					pReceiveData->pMediaUri = NULL;
				}
				preference_get_string(PREF_VP_PREVIEW_URL_RECORDS, &(pReceiveData->pMediaUri));
				if (__VpServiceParserCheckFileIsExist(pReceiveData->pMediaUri) == EINA_FALSE) {
					preference_set_string(PREF_VP_PREVIEW_URL_RECORDS, "");
					VideoSecureLogError(" == %s ==", pReceiveData->pMediaUri);
					if (pReceiveData->pMediaUri) {
						free(pReceiveData->pMediaUri);
						pReceiveData->pMediaUri = NULL;
					}
					vp_media_content_util_get_first_video(&pReceiveData->pMediaUri);
					VideoSecureLogError(" == %s ==", pReceiveData->pMediaUri);
					if (__VpServiceParserCheckFileIsExist(pReceiveData->pMediaUri) == EINA_FALSE) {
						pReceiveData->pMediaUri = strdup(VP_DEFAULT_VIDEO_ITEM_URL_STR);
						VideoSecureLogError(" == %s ==", pReceiveData->pMediaUri);
					}
				}
			}
		}
	}
	pReceiveData->nParentXID = 0;

	__VpServiceParserGetWindowID(pAppSvcHandle, &(pReceiveData->nParentXID));

	pReceiveData->bLaunchMultiWindow = __VpServiceParserGetLaunchMultiWindow(pAppSvcHandle);

	if (pReceiveData->pMediaUri != NULL) {
		pReceiveData->bStreaming	= __VpServiceParserGetStreamingLaunch(pReceiveData->pMediaUri);
	}

	if (pReceiveData->bStreaming) {
		pReceiveData->szDeviceID =  __VpServiceParserGetDeviceID(pAppSvcHandle);
		pReceiveData->szCookie = __VpServiceParserGetCookie(pAppSvcHandle);
		pReceiveData->szProxy = __VpServiceParserGetProxy(pAppSvcHandle);
	}

	pReceiveData->szDMRID = __VpServiceParserGetDMRID(pAppSvcHandle);

	if (pReceiveData->eLaunchType >= VP_LAUNCH_TYPE_GALLERY) {
		pReceiveData->eSortType	= __VpServiceParserGetSortType(pAppSvcHandle);
		pReceiveData->eListType	= __VpServiceParserGetListType(pAppSvcHandle);
	} else if (pReceiveData->eLaunchType == VP_LAUNCH_TYPE_LIST) {
		pReceiveData->eListType	= __VpServiceParserGetListType(pAppSvcHandle);
	} else {
		pReceiveData->eSortType	= VP_VIDEO_SORT_TYPE_BY_NONE;
		pReceiveData->eListType	= VP_VIDEO_PLAY_LIST_TYPE_NONE;
	}

	return EINA_TRUE;
}


void VpServiceParserDestroyServiceData(VpServiceData *pServiceData)
{
	if (pServiceData == NULL) {
		VideoLogError("pServiceData == NULL!!!");
		return;
	}

	if (pServiceData->pMediaUri != NULL) {
		free(pServiceData->pMediaUri);
		pServiceData->pMediaUri	= NULL;
	}

	if (pServiceData->szMediaTitle != NULL) {
		free(pServiceData->szMediaTitle);
		pServiceData->szMediaTitle = NULL;
	}

	if (pServiceData->szDeviceID != NULL) {
		free(pServiceData->szDeviceID);
		pServiceData->szDeviceID = NULL;
	}

	if (pServiceData->szDMRID != NULL) {
		free(pServiceData->szDMRID);
		pServiceData->szDMRID = NULL;
	}

	if (pServiceData->pStoreOrderId != NULL) {
		free(pServiceData->pStoreOrderId);
		pServiceData->pStoreOrderId = NULL;
	}

	if (pServiceData->pStoreServerId != NULL) {
		free(pServiceData->pStoreServerId);
		pServiceData->pStoreServerId = NULL;
	}

	if (pServiceData->pStoreAppId != NULL) {
		free(pServiceData->pStoreAppId);
		pServiceData->pStoreAppId = NULL;
	}

	if (pServiceData->pStoreMvId != NULL) {
		free(pServiceData->pStoreMvId);
		pServiceData->pStoreMvId = NULL;
	}

	int idx = 0;
	if (pServiceData->szMultiPath) {
		for (idx = 0; idx < pServiceData->nMultiPath; idx++) {
			if (pServiceData->szMultiPath[idx]) {
				free(pServiceData->szMultiPath[idx]);
				pServiceData->szMultiPath[idx] = NULL;
			}
		}
		free(pServiceData->szMultiPath);
		pServiceData->szMultiPath = NULL;
	}

	if (pServiceData->szIsSameAP) {
		for (idx = 0; idx < pServiceData->nSameAP; idx++) {
			if (pServiceData->szIsSameAP[idx]) {
				free(pServiceData->szIsSameAP[idx]);
				pServiceData->szIsSameAP[idx] = NULL;
			}
		}
		free(pServiceData->szIsSameAP);
		pServiceData->szIsSameAP = NULL;
	}

	if (pServiceData->szMultiSubTitle) {
		for (idx = 0; idx < pServiceData->nMultiSubTitle; idx++) {
			if (pServiceData->szMultiSubTitle[idx]) {
				free(pServiceData->szMultiSubTitle[idx]);
				pServiceData->szMultiSubTitle[idx] = NULL;
			}
		}
		free(pServiceData->szMultiSubTitle);
		pServiceData->szMultiSubTitle = NULL;
	}


	if (pServiceData->szMultiTitle) {
		for (idx = 0; idx < pServiceData->nMultiTitle; idx++) {
			if (pServiceData->szMultiTitle[idx]) {
				free(pServiceData->szMultiTitle[idx]);
				pServiceData->szMultiTitle[idx] = NULL;
			}
		}
		free(pServiceData->szMultiTitle);
		pServiceData->szMultiTitle = NULL;
	}

	if (pServiceData->szMultiPosition) {
		for (idx = 0; idx < pServiceData->nMultiPosition; idx++) {
			if (pServiceData->szMultiPosition[idx]) {
				free(pServiceData->szMultiPosition[idx]);
				pServiceData->szMultiPosition[idx] = NULL;
			}
		}
		free(pServiceData->szMultiPosition);
		pServiceData->szMultiPosition = NULL;
	}

	if (pServiceData->szMultiDuration) {
		for (idx = 0; idx < pServiceData->nMultiDuration; idx++) {
			if (pServiceData->szMultiDuration[idx]) {
				free(pServiceData->szMultiDuration[idx]);
				pServiceData->szMultiDuration[idx] = NULL;
			}
		}
		free(pServiceData->szMultiDuration);
		pServiceData->szMultiDuration = NULL;
	}

	free(pServiceData);
	pServiceData	= NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
static char *__VpServiceParserGetMediaUri(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return NULL;
	}

	char *pUri = NULL;

	if (__VpServiceParserGetAUL(pAppSvcHandle, &pUri) == EINA_FALSE) {
		if (__VpServiceParserGetAppControl(pAppSvcHandle, &pUri) == EINA_FALSE) {
			VideoLogInfo("No exist URI path.");
			return NULL;
		}
	}

	VideoSecureLogInfo("pUri : %s", pUri);

	if (pUri == NULL) {
		VideoLogError("pUri == NULL!!!");
		return NULL;
	}

	if (strstr(pUri, VP_VIDEO_FILE_PREFIX)) {
		char *szURL = calloc(1, strlen(pUri) - strlen(VP_VIDEO_FILE_PREFIX) + 1);
		if (szURL == NULL) {
			VideoLogError("szURL == NULL!!!");
			return NULL;
		}

		snprintf(szURL, strlen(pUri) - strlen(VP_VIDEO_FILE_PREFIX) + 1, "%s", pUri + strlen(VP_VIDEO_FILE_PREFIX));
		if (pUri) {
			free(pUri);
			pUri = NULL;
		}
		return szURL;
	} else {
		return pUri;
	}

}

static bool __VpServiceParserGetMultiAppControl(app_control_h pAppSvcHandle, const char *key, char ***szArrayVal, int *nLength)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return FALSE;
	}
	int nRet = APP_CONTROL_ERROR_NONE;
	int nLen = 0;

	nRet = app_control_get_extra_data_array(pAppSvcHandle, key, szArrayVal, &nLen);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_get_extra_data_array is fail [0x%x]", nRet);
		return FALSE;
	}

	if (nLen < 1) {
		if (*szArrayVal == NULL) {
			// No sub items,
			free(*szArrayVal);
			*szArrayVal = NULL;
		}

		return FALSE;
	}

	*nLength = nLen;
	VideoLogInfo("Length %d", nLen);

	return TRUE;
}


static VpLaunchType __VpServiceParserGetLaunchType(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return VP_LAUNCH_TYPE_NONE;
	}

	char* pLaunchingByOtherApp = NULL;
	char* pLaunchMode = NULL;
	VpLaunchType	eLaunchType = VP_LAUNCH_TYPE_NONE;

	if (app_control_get_extra_data(pAppSvcHandle, "View Mode", &pLaunchMode) == APP_CONTROL_ERROR_NONE) {
		if (pLaunchMode != NULL) {
			if (strncmp(pLaunchMode, "EMAIL", strlen(pLaunchMode)) == 0) {
				eLaunchType = VP_LAUNCH_TYPE_EMAIL;
				goto RESULT_RETURN;
			}
		}
	}

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_LAUNCHED_BY_APP_KEY, &pLaunchingByOtherApp) != APP_CONTROL_ERROR_NONE) {
		VideoLogWarning("No exist Service handle key of MP_VIDEO_LAUNCHED_BY_APP_KEY");
		eLaunchType	= VP_LAUNCH_TYPE_NONE;
		goto RESULT_RETURN;
	}

	if (pLaunchingByOtherApp == NULL) {
		VideoLogWarning("pLaunchingByOtherApp is NULL");
		goto RESULT_RETURN;
	}

	if (strcmp(pLaunchingByOtherApp, "gallery") == 0) {
		eLaunchType = VP_LAUNCH_TYPE_GALLERY;
	} else if (strcmp(pLaunchingByOtherApp, "myfile") == 0) {
		eLaunchType = VP_LAUNCH_TYPE_MYFILE;
	} else if (strcmp(pLaunchingByOtherApp, "store") == 0) {
		eLaunchType = VP_LAUNCH_TYPE_STORE;
	} else if (strcmp(pLaunchingByOtherApp, "list") == 0) {
		eLaunchType = VP_LAUNCH_TYPE_LIST;
	} else if (strcmp(pLaunchingByOtherApp, "image_viewer") == 0) {
		char	*pEditMode	= NULL;

		if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_EDIT_MODE_KEY, &pEditMode) != APP_CONTROL_ERROR_NONE) {
			eLaunchType	= VP_LAUNCH_TYPE_IMAGE_VIEWER;

			if (pEditMode != NULL) {
				free(pEditMode);
				pEditMode = NULL;
			}
		} else {
			if (pEditMode != NULL) {
				if (strcmp(pEditMode, "trim") == 0) {
					eLaunchType	= VP_LAUNCH_TYPE_IMAGE_VIEWER_TRIM;
				} else {
					eLaunchType	= VP_LAUNCH_TYPE_IMAGE_VIEWER;
				}

				free(pEditMode);
				pEditMode	= NULL;
			} else {
				eLaunchType	= VP_LAUNCH_TYPE_IMAGE_VIEWER;
			}
		}
	} else if (strcmp(pLaunchingByOtherApp, "email") == 0) {
		eLaunchType = VP_LAUNCH_TYPE_EMAIL;
	} else if (strcmp(pLaunchingByOtherApp, "message") == 0) {
		eLaunchType = VP_LAUNCH_TYPE_MMS;
	} else if (strcmp(pLaunchingByOtherApp, "light_play_view") == 0) {
		eLaunchType = VP_LAUNCH_TYPE_SIMPLE;
	} else {
		eLaunchType = VP_LAUNCH_TYPE_NONE;
	}


RESULT_RETURN:
	if (pLaunchingByOtherApp != NULL) {
		free(pLaunchingByOtherApp);
		pLaunchingByOtherApp = NULL;
	}
	VideoLogError("LaunchType = %d", eLaunchType);

	return eLaunchType;
}


static Eina_Bool __VpServiceParserGetLaunchMultiWindow(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}

	Eina_Bool	bMultiWindow 		= EINA_FALSE;
	char 		*pMultiWindowType 	= NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_MULTI_WIN_TYPE_KEY, &pMultiWindowType) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist multi window type token.");
		bMultiWindow	= EINA_FALSE;
	}

	if (pMultiWindowType != NULL) {
		if (strcmp(pMultiWindowType, VP_VIDEO_MULTI_WIN_ON_VALUE) == 0) {
			bMultiWindow	= EINA_TRUE;
		}

		free(pMultiWindowType);
		pMultiWindowType	= NULL;
	}

	return bMultiWindow;
}



static Eina_Bool __VpServiceParserGetStoreDownloadMode(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}

	Eina_Bool	bStoreDownload 		= EINA_FALSE;
	char 		*pStoreDownloadValue 	= NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_STORE_DOWNLOAD, &pStoreDownloadValue) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist store download type");
		bStoreDownload	= EINA_FALSE;
	}

	if (pStoreDownloadValue != NULL) {
		if (strcmp(pStoreDownloadValue, VP_VIDEO_STORE_DOWNLOAD_ON_VALUE) == 0) {
			bStoreDownload	= EINA_TRUE;
		}

		free(pStoreDownloadValue);
		pStoreDownloadValue	= NULL;
	}

	return bStoreDownload;
}

static double __VpServiceParserGetStoreDownloadFileSize(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}

	char 		*pStoreDownloadFileSize	= NULL;
	double		dFileSize = 0;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_STORE_DOWNLOAD_FILE_SIZE, &pStoreDownloadFileSize) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist store download file size");
	}

	if (pStoreDownloadFileSize != NULL) {
		dFileSize = atof(pStoreDownloadFileSize);
		VideoLogInfo("dFileSize : %f", dFileSize);
		free(pStoreDownloadFileSize);
		pStoreDownloadFileSize	= NULL;
	}

	return dFileSize;
}

static int __VpServiceParserGetStoreDownloadPlayedTime(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return 0;
	}

	int nPlayedTime = 0;
	char *pPlayedTime = NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_STORE_DOWNLOAD_PLAYED_TIME, &pPlayedTime) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist multi window type token.");
	}

	if (pPlayedTime != NULL) {
		nPlayedTime = atoi(pPlayedTime);
		free(pPlayedTime);
		pPlayedTime	= NULL;
	}

	return nPlayedTime;
}


static Eina_Bool __VpServiceParserGetRepeatMode(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}

	Eina_Bool	bRepeatMode 		= EINA_FALSE;
	char 		*pRepeatModeValue 	= NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_APP_CONTROL_REPEAT_MODE, &pRepeatModeValue) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist repeat mode value");
		bRepeatMode	= EINA_FALSE;
	}

	if (pRepeatModeValue != NULL) {
		if (strcmp(pRepeatModeValue, VP_VIDEO_APP_CONTROL_REPEAT_VALUE_STOP_AFTER_ALL) == 0) {
			bRepeatMode	= EINA_TRUE;
		}

		free(pRepeatModeValue);
		pRepeatModeValue	= NULL;
	}

	return bRepeatMode;
}


static void __VpServiceParserGetStoreOrderInfo(app_control_h pAppSvcHandle, VpServiceData *pReceiveData)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return;
	}

	if (pReceiveData == NULL) {
		VideoLogError("pReceiveData == NULL!!!");
		return;
	}

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_STORE_ORDER_ID, &pReceiveData->pStoreOrderId) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist VP_VIDEO_STORE_ORDER_ID.");
		goto NOT_EXISTED_INFO;
	}

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_STORE_APP_ID, &pReceiveData->pStoreAppId) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist VP_VIDEO_STORE_APP_ID.");
		goto NOT_EXISTED_INFO;
	}

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_STORE_MV_ID, &pReceiveData->pStoreMvId) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist store_mv_id.");
		goto NOT_EXISTED_INFO;
	}

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_STORE_SERVER_ID, &pReceiveData->pStoreServerId) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist VP_VIDEO_STORE_SERVER_ID.");
		goto NOT_EXISTED_INFO;
	}

	return;

NOT_EXISTED_INFO:
	if (pReceiveData->pStoreOrderId != NULL) {
		free(pReceiveData->pStoreOrderId);
		pReceiveData->pStoreOrderId	= NULL;
	}

	if (pReceiveData->pStoreAppId != NULL) {
		free(pReceiveData->pStoreAppId);
		pReceiveData->pStoreAppId	= NULL;
	}

	if (pReceiveData->pStoreMvId != NULL) {
		free(pReceiveData->pStoreMvId);
		pReceiveData->pStoreMvId	= NULL;
	}

	if (pReceiveData->pStoreServerId != NULL) {
		free(pReceiveData->pStoreServerId);
		pReceiveData->pStoreServerId	= NULL;
	}

	return;
}


static Eina_Bool __VpServiceParserGetStreamingLaunch(char *pUri)
{
	if (pUri == NULL) {
		VideoLogInfo("ERR:  pUri == NULL!!!");
		return EINA_FALSE;
	}

	VideoSecureLogInfo("pUri : %s", pUri);

	if (strstr(pUri, "rtp") != NULL) {
		return EINA_TRUE;
	} else if (strstr(pUri, "rtsp") != NULL) {
		return EINA_TRUE;
	} else if (strstr(pUri, "http") != NULL) {
		return EINA_TRUE;
	} else if (strstr(pUri, "https") != NULL) {
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

static char *__VpServiceParserGetDeviceID(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return NULL;
	}

	char *szDeviceID = NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_DEVICE_ID_KEY, &szDeviceID) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist buntle type.");

		if (szDeviceID != NULL) {
			free(szDeviceID);
			szDeviceID	= NULL;
		}
		return NULL;
	}

	return szDeviceID;

}


static char *__VpServiceParserGetDMRID(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return NULL;
	}

	char *szDMRID = NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_DMR_ID_KEY, &szDMRID) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist buntle type.");

		if (szDMRID != NULL) {
			free(szDMRID);
			szDMRID	= NULL;
		}
		return NULL;
	}
	VideoLogError("#### %s ####", szDMRID);

	return szDMRID;

}


static char *__VpServiceParserGetCookie(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return NULL;
	}

	char *szCookie = NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_COOKIE_KEY, &szCookie) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist buntle type.");

		if (szCookie != NULL) {
			free(szCookie);
			szCookie	= NULL;
		}
		return NULL;
	}
	VideoLogError("== [Cookie : %s] ==", szCookie);

	return szCookie;

}

static char *__VpServiceParserGetProxy(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return NULL;
	}

	char *szProxy = NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_PROXY_KEY, &szProxy) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist buntle type.");

		if (szProxy != NULL) {
			free(szProxy);
			szProxy	= NULL;
		}
		return NULL;
	}
	VideoLogError("== [Proxy : %s] ==", szProxy);

	return szProxy;

}

static char *__VpServiceParserGetMediaTitle(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return NULL;
	}

	char *szMediaTitle = NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_TITLE_KEY, &szMediaTitle) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist app_control_h type.");

		if (szMediaTitle != NULL) {
			free(szMediaTitle);
			szMediaTitle	= NULL;
		}
		return NULL;
	}
	return szMediaTitle;

}

static Eina_Bool __VpServiceParserGetAUL(app_control_h pAppSvcHandle, char **dpUri)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}

	if (dpUri == NULL) {
		VideoLogError("dpUri == NULL!!!");
		return EINA_FALSE;
	}

	if (app_control_get_uri(pAppSvcHandle, dpUri) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist uri");

		if (*dpUri != NULL) {
			free(*dpUri);
			*dpUri	= NULL;
		}

		return EINA_FALSE;
	}

	if (*dpUri != NULL) {
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

static Eina_Bool __VpServiceParserGetAppControl(app_control_h pAppSvcHandle, char **dpUri)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}

	if (dpUri == NULL) {
		VideoLogError("dpUri == NULL!!!");
		return EINA_FALSE;
	}

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_URI_PATH_KEY, dpUri) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist buntle type.");

		if (*dpUri != NULL) {
			free(*dpUri);
			*dpUri	= NULL;
		}

		return EINA_FALSE;
	}

	if (*dpUri != NULL) {
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

static Eina_Bool __VpServiceParserGetWindowID(app_control_h pAppSvcHandle, unsigned int *nXID)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return EINA_FALSE;
	}
#if 0
	char *szValue = NULL;
	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_EXIT_ON_BACK, &szValue) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist buntle type.");

		if (szValue != NULL) {
			free(szValue);
			szValue	= NULL;
		}

		return EINA_FALSE;
	}

	if (!strcmp(szValue, "true")) {
		int nRet = APP_CONTROL_ERROR_NONE;

		nRet = app_control_get_window(pAppSvcHandle, nXID);
		if (nRet != APP_CONTROL_ERROR_NONE) {
			VideoLogWarning("app_control_get_window fail: %d", nRet);
			*nXID = 0;
			return FALSE;
		}

	}
#endif
	return EINA_TRUE;
}

static VpVideoSortType __VpServiceParserGetSortType(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return VP_VIDEO_SORT_TYPE_BY_NONE;
	}

	VpVideoSortType	eSortType = VP_VIDEO_SORT_TYPE_BY_NONE;
	char 		*pSortType = NULL;


	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_SORT_TYPE_KEY, &pSortType) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist gallery list sort order type!!!");
		eSortType	= VP_VIDEO_SORT_TYPE_BY_NONE;
		goto RESULT_RETURN;
	}

	if (pSortType == NULL) {
		VideoLogInfo("pSortType == NULL!!!");
		eSortType	= VP_VIDEO_SORT_TYPE_BY_NONE;
		goto RESULT_RETURN;
	}

	if (!strcmp(pSortType, "name_asc")) {
		eSortType = VP_VIDEO_SORT_TYPE_BY_NAME_ASC;
	} else if (!strcmp(pSortType, "name_desc")) {
		eSortType = VP_VIDEO_SORT_TYPE_BY_NAME_DESC;
	} else if (!strcmp(pSortType, "date_asc")) {
		eSortType = VP_VIDEO_SORT_TYPE_BY_DATE_ASC;
	} else if (!strcmp(pSortType, "date_desc")) {
		eSortType = VP_VIDEO_SORT_TYPE_BY_DATE_DESC;
	} else if (!strcmp(pSortType, "none")) {
		eSortType = VP_VIDEO_SORT_TYPE_BY_NONE;
	} else {
		eSortType = VP_VIDEO_SORT_TYPE_BY_NAME_DESC;
	}


RESULT_RETURN:
	if (pSortType != NULL) {
		free(pSortType);
		pSortType	= NULL;
	}

	return eSortType;
}


static VpVideoPlayListType __VpServiceParserGetListType(app_control_h pAppSvcHandle)
{
	if (pAppSvcHandle == NULL) {
		VideoLogError("pAppSvcHandle == NULL!!!");
		return VP_VIDEO_PLAY_LIST_TYPE_NONE;
	}

	VpVideoPlayListType ePlayType = VP_VIDEO_PLAY_LIST_TYPE_NONE;
	char *pPlayType = NULL;

	if (app_control_get_extra_data(pAppSvcHandle, VP_VIDEO_LIST_TYPE_KEY, &pPlayType) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist gallery play list type!!!");
		ePlayType	= VP_VIDEO_PLAY_LIST_TYPE_NONE;
		goto RESULT_RETURN;
	}

	if (pPlayType == NULL) {
		VideoLogInfo("pPlayType == NULL!!!");
		pPlayType	= VP_VIDEO_PLAY_LIST_TYPE_NONE;
		goto RESULT_RETURN;
	}


	if (!strcmp(pPlayType, "folder")) {
		ePlayType = VP_VIDEO_PLAY_LIST_TYPE_FOLDER;
	} else if (!strcmp(pPlayType, "all_folder_video")) {
		ePlayType = VP_VIDEO_PLAY_LIST_TYPE_ALL_FOLDER_GALLERY;
	} else if (!strcmp(pPlayType, "tag_video")) {
		ePlayType = VP_VIDEO_PLAY_LIST_TYPE_TAG_GALLERY;
	} else if (!strcmp(pPlayType, "favorite"))	{
		ePlayType = VP_VIDEO_PLAY_LIST_TYPE_FAVORITE;
	} else {
		ePlayType = VP_VIDEO_PLAY_LIST_TYPE_NONE;
	}


RESULT_RETURN:
	if (pPlayType != NULL) {
		free(pPlayType);
		pPlayType	= NULL;
	}

	return ePlayType;
}


static Eina_Bool __VpServiceParserCheckURL(char *szURL)
{
	if (szURL == NULL) {
		return EINA_FALSE;
	}

	if (strlen(szURL) <= 0) {
		return EINA_FALSE;
	}

	return EINA_TRUE;
}



static Eina_Bool __VpServiceParserCheckFileIsExist(char *szFilePath)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath == NULL!!!");
		return EINA_FALSE;
	}
	Eina_Bool bIsExist = EINA_FALSE;

	if (vp_file_exists(szFilePath)) {
		bIsExist = EINA_TRUE;
	}

	return bIsExist;
}




