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

#include <runtime_info.h>
#include <network/wifi.h>
#include <media_key.h>
#include <sound_manager.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-view.h"
#include "vp-play-view-priv.h"

#include "vp-play-normal-view.h"
#include "vp-play-multi-view.h"

#include "vp-play-popup.h"
#include "vp-play-button.h"
#include "vp-play-ug.h"

#include "vp-play-util.h"
#include "vp-play-config.h"
#include "vp-play-preference.h"
#include "vp-hollic.h"
#include "vp-sound.h"
#include "vp-device.h"
#include "vp-sensor.h"
#include "vp-mm-player.h"
#include "vp-media-key.h"

#ifdef ENABLE_DRM_FEATURE
#include "vp-drm.h"
#endif
#include "vp-multi-path.h"
#include "vp-media-contents.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-util.h"
#include "vp-avrcp.h"


#define VP_PLAY_VIEW_VOLUME_TIMER		1.0
#define VP_PLAY_VIEW_VOLUME_SPEED_TIMER		0.05
#define VP_PLAY_VIEW_DB_UPDATE_TIMER		1.0

static bool _vp_play_view_create_mode(PlayView *pPlayView);
static bool _vp_play_view_destroy_mode(PlayView *pPlayView);

static void _vp_play_view_destroy_handle(PlayView *pPlayView);
static void _vp_play_view_create_volume_timer(PlayView *pPlayView);

#if 0//Tizen3.0 Build error
/* callback functions */
static Eina_Bool __vp_play_caller_window_event_cb(void *pUserData, int nType, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	if (pPlayView->bViewChange) {
		return ECORE_CALLBACK_PASS_ON;
	}

	Ecore_X_Event_Window_Focus_In *ev = (Ecore_X_Event_Window_Focus_In *)pEvent;

	if (ev->win == pPlayView->nCallerXwinID) {
		elm_exit();
	}

	return ECORE_CALLBACK_PASS_ON;

}

static Eina_Bool __vp_play_view_mouse_in_cb(void *pUserData, int type, void *event)
{
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	Ecore_X_Event_Mouse_In *e = event;

	PlayView *pPlayView = (PlayView *)pUserData;

	if (e->win == pPlayView->nXwinID) {
		VideoLogWarning("Mouse IN");
		pPlayView->bMouseOut = FALSE;
	}
	vp_play_normal_view_rotate_update(pPlayView->pNormalView);
	vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_UP_KEY);
	vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_DOWN_KEY);
	vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_MUTE);
	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_view_mouse_out_cb(void *pUserData, int type, void *event)
{
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}


	Ecore_X_Event_Mouse_Out *e = event;

	PlayView *pPlayView = (PlayView *)pUserData;

	if (e->win == pPlayView->nXwinID) {
		VideoLogWarning("Mouse OUT");
		pPlayView->bMouseOut = TRUE;
	}
	vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_UP_KEY);
	vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_DOWN_KEY);
	vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_MUTE);
	return ECORE_CALLBACK_PASS_ON;
}


static Eina_Bool __vp_play_focus_in_cb(void *pUserData, int nType, void *pEvent)
{
#if 0
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	if (pPlayView->bViewChange) {
		return ECORE_CALLBACK_PASS_ON;
	}

	Ecore_X_Event_Window_Focus_In *ev = (Ecore_X_Event_Window_Focus_In *)pEvent;

	if (ev->win != pPlayView->nXwinID) {
		VideoLogWarning("Invalid Window => cur:0x%x - video:0x%x", ev->win, pPlayView->nXwinID);
		return ECORE_CALLBACK_PASS_ON;
	}

	if (ev->win == pPlayView->nXwinID) {
		VideoLogWarning("Focus In");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
// If type is EMAIL/MESSAGE MODE,
// Email -> play -> detail -> popup -> focus in cb -> resume function -> close all popup(in resume function)
			vp_play_normal_view_set_share_panel_state(pPlayView->pNormalView);
			if (!vp_play_normal_view_update(pPlayView->pNormalView)) {
				VideoLogError("vp_play_normal_view_update handle is null");
				return ECORE_CALLBACK_PASS_ON;
			}
		}
	}
#endif
	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_focus_out_cb(void *pUserData, int nType, void *pEvent)
{
#if 0
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	Ecore_X_Event_Window_Focus_Out *ev = (Ecore_X_Event_Window_Focus_Out *)pEvent;

	if (pPlayView->bViewChange) {
		return ECORE_CALLBACK_PASS_ON;
	}

	if (ev->win != pPlayView->nXwinID) {
		VideoLogWarning("Invalid Window => cur:0x%x - video:0x%x", ev->win, pPlayView->nXwinID);
		return ECORE_CALLBACK_PASS_ON;
	}

	if (ev->win == pPlayView->nXwinID) {
		VideoLogWarning("Focus Out");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		}
	}
#endif
	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_visibility_cb(void *pUserData, int nType, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	Ecore_X_Event_Window_Visibility_Change* ev = (Ecore_X_Event_Window_Visibility_Change *)pEvent;

	if (ev->win != pPlayView->nXwinID)  {
		VideoLogWarning("Invalid Window => cur:0x%x - video:0x%x", ev->win, pPlayView->nXwinID);
		return ECORE_CALLBACK_PASS_ON;
	}

	if (pPlayView->bViewChange) {
		return ECORE_CALLBACK_PASS_ON;
	}


	if (ev->win == pPlayView->nXwinID) {
		if (ev->fully_obscured == 1) {
			VideoLogWarning("hide main window");
			pPlayView->bVisible = FALSE;
			if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
				if (!vp_play_normal_view_pause(pPlayView->pNormalView)) {
					VideoLogError("vp_play_normal_view_resume handle is null");
					return ECORE_CALLBACK_PASS_ON;
				}
				if (!vp_play_normal_view_update(pPlayView->pNormalView)) {
					VideoLogError("vp_play_normal_view_update handle is null");
					return ECORE_CALLBACK_PASS_ON;
				}
			}
		} else {
			VideoLogWarning("show main window");
			pPlayView->bVisible = TRUE;
			if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
				if (!vp_play_normal_view_resume(pPlayView->pNormalView)) {
					VideoLogError("vp_play_normal_view_resume handle is null");
					return ECORE_CALLBACK_PASS_ON;
				}
				if (!vp_play_normal_view_update(pPlayView->pNormalView)) {
					VideoLogError("vp_play_normal_view_update handle is null");
					return ECORE_CALLBACK_PASS_ON;
				}
			}
		}
	}

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_client_message_cb(void *pUserData, int nType, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	Ecore_X_Event_Client_Message *ev = (Ecore_X_Event_Client_Message *) pEvent;

	if (ev->win != pPlayView->nXwinID) {
		VideoLogWarning("Invalid Window => cur:0x%x - video:0x%x", ev->win, pPlayView->nXwinID);
		return ECORE_CALLBACK_PASS_ON;
	}

	if (pPlayView->bViewChange) {
		return ECORE_CALLBACK_PASS_ON;
	}

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_view_window_property_change(void *pUserData, int type, void *event)
{
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	Ecore_X_Event_Window_Property *e = event;

	PlayView *pPlayView = (PlayView *)pUserData;

	if (e->win == pPlayView->nXwinID) {
		bool bVideoOnly = FALSE;
		if (pPlayView->bActivate == FALSE) {
			VideoLogInfo("__vp_play_view_window_property_change");
			vp_play_view_unrealize(pPlayView);
		} else {
			if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
				if (vp_play_normal_view_player_state_changed_get(pPlayView->pNormalView)) {
					vp_play_normal_view_set_video_only(pPlayView->pNormalView, bVideoOnly);
				}
			}
		}
	} else if (pPlayView->pMultiView) {
		if (e->atom == ECORE_X_ATOM_WM_STATE) {
			Ecore_X_Window_State_Hint state;
			state = ecore_x_icccm_state_get(e->win);

			switch (state) {
			case ECORE_X_WINDOW_STATE_HINT_ICONIC:
				VideoLogInfo("MINI : Iconified");
				break;
			case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
				VideoLogInfo("MINI : Hide");
				/* pause */
				vp_play_multi_view_get_last_position(pPlayView->pMultiView, &(pPlayView->nStartPosition));
				vp_play_multi_view_get_audio_track(pPlayView->pMultiView, &(pPlayView->nAudioIndex));
				VP_FREE(pPlayView->szSubtitleURL);
				vp_play_multi_view_get_subtitle(pPlayView->pMultiView, &(pPlayView->szSubtitleURL));
				vp_play_multi_view_pause(pPlayView->pMultiView);
				if (!vp_play_multi_view_set_manual_pause(pPlayView->pMultiView, TRUE)) {
					VideoLogError("vp_play_normal_view_set_manual_pause fail");
				}
				break;
			case ECORE_X_WINDOW_STATE_HINT_NORMAL:
				VideoLogInfo("MINI : Show");
				break;
			default:
				// do something for handling error
				break;
			}
		}
	}

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_delete_request_cb(void *pUserData, int nType, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	VideoLogWarning("DELETE REQUEST CALL");

	Ecore_X_Event_Window_Delete_Request *ev = (Ecore_X_Event_Window_Delete_Request *) pEvent;

	if (ev->win != pPlayView->nXwinID) {
		VideoLogWarning("Invalid Window => cur:0x%x - video:0x%x", ev->win, pPlayView->nXwinID);
		return ECORE_CALLBACK_PASS_ON;
	}

	if (pPlayView->bViewChange) {
		return ECORE_CALLBACK_PASS_ON;
	}

	elm_exit();

	return ECORE_CALLBACK_PASS_ON;
}
#endif


static Eina_Bool __vp_play_view_volume_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	PlayView *pPlayView = (PlayView *)pUserData;
	if (pPlayView->bVolumeIncrease) {
		vp_play_normal_view_volume_increase(pPlayView->pNormalView);
	} else {
		vp_play_normal_view_volume_decrease(pPlayView->pNormalView);
	}

	VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);

	pPlayView->pVolumeTimer = ecore_timer_add(VP_PLAY_VIEW_VOLUME_SPEED_TIMER,
	                          __vp_play_view_volume_timer_cb, (void *)pPlayView);

	return EINA_FALSE;
}

