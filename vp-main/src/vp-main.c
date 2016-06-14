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
//#include <aul.h>
#include <system_settings.h>
#include "vp-main.h"
#include "vp-service-parser.h"
#include "vp-play-log.h"
#include "vp-play-view.h"

static void __appResume(void *pUserData);

static void __vp_main_realize_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
}

static void __vp_main_destroy_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
//	VpAppData *pAppData = (VpAppData*)pUserData;
//	pAppData->pPlayView = NULL;

	//elm_exit();
}

static bool __appCreate(void *pUserData)
{
	VideoLogWarning("== APP CREATE ==");

	if (pUserData == NULL) {
		VideoLogError("[ERR] No exist pUserData.");
		return FALSE;
	}

	char locale_path[1024] = {0};
	char *path = app_get_resource_path();
	if (path == NULL) {
		return FALSE;
	}
	snprintf(locale_path, 1024, "%s%s", path, "locale");
	VideoLogDebug("Locale Path is: %s", locale_path);
	bindtextdomain("video-player", locale_path);
	free(path);

	VpAppData *pAppData = (VpAppData*)pUserData;

	pAppData->bPause 		= FALSE;
	pAppData->bIsRelaunch	= FALSE;

	return TRUE;
}

static Eina_Bool __appControl_idler_cb(void *data)
{
	VpAppData *pAppData = (VpAppData*)data;


	VideoLogPerfS("============================== RESET =============================");

	video_play_launching_type_t nLaunchingType = VIDEO_PLAY_TYPE_OTHER;
	video_play_mode_t nMode = VIDEO_PLAY_MODE_NORMAL_VIEW;
	bool bRelaunchCaller = FALSE;
	VideoLogError("LaunchType = %d", pAppData->pServiceData->eLaunchType);

	switch (pAppData->pServiceData->eLaunchType) {
	case VP_LAUNCH_TYPE_STORE:
		nLaunchingType = VIDEO_PLAY_TYPE_STORE;
		bRelaunchCaller = TRUE;
		VideoLogWarning("== VIDEO_PLAY_TYPE_STORE ==");
		break;
	case VP_LAUNCH_TYPE_LIST:
		nLaunchingType = VIDEO_PLAY_TYPE_LIST;
		bRelaunchCaller = TRUE;
		VideoLogWarning("== VIDEO_PLAY_TYPE_LIST ==");
		break;
	case VP_LAUNCH_TYPE_NONE:
	case VP_LAUNCH_TYPE_MAX:
		nLaunchingType = VIDEO_PLAY_TYPE_OTHER;
		VideoLogWarning("== VIDEO_PLAY_TYPE_OTHER ==");
		break;
	case VP_LAUNCH_TYPE_EMAIL:
		nLaunchingType = VIDEO_PLAY_TYPE_EMAIL;
		VideoLogWarning("== VIDEO_PLAY_TYPE_EMAIL ==");
		break;
	case VP_LAUNCH_TYPE_MMS:
		nLaunchingType = VIDEO_PLAY_TYPE_MESSAGE;
		VideoLogWarning("== VIDEO_PLAY_TYPE_MESSAGE ==");
		break;
	case VP_LAUNCH_TYPE_MYFILE:
		nLaunchingType = VIDEO_PLAY_TYPE_MYFILE;
		VideoLogWarning("== VP_LAUNCH_TYPE_MYFILE ==");
		break;
	case VP_LAUNCH_TYPE_GALLERY:
		nLaunchingType = VIDEO_PLAY_TYPE_GALLERY;
		VideoLogWarning("== VIDEO_PLAY_TYPE_GALLERY ==");
		break;
	case VP_LAUNCH_TYPE_IMAGE_VIEWER:
		nLaunchingType = VIDEO_PLAY_TYPE_GALLERY;
		VideoLogWarning("== VIDEO_PLAY_TYPE_GALLERY ==");
		break;
	case VP_LAUNCH_TYPE_IMAGE_VIEWER_TRIM:
		nLaunchingType = VIDEO_PLAY_TYPE_GALLERY;
		VideoLogWarning("== VP_LAUNCH_TYPE_IMAGE_VIEWER_TRIM ==");
		break;
	case VP_LAUNCH_TYPE_MULTI_PATH:
		nLaunchingType = VIDEO_PLAY_TYPE_MULTI_PATH;
		break;
	case VP_LAUNCH_TYPE_SIMPLE:
		nLaunchingType = VIDEO_PLAY_TYPE_PREVIEW;
		VideoLogWarning("== VIDEO_PLAY_TYPE_PREVIEW ==");
		break;
	}

	if (pAppData->pServiceData->bStreaming) {
		nLaunchingType = VIDEO_PLAY_TYPE_WEB;
	}

	bool bPopupMode = FALSE;
	vp_play_view_get_popup_mode(pAppData->pPlayView, &bPopupMode);

	bool bIsRealize = FALSE;

	if (vp_play_view_is_realize(pAppData->pPlayView, &bIsRealize)) {
		if (bPopupMode == FALSE && bIsRealize == FALSE && pAppData->bPause == FALSE) {
			VideoLogError("[ERR] Not Yet reailze");
			return ECORE_CALLBACK_DONE;
		}
	}

	if ((bPopupMode == TRUE && pAppData->pServiceData->szDMRID == NULL)
	        ||	(g_strcmp0(pAppData->pValStartup, "2") == 0 && bIsRealize)) {// pAppData->values[0] == 2 for multi window; change video url only, don't destroy the player window.
		char *szCurURL = NULL;
		bool bVisible = FALSE;

		if (!vp_play_view_get_url(pAppData->pPlayView , &szCurURL)) {
			VideoLogError("vp_play_view_set_url is fail");
		}
		if (!vp_play_view_get_visible_status(pAppData->pPlayView , &bVisible)) {
			VideoLogError("vp_play_view_get_visible_status is fail");
		}

		if (szCurURL && bVisible) {
			char *req_uri = NULL;
			if (pAppData->pServiceData->pMediaUri) {
				req_uri = strdup(pAppData->pServiceData->pMediaUri);
			} else if (nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
				if (pAppData->pServiceData->szMultiPath[0] != NULL) {
					char *multi_path = strstr(pAppData->pServiceData->szMultiPath[0], "file://");

					if (multi_path != NULL) {
						req_uri = strdup(multi_path);
					}
				}
			}
			if (req_uri) {
				if (g_strcmp0(req_uri, szCurURL) == 0) {
					VideoLogError("skip for same file");

					free(szCurURL);
					szCurURL = NULL;
					free(req_uri);
					req_uri = NULL;

					Evas_Object *pWin = vp_play_view_get_main_window(pAppData->pPlayView);
					if (pWin) {
						if (nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE || nLaunchingType == VIDEO_PLAY_TYPE_EMAIL) {
							//if current mini mode, change it to full mode
							VideoLogWarning("from Message type and Email type. change mini mode to full mode");
							vp_play_view_change_style_minimode_to_fullmode(pAppData->pPlayView, nLaunchingType);
						}
						elm_win_activate(pWin);
					}

					return ECORE_CALLBACK_DONE;
				}
				free(req_uri);
				req_uri = NULL;
			} else {
				VideoLogError("skip for invalid input");

				free(szCurURL);
				szCurURL = NULL;

				return ECORE_CALLBACK_DONE;
			}
			if (szCurURL) {
				free(szCurURL);
			}
			szCurURL = NULL;
		}

		if (!vp_play_view_unrealize(pAppData->pPlayView)) {
			VideoLogError("vp_play_view_unrealize is fail");
		}

		if (!vp_play_view_set_url(pAppData->pPlayView , pAppData->pServiceData->pMediaUri)) {
			VideoLogError("vp_play_view_set_url is fail");
		}

		vp_play_view_set_launching_mode(pAppData->pPlayView, nLaunchingType);
		if (szCurURL) {
			free(szCurURL);
			szCurURL = NULL;
		}
	} else {
		if (pAppData->pPlayView) {
#ifdef _PERF_TEST_
			vp_play_view_reset(pAppData->pPlayView, nMode);
#else
			vp_play_view_destroy(pAppData->pPlayView);
#endif
			pAppData->pPlayView = NULL;
		}

		if (pAppData->pPlayView == NULL) {
			pAppData->pPlayView = vp_play_view_create(NULL, pAppData->pServiceData->nParentXID, nMode, nLaunchingType);
			if (pAppData->pPlayView	== NULL) {
				VideoLogError("vp_play_view_create is fail");
				elm_exit();
				return ECORE_CALLBACK_DONE;
			}
		}


		Evas_Object *pWin		= NULL;
		pWin					= vp_play_view_get_main_window(pAppData->pPlayView);

		evas_object_show(pWin);
		elm_win_activate(pWin);


		if (!vp_play_view_set_callback(pAppData->pPlayView, VIDEO_PLAY_CB_TYPE_REALIZE, __vp_main_realize_cb)) {
			VideoLogError("vp_play_view_set_callback is fail");
			return ECORE_CALLBACK_DONE;
		}

		if (!vp_play_view_set_callback(pAppData->pPlayView, VIDEO_PLAY_CB_TYPE_DESTORY, __vp_main_destroy_cb)) {
			VideoLogError("vp_play_view_set_callback is fail");
			return ECORE_CALLBACK_DONE;
		}

		if (!vp_play_view_set_user_data(pAppData->pPlayView, (void *)pAppData)) {
			VideoLogError("vp_play_view_set_user_data is fail");
			return ECORE_CALLBACK_DONE;
		}
	}


	if (nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		if (!vp_play_view_set_cookie(pAppData->pPlayView, pAppData->pServiceData->szCookie)) {
			VideoLogError("vp_play_view_set_cookie is fail");
		}

		if (!vp_play_view_set_proxy(pAppData->pPlayView, pAppData->pServiceData->szProxy)) {
			VideoLogError("vp_play_view_set_proxy is fail");
		}
	}

	if (pAppData->pServiceData->szDMRID) {
		VideoLogWarning("DMR ID : %s", pAppData->pServiceData->szDMRID);
		if (!vp_play_view_set_dmr_id(pAppData->pPlayView , pAppData->pServiceData->szDMRID)) {
			VideoLogError("vp_play_view_set_dmr_id is fail");
		}
	}

	if (nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH && pAppData->pServiceData->szMultiPath) {
		int idx = 0;

		if (!vp_play_view_set_stop_after_repeat_mode(pAppData->pPlayView , pAppData->pServiceData->bStopAfterRepeat)) {
			VideoLogError("vp_play_view_set_stop_after_repeat_mode is fail");
		}

		for (idx = 0; idx < pAppData->pServiceData->nMultiPath; idx++) {
			if (pAppData->pServiceData->szMultiPath[idx] == NULL) {
				continue;
			}

			if (pAppData->pServiceData->szMultiPath[idx]) {
				char *szSubtitle = NULL;
				char *szTitle = NULL;
				int nPosition = 0;
				int nDuration = 0;

				if (idx == 0) {
					if (!vp_play_view_set_url(pAppData->pPlayView , pAppData->pServiceData->szMultiPath[idx])) {
						VideoLogError("vp_play_view_set_url is fail");
					}
				}
				if (pAppData->pServiceData->szMultiTitle) {
					if (pAppData->pServiceData->szMultiTitle[idx]) {
						szTitle = pAppData->pServiceData->szMultiTitle[idx];
					}
				}
				if (pAppData->pServiceData->szMultiSubTitle) {
					if (pAppData->pServiceData->szMultiSubTitle[idx]) {
						szSubtitle = pAppData->pServiceData->szMultiSubTitle[idx];
					}
				}
				if (pAppData->pServiceData->szMultiPosition) {
					if (pAppData->pServiceData->szMultiPosition[idx]) {
						nPosition = atoi(pAppData->pServiceData->szMultiPosition[idx]);
					}
				}
				if (pAppData->pServiceData->szMultiDuration) {
					if (pAppData->pServiceData->szMultiDuration[idx]) {
						nDuration = atoi(pAppData->pServiceData->szMultiDuration[idx]);
					}
				}
				vp_play_view_add_multi_path(pAppData->pPlayView,
				                            pAppData->pServiceData->szMultiPath[idx],
				                            szTitle,
				                            szSubtitle,
				                            nPosition,
				                            nDuration,
				                            FALSE);
			}
		}
	} else {
		int nPosition = 0;
		if (pAppData->pServiceData->bStoreDownload) {
			nPosition =  pAppData->pServiceData->nStorePlayedTime;
		}
		if (!vp_play_view_set_start_position(pAppData->pPlayView , nPosition)) {
			VideoLogError("vp_play_view_set_start_position is fail");
		}

		if (!vp_play_view_set_url(pAppData->pPlayView , pAppData->pServiceData->pMediaUri)) {
			VideoLogError("vp_play_view_set_url is fail");
		}

		if (!vp_play_view_set_store_download_mode(pAppData->pPlayView , pAppData->pServiceData->bStoreDownload)) {
			VideoLogError("vp_play_view_set_store_download_mode is fail");
		}

		if (!vp_play_view_set_store_download_filesize(pAppData->pPlayView , pAppData->pServiceData->dStoreDL_FileSize)) {
			VideoLogError("vp_play_view_set_store_download_filesize is fail");
		}

		if (pAppData->pServiceData->bStoreDownload == EINA_TRUE && pAppData->pServiceData->pStoreOrderId != NULL) {
			if (!vp_play_view_set_store_order_info(pAppData->pPlayView ,
			                                       pAppData->pServiceData->pStoreOrderId,
			                                       pAppData->pServiceData->pStoreServerId,
			                                       pAppData->pServiceData->pStoreAppId,
			                                       pAppData->pServiceData->pStoreMvId)) {
				VideoLogError("vp_play_view_set_store_order_info is fail");
			}
		}
	}

	if (pAppData->pServiceData->eListType == VP_VIDEO_PLAY_LIST_TYPE_FOLDER) {
		vp_play_view_set_list_mode(pAppData->pPlayView, VIDEO_PLAY_LIST_TYPE_FOLDER);
	} else {
		vp_play_view_set_list_mode(pAppData->pPlayView, VIDEO_PLAY_LIST_TYPE_ALL);
	}

	if (pAppData->pServiceData->szDeviceID) {
		VideoLogWarning("Device ID : %s", pAppData->pServiceData->szDeviceID);
		if (!vp_play_view_set_device_id(pAppData->pPlayView , pAppData->pServiceData->szDeviceID)) {
			VideoLogError("vp_play_view_set_device_id is fail");
		}
	}

	if (pAppData->pServiceData->szMediaTitle) {
		VideoLogWarning("szMediaTitle : %s", pAppData->pServiceData->szMediaTitle);
		if (!vp_play_view_set_media_title(pAppData->pPlayView , pAppData->pServiceData->szMediaTitle)) {
			VideoLogError("vp_play_view_set_media_title is fail");
		}
	}

	if (!vp_play_view_set_relaunch_caller(pAppData->pPlayView, bRelaunchCaller)) {
		VideoLogError("vp_play_view_set_user_data is fail");
		return ECORE_CALLBACK_DONE;
	}

	if (!vp_play_view_realize(pAppData->pPlayView)) {
		VideoLogError("vp_play_view_realize is fail");
		return ECORE_CALLBACK_DONE;
	}

	VideoLogPerfE("============================== RESET =============================");

	if (pAppData->pAppSvcIdler) {
		ecore_idler_del(pAppData->pAppSvcIdler);
		pAppData->pAppSvcIdler = NULL;
	}

	return ECORE_CALLBACK_DONE;
}

