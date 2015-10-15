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

#include "mp-util-preference.h"
#include "vp-play-log.h"
#include "mp-video-value-define.h"

/* keys */
#define MP_UTIL_PREFERENCE_KEY_MULTI_PLAY_FLAG	"preference/org.tizen.videos/multi_play"
#define MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS "preference/org.tizen.videos/preview_url_videos"

/* storage */
bool mp_util_preference_set_multi_play_status(bool set)
{
	int nErr = 0;
	nErr = preference_set_boolean(MP_UTIL_PREFERENCE_KEY_MULTI_PLAY_FLAG,
				       set);
	if (nErr != 0) {
		VideoLogError
		("SET MP_UTIL_PREFERENCE_KEY_MULTI_PLAY_FLAG is fail [0x%x]",
		 nErr);
		return FALSE;
	}
	return TRUE;
}

bool mp_util_preference_get_multi_play_status(bool *bMultiPlay)
{
	int nErr = 0;
	bool nVal = false;

	nErr = preference_get_boolean(MP_UTIL_PREFERENCE_KEY_MULTI_PLAY_FLAG,
				       &nVal);
	if (nErr != 0) {
		VideoLogError
		("GET MP_UTIL_PREFERENCE_KEY_MULTI_PLAY_FLAG is fail [0x%x]",
		 nErr);
		return FALSE;
	}

	*bMultiPlay = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

char *mp_util_preference_get_preview(void)
{
	char *szPreview = NULL;
	int nErr = 0;

	nErr = preference_get_string(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS,
				      &szPreview);
	if (nErr != 0) {
		VideoLogError("fail to retrieve preview url [0x%x]", nErr);
	}
	VideoSecureLogInfo("szPreviewUrl : %s", szPreview);

	if (szPreview) {
		if (strlen(szPreview) < 1) {
			free(szPreview);
			szPreview = NULL;
		}
	}
	return szPreview;
}

bool mp_util_preference_set_preview(char *szPreview)
{
	VideoSecureLogInfo("szPreviewUrl : %s", szPreview);

	bool bRet = FALSE;

	if (szPreview) {
		if (preference_set_string
				(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS, 
				szPreview)) {
			VideoLogError("Fail to write url.");
			bRet = FALSE;
		} else {
			bRet = TRUE;
		}
	} else {
		if (preference_set_string
				(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS, 
				"")) {
			VideoLogError("Fail to write url.");
			bRet = FALSE;
		} else {
			bRet = TRUE;
		}
	}

	return bRet;
}

bool mp_util_preference_set_preview_change(preference_changed_cb callback,
		void *pUserData)
{

	bool bRet = FALSE;
	int nErr = 0;

	nErr = preference_set_changed_cb
		(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS, callback,
		pUserData);
	if (nErr != 0) {
		VideoLogError("Failed to register callback [0x%x]", nErr);
	} else {
		bRet = TRUE;
	}

	return bRet;
}

bool mp_util_preference_unset_preview_change()
{
	bool bRet = FALSE;
	int nErr = 0;

	nErr = preference_unset_changed_cb
		(MP_UTIL_PREFERENCE_KEY_PREVIEW_URL_VIDEOS);
	if (nErr != 0) {
		VideoLogError("failed to unregister [0x%x]", nErr);
	} else {
		bRet = TRUE;
	}

	return bRet;
}

void mp_util_preference_init()
{
	/*if add preference key. it is necessary to init new preference here.*/
}