static Eina_Bool __vp_play_view_hw_key_long_press_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	PlayView *pPlayView = (PlayView *)pUserData;
	pPlayView->pHwKeyLongPressTimer = NULL;

	return EINA_FALSE;
}

static Eina_Bool __vp_play_view_media_long_press_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	VP_EVAS_TIMER_DEL(pPlayView->pMediaLongPressTimer);

	return EINA_FALSE;
}

static Eina_Bool __vp_play_view_media_key_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	if (pPlayView->nMediaKeyPressCount == 1) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_resume_or_pause(pPlayView->pNormalView);
		}

	} else if (pPlayView->nMediaKeyPressCount == 2) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_next_play(pPlayView->pNormalView);
		}
	} else if (pPlayView->nMediaKeyPressCount == 3) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_prev_play(pPlayView->pNormalView);
		}
	}
	pPlayView->nMediaKeyPressCount = 0;

	VP_EVAS_TIMER_DEL(pPlayView->pMediaKeyTimer);

	return EINA_FALSE;
}

static void __vp_play_view_noti_pipe_cb(void *pUserData, void *pBuf, int nByte)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		vp_play_normal_view_hide_sound_popup(pPlayView->pNormalView);
		vp_play_normal_view_update(pPlayView->pNormalView);
	}
}

static void __vp_play_sound_route_change_cb(int nRoute, bool bAvailable, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	if (pPlayView->pNotiPipe) {
		ecore_pipe_write(pPlayView->pNotiPipe, pUserData, sizeof(PlayView));
	}
}

static void __vp_play_sound_volume_change_cb(int nType, int nVolume, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		vp_play_normal_view_volume_update(pPlayView->pNormalView);
	}
}

static void __vp_play_rotate_changed_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return;
	}
	PlayView *pPlayView = (PlayView *)pUserData;

	int nAngle = elm_win_rotation_get((Evas_Object *)pObj);
	video_play_rotate_t nRotate = VIDEO_PLAY_ROTATE_NONE;

	switch (nAngle) {
	case 0:
		nRotate = VIDEO_PLAY_ROTATE_NONE;
		break;
	case 90:
		nRotate = VIDEO_PLAY_ROTATE_90;
		break;
	case 180:
		nRotate = VIDEO_PLAY_ROTATE_180;
		break;
	case 270:
		nRotate = VIDEO_PLAY_ROTATE_270;
		break;
	}

	VideoLogWarning("CHANGE ROTATE : %d", nRotate);

	if (pPlayView->nRotate != nRotate) {
		vp_play_view_set_rotate(pPlayView, nRotate);
	}
}

static void __vp_play_view_realize_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return;
	}

	vp_util_lock_cpu();

	PlayView *pPlayView = (PlayView *)pUserData;


	/* send event to parent */
	if (pPlayView->pCallback == NULL) {
		VideoLogError("PlayView Callback is NULL");
		return;
	}

	if (pPlayView->pCallback->pRealizeCb == NULL) {
		VideoLogError("PlayView Realize Callback is NULL");
		return;
	}

	pPlayView->pCallback->pRealizeCb(pPlayView->pUserData);
}

static void __vp_play_view_destroy_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	/* send event to parent */
	if (!pPlayView->pCallback) {
		VideoLogError("PlayView Callback is NULL");
		return;
	}

	if (!pPlayView->pCallback->pDestroyCb) {
		VideoLogError("PlayView Realize Callback is NULL");
		return;
	}

	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pKeyEventDown);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pKeyEventUp);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pFocusIn);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pFocusOut);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pVisiblility);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pClientMessage);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pDeleteRequest);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pPropertyHandler);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pMouseInHandler);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pMouseOutHandler);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pPalmHoldEventHandler);

	pPlayView->pCallback->pDestroyCb(pPlayView->pUserData);

	elm_exit();
}

static Eina_Bool __vp_play_view_event_key_down_cb(void *pUserData, int nType, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return EINA_FALSE;
	}

	if (!pEvent) {
		VideoLogError("pEvent handle is NULL");
		return EINA_FALSE;
	}


	PlayView *pPlayView = (PlayView *)pUserData;
	Ecore_Event_Key *pKeyEvent = (Ecore_Event_Key *)pEvent;

	/* default operation */
	VideoLogWarning("HW EVENT KEY DOWN:%s", pKeyEvent->keyname);

	if (!strcmp(pKeyEvent->keyname, VP_END_KEY)) {
		VideoLogWarning("Back(End) key");
	} else if (!strcmp(pKeyEvent->keyname, VP_MENU_KEY)) {
		VideoLogWarning("Menu key");
	} else if (!strcmp(pKeyEvent->keyname, VP_HOME_KEY)) {
		VideoLogWarning("HOME_KEY");

		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			bool bLock = FALSE;
			pPlayView->bManualAppPause = TRUE;
			vp_play_normal_view_get_lock_screen(pPlayView->pNormalView, &bLock);
			if (bLock) {
				vp_play_normal_view_set_lock_screen(pPlayView->pNormalView, bLock);
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_MEDIA_KEY)) {
		VP_EVAS_TIMER_DEL(pPlayView->pMediaLongPressTimer);
		pPlayView->pMediaLongPressTimer = ecore_timer_add(0.5, __vp_play_view_media_long_press_timer_cb, (void *)pPlayView);
	} else if (!strcmp(pKeyEvent->keyname, VP_VOLUME_DOWN_KEY)) {
		VideoLogWarning("VOLUME DOWN");
		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		sound_manager_set_current_sound_type(SOUND_TYPE_MEDIA);
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
			if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE  || nPopupState == VP_PLAY_NORMAL_VIEW_CTX_POPUP) {
				vp_play_normal_view_volume_decrease(pPlayView->pNormalView);
				pPlayView->bVolumeIncrease = FALSE;
				_vp_play_view_create_volume_timer(pPlayView);
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, "Down")) {
		VideoLogWarning("KEY DOWN");
		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
			if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE) {
				if (vp_play_normal_view_get_main_layout_focus_state(pPlayView->pNormalView) == FALSE) {
					vp_play_normal_view_volume_decrease(pPlayView->pNormalView);
					pPlayView->bVolumeIncrease = FALSE;
					_vp_play_view_create_volume_timer(pPlayView);
				}
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_VOLUME_UP_KEY)) {
		VideoLogWarning("VOLUME UP");
		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		sound_manager_set_current_sound_type(SOUND_TYPE_MEDIA);
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
			if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE  || nPopupState == VP_PLAY_NORMAL_VIEW_CTX_POPUP) {
				vp_play_normal_view_volume_increase(pPlayView->pNormalView);
				pPlayView->bVolumeIncrease = TRUE;
				_vp_play_view_create_volume_timer(pPlayView);
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, "Up")) {
		VideoLogWarning("KEY UP");
		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
			if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE) {
				if (vp_play_normal_view_get_main_layout_focus_state(pPlayView->pNormalView) == FALSE) {
					vp_play_normal_view_volume_increase(pPlayView->pNormalView);
					pPlayView->bVolumeIncrease = TRUE;
					_vp_play_view_create_volume_timer(pPlayView);
				}
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_VOLUME_MUTE)) {
		VideoLogWarning("VOLUME MUTE");
		sound_manager_set_current_sound_type(SOUND_TYPE_MEDIA);
		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_volume_mute(pPlayView->pNormalView);
			pPlayView->bVolumeIncrease = TRUE;
			_vp_play_view_create_volume_timer(pPlayView);
		}

	} else if (!strcmp(pKeyEvent->keyname, VP_POWER_KEY) || !strcmp(pKeyEvent->keyname, VP_SCREENSAVER_KEY)) {
		VideoLogWarning("POWER KEY or SCREENSAVER KEY");

		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			VP_EVAS_TIMER_DEL(pPlayView->pHwKeyLongPressTimer);
			pPlayView->pHwKeyLongPressTimer = ecore_timer_add(0.5, __vp_play_view_hw_key_long_press_timer_cb, (void *)pPlayView);

			vp_play_normal_check_detail_view(pPlayView->pNormalView);
			bool bPause = FALSE;
			vp_play_normal_view_get_pause_status(pPlayView->pNormalView, &bPause);
			if (bPause) {
				vp_play_util_set_lock_power_key();
			}

			return EINA_TRUE;
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_KEY_CANCLE) || !strcmp(pKeyEvent->keyname, "Esc")) {
		VideoLogWarning("CANCLE KEY");
		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
	} else if (!strcmp(pKeyEvent->keyname, VP_PLAY_KEY) || !strcmp(pKeyEvent->keyname, VP_PAUSE_KEY)) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_resume_or_pause(pPlayView->pNormalView);
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_PLAY_NEXT_SONG_KEY)) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_next_play(pPlayView->pNormalView);
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_PLAY_PREV_SONG_KEY)) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_prev_play(pPlayView->pNormalView);
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_FORWARD_KEY)) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
			if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE || nPopupState == VP_PLAY_NORMAL_VIEW_CTX_POPUP) {
				if (vp_play_normal_view_get_main_layout_focus_state(pPlayView->pNormalView) == FALSE) {
					if (vp_play_normal_view_get_main_layout_show_state(pPlayView->pNormalView) == FALSE) {
						vp_play_normal_view_show_main_layout(pPlayView->pNormalView);
					}
					if (pPlayView->bKeySpeed == FALSE) {
						vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, FALSE, TRUE);
						pPlayView->bKeySpeed = TRUE;
					}
				}
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_REWIND_KEY)) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
			if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE || nPopupState == VP_PLAY_NORMAL_VIEW_CTX_POPUP) {
				if (vp_play_normal_view_get_main_layout_focus_state(pPlayView->pNormalView) == FALSE) {
					if (vp_play_normal_view_get_main_layout_show_state(pPlayView->pNormalView) == FALSE) {
						vp_play_normal_view_show_main_layout(pPlayView->pNormalView);
					}
					if (pPlayView->bKeySpeed == FALSE) {
						vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, FALSE, FALSE);
						pPlayView->bKeySpeed = TRUE;
					}
				}
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, "Tab")) {
		VideoLogInfo("Tab Press Down");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			bool bMainShowState = vp_play_normal_view_get_main_layout_show_state(pPlayView->pNormalView);
			if (bMainShowState) {
				VideoLogInfo("main focus in");
				vp_play_normal_view_set_main_layout_focus_in(pPlayView->pNormalView);
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, "Return")) {
		VideoLogInfo("Return Press Down");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			bool bMainFocusState = vp_play_normal_view_get_main_layout_focus_state(pPlayView->pNormalView);
			if (bMainFocusState == FALSE) {
				vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
				if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE) {
					vp_play_normal_view_set_resume_or_pause(pPlayView->pNormalView);
				}
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, "space")) {
		VideoLogInfo("space Press Down");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_normalview_popup_style_e nPopupState = vp_play_normal_view_get_popup_show_state(pPlayView->pNormalView);
			if (nPopupState == VP_PLAY_NORMAL_VIEW_POPUP_NONE) {
				vp_play_normal_view_set_resume_or_pause(pPlayView->pNormalView);
			}
		}
	}

	//For normal view hide control layout
	if (!strcmp(pKeyEvent->keyname, "Tab") || !strcmp(pKeyEvent->keyname, "Right") || !strcmp(pKeyEvent->keyname, "Left")
	        || !strcmp(pKeyEvent->keyname, "Up") || !strcmp(pKeyEvent->keyname, "Down")) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_create_main_layout_hide_timer(pPlayView->pNormalView);
		}
	}
	/* send event to parent */
	if (pPlayView->pCallback == NULL) {
		VideoLogError("PlayView Callback is NULL");
		return EINA_FALSE;
	}

	if (pPlayView->pCallback->pEventKeyDown == NULL) {
		VideoLogError("PlayView pEventKeyDown Callback is NULL");
		return EINA_FALSE;
	}

	pPlayView->pCallback->pEventKeyDown(pPlayView->pUserData, nType, pEvent);

	return EINA_FALSE;
}