#if 0
static int __raise_win(Window win)
{
	XEvent xev;
	Window root;
	static Atom a_active_win;

	Display *d = XOpenDisplay(NULL);

	if (!a_active_win) {
		a_active_win = XInternAtom(d, "_NET_ACTIVE_WINDOW", False);
	}

	root = XDefaultRootWindow(d);

	xev.xclient.type = ClientMessage;
	xev.xclient.display = d;
	xev.xclient.window = win;
	xev.xclient.message_type = a_active_win;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = CurrentTime;
	xev.xclient.data.l[2] = 0;
	xev.xclient.data.l[3] = 0;
	xev.xclient.data.l[4] = 0;
	XSendEvent(d, root, False,
	           SubstructureRedirectMask | SubstructureNotifyMask, &xev);

	XCloseDisplay(d);

	return 0;
}
#endif

static void __appControl(app_control_h pAppSvcHandle, void *pUserData)
{
	VideoLogWarning("== APP SERVICE ==");

	if (pAppSvcHandle == NULL) {
		VideoLogError("[ERR] No exist pAppSvcHandle.");
		return;
	}

	if (pUserData == NULL) {
		VideoLogError("[ERR] No exist pUserData.");
		return;
	}

	VpAppData *pAppData = (VpAppData*)pUserData;

	char *resume_check = NULL;
	if (app_control_get_extra_data(pAppSvcHandle, "startup_type", &resume_check) != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("No exist startup_type");
	} else {
		VideoLogInfo("temp_resume_check : %s", resume_check);
		if (resume_check) {
			if (strcmp(resume_check, "1") == 0) {
				VideoLogInfo("do resume");
				free(resume_check);

				VpAppData *pAppData = (VpAppData*)pUserData;
				Evas_Object *pWin		= NULL;
				pWin = vp_play_view_get_main_window(pAppData->pPlayView);
				if (pWin) {
					//__raise_win(elm_win_xwindow_get(pWin));
					__appResume(pUserData);
					return;
				} else {
					VideoLogError("No exist pWin");
				}
			}
		}
	}

	if (pAppData->pServiceData != NULL) {
		VpServiceParserDestroyServiceData(pAppData->pServiceData);
		pAppData->pServiceData	= NULL;
	}

	pAppData->pServiceData	= calloc(1, sizeof(VpServiceData));

	if (VpServiceParserGetServiceData(pAppSvcHandle, pAppData->pServiceData) == EINA_FALSE) {
		VideoLogError("[ERR] VpServiceParserGetServiceData fail!!!");
		if (pAppData->pServiceData != NULL) {
			VpServiceParserDestroyServiceData(pAppData->pServiceData);
			pAppData->pServiceData	= NULL;
		}

		return;
	}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// for sidesync
//////////////////////////////////////////////////////////////////////////////////////////////////////
	if (pAppData->pServiceData->pMediaUri != NULL) {
		bool	is_pss	= FALSE;

		VideoSecureLogInfo("is_pss : [%d]", is_pss);

		if (is_pss == TRUE) {
			VideoSecureLogInfo("ss-play: %s", pAppData->pServiceData->pMediaUri);
			return;
		}
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////
// for split window
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _PERF_TEST_
	pAppData->pAppSvcIdler = ecore_idler_add(__appControl_idler_cb, (void*)pAppData);
#else
	__appControl_idler_cb((void*)pAppData);
#endif
}


static void __appExit(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("[ERR] No exist pUserData.");
		return;
	}

	VideoLogWarning("== APP EXIT ==");

	VpAppData	*pAppData 	= (VpAppData*)pUserData;

	if (pAppData->pServiceData != NULL) {
		VpServiceParserDestroyServiceData(pAppData->pServiceData);
		pAppData->pServiceData	= NULL;
	}

	if (pAppData->pPlayView) {
		vp_play_view_destroy(pAppData->pPlayView);
		pAppData->pPlayView = NULL;
	}

	if (pAppData->pValStartup != NULL) {
		free(pAppData->pValStartup);
		pAppData->pValStartup	= NULL;
	}

	if (pAppData->pValLayout != NULL) {
		free(pAppData->pValLayout);
		pAppData->pValLayout	= NULL;
	}
}


