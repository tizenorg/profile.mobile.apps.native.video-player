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


#include <app_preference.h>
#include "mp-util.h"
#ifdef ENABLE_DRM_FEATURE
#include "mp-drm-ctrl.h"
#endif
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-util-config.h"

#define PREF_MP_MULTI_PLAY_FLAG  "preference/org.tizen.videos/multi_play"
#define MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS "preference/org.tizen.videos/preview_url_videos"


bool mp_util_config_set_multi_play_status(bool bMultiPlay)
{
	int nErr = 0;

	nErr = preference_set_boolean(PREF_MP_MULTI_PLAY_FLAG, bMultiPlay);
	if (nErr != 0) {
		VideoLogError
		("SET PREF_MP_MULTI_PLAY_FLAG is fail [0x%x]",
		 nErr);
		return FALSE;
	}

	return TRUE;
}

bool mp_util_config_get_multi_play_status(bool *bMultiPlay)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(PREF_MP_MULTI_PLAY_FLAG, &nVal);
	if (nErr != 0) {
		VideoLogError
		("GET PREF_MP_MULTI_PLAY_FLAG is fail [0x%x]",
		 nErr);
		return FALSE;
	}

	*bMultiPlay = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool mp_util_config_set_preview(char *szPreviewUrl)
{
	VideoSecureLogInfo("szPreviewUrl : %s", szPreviewUrl);

	bool bRet = FALSE;

	if (szPreviewUrl) {
		if (preference_set_string
				(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS,
				szPreviewUrl)) {
			VideoLogError("Fail to write szPreviewUrl.");
			bRet = FALSE;
		} else {
			bRet = TRUE;
		}
	} else {
		if (preference_set_string(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS, "")) {
			VideoLogError("Fail to write szPreviewUrl.");
			bRet = FALSE;
		} else {
			bRet = TRUE;
		}
	}

	return bRet;
}

char *mp_util_config_get_preview(void)
{
	char *pTmpVideoUrl = NULL;
	int nErr = 0;

	nErr = preference_get_string(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS, &pTmpVideoUrl);
	if (nErr != 0 || !pTmpVideoUrl) {
		VideoLogError("fail to retrive url [0x%x]",nErr);
		return NULL;
	}

	if (strlen(pTmpVideoUrl) < 1) {
		free(pTmpVideoUrl);
		pTmpVideoUrl = NULL;
	}

	return pTmpVideoUrl;
}

bool mp_util_config_preview_key_changed_cb(preference_changed_cb callback,
		void *pUserData)
{
	int nErr = 0;
	nErr = preference_set_changed_cb(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS,
					callback, pUserData);
	if (nErr != 0) {
		VideoLogError("failed to register callback [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool mp_util_config_ignore_preview_key_changed_cb()
{
	int nErr = 0;
	nErr = preference_unset_changed_cb(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS);
	if (nErr != 0) {
		VideoLogError("failed to unregister [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