static Eina_Bool __vp_play_view_event_key_up_cb(void *pUserData, int nType, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("PlayView handle is NULL");
		return EINA_FALSE;
	}

	if (!pEvent) {
		VideoLogError("pEvent handle is NULL");
		return EINA_FALSE;
	}

	PlayView *pPlayView = (PlayView *)pUserData;
	Ecore_Event_Key *pKeyEvent = (Ecore_Event_Key *)pEvent;

	/* default operation */
	VideoLogWarning("HW EVENT KEY UP:%s", pKeyEvent->keyname);

	if (!strcmp(pKeyEvent->keyname, VP_END_KEY)) {
		VideoLogWarning("Back(End) key");
	} else if (!strcmp(pKeyEvent->keyname, VP_MENU_KEY) || !strcmp(pKeyEvent->keyname, "Menu")) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			vp_play_normal_view_on_more_menu(pPlayView->pNormalView);
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_HOME_KEY)) {
		VideoLogWarning("HOME_KEY");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			bool bLock = FALSE;
			vp_play_normal_view_get_lock_screen(pPlayView->pNormalView, &bLock);
			if (!bLock) {
				if (!vp_play_normal_view_pause(pPlayView->pNormalView)) {
					VideoLogError("vp_play_normal_view_pause fail");
				}
				if (!vp_play_normal_view_update(pPlayView->pNormalView)) {
					VideoLogError("vp_play_normal_view_update fail");
				}
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_VOLUME_DOWN_KEY) || !strcmp(pKeyEvent->keyname, "Down")) {
		VideoLogWarning("VOLUME DOWN");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_VOLUME_UP_KEY) || !strcmp(pKeyEvent->keyname, "Up")) {
		VideoLogWarning("VOLUME UP");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_VOLUME_MUTE)) {
		VideoLogWarning("VOLUME MUTE");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_MEDIA_KEY)) {
		if (pPlayView->nMediaKeyPressCount > 3) {
			VideoLogWarning("pressed more than 3times");
			return EINA_FALSE;
		}

		if (pPlayView->pMediaLongPressTimer) {
			VP_EVAS_TIMER_DEL(pPlayView->pMediaKeyTimer);
			pPlayView->pMediaKeyTimer = ecore_timer_add(0.5, __vp_play_view_media_key_timer_cb, (void *)pPlayView);
			pPlayView->nMediaKeyPressCount++;
		} else {
			VP_EVAS_TIMER_DEL(pPlayView->pMediaLongPressTimer);
			//vp_play_app_launch_svoice(pPlayView->pWin, (void *)pPlayView);
			pPlayView->nMediaKeyPressCount = 0;
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_POWER_KEY) || !strcmp(pKeyEvent->keyname, VP_SCREENSAVER_KEY)) {
		VideoLogWarning("POWER KEY");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			if (pPlayView->pHwKeyLongPressTimer) {
				VideoLogWarning("HW Key: It is click event!");
				VP_EVAS_TIMER_DEL(pPlayView->pHwKeyLongPressTimer);
				bool bLock = FALSE;
				vp_play_normal_view_get_lock_screen(pPlayView->pNormalView, &bLock);
				bLock = !bLock;
				vp_play_normal_view_set_lock_screen(pPlayView->pNormalView, bLock);
			}

			bool bPause = FALSE;
			vp_play_normal_view_get_pause_status(pPlayView->pNormalView, &bPause);
			if (bPause) {
				vp_play_util_set_unlock_power_key();
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_KEY_CANCLE) || !strcmp(pKeyEvent->keyname, "Esc")) {
		VideoLogWarning("CANCLE KEY");

		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
	} else if (!strcmp(pKeyEvent->keyname, VP_FORWARD_KEY)) {
		VideoLogWarning("Right");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			if (pPlayView->bKeySpeed == TRUE) {
				vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, TRUE, TRUE);
				pPlayView->bKeySpeed = FALSE;
			}
		}
	} else if (!strcmp(pKeyEvent->keyname, VP_REWIND_KEY)) {
		VideoLogWarning("Left");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			if (pPlayView->bKeySpeed == TRUE) {
				vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, TRUE, FALSE);
				pPlayView->bKeySpeed = FALSE;
			}
		}
	}

	/* send event to parent */
	if (pPlayView->pCallback == NULL) {
		VideoLogWarning("PlayView Callback is NULL");
		return EINA_FALSE;
	}

	if (pPlayView->pCallback->pEventKeyUp == NULL) {
		VideoLogWarning("PlayView pEventKeyUp Callback is NULL");
		return EINA_FALSE;
	}

	pPlayView->pCallback->pEventKeyUp(pPlayView->pUserData, nType, pEvent);

	return EINA_FALSE;
}

#ifdef OLD_SENSOR_API
static void __vp_play_view_sensor_event_cb(vp_sensor_type_t nType, unsigned long long timestamp, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	if (nType != VP_SENSOR_TYPE_FACEDOWN) {
		VideoLogWarning("Invalid sensor type");
		return;
	}

	video_sound_device_type_t nSoundDevType = VP_SOUND_DEVICE_NONE;
	vp_sound_get_active_device(&nSoundDevType);

	if (nSoundDevType == VP_SOUND_DEVICE_EARJACK || nSoundDevType == VP_SOUND_DEVICE_BLUETOOTH) {
		VideoLogWarning("Earjack or Bluetooth is connected. ignore sensor event");
		return;

	}

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_pause(pPlayView->pNormalView)) {
			VideoLogError("vp_play_normal_view_realize fail");
		}
		if (!vp_play_normal_view_set_manual_pause(pPlayView->pNormalView, TRUE)) {
			VideoLogError("vp_play_normal_view_set_manual_pause fail");
		}
	}
}
#endif

static void __vp_play_view_rotate_lock_key_changed_cb(system_settings_key_e key, void *pUserData)
{

	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;
	bool bLockState = FALSE;

	if (!vp_play_config_get_rotate_lock_status(&bLockState)) {
		VideoLogError("vp_play_config_get_rotate_lock_status is fail");
		return;
	}

	if (bLockState == FALSE) {
		elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, 0);
	} else {
		elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, -1);
	}
}

static void __vp_play_view_wifi_key_changed_cb(wifi_connection_state_e state, wifi_ap_h ap, void *user_data)
{
	VideoLogError("not supported");
}

