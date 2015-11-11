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

#include "vp-play-macro-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"

#include "vp-play-util.h"

#include "vp-play-ug.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-play-normal-view.h"


#define MESSAGE_UG_NAME 		"msg-composer-efl"
#define EMAIL_UG_NAME 			"email-composer-efl"
#define BLUETOOTH_UG_NAME 		"setting-bluetooth-efl"
#define FTM_UG_NAME			"fileshare-efl"
#define VIDEOS_APP_NAME			"org.tizen.videos"
#define BROWSER_APP_NAME 		"org.tizen.browser"
#define VIDEO_STORE_DOWNLOAD_SVC	"org.tizen.video-store-download-svc"


#define	VP_VIDEOS_VALUE_TRUE		"true"
#define	VP_VIDEOS_VALUE_FALSE		"false"

#define	VP_VIDEOS_ONLY_RAISE		"raise"

/* callback functions */
static void __vp_play_ug_reply_cb(app_control_h pRequest,
                                  app_control_h pReply,
                                  app_control_result_e nResult,
                                  void *pUserData)
{
	if (pRequest) {
		char *appId;
		app_control_get_app_id(pRequest, &appId);
		VideoLogWarning("Request : %s", appId);
		VP_FREE(appId);
	}

	if (pReply) {
		char *appId;
		app_control_get_app_id(pReply, &appId);
		VideoLogWarning("Reply : %s", appId);
		VP_FREE(appId);
	}
	vp_play_normal_view_set_share_panel_state(pUserData);
}

bool vp_play_app_launch_videos(Evas_Object *pWin, bool bRaise,
                               void *pUserData)
{
	app_control_h pService = NULL;

	int nRet = 0;
	nRet = app_control_create(&pService);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_create is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet =
	    app_control_set_operation(pService, APP_CONTROL_OPERATION_VIEW);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_set_operation is fail [0x%x]", nRet);
		goto Execption;
	}
	if (bRaise) {
		nRet =
		    app_control_add_extra_data(pService, VP_VIDEOS_ONLY_RAISE,
		                               VP_VIDEOS_VALUE_TRUE);
		if (nRet != APP_CONTROL_ERROR_NONE) {
			VideoLogError("app_control_add_extra_data is fail [0x%x]",
			              nRet);
			goto Execption;
		}
	} else {
		nRet =
		    app_control_add_extra_data(pService, VP_VIDEOS_ONLY_RAISE,
		                               VP_VIDEOS_VALUE_FALSE);
		if (nRet != APP_CONTROL_ERROR_NONE) {
			VideoLogError("app_control_add_extra_data is fail [0x%x]",
			              nRet);
			goto Execption;
		}
	}

	nRet = app_control_set_app_id(pService, VIDEOS_APP_NAME);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_set_app_id is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet =
	    app_control_send_launch_request(pService, __vp_play_ug_reply_cb,
	                                    pUserData);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_send_launch_request is fail [0x%x]",
		              nRet);
		goto Execption;
	}

	app_control_destroy(pService);
	pService = NULL;

	return TRUE;

Execption:
	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}
	return FALSE;
}

bool vp_play_app_launch_share_panel(Evas_Object *pWin,
                                    const char *szMediaURL,
                                    void *pUserData)
{
	if (szMediaURL == NULL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	app_control_h pService = NULL;
	char *szFileName = NULL;

	int nRet = 0;
	nRet = app_control_create(&pService);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_create is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_set_operation(pService, VP_SHARE_OPERATION_SINGLE);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_add_extra_data is fail [0x%x]", nRet);
		goto Execption;
	}
	nRet = app_control_add_extra_data(pService, "http://tizen.org/appcontrol/data/path", szMediaURL);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_add_extra_data is fail [0x%x]", nRet);
		goto Execption;
	}
	if (!strstr(szMediaURL, VP_PLAY_FILE_PREFIX)) {
		szFileName = g_strdup_printf("file://%s", szMediaURL);
	} else {
		szFileName = g_strdup_printf("%s", szMediaURL);
	}

	VideoSecureLogInfo("file : %s", szFileName);

	nRet = app_control_set_uri(pService, szFileName);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_add_extra_data is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet =
	    app_control_send_launch_request(pService, __vp_play_ug_reply_cb,
	                                    pUserData);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_send_launch_request is fail [0x%x]",
		              nRet);
		goto Execption;
	}
	VP_FREE(szFileName);

	app_control_destroy(pService);
	pService = NULL;

	return TRUE;

Execption:
	VP_FREE(szFileName);

	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}
	return FALSE;
}

