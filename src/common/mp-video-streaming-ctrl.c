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
#include <string.h>
#include <net_connection.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "video-player.h"
#include "mp-video-string-define.h"
#include "mp-video-streaming-ctrl.h"


#define NETWORK_TIMEOUT		30.0

static MpNetworkType nNetworkStatus = MP_NETWORK_TYPE_OFF;

static Ecore_Pipe *pEcorePipeHandleForPause = NULL;
static Ecore_Pipe *pEcorePipeHandleForResume = NULL;

static Eina_Bool bIsInitCallbackFunc = FALSE;

static Ecore_Timer *pNetworkTimeoutTimer = NULL;

static connection_h gConnection = NULL;

/*
 * Callback function.
 */

static void mp_streaming_ctrl_exit_cb(void *pUserData,
				      Evas_Object *pObject,
				      void *pEventInfo)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("No exist pUserData");
		return;
	}

	VideoAppData *pAppData = (VideoAppData *) pUserData;

	pAppData->VideoAppCtrlCallbackFunc.VideoAppCtrlExit(pAppData);
}

static int mp_streaming_ctrl_resume_cb(void *pUserData)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("No exist pUserData");
		return 0;
	}

	VideoAppData *pAppData = (VideoAppData *) pUserData;

	if (pNetworkTimeoutTimer) {
		ecore_timer_del(pNetworkTimeoutTimer);
		pNetworkTimeoutTimer = NULL;
	}

	pAppData->VideoAppCtrlCallbackFunc.
	VideoAppCtrlStopByAppPause(pAppData);
	pAppData->VideoAppCtrlCallbackFunc.
	VideoAppCtrlStartByAppResume(pAppData);

	return 0;
}

static Eina_Bool mp_streaming_ctrl_popup_cb(void *pUserData)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("No exist pUserData");
		pNetworkTimeoutTimer = NULL;
		return EINA_FALSE;
	}

	VideoAppData *pAppData = (VideoAppData *) pUserData;

	mp_util_notify_popup(pAppData, VIDEOS_COMMON_HEADER_ERROR,
			     VIDEOS_NOTIPOP_MSG_CONNECTION_FAIL,
			     mp_streaming_ctrl_exit_cb);

	pNetworkTimeoutTimer = NULL;
	return EINA_FALSE;
}

static int mp_streaming_ctrl_pause_cb(void *pUserData)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("No exist pUserData");
		return 0;
	}

	VideoAppData *pAppData = (VideoAppData *) pUserData;

	pAppData->VideoAppCtrlCallbackFunc.
	VideoAppCtrlStopByAppPause(pAppData);

	if (pNetworkTimeoutTimer) {
		ecore_timer_del(pNetworkTimeoutTimer);
		pNetworkTimeoutTimer = NULL;
	}

	pNetworkTimeoutTimer = ecore_timer_add(NETWORK_TIMEOUT,
					       mp_streaming_ctrl_popup_cb,
					       pAppData);

	return 0;
}

static void mp_streaming_ctrl_network_status_cb(connection_type_e type,  void *pUserData)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("User data is NULL.");
		return;
	}

	switch (type) {
	case CONNECTION_TYPE_DISCONNECTED:
		nNetworkStatus = MP_NETWORK_TYPE_OFF;
		VideoLogInfo("MP_NETWORK_TYPE_OFF");
		/*
		   if (pEcorePipeHandleForPause) {
		   if (!ecore_pipe_write(pEcorePipeHandleForPause,
		   pUserData, sizeof(VideoAppData))) {
		   VideoLogDebug("FAIL PIPE WRITE");
		   }
		   }
		 */
		break;

	case CONNECTION_TYPE_CELLULAR:
		VideoLogInfo("MP_NETWORK_TYPE_CELLULAR");
		/*
		   if (pEcorePipeHandleForResume) {
		   if (!ecore_pipe_write(pEcorePipeHandleForResume,
		   pUserData, sizeof(VideoAppData))) {
		   VideoLogDebug("FAIL PIPE WRITE");
		   }
		   }
		   nNetworkStatus = MP_NETWORK_TYPE_CELLULAR;
		 */
		break;

	case CONNECTION_TYPE_WIFI:
		VideoLogInfo("MP_NETWORK_TYPE_WIFI");
		/*
		   if (pEcorePipeHandleForResume) {
		   if (!ecore_pipe_write(pEcorePipeHandleForResume,
		   pUserData, sizeof(VideoAppData))) {
		   VideoLogDebug("FAIL PIPE WRITE");
		   }
		   }
		   nNetworkStatus = MP_NETWORK_TYPE_WIFI;
		 */
		break;

	default:
		nNetworkStatus = MP_NETWORK_TYPE_OFF;
		VideoLogInfo("UNKNOWN NETWORK STATE");
		if (pEcorePipeHandleForPause) {
			if (!ecore_pipe_write(pEcorePipeHandleForPause,
					      pUserData, 
					      sizeof(VideoAppData))) {
				VideoLogDebug("FAIL PIPE WRITE");
			}
		}
		break;
	}
}