static void __vp_play_view_delete_request_cb(void *pUserData, Evas_Object *pEvasObject, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	evas_object_smart_callback_del(pEvasObject, "delete,request", __vp_play_view_delete_request_cb);

	PlayView *pPlayView = (PlayView *)pUserData;
	VideoLogError("== delete request call ==");

	_vp_play_view_destroy_handle(pPlayView);

	elm_exit();
}

/* internal functions */
static void __vp_play_view_func_change_mode(void *pViewHandle, video_play_mode_t nMode, const char *szMediaURL, int nStartPos, bool bManualPause)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	if (nMode == pPlayView->nPlayMode) {
		VideoLogWarning("Skip for Same mode");
		return;
	}

	if (pPlayView->bViewChange) {
		VideoLogWarning("Skip for Same mode");
		return;
	}
	VideoLogWarning(">> Change mode");

	pPlayView->bViewChange = TRUE;

	VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);

	VP_FREE(pPlayView->szMediaURL);
	VP_STRDUP(pPlayView->szMediaURL, szMediaURL);

	/* destroy old view */
	bool bIsTempRealize = pPlayView->bRealized;
	vp_play_view_unrealize(pPlayView);

	if (nStartPos >= 0) {
		pPlayView->nStartPosition = nStartPos;
	}

	pPlayView->bStartPause = bManualPause;

	_vp_play_view_destroy_mode(pPlayView);

	//only for naviitem list test
	//int count = eina_list_count(elm_naviframe_items_get(pPlayView->pNaviframe));
	//VideoLogInfo("naviitems = %d", count);

	//bool bRender = FALSE;
	VideoLogInfo("previous model = %d,current view = %d", pPlayView->nPlayMode, nMode);

	pPlayView->nPlayMode = nMode;

	bool bLockState = FALSE;
	if (!vp_play_config_get_rotate_lock_status(&bLockState)) {
		VideoLogError("vp_play_config_get_rotate_lock_status is fail");
		return;
	}

	if (bLockState == FALSE) {
		// if auto rotation is locked. set player window as orientation 0
		elm_win_rotation_with_resize_set(pPlayView->pWin, APP_DEVICE_ORIENTATION_0);
		vp_play_view_set_rotate(pPlayView, VIDEO_PLAY_ROTATE_NONE);
	}

	/* create new view */
	if (!_vp_play_view_create_mode(pPlayView)) {
		VideoLogWarning("_vp_play_view_create_mode fail");
		return;
	}

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_set_url(pPlayView->pNormalView, pPlayView->szMediaURL)) {
			VideoLogError("vp_play_normal_view_set_url fail");
			return;
		}
		if (!vp_play_normal_view_set_sound_filter(pPlayView->pNormalView, pPlayView->nSoundAlive)) {
			VideoLogError("vp_play_normal_view_set_sound_filter handle is null");
			return;
		}

		if (!vp_play_normal_view_set_manual_pause(pPlayView->pNormalView, pPlayView->bStartPause)) {
			VideoLogError("vp_play_normal_view_set_manual_pause fail");
			return;
		}

		if (!vp_play_normal_view_set_rotate(pPlayView->pNormalView, pPlayView->nRotate, FALSE)) {
			VideoLogError("vp_play_normal_view_set_rotate fail");
			return;
		}
	}

	if (bIsTempRealize) {
		vp_play_view_realize(pPlayView);
	}

	pPlayView->bViewChange = FALSE;
	VideoLogWarning("== Complete View Change ==");

	//Dead-Code----bRender is false
	//if (bRender) {
	//	elm_win_norender_pop(pPlayView->pWin);
	//}
}

static void __vp_play_view_func_exit(void *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("play view handle is NULL");
		return;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->bManualAppPause = TRUE;

	/* launching videos
		Scenario 1 : Videos - Play - MultiWin - Idle - Change to PlayView(Maximize) - back => Launch Videos.
	*/
	if (pPlayView->bRelaunchCaller &&
	        pPlayView->bActivate == TRUE) {

		vp_play_app_launch_videos(pPlayView->pWin, TRUE, (void *)pPlayView);
	}
	__vp_play_view_destroy_cb((void *)pPlayView);
}


#ifdef NOT_USED
static void __vp_play_view_sensor_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;

	vp_play_sensor_popup_destroy(pPlayView->pSensorPopup);
	pPlayView->pSensorPopup = NULL;
}
#endif

#if 1
#endif

/* internal functions */
static void _vp_play_view_init_config(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_CUSTOM_THEME);
	free(path);
	elm_theme_extension_add(NULL, edj_path);
}

static void _vp_play_view_deinit_config(PlayView *pPlayView)
{
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_CUSTOM_THEME);
	free(path);

	elm_theme_extension_del(NULL, edj_path);
}

static void _vp_play_view_create_volume_timer(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);

	pPlayView->pVolumeTimer = ecore_timer_add(VP_PLAY_VIEW_VOLUME_TIMER,
	                          __vp_play_view_volume_timer_cb, (void *)pPlayView);

}

static Evas_Object *_vp_play_view_create_window(Evas_Object *pParent, const char *pName)
{
	Evas_Object *pObj = NULL;

	pObj = elm_win_add(pParent, pName, ELM_WIN_BASIC);
	if (pObj == NULL) {
		VideoLogError("elm_win_add object is NULL");
		return NULL;
	}

	elm_win_title_set(pObj, pName);
	evas_object_smart_callback_add(pObj, "delete,request",
	                               __vp_play_view_delete_request_cb, NULL);

	/*elm_win_indicator_mode_set(pObj, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(pObj, ELM_WIN_INDICATOR_TRANSPARENT);*/
	elm_win_indicator_mode_set(pObj, ELM_WIN_INDICATOR_HIDE);

	if (elm_win_wm_rotation_supported_get(pObj)) {
		const int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(pObj, rots, 4);
	}

	//evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_play_view_create_render_rect(Evas_Object *pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas *pEvas = evas_object_evas_get(pParent);
	Evas_Object *pObj = evas_object_rectangle_add(pEvas);
	if (pObj == NULL) {
		VideoLogError("render_rect object is NULL");
		return NULL;
	}

	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(pObj, 0, 0, 0, 0);
	evas_object_render_op_set(pObj, EVAS_RENDER_COPY);
	evas_object_show(pObj);
	elm_win_resize_object_add(pParent, pObj);

	return pObj;
}

static Evas_Object *_vp_play_view_create_conformant(Evas_Object *pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_conformant_add(pParent);
	if (pObj == NULL) {
		VideoLogError("elm_conformant_add object is NULL");
		return NULL;
	}

	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_conformant_set(pParent, EINA_TRUE);
	elm_win_resize_object_add(pParent, pObj);

	elm_object_signal_emit(pObj, "elm,state,indicator,overlap", "");
	evas_object_data_set(pObj, "overlap", (void *)EINA_TRUE);

	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_play_view_create_layout(Evas_Object *pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;
	Eina_Bool bRet = EINA_FALSE;

	pObj = elm_layout_add(pParent);
	if (!pObj) {
		VideoLogError("elm_layout_add object is NULL");
		return NULL;
	}

	bRet = elm_layout_theme_set(pObj, VP_PLAY_CUSTOM_LAYOUT_KLASS,
	                            VP_PLAY_CUSTOM_LAYOUT_GROUP,
	                            VP_PLAY_CUSTOM_LAYOUT_STYLE);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_theme_set fail");
	}
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(pParent, pObj);

	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_play_view_create_naviframe(Evas_Object *pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;
	pObj = elm_naviframe_add(pParent);
	if (!pObj) {
		VideoLogError("elm_naviframe_add object is NULL");
		return NULL;
	}

	eext_object_event_callback_add(pObj, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(pObj, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	elm_object_part_content_set(pParent, "elm.swallow.content", pObj);
	elm_object_focus_set(pObj, EINA_TRUE);

	elm_naviframe_prev_btn_auto_pushed_set(pObj, EINA_FALSE);

	evas_object_show(pObj);

	return pObj;
}

static bool _vp_play_view_init_layout(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return FALSE;
	}
	VideoLogWarning(">> Play view : Init layout");

	pPlayView->pWin = _vp_play_view_create_window(pPlayView->pParent, VP_PLAY_MAIN_WINDOW_NAME);
	if (pPlayView->pWin == NULL) {
		VideoLogError("pWin is null");
		return FALSE;
	}

	//elm_win_alpha_set(pPlayView->pWin, EINA_TRUE);
	//elm_win_shaped_set(pPlayView->pWin, EINA_TRUE);

	evas_object_smart_callback_add(pPlayView->pWin, "wm,rotation,changed", __vp_play_rotate_changed_cb, pPlayView);

	pPlayView->pRenderRect = _vp_play_view_create_render_rect(pPlayView->pWin);
	if (pPlayView->pRenderRect == NULL) {
		VideoLogError("pRenderRect is null");
		return FALSE;
	}
#ifdef _PERF_TEST_
	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		// Move to : NormalView / Realize.
	} else
#endif
	{
		pPlayView->pConformant = _vp_play_view_create_conformant(pPlayView->pWin);
		if (pPlayView->pConformant == NULL) {
			VideoLogError("pConformant is null");
			return FALSE;
		}

		pPlayView->pMainLayout = _vp_play_view_create_layout(pPlayView->pConformant);
		if (pPlayView->pMainLayout == NULL) {
			VideoLogError("pMainLayout is null");
			return FALSE;
		}
		pPlayView->pNaviframe = _vp_play_view_create_naviframe(pPlayView->pMainLayout);
		if (pPlayView->pNaviframe == NULL) {
			VideoLogError("pNaviframe is null");
			return FALSE;
		}
	}

	/*rotation check*/
	VideoLogInfo("<< Play view : Checking initial rotation!");

	int nAngle = elm_win_rotation_get((Evas_Object *)pPlayView->pWin);
	video_play_rotate_t nRotate = VIDEO_PLAY_ROTATE_NONE;

	switch (nAngle) {
	case 0:
		nRotate = VIDEO_PLAY_ROTATE_NONE;
		break;
	case 90:
		nRotate = VIDEO_PLAY_ROTATE_90;
		break;
	case 180:
		nRotate = VIDEO_PLAY_ROTATE_180;
		break;
	case 270:
		nRotate = VIDEO_PLAY_ROTATE_270;
		break;
	}

	if (pPlayView->nRotate != nRotate) {
		vp_play_view_set_rotate(pPlayView, nRotate);
	}
	return TRUE;
}

static bool _vp_play_view_create_mode(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return FALSE;
	}

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		pPlayView->pNormalView = vp_play_normal_view_create(pPlayView, pPlayView->nLaunchingType);
		if (pPlayView->pNormalView == NULL) {
			VideoLogError("vp_play_normal_view_create handle is null");
			return FALSE;
		}
	}

	return TRUE;
}

