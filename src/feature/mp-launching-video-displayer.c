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
#include <unistd.h>
#include <stdbool.h>
#include <Ecore_Evas.h>
#include <Elementary.h>
#include <Eina.h>

#include "vp-play-log.h"
#include "mp-util.h"
#include "mp-video-log.h"
#include "vp-play-view.h"
#include "mp-video-value-define.h"
#include "mp-launching-video-displayer.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-util-config.h"

/*"g_pLaunchTimer" use for check launch time between twice click. 
 If click two videos between 1 sec. Only play the first video.*/
/* it will be auto free after 1 sec. it will auto free alse after 
 video process is killed.*/
/* if this timer is not free. not create it again.*/
static Ecore_Timer *g_pLaunchTimer = NULL;

static void __mp_launch_video_reply_cb(app_control_h pRequest,
				       app_control_h pReply,
				       app_control_result_e nResult,
				       void *pUserData)
{

	if (pRequest) {
		char *appId;
		app_control_get_app_id(pRequest, &appId);
		VideoLogWarning("Request : %s", appId);
		if (appId) {
			free(appId);
			appId = NULL;
		}
	}

	if (pReply) {
		char *appId;
		app_control_get_app_id(pReply, &appId);
		VideoLogWarning("Reply : %s", appId);
		if (appId) {
			free(appId);
			appId = NULL;
		}
	}
}

static Eina_Bool __mp_launch_video_timer_cb(void *pUserData)
{
	g_pLaunchTimer = NULL;

	return FALSE;
}

/*////////////////////////////////////////////////////////////////////////*/
/* External APIs*/
/*used together with mp_launch_video_play. if first to check, then to play, 
use below API*/
bool mp_launch_video_allow_check(void)
{
	if (g_pLaunchTimer) {
		bool bMultiPlay = FALSE;
		mp_util_config_get_multi_play_status(&bMultiPlay);
		if (bMultiPlay) {
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

void mp_launch_video_play(char *szMediaUrl, MpPlayerType nPlayerType,
			  char *szDeviceID)
{

	if (!szMediaUrl) {
		VideoLogError("[ERR] No existed szMediaUrl.");
		return;
	}
	bool bMultiPlay = FALSE;
	mp_util_config_get_multi_play_status(&bMultiPlay);

	if (!bMultiPlay) {
		if (!g_pLaunchTimer) {
			g_pLaunchTimer =
				ecore_timer_add(1.0, __mp_launch_video_timer_cb,
						 NULL);
		} else {
			VideoLogWarning("need more than 1.0 sec between twice click");
			return;
		}
	}

	app_control_h pService = NULL;

	VideoSecureLogInfo("start play:%s,%d,%s,%d", szMediaUrl, nPlayerType,
			   szDeviceID, bMultiPlay);

	int nRet = 0;
	nRet = app_control_create(&pService);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_create is fail [0x%x]", nRet);
	}

	nRet =
		app_control_set_operation(pService, APP_CONTROL_OPERATION_VIEW);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_set_operation is fail [0x%x]", nRet);
	}

	nRet = app_control_set_uri(pService, szMediaUrl);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_set_uri is fail [0x%x]", nRet);
	}

	if (nPlayerType == MP_PLAYER_TYPE_VIDEO) {
		nRet =
			app_control_add_extra_data(pService, "launching_application",
						   "list");
		if (nRet != APP_CONTROL_ERROR_NONE) {
			VideoLogError("app_control_add_extra_data is fail [0x%x]",
				      nRet);
		}
	}

	if (szDeviceID) {
		VideoLogWarning("== Device ID : %s ==", szDeviceID);
		nRet =
			app_control_add_extra_data(pService, "Device id", szDeviceID);
		if (nRet != APP_CONTROL_ERROR_NONE) {
			VideoLogError("app_control_add_extra_data is fail [0x%x]",
				      nRet);
		}
	}
#ifdef VS_FEATURE_FOLDER_VIEW	/**/
	int nViewType = mp_view_as_ctrl_get_type();

	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
		VideoLogInfo("folder view");
		nRet =
			app_control_add_extra_data(pService, "video_list_type",
						   "folder");
		if (nRet != APP_CONTROL_ERROR_NONE) {
			VideoLogError("app_control_add_extra_data is fail [0x%x]",
				      nRet);
		}
	}
#endif				/**/
	nRet = app_control_add_extra_data(pService, "path", szMediaUrl);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_add_extra_data is fail [0x%x]", nRet);
	}

	nRet = app_control_set_app_id(pService, "org.tizen.video-player");
	if (nRet != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_set_app_id is fail [0x%x]", nRet);
	}

	nRet =
		app_control_send_launch_request(pService,
						__mp_launch_video_reply_cb, NULL);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		if (nRet == APP_CONTROL_ERROR_INVALID_PARAMETER) {
			VideoLogError
			("app_control_send_launch_request is fail APP_CONTROL_ERROR_INVALID_PARAMETER [0x%x]",
			 nRet);
		} else if (nRet == APP_CONTROL_ERROR_OUT_OF_MEMORY) {
			VideoLogError
			("app_control_send_launch_request is fail APP_CONTROL_ERROR_OUT_OF_MEMORY [0x%x]",
			 nRet);
		} else if (nRet == APP_CONTROL_ERROR_APP_NOT_FOUND) {
			VideoLogError
			("app_control_send_launch_request is fail APP_CONTROL_ERROR_APP_NOT_FOUND [0x%x]",
			 nRet);
		} else if (nRet == APP_CONTROL_ERROR_KEY_NOT_FOUND) {
			VideoLogError
			("app_control_send_launch_request is fail APP_CONTROL_ERROR_KEY_NOT_FOUND [0x%x]",
			 nRet);
		} else if (nRet == APP_CONTROL_ERROR_KEY_REJECTED) {
			VideoLogError
			("app_control_send_launch_request is fail APP_CONTROL_ERROR_KEY_REJECTED [0x%x]",
			 nRet);
		} else if (nRet == APP_CONTROL_ERROR_INVALID_DATA_TYPE) {
			VideoLogError
			("app_control_send_launch_request is fail APP_CONTROL_ERROR_INVALID_DATA_TYPE [0x%x]",
			 nRet);
		} else if (nRet == APP_CONTROL_ERROR_LAUNCH_REJECTED) {
			VideoLogError
			("app_control_send_launch_request is fail APP_CONTROL_ERROR_LAUNCH_REJECTED [0x%x]",
			 nRet);
		}
	}

	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}

}

void mp_launch_video_stop(void)
{
	VideoLogInfo("");
	/*if needed, to do*/
}