static void mp_streaming_ctrl_pipe_init(void *pUserData)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("No exist pUserData");
		return;
	}

	VideoAppData *pAppData = (VideoAppData *) pUserData;

	if (pEcorePipeHandleForPause) {
		ecore_pipe_del(pEcorePipeHandleForPause);
		pEcorePipeHandleForPause = NULL;
	}

	pEcorePipeHandleForPause = ecore_pipe_add((Ecore_Pipe_Cb)
				   mp_streaming_ctrl_pause_cb,
				   (void *) pAppData);

	if (pEcorePipeHandleForResume) {
		ecore_pipe_del(pEcorePipeHandleForResume);
		pEcorePipeHandleForResume = NULL;
	}

	pEcorePipeHandleForResume = ecore_pipe_add((Ecore_Pipe_Cb)
				    mp_streaming_ctrl_resume_cb,
				    (void *) pAppData);

}

void mp_streaming_ctrl_destroy(void *pUserData)
{
	VideoLogInfo("");

	if (gConnection) {
		if (connection_unset_type_changed_cb(gConnection) != CONNECTION_ERROR_NONE) {
			VideoLogError("failed to unregister network status callback");
		}

		if (connection_destroy(gConnection) != CONNECTION_ERROR_NONE) {
			VideoLogError("failed to destroy connection handle");
		}
		gConnection = NULL;
	}

	if (pNetworkTimeoutTimer) {
		ecore_timer_del(pNetworkTimeoutTimer);
		pNetworkTimeoutTimer = NULL;
	}

	if (pEcorePipeHandleForPause) {
		ecore_pipe_del(pEcorePipeHandleForPause);
		pEcorePipeHandleForPause = NULL;
	}

	if (pEcorePipeHandleForResume) {
		ecore_pipe_del(pEcorePipeHandleForResume);
		pEcorePipeHandleForResume = NULL;
	}
}

bool mp_streaming_ctrl_init(void *pUserData)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("[ERR] No exist pUserData.");
		return FALSE;
	}

	VideoAppData *pAppData = (VideoAppData *)pUserData;
	char szTmp[STR_LEN_MAX] = {0,};
	bool bNetworkState = FALSE;
	int ret = 0;
	connection_type_e con_type;

	if (bIsInitCallbackFunc == EINA_FALSE) {
		mp_streaming_ctrl_pipe_init(pUserData);

		ret = connection_create(&gConnection);
		if (ret != CONNECTION_ERROR_NONE) {
			VideoLogInfo("Fail to create connection handle [%d]\n", ret);
	 	}

		if (gConnection) {
			ret = connection_set_type_changed_cb(gConnection,
					mp_streaming_ctrl_network_status_cb,
					(void *)pUserData);
			if (ret != CONNECTION_ERROR_NONE) {
				VideoLogWarning("Fail to register network status callback [%d]\n", ret);
			}
		}

		bIsInitCallbackFunc = EINA_TRUE;
	}

	if (pNetworkTimeoutTimer) {
		ecore_timer_del(pNetworkTimeoutTimer);
		pNetworkTimeoutTimer = NULL;
	}

	ret = connection_get_type(gConnection, &con_type);
	if (ret != CONNECTION_ERROR_NONE) {
		ERROR_TRACE("Fail to get network state [%d]\n", ret);
		connection_destroy(connection);
		return bNetworkState;
	}

	switch (con_type) {
	case CONNECTION_TYPE_DISCONNECTED:
		nNetworkStatus = MP_NETWORK_TYPE_OFF;
		VideoLogInfo("MP_NETWORK_TYPE_OFF");
		pNetworkTimeoutTimer = ecore_timer_add(NETWORK_TIMEOUT,
						       mp_streaming_ctrl_popup_cb,
						       pAppData);
		bNetworkState = FALSE;
		break;

	case CONNECTION_TYPE_WIFI:
		nNetworkStatus = MP_NETWORK_TYPE_WIFI;
		VideoLogInfo("MP_NETWORK_TYPE_CELLULAR");
		ret = connection_get_wifi_state(gConnection, &wifi_state);
		if (ret == CONNECTION_ERROR_NONE && wifi_state == CONNECTION_WIFI_STATE_CONNECTED) {
			bNetworkState = TRUE;
		}
		break;

	case CONNECTION_TYPE_CELLULAR:
		nNetworkStatus = MP_NETWORK_TYPE_CELLULAR;
		VideoLogInfo("MP_NETWORK_TYPE_CELLULAR");
		ret = connection_get_cellular_state(gConnection, &cellular_state);
		if (ret == CONNECTION_ERROR_NONE && cellular_state == CONNECTION_CELLULAR_STATE_CONNECTED) {
			bNetworkState = TRUE;
		}
		break;
	default:
		VideoLogInfo("connection type is not supported: %d", con_type);
	}

	return bNetworkState;
}