static bool _vp_play_view_destroy_mode(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return FALSE;
	}

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_unrealize(pPlayView->pNormalView)) {
			VideoLogError("vp_play_normal_view_unrealize fail");
			return FALSE;
		}

		vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_UP_KEY);
		vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_DOWN_KEY);
		vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_MUTE);

		vp_play_normal_view_destroy(pPlayView->pNormalView);
		pPlayView->pNormalView = NULL;
	}

	return TRUE;
}

static void _vp_play_view_create_callback(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return;
	}


	pPlayView->pCallback = (PlayViewCbFunc *)calloc(1, sizeof(PlayViewCbFunc));
	if (pPlayView->pCallback == NULL) {
		VideoLogError("playview Callback alloc fail");
		return;
	}

	pPlayView->pCallback->pRealizeCb = NULL;
	pPlayView->pCallback->pDestroyCb = NULL;
	pPlayView->pCallback->pEventKeyDown = NULL;
	pPlayView->pCallback->pEventKeyUp = NULL;

	return;
}

static void _vp_play_view_destroy_callback(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return;
	}

	if (!pPlayView->pCallback) {
		VideoLogError("play view Callback is NULL");
		return;
	}

	pPlayView->pCallback->pRealizeCb = NULL;
	pPlayView->pCallback->pDestroyCb = NULL;
	pPlayView->pCallback->pEventKeyDown = NULL;
	pPlayView->pCallback->pEventKeyUp = NULL;

	VP_FREE(pPlayView->pCallback);
}

static void _vp_play_view_create_func(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return;
	}


	pPlayView->pFunc = (PlayViewFunc *)calloc(1, sizeof(PlayViewFunc));
	if (pPlayView->pFunc == NULL) {
		VideoLogError("playview func alloc fail");
		return;
	}

	pPlayView->pFunc->vp_play_func_change_mode = __vp_play_view_func_change_mode;
	pPlayView->pFunc->vp_play_func_exit = __vp_play_view_func_exit;

}

static void _vp_play_view_destroy_func(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return;
	}

	if (!pPlayView->pFunc) {
		VideoLogError("play view func is NULL");
		return;
	}

	pPlayView->pFunc->vp_play_func_change_mode = NULL;
	pPlayView->pFunc->vp_play_func_exit = NULL;

	VP_FREE(pPlayView->pFunc);
}

static void _vp_play_view_destroy_handle(PlayView *pPlayView)
{
	if (!pPlayView) {
		VideoLogError("play view handle is NULL");
		return;
	}

	_vp_play_view_destroy_callback(pPlayView);
	_vp_play_view_destroy_func(pPlayView);
#ifdef OLD_SENSOR_API
	vp_sensor_destroy(pPlayView->pSensorHandle);
	pPlayView->pSensorHandle = NULL;

	vp_play_sensor_popup_destroy(pPlayView->pSensorPopup);
	pPlayView->pSensorPopup = NULL;
#endif

	VP_EVAS_TIMER_DEL(pPlayView->pDbUpdateTimer);
	VP_EVAS_TIMER_DEL(pPlayView->pSbeamTimer);
	VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
	VP_EVAS_TIMER_DEL(pPlayView->pHwKeyLongPressTimer);
	VP_EVAS_TIMER_DEL(pPlayView->pMediaKeyTimer);
	VP_EVAS_TIMER_DEL(pPlayView->pMediaLongPressTimer);
	VP_EVAS_IDLER_DEL(pPlayView->pHideIdler);

	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pKeyEventDown);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pKeyEventUp);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pFocusIn);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pFocusOut);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pVisiblility);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pClientMessage);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pDeleteRequest);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pPropertyHandler);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pMouseInHandler);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pMouseOutHandler);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pPalmHoldEventHandler);
	VP_EVAS_EVENT_HANDLER_DEL(pPlayView->pCallerWinEventHandler);

	VP_EVAS_PIPE_DEL(pPlayView->pNotiPipe);

	if (pPlayView->pNormalView) {
		vp_play_normal_view_destroy(pPlayView->pNormalView);
		pPlayView->pNormalView = NULL;
	}

	VP_EVAS_DEL(pPlayView->pMainLayout);
	VP_EVAS_DEL(pPlayView->pNaviframe);
	VP_EVAS_DEL(pPlayView->pConformant);
	VP_EVAS_DEL(pPlayView->pRenderRect);
	VP_EVAS_DEL(pPlayView->pBackground);
	VP_EVAS_DEL(pPlayView->pWin);

	VP_FREE(pPlayView->szSubtitleURL);
	VP_FREE(pPlayView->szMediaURL);
	VP_FREE(pPlayView->szDeviceID);
	VP_FREE(pPlayView->szAutoPlay);
	VP_FREE(pPlayView->szDMRID);
	VP_FREE(pPlayView->szCookie);
	VP_FREE(pPlayView->szProxy);

	_vp_play_view_deinit_config(pPlayView);

	free(pPlayView);
	pPlayView = NULL;
}

static void _vp_play_view_net_status_changed_cb(connection_type_e type, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	PlayView *pPlayView = (PlayView *)pUserData;
	int net_status = type;

	VideoSecureLogInfo("Changed : NETWORK_STATUS(%d)", net_status);

	if (pPlayView->nLaunchingType != VIDEO_PLAY_TYPE_WEB) {
		return;
	}

	if (vp_play_util_rtsp_url_check((const char*)pPlayView->szMediaURL) == FALSE) {
		VideoLogWarning("Not a RTSP/RTP url. skip wifi key changed signal");
		return;
	} else {
		VideoLogWarning("RTSP/RTP url");
	}
	/*
	   Network change info
	   WIFI(2) -> OFF(0) -> CELLULAR(1)
	   CELLULAR(1) -> WIFI(2)
	*/
	if (net_status == CONNECTION_TYPE_DISCONNECTED) {
		VideoSecureLogInfo("network off");
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			VideoLogInfo("normal view disconnect");
			vp_play_normal_view_web_type_disconnect(pPlayView->pNormalView);
		}
	} else if (net_status == CONNECTION_TYPE_CELLULAR) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			VideoLogInfo("normal view reconnection");
			vp_play_normal_view_web_type_reconnect(pPlayView->pNormalView);
		}
	} else if (net_status == CONNECTION_TYPE_WIFI) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			VideoLogInfo("normal view reconnection");
			vp_play_normal_view_web_type_reconnect(pPlayView->pNormalView);
		}
	}
}

static void __vp_play_view_holl_ic_changed_cb(bool bCover_state, void *pUserData)
{
	// bCover_state : TRUE(Cover Opened), bCover_state : FALSE(Cover closed)
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pUserData;
	if (pPlayView->bActivate == FALSE) {
		return;
	}
	VideoLogWarning("bCover_state : %d", bCover_state);

	if (bCover_state == FALSE) {
		if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
			if (!vp_play_normal_view_pause(pPlayView->pNormalView)) {
				VideoLogError("vp_play_normal_view_realize fail");
			}
			if (!vp_play_normal_view_set_manual_pause(pPlayView->pNormalView, TRUE)) {
				VideoLogError("vp_play_normal_view_set_manual_pause fail");
			}
		}
	}
}

static void _vp_play_view_ignore_callbacks(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	PlayView *pPlayView = (PlayView *)pUserData;
	if (pPlayView->connection) {
		if (connection_unset_type_changed_cb(pPlayView->connection) != CONNECTION_ERROR_NONE) {
			VideoLogError("failed to unregister network status callback");
		}

		if (connection_destroy(pPlayView->connection) != CONNECTION_ERROR_NONE) {
			VideoLogError("failed to destroy connection handle");
		}
		pPlayView->connection = NULL;
	}
	vp_play_config_ignore_rotate_state_key_changed_cb();
	vp_play_config_ignore_battery_charge_state_key_changed_cb();
	if (pPlayView->bWifi) {
		vp_play_config_ignore_wifi_state_key_changed_cb();
	}
}