static void __appPause(void *pUserData)
{
	VideoLogWarning("== APP PAUSE ==");

	if (pUserData == NULL) {
		VideoLogError("[ERR] No exist pUserData.");
		return;
	}

	VpAppData *pAppData = (VpAppData*)pUserData;
	pAppData->bPause = TRUE;
#if 1
	if (!vp_play_view_unrealize(pAppData->pPlayView)) {
		VideoLogWarning("vp_play_view_unrealize is fail");
	}

#else
	if (!vp_play_view_get_popup_mode(pAppData->pPlayView, &bPopupMode)) {
		VideoLogWarning("vp_play_view_get_popup_mode is fail");
	}

	if (bPopupMode == FALSE) {
		if (!vp_play_view_unrealize(pAppData->pPlayView)) {
			VideoLogWarning("vp_play_view_unrealize is fail");
		}
	}
#endif
}


static void __appResume(void *pUserData)
{
	VideoLogWarning("== APP RESUME ==");

	if (pUserData == NULL) {
		VideoLogError("[ERR] No exist pUserData.");
		return;
	}

	VpAppData *pAppData = (VpAppData*)pUserData;
	pAppData->bPause = FALSE;


#if 1
	if (!vp_play_view_realize(pAppData->pPlayView)) {
		VideoLogError("vp_play_view_realize is fail");
	}
	vp_play_view_live_stream_realize(pAppData->pPlayView);
#else
	if (!vp_play_view_get_popup_mode(pAppData->pPlayView, &bPopupMode)) {
		VideoLogWarning("vp_play_view_get_popup_mode is fail");
	}

	if (bPopupMode == FALSE) {
		if (!vp_play_view_realize(pAppData->pPlayView)) {
			VideoLogError("vp_play_view_realize is fail");
		}
	}
#endif
}