static Eina_Bool __vp_play_view_db_update_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return ECORE_CALLBACK_CANCEL;
	}
	PlayView *pPlayView = (PlayView *)pUserData;
	pPlayView->pDbUpdateTimer = NULL;

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		vp_play_normal_view_db_change(pPlayView->pNormalView);
	}

	return ECORE_CALLBACK_CANCEL;
}

static void __vp_playview_media_update_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	PlayView *pPlayView = (PlayView *)pUserData;

	VP_EVAS_TIMER_DEL(pPlayView->pDbUpdateTimer);
	pPlayView->pDbUpdateTimer = ecore_timer_add(VP_PLAY_VIEW_DB_UPDATE_TIMER, __vp_play_view_db_update_timer_cb, (void *)pUserData);
}

play_view_handle vp_play_view_create(Evas_Object *pParent, Ecore_X_Window nParentXID, video_play_mode_t nMode, video_play_launching_type_t nLaunchingType)
{
	if (!pParent) {
		VideoLogWarning("pParent handle is NULL");
	}

	VideoLogWarning("");

	PlayView *pPlayView = NULL;

	pPlayView = calloc(1, sizeof(PlayView));
	if (!pPlayView) {
		VideoLogError("PlayView alloc fail");
		return NULL;
	}

#if 0//Tizen3.0 Build error
//Instead specify h/w acceleration in the application xml.
#ifndef _USE_PROCESS_POOL

#ifdef TARGET
	VideoLogWarning("opengl_x11");
	elm_config_preferred_engine_set("opengl_x11");
#else
	VideoLogWarning("software_x11");
	elm_config_preferred_engine_set("software_x11");
#endif

#endif
#endif

	pPlayView->pParent = pParent;
	pPlayView->nPlayMode = nMode;
	pPlayView->nLaunchingType = nLaunchingType;

	/* init preference */
	vp_play_preference_init();

	/* set custom theme */
	_vp_play_view_init_config(pPlayView);

	/* create callback */
	_vp_play_view_create_callback(pPlayView);

	/* create internal function */
	_vp_play_view_create_func(pPlayView);

	if (!_vp_play_view_init_layout(pPlayView)) {
		VideoLogError("_vp_play_view_init_layout is fail");
		_vp_play_view_destroy_handle(pPlayView);
		return NULL;
	}

#if 0//Tizen3.0 Build error
	pPlayView->nXwinID = elm_win_xwindow_get(pPlayView->pWin);
#endif
	pPlayView->fPlaySpeed = 1.0;

#if 0//Tizen3.0 Build error
	if (nParentXID > 0) {
		ecore_x_icccm_transient_for_set(pPlayView->nXwinID, nParentXID);
		ecore_x_window_client_manage(nParentXID);
		pPlayView->nCallerXwinID = nParentXID;
		pPlayView->pCallerWinEventHandler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, (void *)__vp_play_caller_window_event_cb, (void *)pPlayView);
	}
#endif

	pPlayView->bActivate = TRUE;

	//elm_win_activate(pPlayView->pWin);

	if (!_vp_play_view_create_mode(pPlayView)) {
		VideoLogError("_vp_play_view_handle_create is fail");
		_vp_play_view_destroy_handle(pPlayView);
		return NULL;
	}

	pPlayView->pKeyEventDown = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, (void *)__vp_play_view_event_key_down_cb, (void *)pPlayView);
	pPlayView->pKeyEventUp = ecore_event_handler_add(ECORE_EVENT_KEY_UP, (void *)__vp_play_view_event_key_up_cb, (void *)pPlayView);
#if 0//Tizen3.0 Build error
	pPlayView->pFocusIn = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, __vp_play_focus_in_cb, (void *)pPlayView);
	pPlayView->pFocusOut = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, __vp_play_focus_out_cb, (void *)pPlayView);
	pPlayView->pVisiblility = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, __vp_play_visibility_cb, (void *)pPlayView);
	pPlayView->pClientMessage = ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, __vp_play_client_message_cb, (void *)pPlayView);
	pPlayView->pDeleteRequest = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, __vp_play_delete_request_cb, (void *)pPlayView);
	pPlayView->pPropertyHandler = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY, __vp_play_view_window_property_change, (void *)pPlayView);
	pPlayView->pMouseInHandler = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_IN, __vp_play_view_mouse_in_cb, (void *)pPlayView);
	pPlayView->pMouseOutHandler = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_OUT, __vp_play_view_mouse_out_cb, (void *)pPlayView);
#endif

	pPlayView->pNotiPipe = ecore_pipe_add((Ecore_Pipe_Cb)__vp_play_view_noti_pipe_cb, (void *)pPlayView);
#ifdef OLD_SENSOR_API
	pPlayView->pSensorHandle = vp_sensor_create(__vp_play_view_sensor_event_cb);
	if (pPlayView->pSensorHandle) {
		vp_sensor_set_user_data(pPlayView->pSensorHandle, (void *)pPlayView);
	}
#endif

	pPlayView->bAVRCP = vp_avrcp_initialize();
	pPlayView->bWifi = vp_play_wifi_initialize();

	bool bWifiConnect = FALSE;
	vp_play_config_get_wifi_direct_connect_status(&bWifiConnect);

	vp_media_contents_set_update_cb(__vp_playview_media_update_cb, (void *)pPlayView);

	return pPlayView;
}

#ifdef _PERF_TEST_
void vp_play_view_reset(play_view_handle pViewHandle, int nMode)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	if (nMode != pPlayView->nPlayMode) {
		VideoLogError("Another mode(%d), reset playview", nMode);
		vp_play_view_destroy(pViewHandle);
	}
}
#endif

void vp_play_view_destroy(play_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return;
	}

	VideoLogWarning("");

	PlayView *pPlayView = (PlayView *)pViewHandle;

	vp_play_config_set_multi_play_status(FALSE);

	pPlayView->bDestroy = TRUE;

	vp_media_contents_unset_update_cb();

	vp_play_view_unrealize(pPlayView);

	if (pPlayView->pMultiPathList) {
		vp_multi_path_clear_item(pPlayView->pMultiPathList);
		g_list_free(pPlayView->pMultiPathList);
		pPlayView->pMultiPathList = NULL;
	}

	_vp_play_view_destroy_handle(pPlayView);

	VideoLogInfo("<< Play View : Destroy");
}

bool vp_play_view_live_stream_realize(play_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	nState = vp_controller_normal_view_get_played_state(pPlayView->pNormalView);
	if (nState == VP_MM_PLAYER_STATE_IDLE) {
		return TRUE;
	}
	int nDuration = 0;
	if (vp_play_normal_view_get_video_duration(pPlayView->pNormalView, &nDuration)) {
		pPlayView->nDuration = nDuration;
	}
	VideoLogInfo(">> Play View : %d",pPlayView->nDuration);
	if (pPlayView->nDuration == 0) {
		vp_play_normal_view_play_start(pPlayView->pNormalView);
		return TRUE;
	}else{
		if(pPlayView->bIsPlayBeforeShare)
		{
			vp_play_normal_view_set_resume_or_pause(pPlayView->pNormalView);
			return TRUE;
		}
	}
	return TRUE;
}

bool vp_play_view_realize(play_view_handle pViewHandle)
{
        int error = SOUND_MANAGER_ERROR_NONE;
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VideoLogInfo(">> Play View : Realize");

	pPlayView->bActivate = TRUE;
	pPlayView->bVisible = TRUE;
	vp_play_normal_view_key_create(pPlayView, pPlayView->pNormalView);
	vp_play_normal_view_set_share_panel_state(pPlayView->pNormalView);

	sound_stream_focus_state_e state_for_playback;
	sound_stream_focus_state_e state_for_recording;
	int ret = -1;
	ret = sound_manager_get_focus_state(pPlayView->stream_info, &state_for_playback, &state_for_recording);
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("failed to get focus state error[%x]", ret);
       	}
	if (state_for_playback == SOUND_STREAM_FOCUS_STATE_RELEASED) {
		error = sound_manager_acquire_focus(pPlayView->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (error != SOUND_MANAGER_ERROR_NONE) {
			VideoLogError("failed to acquire focus [%x]", error);
		}
		sound_manager_get_focus_reacquisition(pPlayView->stream_info, &pPlayView->reacquire_state);
		if (pPlayView->reacquire_state == EINA_FALSE)
			sound_manager_set_focus_reacquisition(pPlayView->stream_info, EINA_TRUE);
	}
	if (pPlayView->bRealized == TRUE) {
		VideoLogWarning("Already Realize state");
		//return TRUE;
	}

	/*unregister callbacks*/
	_vp_play_view_ignore_callbacks(pPlayView);

	if (pPlayView->pHollICHandle) {
		vp_hollic_destroy_handle(pPlayView->pHollICHandle);
		pPlayView->pHollICHandle = NULL;
	}
	pPlayView->pHollICHandle = vp_hollic_create_handle(__vp_play_view_holl_ic_changed_cb, (void *)pPlayView);
#ifdef OLD_SENSOR_API
	vp_sensor_realize(pPlayView->pSensorHandle);
#endif
	vp_sound_init_session(pViewHandle);

	vp_sound_set_route_change_cb(__vp_play_sound_route_change_cb, (void *)pPlayView);
	vp_sound_set_volume_change_cb(__vp_play_sound_volume_change_cb, (void *)pPlayView);
	if (!pPlayView->connection) {
		int ret = connection_create(pPlayView->connection);
		if (ret != CONNECTION_ERROR_NONE || !pPlayView->connection) {
			VideoLogWarning("Fail to create connection handle [%d]\n", ret);
		}

		if (pPlayView->connection) {
			ret = connection_set_type_changed_cb(pPlayView->connection,
			                                     _vp_play_view_net_status_changed_cb,
			                                     (void *)pPlayView);
			if (ret != CONNECTION_ERROR_NONE) {
				VideoLogWarning("Fail to register network status callback [%d]\n", ret);
			}
		}
	}
	vp_play_config_set_rotate_state_key_changed_cb(__vp_play_view_rotate_lock_key_changed_cb, (void *)pPlayView);
	if (pPlayView->bWifi) {
		vp_play_config_set_wifi_state_key_changed_cb(__vp_play_view_wifi_key_changed_cb, (void *)pPlayView);
	}

//	int nColorTone = 0;
//	bool bOutdoor = FALSE;

//	vp_play_config_get_color_tone_key(&nColorTone);
//	vp_play_config_get_outdoor_key(&bOutdoor);

//	vp_device_display_init(nColorTone, (int)bOutdoor);
	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_UP_KEY);
		vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_DOWN_KEY);
		vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_MUTE);

		vp_play_util_key_grab(pPlayView->pWin, VP_PLAYPAUSE_KEY);
		vp_play_util_key_grab(pPlayView->pWin, VP_PLAY_NEXT_SONG_KEY);
		vp_play_util_key_grab(pPlayView->pWin, VP_PLAY_PREV_SONG_KEY);

		vp_play_util_key_grab(pPlayView->pWin, VP_POWER_KEY);

		bool bVideoOnly = FALSE;
		VideoLogWarning("video output: internal display");
		vp_play_preference_set_allow_dock_connect_key(FALSE);

		if (pPlayView->szDMRID) {
			if (!vp_play_normal_view_set_manual_pause((normal_view_handle)pPlayView->pNormalView, TRUE)) {
				VideoLogError("vp_play_normal_view_set_start_position fail");
				return FALSE;
			}
			if (!vp_play_normal_view_set_video_only(pPlayView->pNormalView, TRUE)) {
				VideoLogError("vp_play_normal_view_set_video_only fail");
				return FALSE;
			}
		} else {
			if (!vp_play_normal_view_set_video_only(pPlayView->pNormalView, bVideoOnly)) {
				VideoLogError("vp_play_normal_view_set_video_only fail");
				return FALSE;
			}
		}

		if (!vp_play_normal_view_set_start_position((normal_view_handle)pPlayView->pNormalView, pPlayView->nStartPosition)) {
			VideoLogError("vp_play_normal_view_set_start_position fail");
			return FALSE;
		}

		if (!vp_play_normal_view_set_audio_track((normal_view_handle)pPlayView->pNormalView, pPlayView->nAudioIndex)) {
			VideoLogError("vp_play_normal_view_set_audio_track fail");
			return FALSE;
		}

		if (!vp_play_normal_view_set_subtitle((normal_view_handle)pPlayView->pNormalView, pPlayView->szSubtitleURL)) {
			VideoLogInfo("vp_play_normal_view_set_subtitle fail");
		}

		if (!vp_play_normal_view_realize((normal_view_handle)pPlayView->pNormalView)) {
			VideoLogError("vp_play_normal_view_realize fail");
			vp_play_view_unrealize(pPlayView);
			return FALSE;
		}
	}

	pPlayView->bRealized = TRUE;

	__vp_play_view_realize_cb((void *)pPlayView);

	VideoLogWarning("<< Play View : Realize");

	return TRUE;
}

bool vp_play_view_unrealize(play_view_handle pViewHandle)
{
        int error = SOUND_MANAGER_ERROR_NONE;
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->bActivate = FALSE;

	if (pPlayView->bRealized == FALSE) {
		VideoLogInfo("Already unrealized : playview");
//		return TRUE;
	}

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		vp_play_preference_set_allow_dock_connect_key(TRUE);

		VP_EVAS_TIMER_DEL(pPlayView->pVolumeTimer);
		VP_EVAS_TIMER_DEL(pPlayView->pHwKeyLongPressTimer);
		VP_EVAS_TIMER_DEL(pPlayView->pMediaKeyTimer);
		VP_EVAS_TIMER_DEL(pPlayView->pMediaLongPressTimer);

		vp_play_util_key_ungrab(pPlayView->pWin, VP_POWER_KEY);

		vp_play_util_key_ungrab(pPlayView->pWin, VP_PLAYPAUSE_KEY);
		vp_play_util_key_ungrab(pPlayView->pWin, VP_PLAY_NEXT_SONG_KEY);
		vp_play_util_key_ungrab(pPlayView->pWin, VP_PLAY_PREV_SONG_KEY);

		vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_UP_KEY);
		vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_DOWN_KEY);
		vp_play_util_key_ungrab(pPlayView->pWin, VP_VOLUME_MUTE);
		vp_play_util_key_ungrab(pPlayView->pWin, VP_HOME_KEY);
		sound_manager_unset_current_sound_type();
		sound_stream_focus_state_e state_for_playback;
		sound_stream_focus_state_e state_for_recording;
		int ret = -1;
		ret = sound_manager_get_focus_state(pPlayView->stream_info, &state_for_playback, &state_for_recording);
		if (ret != SOUND_MANAGER_ERROR_NONE) {
			VideoLogError("failed to get focus state error[%x]", ret);
        	}
		if (state_for_playback != SOUND_STREAM_FOCUS_STATE_RELEASED) {
			error = sound_manager_release_focus(pPlayView->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
			if (error != SOUND_MANAGER_ERROR_NONE) {
				VideoLogError("failed to release focus error[%x]", error);
	        	}
		}
		media_key_release();
		int nDuration = 0;
		if (vp_play_normal_view_get_video_duration(pPlayView->pNormalView, &nDuration)) {
			pPlayView->nDuration = nDuration;
		}
		if (pPlayView->nDuration == 0) {
			if (!vp_play_normal_view_stop_player(pPlayView->pNormalView)) {
				VideoLogWarning("stop failed");
			}
		}
	}

	if (pPlayView->bDestroy == FALSE) {
		VideoLogWarning("Skip for screen mirroring mode");
		//return TRUE;
	}

	pPlayView->bRealized = FALSE;
	vp_hollic_destroy_handle(pPlayView->pHollICHandle);
	pPlayView->pHollICHandle = NULL;
#ifdef OLD_SENSOR_API
	vp_sensor_unrealize(pPlayView->pSensorHandle);
#endif

#if 1
//	vp_device_display_deinit();
#endif

	vp_sound_deinit_session(pPlayView);

	if (pPlayView->bAVRCP) {
		vp_avrcp_noti_player_state(VP_MM_PLAYER_STATE_STOP);
		vp_avrcp_deinitialize();
		pPlayView->bAVRCP = FALSE;
	}

	_vp_play_view_ignore_callbacks(pPlayView);

	if (pPlayView->bWifi) {
		vp_play_wifi_deinitialize();
		pPlayView->bWifi = FALSE;
	}

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		int nCurPos = 0;
		int nAudioIndex = 0;
		video_sound_alive_t nSoundAlive = VIDEO_SA_NONE;

		if (vp_play_normal_view_get_last_position(pPlayView->pNormalView, &nCurPos)) {
			pPlayView->nStartPosition = nCurPos;
		}

		if (vp_play_normal_view_get_sound_filter(pPlayView->pNormalView, &nSoundAlive)) {
			pPlayView->nSoundAlive = nSoundAlive;
		}

		if (vp_play_normal_view_get_audio_track(pPlayView->pNormalView, &nAudioIndex)) {
			pPlayView->nAudioIndex = nAudioIndex;
		}

		VP_FREE(pPlayView->szSubtitleURL);
		//vp_play_normal_view_get_subtitle(pPlayView->pNormalView, &(pPlayView->szSubtitleURL));

		VideoLogInfo("vp_play_normal_view_unrealize START");
		if (!vp_play_normal_view_unrealize(pPlayView->pNormalView)) {
			VideoLogError("vp_play_normal_view_unrealize fail");
		}
	}

	VideoLogWarning("<< ----------");

	return TRUE;
}
bool vp_play_view_is_realize(play_view_handle pViewHandle, bool *bRealized)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;
	bool bRealize = FALSE;

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_is_realize(pPlayView->pNormalView, &bRealize)) {
			VideoLogError("vp_play_normal_view_is_realize is fail");
			return FALSE;
		}
	}

	*bRealized = bRealize;
	return TRUE;
}

bool vp_play_view_set_user_data(play_view_handle pViewHandle, void *pUserData)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->pUserData = pUserData;

	return TRUE;
}

bool vp_play_view_set_url(play_view_handle pViewHandle, const char *szMediaURL)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_FREE(pPlayView->szMediaURL);
	char *szConvert = vp_play_util_remove_prefix_to_url((char *)szMediaURL);
	if (szConvert) {
		VP_STRDUP(pPlayView->szMediaURL, szConvert);
	} else {
		VP_STRDUP(pPlayView->szMediaURL, szMediaURL);
	}
	VP_FREE(szConvert);

	char *szPreviewURL = NULL;
	vp_play_config_get_preview_url_videos(&szPreviewURL);
	if (szPreviewURL) {
		if (strcmp(szPreviewURL, szMediaURL) == 0) {
			int nAudioTrack = 0;
			vp_play_preference_get_preview_audio_track(&nAudioTrack);
			pPlayView->nAudioIndex = nAudioTrack;
		}
	}
	VP_FREE(szPreviewURL);

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_set_url(pPlayView->pNormalView, pPlayView->szMediaURL)) {
			VideoLogError("vp_play_normal_view_set_url fail");
			return FALSE;
		}
	}

	return TRUE;
}