void vp_low_battery(app_event_info_h event_info, void *data)
{
	VpAppData *ad = (VpAppData *)data;
	app_event_low_battery_status_e status = -1;
	int ret = app_event_get_low_battery_status(event_info, &status);
	if (ret == APP_ERROR_NONE) {
		if (status <= APP_EVENT_LOW_BATTERY_POWER_OFF) {
			__appExit(ad);
		}
	} else {
		VideoLogInfo("Failed to get battery status. ret %d", ret);
	}
	return;
}

static void __appUpdateLanguage(app_event_info_h pEventInfo, void *pUserData)
{
	char *locale = NULL;
	int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);

	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		VideoLogInfo("[ERR] failed to get updated language!!! [retcode = retcode]", retcode);
	} else {
		if (locale) {
			elm_language_set(locale);
			free(locale);
		}
	}
}

EXPORT_API int main(int argc, char *argv[])
{
	VpAppData sAppData	= {0,};
	int nRet = -1;
	ui_app_lifecycle_callback_s st_appEventService;
	app_event_handler_h hLanguageChangedHandle;
	app_event_handler_h hLowBatteryHandle;
	nRet = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, __appUpdateLanguage, (void*)&sAppData);
	if (nRet != APP_ERROR_NONE) {
		VideoLogError("APP_EVENT_LANGUAGE_CHANGED  ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;

	}
	nRet = ui_app_add_event_handler(&hLowBatteryHandle, APP_EVENT_LOW_BATTERY, vp_low_battery, (void*)&sAppData);
	if (nRet != APP_ERROR_NONE) {
		VideoLogError("APP_EVENT_LOW_BATTERY  ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}

	st_appEventService.create 			= __appCreate;
	st_appEventService.terminate 			= __appExit;
	st_appEventService.pause 			= __appPause;
	st_appEventService.resume 			= __appResume;
	st_appEventService.app_control 			= __appControl;
	//st_appEventService.low_memory 			= NULL;
	//st_appEventService.low_battery 			= NULL;
	//st_appEventService.device_orientation 		= NULL;
	//st_appEventService.language_changed 		= __appUpdateLanguage;
	//st_appEventService.region_format_changed 	= NULL;

	if (ui_app_main(argc, argv, &st_appEventService, &sAppData) != APP_ERROR_NONE) {
		return -1;
	}

	return 0;
}