bool vp_play_view_get_url(play_view_handle pViewHandle, char **szMediaURL)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_STRDUP(*szMediaURL, pPlayView->szMediaURL);

	return TRUE;
}

bool vp_play_view_get_visible_status(play_view_handle pViewHandle, bool *bVisible)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	*bVisible = pPlayView->bVisible;

	return TRUE;
}

bool vp_play_view_add_multi_path(play_view_handle pViewHandle,
                                 const char *szURL,
                                 char *szTitle,
                                 char *szSubTitle,
                                 int nPosition,
                                 int nDuration,
                                 bool bIsSameAP)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	if (!szURL) {
		VideoLogError("szURL is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VideoSecureLogInfo("URL[%s],SubTitle[%s],Pos[%d], Duration[%d], Title[%s], SameAP[%d]", szURL, szSubTitle, nPosition, nDuration, szTitle, (int)bIsSameAP);

	if (!vp_multi_path_add_item(&(pPlayView->pMultiPathList), szURL, szTitle, szSubTitle, nPosition, nDuration, bIsSameAP)) {
		VideoLogError("vp_multi_path_add_item is fail");
		return FALSE;
	}

	return TRUE;
}

bool vp_play_view_set_device_id(play_view_handle pViewHandle, const char *szDeviceID)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	if (!szDeviceID) {
		VideoLogError("szDeviceID is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_FREE(pPlayView->szDeviceID);
	VP_STRDUP(pPlayView->szDeviceID, szDeviceID);
	return TRUE;
}

bool vp_play_view_set_auto_play_setting(play_view_handle pViewHandle, const char *autoplay)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	if (!autoplay) {
		VideoLogError("autoplay is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;
	VP_FREE(pPlayView->szAutoPlay);
	VP_STRDUP(pPlayView->szAutoPlay, autoplay);
	return TRUE;
}

bool vp_play_view_set_dmr_id(play_view_handle pViewHandle, const char *szDMRID)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	if (!szDMRID) {
		VideoLogError("szDMRID is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_FREE(pPlayView->szDMRID);
	VP_STRDUP(pPlayView->szDMRID, szDMRID);

	return TRUE;
}

bool vp_play_view_set_media_title(play_view_handle pViewHandle, const char *szMediaTitle)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	if (!szMediaTitle) {
		VideoLogError("szMediaTitle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_FREE(pPlayView->szMediaTitle);
	VP_STRDUP(pPlayView->szMediaTitle, szMediaTitle);

	return TRUE;
}

bool vp_play_view_set_start_position(play_view_handle pViewHandle, int nPosition)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->nStartPosition = nPosition;

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_set_start_position(pPlayView->pNormalView, nPosition)) {
			VideoLogError("vp_play_normal_view_set_url fail");
			return FALSE;
		}
	}

	return TRUE;
}

bool vp_play_view_set_rotate(play_view_handle pViewHandle, video_play_rotate_t nRotate)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_set_rotate(pPlayView->pNormalView, nRotate, TRUE)) {
			VideoLogError("vp_play_normal_view_set_rotate fail");
			return FALSE;
		}
	}

	pPlayView->nRotate = nRotate;

	return TRUE;
}

bool vp_play_view_get_popup_mode(play_view_handle pViewHandle, bool *bPopupMode)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	*bPopupMode = FALSE;

	return TRUE;
}

bool vp_play_view_set_relaunch_caller(play_view_handle pViewHandle, bool bRelaunch)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->bRelaunchCaller = bRelaunch;

	return TRUE;
}

bool vp_play_view_set_launching_mode(play_view_handle pViewHandle, video_play_launching_type_t nLaunchingType)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->nLaunchingType = nLaunchingType;
	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		if (!vp_play_normal_view_set_launching_mode(pPlayView->pNormalView, nLaunchingType)) {
			VideoLogError("vp_play_normal_view_set_launching_mode is fail");
			return FALSE;
		}
	}

	return TRUE;
}

bool vp_play_view_set_list_mode(play_view_handle pViewHandle, video_play_list_type_t nListType)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->nListType = nListType;

	return TRUE;
}

bool vp_play_view_set_cookie(play_view_handle pViewHandle, const char *szCookie)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_FREE(pPlayView->szCookie);

	VP_STRDUP(pPlayView->szCookie, szCookie);

	return TRUE;
}

bool vp_play_view_set_proxy(play_view_handle pViewHandle, const char *szProxy)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_FREE(pPlayView->szProxy);

	VP_STRDUP(pPlayView->szProxy, szProxy);

	return TRUE;
}

bool vp_play_view_set_store_download_mode(play_view_handle pViewHandle, bool bStoreDownload)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->bStoreDownload = bStoreDownload;

	return TRUE;
}

bool vp_play_view_set_stop_after_repeat_mode(play_view_handle pViewHandle, bool bStopAfterRepeat)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->bStopAfterRepeat = bStopAfterRepeat;

	return TRUE;
}

bool vp_play_view_set_store_download_filesize(play_view_handle pViewHandle, double dStoreDL_FileSize)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->dStoreDL_FileSize = dStoreDL_FileSize;

	return TRUE;
}

bool vp_play_view_set_store_order_info(play_view_handle pViewHandle, char *pStoreOrderId, char *pStoreServerId, char *pStoreAppId, char *pStoreMvId)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	if (!pStoreOrderId) {
		VideoLogError("pStoreOrderId is NULL");
		return FALSE;
	}

	if (!pStoreServerId) {
		VideoLogError("pStoreServerId is NULL");
		return FALSE;
	}

	if (!pStoreAppId) {
		VideoLogError("pStoreAppId is NULL");
		return FALSE;
	}

	if (!pStoreMvId) {
		VideoLogError("pStoreMvId is NULL");
		return FALSE;
	}

	VideoSecureLogInfo("pStoreOrderId : [%s], pStoreServerId : [%s], pStoreAppId : [%s], pStoreMvId : [%s]", pStoreOrderId, pStoreServerId, pStoreAppId, pStoreMvId);


	PlayView *pPlayView = (PlayView *)pViewHandle;

	VP_FREE(pPlayView->pStoreOrderId);
	VP_STRDUP(pPlayView->pStoreOrderId, pStoreOrderId);

	VP_FREE(pPlayView->pStoreServerId);
	VP_STRDUP(pPlayView->pStoreServerId, pStoreServerId);

	VP_FREE(pPlayView->pStoreAppId);
	VP_STRDUP(pPlayView->pStoreAppId, pStoreAppId);

	VP_FREE(pPlayView->pStoreMvId);
	VP_STRDUP(pPlayView->pStoreMvId, pStoreMvId);

	return TRUE;
}

bool vp_play_view_update(play_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	if (pPlayView->nPlayMode == VIDEO_PLAY_MODE_NORMAL_VIEW) {
		vp_play_normal_view_update(pPlayView->pNormalView);
	}

	return TRUE;
}

bool vp_play_view_set_callback(play_view_handle pViewHandle, video_play_callback_type_t nCbType, void *pFunc)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	if (pPlayView->pCallback == NULL) {
		VideoLogError("PlayView Callback is NULL");
		return FALSE;
	}

	if (nCbType == VIDEO_PLAY_CB_TYPE_REALIZE) {
		pPlayView->pCallback->pRealizeCb = pFunc;
	} else if (nCbType == VIDEO_PLAY_CB_TYPE_DESTORY) {
		pPlayView->pCallback->pDestroyCb = pFunc;
	} else if (nCbType == VIDEO_PLAY_CB_TYPE_EVENT_KEY_DOWN) {
		pPlayView->pCallback->pEventKeyDown = pFunc;
	} else if (nCbType == VIDEO_PLAY_CB_TYPE_EVENT_KEY_UP) {
		pPlayView->pCallback->pEventKeyUp = pFunc;
	}

	return TRUE;
}

bool vp_play_view_unset_callback(play_view_handle pViewHandle, video_play_callback_type_t nCbType)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	if (!pPlayView->pCallback) {
		VideoLogError("PlayView Callback is NULL");
		return FALSE;
	}

	if (nCbType == VIDEO_PLAY_CB_TYPE_REALIZE) {
		pPlayView->pCallback->pRealizeCb = NULL;
	} else if (nCbType == VIDEO_PLAY_CB_TYPE_DESTORY) {
		pPlayView->pCallback->pDestroyCb = NULL;
	} else if (nCbType == VIDEO_PLAY_CB_TYPE_EVENT_KEY_DOWN) {
		pPlayView->pCallback->pEventKeyDown = NULL;
	} else if (nCbType == VIDEO_PLAY_CB_TYPE_EVENT_KEY_UP) {
		pPlayView->pCallback->pEventKeyUp = NULL;
	}

	return TRUE;
}

Evas_Object *vp_play_view_get_main_window(play_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	PlayView *pPlayView = (PlayView *)pViewHandle;

	return pPlayView->pWin;
}

bool vp_play_view_change_style_minimode_to_fullmode(play_view_handle pViewHandle, video_play_launching_type_t nLaunchingType)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}

	PlayView *pPlayView = (PlayView *)pViewHandle;

	pPlayView->nLaunchingType = nLaunchingType;
	if (nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE || nLaunchingType == VIDEO_PLAY_TYPE_EMAIL) {
		//change multi mode to mini mode
	}

	return TRUE;
}
