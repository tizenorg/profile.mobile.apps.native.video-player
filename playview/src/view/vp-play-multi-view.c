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
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Input.h>
//#include <Ecore_X.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-multi-view.h"

#include "vp-play-popup.h"
#include "vp-play-subtitle.h"
#include "vp-play-loading-ani.h"
#include "vp-play-util.h"
#include "vp-play-button.h"
#include "vp-play-config.h"
#include "vp-play-preference.h"

#include "vp-mm-player.h"

#include "vp-media-contents.h"
#include "vp-device.h"
#include "vp-media-key.h"
#include "vp-sound.h"
#ifdef ENABLE_DRM_FEATURE
#include "vp-drm.h"
#endif
#include "vp-multi-path.h"

#include "VppDownload.h"
#include "vp-play-ug.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-util.h"
#include "vp-file-util.h"

#define VP_MULTI_DEFAULT_WIDTH		((3+60+342+3) * elm_config_scale_get())
#define VP_MULTI_DEFAULT_HEIGHT		((3+60+342+3) * elm_config_scale_get())
#define VP_MULTI_HIDE_LAYOUT_TIMER_INTERVAL		5.0
#define VP_MULTI_LONG_PRESS_TIMER_INTERVAL		1.0

#define VP_MULTI_SPEED_VALUE_MAX			64

typedef struct _MultiView {
	PlayView *pPlayView;

	Evas_Object *pWin;
	Ecore_X_Window nXwinID;
	Evas_Object *pNaviFrame;
	Elm_Object_Item *pNaviItem;

	Evas_Object *pLayout;
	Evas_Object *pVideoSink;

	Evas_Object *pExitBtn;
	Evas_Object *pFullSizeBtn;
	Evas_Object *pMinSizeBtn;
	Evas_Object *pResizeBtn;

	Evas_Object *pImageBufferObj;

	Evas_Object *pPlayBtn;
	Evas_Object *pPauseBtn;
	Evas_Object *pPlayFocusBtn;
	Evas_Object *pPrevBtn;
	Evas_Object *pNextBtn;
	Evas_Object *pExitPopupWin;
	Evas_Object *pPopup;
	Evas_Object *pMainEventRect;
	Evas_Object *pGestureRect;

	Evas_Object *pLoadingAni;

	Ecore_Timer *pHideTimer;

	Ecore_Event_Handler *pMouseDownHandle;
	Ecore_Event_Handler *pMouseUpHandle;
	Ecore_Event_Handler *pMouseMoveHandle;
	Ecore_Event_Handler *pConfigureNotify;

	GList *pMediaItemList;
	video_play_repeat_mode_t nRepeatMode;
	video_play_launching_type_t nLaunchingType;

	Ecore_Idler *pControlIdler;

	mm_player_handle pPlayerHandle;

	char *szMediaURL;
	char *szSubtitleURL;

	int nStartPosition;
	int nCurPosition;
	bool bManualPause;

	bool bShowLayout;
	bool bMainFocusState;
	bool bReapeatMode;
	bool bActivate;
	bool bIsRealize;
	bool bFloatingMode;
	bool bIsResizeDone;
	bool bNoContentMode;

	bool bBufferingComplete;

	vp_mm_player_state_t nPlayerState;

	unsigned int nMousePosX;
	unsigned int nMousePosY;
	unsigned int nMouseButtons;

	bool bTabStart;
	bool bMouseDown;
	video_sound_alive_t nSoundAlive;

	video_play_rotate_t nRotate;
	media_key_handle pMediaKey;

	VppDownloadHandle pDownloadHandle;

	subtitle_handle pSubtitle;
	Ecore_Timer *pSubtitleTimer;
	bool bIsExistSubtitle;

	Evas_Coord_Rectangle nVideoSinkRect;

	bool bSeekComplete;
	int nDuration;
	bool bHLSMode;

	Ecore_Timer *pSpeedTimer;
	bool bSpeedFF;
	int nSpeedValue;
	bool bSpeedDown;

	int nAudioTrackIndex;

	bool bCancelKeyPress;
	double fPlaySpeed;
	float fSubtitleSyncValue;
	Ecore_Pipe *pPreparePipe;
} MultiView;

static bool _vp_play_multi_view_play_start(MultiView *pMultiView,
                        bool bCheckDRM);
static void _vp_play_multi_view_on_next_play(MultiView *pMultiView,
                        bool bManual);
static void _vp_play_multi_view_on_prev_play(MultiView *pMultiView,
                        bool bManual, bool bVoice);
static void _vp_play_multi_view_set_preview(MultiView *pMultiView);
static void _vp_play_multi_view_set_played_time(MultiView *pMultiView);
static bool _vp_play_multi_view_check_during_call(MultiView *pMultiView);

static Evas_Object *_vp_play_multi_view_create_image_sink(void *pParent,
                        void *pUserData);

static void _vp_play_multi_view_set_play_state(MultiView *pMultiView);
static void _vp_play_multi_view_show_layout(MultiView *pMultiView);
static void _vp_play_multi_view_hide_layout(MultiView *pMultiView);
static void _vp_play_multi_view_create_layout_hide_timer(MultiView *
                        pMultiView);
static void _vp_play_multi_view_destroy_handle(MultiView *pMultiView);
static void _vp_play_multi_view_speed_for_steps(MultiView *pMultiView,
                        bool bSpeedFF);

//Focus UI
static void _vp_play_multi_view_set_button_focus_sequence(MultiView *
                        pMultiView);

/* callback functions */

static Eina_Bool __vp_multi_hide_layout_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	MultiView *pMultiView = (MultiView *) pUserData;

	VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pMultiView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (nState != VP_MM_PLAYER_STATE_PLAYING) {
		return EINA_FALSE;
	}

	_vp_play_multi_view_hide_layout(pMultiView);

	return EINA_FALSE;
}

static Eina_Bool __vp_multi_subtitle_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	MultiView *pMultiView = (MultiView *) pUserData;

	VP_EVAS_TIMER_DEL(pMultiView->pSubtitleTimer);

	vp_play_subtitle_set_text(pMultiView->pSubtitle, NULL);

	return EINA_FALSE;
}

static Eina_Bool __vp_multi_speed_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	MultiView *pMultiView = (MultiView *) pUserData;


	if (pMultiView->bSpeedDown) {
		pMultiView->nSpeedValue++;
	}
	//SPEED: *2, *4, *8, *16, *32
	int nSeedVal = pMultiView->nSpeedValue;

	int nStep = (int) pow(2.0, (double)(nSeedVal - 1));

	VideoLogWarning("pMultiView->nSpeedValue : %d",
	                pMultiView->nSpeedValue);
	if (pMultiView->nSpeedValue >= 2) {
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_SHOW, "*");
	} else {
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
		if (pMultiView->bManualPause == FALSE) {
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state
			        (pMultiView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pMultiView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				_vp_play_multi_view_set_play_state(pMultiView);
			}
			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
		}
		return EINA_FALSE;
	}

	if (nStep > VP_MULTI_SPEED_VALUE_MAX) {
		nStep = VP_MULTI_SPEED_VALUE_MAX;
	}

	char szTxt[5] = { 0, };
	snprintf(szTxt, 5, "%d X", nStep);
	elm_object_part_text_set(pMultiView->pLayout,
	                         VP_PLAY_PART_MULTI_SPEED_TXT, szTxt);

	if (pMultiView->bSeekComplete == FALSE) {
		VideoLogError("bSeekComplete is fail");
		return EINA_TRUE;
	}

	if (pMultiView->bManualPause == FALSE) {
		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

		if (!vp_mm_player_get_state(pMultiView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}

		if (nState == VP_MM_PLAYER_STATE_PLAYING) {
			if (!vp_mm_player_pause(pMultiView->pPlayerHandle)) {
				VideoLogError("Pause Fail");
				return EINA_TRUE;
			}
			_vp_play_multi_view_set_play_state(pMultiView);
		}
	}

	int nSetPosition = 0;
	int nCurPosition = 0;

	if (!vp_mm_player_get_position
	        (pMultiView->pPlayerHandle, &nCurPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return EINA_TRUE;
	}

	if (pMultiView->nDuration <= nCurPosition) {
		VideoLogError("nCurPosition : %d, Duration : %d", nCurPosition,
		              pMultiView->nDuration);
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
		VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		return EINA_FALSE;
	}

	if (nCurPosition <= 0 && pMultiView->bSpeedFF == FALSE) {
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");

		if (pMultiView->bManualPause == FALSE
		        && pMultiView->bSpeedDown == FALSE) {
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state
			        (pMultiView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pMultiView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				_vp_play_multi_view_set_play_state(pMultiView);
			}
			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
			return EINA_FALSE;
		}

		return EINA_TRUE;
	}

	if (pMultiView->bSpeedFF) {
		nSetPosition = nCurPosition + (nStep * 1000);
	} else {
		nSetPosition = nCurPosition - (nStep * 1000);
	}

	if (pMultiView->nDuration < nSetPosition) {
		nSetPosition = pMultiView->nDuration;
	}

	if (nSetPosition < 0) {
		nSetPosition = 0;
	}

	if (vp_mm_player_set_position
	        (pMultiView->pPlayerHandle, nSetPosition)) {
		pMultiView->bSeekComplete = FALSE;
		pMultiView->nCurPosition = nSetPosition;
	}

	return EINA_TRUE;
}

static void __vp_multi_error_popup_time_out_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	VP_EVAS_DEL(pMultiView->pPopup);

	_vp_play_multi_view_on_next_play(pMultiView, FALSE);
}

static void __vp_multi_drm_yes_button_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pObj == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	VP_EVAS_DEL(pObj);

	_vp_play_multi_view_play_start(pMultiView, FALSE);

}

static void __vp_multi_drm_no_button_cb(void *pUserData,
                                        Evas_Object *pObj,
                                        void *pEventInfo)
{
	if (pObj == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	MultiView *pMultiView = (MultiView *) pUserData;

	VP_EVAS_DEL(pObj);

	_vp_play_multi_view_on_next_play(pMultiView, FALSE);
}

static void __vp_multi_view_prepare_cb(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	if (pMultiView->pPreparePipe != NULL) {
		ecore_pipe_write(pMultiView->pPreparePipe, pMultiView,
		                 sizeof(MultiView));
	}
}

static void __vp_multi_view_completed_cb(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;
	PlayView *pPlayView = pMultiView->pPlayView;
	if (pPlayView == NULL) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	if (pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_PREVIEW &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_MESSAGE &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_EMAIL) {
		vp_media_contents_set_played_position(pMultiView->szMediaURL, 0);
	} else if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
	}
	vp_mm_player_unrealize(pMultiView->pPlayerHandle);

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW) {
		if (pPlayView->pFunc == NULL) {
			VideoLogError("pPlayView pFunc is NULL");
			return;
		}

		if (pPlayView->pFunc->vp_play_func_exit == NULL) {
			VideoLogError("pPlayView exit Func is NULL");
			return;
		}
		vp_play_util_set_unlock_power_key();

		pPlayView->pFunc->vp_play_func_exit(pPlayView);

		return;
	}

	_vp_play_multi_view_on_next_play(pMultiView, FALSE);

}

static void __vp_multi_view_seek_completed_cb(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	pMultiView->bSeekComplete = TRUE;

	if (vp_play_util_check_streaming(pMultiView->szMediaURL)) {
		if (pMultiView->bBufferingComplete == FALSE) {
			VideoLogWarning("Skip for buffering state");
			if (pMultiView->nStartPosition > 0) {
				pMultiView->nStartPosition = 0;
				pMultiView->bIsRealize = TRUE;
			}
			return;
		}
	}

	if (pMultiView->pLoadingAni) {
		vp_play_loading_ani_destroy(pMultiView->pLoadingAni);
		pMultiView->pLoadingAni = NULL;
	}

	if (pMultiView->nStartPosition > 0) {
		if (pMultiView->bManualPause) {
			vp_mm_player_pause(pMultiView->pPlayerHandle);
		} else {
			vp_mm_player_play(pMultiView->pPlayerHandle);
		}
		pMultiView->nStartPosition = 0;
		pMultiView->bIsRealize = TRUE;
	}

	if (pMultiView->bManualPause == FALSE &&
	        pMultiView->pSpeedTimer == NULL) {
		vp_mm_player_play(pMultiView->pPlayerHandle);
	}

	if (!vp_mm_player_set_subtitle_position
	        (pMultiView->pPlayerHandle,
	         pMultiView->fSubtitleSyncValue * 1000)) {
		VideoLogError("vp_mm_player_set_subtitle_position is fail");
	}

	_vp_play_multi_view_set_play_state(pMultiView);

}

static void __vp_multi_view_interrupted_cb(vp_mm_player_interrupt_t nCode,
                        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;
	VideoLogWarning("nInterrupt : %d", nCode);

	if (nCode == VP_MM_PLAYER_INTERRUPTED_COMPLETED) {
		if (pMultiView->bManualPause == FALSE) {
			vp_mm_player_play(pMultiView->pPlayerHandle);
		}
	} else if (nCode == VP_MM_PLAYER_INTERRUPTED_BY_MEDIA) {
		pMultiView->bManualPause = TRUE;
	}

	if (nCode == VP_MM_PLAYER_INTERRUPTED_BY_CALL) {
		MultiView *pMultiView = (MultiView *) pUserData;

		vp_play_util_status_noti_show
		(VP_PLAY_STRING_UNABLE_TO_PLAY_VIDEO_DURING_CALL);
		_vp_play_multi_view_set_played_time(pMultiView);
		_vp_play_multi_view_set_preview(pMultiView);

		PlayView *pPlayView = pMultiView->pPlayView;
		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			return;
		}

		if (pPlayView->pFunc == NULL) {
			VideoLogError("pPlayView->pFunc is NULL");
			return;

		}
		pPlayView->pFunc->vp_play_func_exit(pPlayView);
	} else {
		_vp_play_multi_view_set_play_state(pMultiView);
	}

	if (nCode == VP_MM_PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT) {
		MultiView *pMultiView = (MultiView *) pUserData;

		//vp_play_util_status_noti_show(VP_VIDEO_STRING_VIDEO_CLOSED_NOTI_POPUP);

		_vp_play_multi_view_set_played_time(pMultiView);
		_vp_play_multi_view_set_preview(pMultiView);

		PlayView *pPlayView = pMultiView->pPlayView;
		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			return;
		}

		if (pPlayView->pFunc == NULL) {
			VideoLogError("pPlayView->pFunc is NULL");
			return;

		}
		vp_play_util_set_unlock_power_key();
		_vp_play_multi_view_set_play_state(pMultiView);

		//pPlayView->pFunc->vp_play_func_exit(pPlayView);
	}
}

static void __vp_multi_view_error_cb(vp_mm_player_error_t nError,
                                     void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	VideoLogWarning("Error triggered: 0x%x", nError);

	MultiView *pMultiView = (MultiView *) pUserData;
	PlayView *pPlayView = pMultiView->pPlayView;

	VP_EVAS_DEL(pMultiView->pPopup);
	pMultiView->pPopup = NULL;

	if (pPlayView == NULL) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	switch (nError) {
	case VP_MM_PLAYER_ERR_OUT_OF_MEMORY:
	case VP_MM_PLAYER_ERR_INVALID_OPERATION:
	case VP_MM_PLAYER_ERR_FILE_NO_SPACE_ON_DEVICE:
	case VP_MM_PLAYER_ERR_INVALID_PARAMETER:
		vp_play_util_status_noti_show(VP_PLAY_STRING_ERROR_UNABLE_PLAY);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		break;
	case VP_MM_PLAYER_ERR_NO_SUCH_FILE:
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_ERROR_FILE_NOT_EXIST);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		break;


	case VP_MM_PLAYER_ERR_SEEK_FAILED:
		VideoLogError("VP_MM_PLAYER_ERR_SEEK_FAILED");
		break;
	case VP_MM_PLAYER_ERR_INVALID_STATE:
		VideoLogError("VP_MM_PLAYER_ERR_INVALID_STATE");
		break;
	case VP_MM_PLAYER_ERR_SOUND_POLICY:
		VideoLogError("VP_MM_PLAYER_ERR_SOUND_POLICY");
		break;
	case VP_MM_PLAYER_ERR_VIDEO_CAPTUR_FAILED:
		VideoLogError("VP_MM_PLAYER_ERR_VIDEO_CAPTUR_FAILED");
		break;
	case VP_MM_PLAYER_ERR_NOT_SUPPORTED_FILE:
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_ERROR_UNSUPPORTED_FORMAT);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);

		break;
	case VP_MM_PLAYER_ERR_INVALID_URI:
		vp_play_util_status_noti_show(VP_PLAY_STRING_ERROR_INVALID_URL);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		break;

	case VP_MM_PLAYER_ERR_CONNECTION_FAILED:
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_ERROR_CONNECTION_FAILED);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		break;
	case VP_MM_PLAYER_ERR_DRM_EXPIRED:
	case VP_MM_PLAYER_ERR_DRM_FUTURE_USE:
		VideoLogError("DRM BUG");
		vp_play_util_status_noti_show(VP_PLAY_STRING_ERROR_UNABLE_PLAY);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		break;
	case VP_MM_PLAYER_ERR_DRM_NO_LICENSE:
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_DIVX_DRM_AUTHORIZATION_ERROR);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		break;
	default:
		VideoLogError("UNKNOWN TYPE : %d", nError);
		break;
	}

}

static bool _vp_play_multi_view_create_loading_ani(MultiView *pMultiView)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;
	pParent = pMultiView->pLayout;

	if (pMultiView->pLoadingAni) {
		vp_play_loading_ani_destroy(pMultiView->pLoadingAni);
		pMultiView->pLoadingAni = NULL;
	}

	pMultiView->pLoadingAni =
	    vp_play_loading_ani_create(pParent, VIDEO_LOADING_SIZE_LARGE);
	if (pMultiView->pLoadingAni == NULL) {
		VideoLogError("pLoadingAni create fail");
		return FALSE;
	}

	elm_object_part_content_set(pParent,
	                            VP_PLAY_SWALLOW_MULTI_LOADING_ANI,
	                            pMultiView->pLoadingAni);

	return TRUE;
}

static void __vp_multi_view_buffering_cb(int nPercent, void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	if (nPercent == 100) {
		VideoLogError("Buffering complete");
		pMultiView->bBufferingComplete = TRUE;
		if (pMultiView->pLoadingAni) {
			vp_play_loading_ani_destroy(pMultiView->pLoadingAni);
			pMultiView->pLoadingAni = NULL;
		}
#if 1
		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

		if (!vp_mm_player_get_state(pMultiView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}

		if (nState != VP_MM_PLAYER_STATE_PLAYING) {
			if (pMultiView->bManualPause) {
				vp_mm_player_set_visible(pMultiView->pPlayerHandle, TRUE);
				vp_mm_player_pause(pMultiView->pPlayerHandle);
			} else {
				vp_mm_player_play(pMultiView->pPlayerHandle);
			}
			_vp_play_multi_view_set_play_state(pMultiView);
		}
#endif
	} else {
		pMultiView->bBufferingComplete = FALSE;
		if (pMultiView->bManualPause == FALSE) {
			vp_play_util_set_lock_power_key();
		}
		// Remove loading animation during speed playback.
		if (pMultiView->pSpeedTimer) {
			//
		} else if (pMultiView->pLoadingAni == NULL) {
			if (!_vp_play_multi_view_create_loading_ani(pMultiView)) {
				VideoLogError
				("_vp_play_multi_view_create_loading_ani fail");
				return;
			}
		}
	}

}

static void __vp_multi_view_subtitle_updated_cb(unsigned long nDuration,
                        char *text,
                        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	bool bIsWhiteSpace = vp_play_util_check_white_space(text);

	VP_EVAS_TIMER_DEL(pMultiView->pSubtitleTimer);

	if (bIsWhiteSpace) {
		vp_play_subtitle_set_text(pMultiView->pSubtitle, "");
		return;
	}

	double dTimeInterval = (double) nDuration / (double) 1000;

	pMultiView->pSubtitleTimer = ecore_timer_add(dTimeInterval,
	                             __vp_multi_subtitle_timer_cb,
	                             (void *) pMultiView);

	vp_play_subtitle_set_text(pMultiView->pSubtitle, text);


}

static void __vp_multi_view_pd_message_cb(vp_mm_player_pd_message_t nType,
        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

}

static void __vp_multi_view_missed_plugin_cb(vp_mm_player_missed_plugin_t
                        nType, void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("");

	if (nType == VP_MM_PLAYER_MISSED_PLUGIN_AUDIO) {
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_ERROR_UNSUPPORTED_AUDIO);
	} else if (nType == VP_MM_PLAYER_MISSED_PLUGIN_VIDEO) {
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_ERROR_ONLY_AUDIO_DATA_SUPPORTED);
	}
}

static void __vp_multi_view_image_buffer_cb(char *pBuffer, int nSize,
                        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	VideoLogWarning("%p, %d", pBuffer, nSize);

	MultiView *pMultiView = (MultiView *) pUserData;
	if (pMultiView->pImageBufferObj) {
		elm_object_part_content_unset(pMultiView->pLayout,
		                              VP_PLAY_SWALLOW_MULTI_IMAGE_BUFFER);
		VP_EVAS_DEL(pMultiView->pImageBufferObj);
	}

	vp_play_util_save_file(VP_PLAY_IMAGE_BUFFER_PATH, pBuffer, nSize);

	pMultiView->pImageBufferObj = elm_image_add(pMultiView->pLayout);

	elm_image_file_set(pMultiView->pImageBufferObj,
	                   VP_PLAY_IMAGE_BUFFER_PATH, NULL);

	elm_image_resizable_set(pMultiView->pImageBufferObj, EINA_TRUE,
	                        EINA_TRUE);

	evas_object_size_hint_weight_set(pMultiView->pImageBufferObj,
	                                 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pMultiView->pImageBufferObj,
	                                EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(pMultiView->pImageBufferObj);

	elm_object_part_content_set(pMultiView->pLayout,
	                            VP_PLAY_SWALLOW_MULTI_IMAGE_BUFFER,
	                            pMultiView->pImageBufferObj);
}

static void __vp_play_multi_view_imagesink_resize_cb(void *pUserData,
                        Evas *pEvas,
                        Evas_Object *pObj,
                        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return;
	}

	VideoLogInfo("");

	MultiView *pMultiView = (MultiView *) pUserData;

	Evas_Coord_Rectangle nOldRect = pMultiView->nVideoSinkRect;
	Evas_Coord_Rectangle nCurRect = { 0, };

	evas_object_geometry_get(pObj, &nCurRect.x, &nCurRect.y, &nCurRect.w,
	                         &nCurRect.h);
//      evas_object_image_fill_set(pObj, 0, 0, nCurRect.w, nCurRect.h);
//      evas_object_image_size_set(pObj, nCurRect.w, nCurRect.h);

	if (nCurRect.x != nOldRect.x ||
	        nCurRect.y != nOldRect.y ||
	        nCurRect.w != nOldRect.w || nCurRect.h != nOldRect.h) {
		VideoLogWarning("Change Position");
	}

	bool bShow = FALSE;
	vp_play_preference_get_subtitle_show_key(&bShow);
	if (bShow) {
		bool bIsRealize = FALSE;
		int nSize = 0;
		vp_play_subtitle_is_realize(pMultiView->pSubtitle, &bIsRealize);
		if (bIsRealize) {
			float fZoom = 1.0;
			int nWinW = 0;
			int nWinH = 0;

			PlayView *pPlayView = pMultiView->pPlayView;
			elm_win_screen_size_get(pPlayView->pWin, NULL, NULL, &nWinW,
			                        &nWinH);

			fZoom = (float)((float)(nCurRect.w) / (float)(nWinW));

			vp_play_preference_get_subtitle_size_key(&nSize);
			vp_play_subtitle_set_size(pMultiView->pSubtitle, nSize);

			vp_play_subtitle_set_size_zoom(pMultiView->pSubtitle, fZoom);
		}
	}
}

#ifdef ENABLE_MULTI_VEIW
static void __vp_play_multi_view_layout_mouse_down_cb(void *pUserData,
                        Evas *e,
                        Evas_Object *pObj,
                        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	Evas_Event_Mouse_Down *pMouseDownEvent =
	    (Evas_Event_Mouse_Down *) pEvent;
	Evas_Coord_Rectangle nCurRect = { 0, };
	evas_object_geometry_get(pObj, &nCurRect.x, &nCurRect.y, &nCurRect.w,
	                         &nCurRect.h);


	if (pMultiView->bIsRealize == FALSE) {
		return;
	}

	if (pMouseDownEvent->canvas.x > nCurRect.w - VP_MULTI_RESIZE_TERM &&
	        pMouseDownEvent->canvas.y > nCurRect.h - VP_MULTI_RESIZE_TERM) {

		evas_object_size_hint_min_set(pMultiView->pWin,
		                              VP_MULTI_DEFAULT_WIDTH,
		                              VP_MULTI_DEFAULT_HEIGHT);
		int x = 0;
		int y = 0;
		ecore_x_pointer_last_xy_get(&x, &y);
		ecore_x_mouse_up_send(pMultiView->nXwinID, x, y, 1);
		ecore_x_pointer_ungrab();
		ecore_x_netwm_moveresize_request_send(pMultiView->nXwinID, x, y,
		                                      ECORE_X_NETWM_DIRECTION_SIZE_BR,
		                                      1);
	}

}
#endif

static Evas_Event_Flags __vp_multi_view_gesture_n_tab_start_cb(void
                        *pUserData,
                        void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}
	VideoLogWarning("");

	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) pEventInfo;
	MultiView *pMultiView = (MultiView *) pUserData;

	if (p->n <= 1) {
		pMultiView->bTabStart = TRUE;
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags __vp_multi_view_gesture_n_tab_end_cb(void
                        *pUserData,
                        void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}
	VideoLogWarning("");

	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) pEventInfo;

	MultiView *pMultiView = (MultiView *) pUserData;
	pMultiView->bTabStart = FALSE;

	if (p->n == 1) {

		if (pMultiView->bShowLayout == FALSE) {
			_vp_play_multi_view_show_layout(pMultiView);
		} else {
			_vp_play_multi_view_hide_layout(pMultiView);
		}

	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags __vp_multi_view_gesture_n_tab_abort_cb(void
                        *pUserData,
                        void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}
	VideoLogWarning("");

	//Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) pEventInfo;
	MultiView *pMultiView = (MultiView *) pUserData;
	pMultiView->bTabStart = FALSE;

	return EVAS_EVENT_FLAG_NONE;
}

static Eina_Bool __vp_play_multi_view_mouse_down_cb(void *pUserData,
                        int nType,
                        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}
	MultiView *pMultiView = (MultiView *) pUserData;
	Ecore_Event_Mouse_Button *pMouseEvent = pEvent;

	if (pMouseEvent->window != pMultiView->nXwinID) {
		return ECORE_CALLBACK_PASS_ON;
	}

	if (pMultiView->bMouseDown) {
		VideoLogWarning("Already mouse Down");
		return ECORE_CALLBACK_PASS_ON;
	}

	pMultiView->bMouseDown = TRUE;
	pMultiView->nMousePosX = pMouseEvent->root.x;
	pMultiView->nMousePosY = pMouseEvent->root.y;
	pMultiView->nMouseButtons = pMouseEvent->buttons;

	if (pMultiView->nMouseButtons == 0) {
		VideoLogWarning("Invalid Mouse button value : %d",
		                pMultiView->nMouseButtons);
		pMultiView->nMouseButtons = 1;
	}

	return ECORE_CALLBACK_PASS_ON;

}

static Eina_Bool __vp_play_multi_view_mouse_up_cb(void *pUserData,
                        int nType, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	MultiView *pMultiView = (MultiView *) pUserData;
	Ecore_Event_Mouse_Button *pMouseEvent = pEvent;

	if (pMouseEvent->window != pMultiView->nXwinID) {
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pMultiView->bMouseDown) {
		//VideoLogWarning("Not Mouse Down State");
		return ECORE_CALLBACK_PASS_ON;
	}

	pMultiView->bMouseDown = FALSE;
	pMultiView->nMousePosX = 0;
	pMultiView->nMousePosY = 0;

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_multi_view_mouse_move_cb(void *pUserData,
                        int nType,
                        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	Ecore_Event_Mouse_Move *pMouseEvent = pEvent;

	if (pMouseEvent->window != pMultiView->nXwinID) {
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pMultiView->bMouseDown || pMultiView->bTabStart) {
		//VideoLogWarning("Not Mouse Down State");
		return ECORE_CALLBACK_PASS_ON;
	}

	double l =
	    sqrt(pow
	         ((float)(pMultiView->nMousePosX - pMouseEvent->root.x),
	          2) + pow((float)(pMultiView->nMousePosY -
	                           pMouseEvent->root.y), 2));

	if (l >= 30.0f && pMouseEvent->multi.device == 0) {
		int nX = 0;
		int nY = 0;
		ecore_x_pointer_last_xy_get(&nX, &nY);
		if (nX != pMouseEvent->root.x) {
			nX = (nX + pMouseEvent->root.x) / 2;
		}
		if (nY != pMouseEvent->root.y) {
			nY = (nY + pMouseEvent->root.y) / 2;
		}

		ecore_x_mouse_up_send(pMultiView->nXwinID, nX, nY,
		                      pMultiView->nMouseButtons);
		ecore_x_pointer_ungrab();

		ecore_x_netwm_moveresize_request_send(pMultiView->nXwinID, nX, nY,
		                                      ECORE_X_NETWM_DIRECTION_MOVE,
		                                      pMultiView->nMouseButtons);

		pMultiView->bMouseDown = FALSE;
		pMultiView->nMousePosX = 0;
		pMultiView->nMousePosY = 0;
	}


	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_multi_view_configure_cb(void *pUserData,
                        int type, void *pEvent)
{
	Ecore_Evas *ee;
	Ecore_X_Event_Window_Configure *e;
	int ee_w = 0;
	int ee_h = 0;

	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (!pEvent) {
		VideoLogError("[ERR] No exist pEvent.");
		return ECORE_CALLBACK_PASS_ON;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	if (pMultiView->bIsResizeDone) {
		return ECORE_CALLBACK_PASS_ON;
	}

	e = pEvent;
	ee = ecore_evas_ecore_evas_get(evas_object_evas_get
	                               (pMultiView->pWin));

	if (!ee) {
		VideoLogError("[ERR] Ecore_Evas has NULL");
		return ECORE_CALLBACK_PASS_ON;	/* pass on event */
	}

	ecore_evas_geometry_get(ee, NULL, NULL, &ee_w, &ee_h);

	if (e->win != ecore_evas_window_get(ee)) {
		VideoLogError("[ERR] Event window and ecore window is different");
		return ECORE_CALLBACK_PASS_ON;
	}
	int nAngle = elm_win_rotation_get(pMultiView->pWin);
	video_play_rotate_t nCurRot = VIDEO_PLAY_ROTATE_NONE;
	switch (nAngle) {
	case 0:
		nCurRot = VIDEO_PLAY_ROTATE_NONE;
		break;
	case 90:
		nCurRot = VIDEO_PLAY_ROTATE_90;
		break;
	case 180:
		nCurRot = VIDEO_PLAY_ROTATE_180;
		break;
	case 270:
		nCurRot = VIDEO_PLAY_ROTATE_270;
		break;
	}

	if (pMultiView->bFloatingMode == TRUE) {
		bool bIsResize = FALSE;

		if (nCurRot == VIDEO_PLAY_ROTATE_NONE ||
		        nCurRot == VIDEO_PLAY_ROTATE_180) {
			if (e->x == 0 && e->y == 0 && e->w == ee_w && e->h == ee_h) {
				bIsResize = TRUE;
			}
		} else {
			if (e->x == 0 && e->y == 0 && e->h == ee_w && e->w == ee_h) {
				bIsResize = TRUE;
			}
		}

		if (bIsResize == TRUE) {
			pMultiView->bFloatingMode = FALSE;
			pMultiView->bIsResizeDone = TRUE;

			PlayView *pPlayView = pMultiView->pPlayView;

			if (pPlayView == NULL) {
				VideoLogError("pPlayView is NULL");
				return ECORE_CALLBACK_PASS_ON;
			}

			pPlayView->nRotate = nCurRot;

			if (pPlayView->pFunc == NULL) {
				VideoLogError("pPlayView->pFunc is NULL");
				return ECORE_CALLBACK_PASS_ON;
			}

			int nCurPos = 0;
			vp_play_multi_view_get_last_position(pMultiView, &nCurPos);

			VP_EVAS_DISABLE_SET(pMultiView->pExitBtn, EINA_TRUE);
			VP_EVAS_DISABLE_SET(pMultiView->pFullSizeBtn, EINA_TRUE);
			VP_EVAS_DISABLE_SET(pMultiView->pMinSizeBtn, EINA_TRUE);

			pPlayView->pFunc->vp_play_func_change_mode(pPlayView,
			        VIDEO_PLAY_MODE_NORMAL_VIEW,
			        pMultiView->
			        szMediaURL,
			        nCurPos,
			        pMultiView->
			        bManualPause);
		}
	}

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vp_play_multi_view_control_idler_cb(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return ECORE_CALLBACK_CANCEL;
	}
	MultiView *pMultiView = (MultiView *) pUserData;

	_vp_play_multi_view_set_play_state(pMultiView);

	pMultiView->pControlIdler = NULL;

	return ECORE_CALLBACK_CANCEL;
}

static void __vp_play_multi_view_btn_clicked_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	bool bCallOn = FALSE;
	vp_play_config_get_call_state(&bCallOn);

	if (pMultiView->pExitBtn != pObj && pMultiView->pFullSizeBtn != pObj
	        && pMultiView->pMinSizeBtn != pObj && bCallOn) {
		VideoLogWarning("Call is On");
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_UNABLE_TO_PLAY_VIDEO_DURING_CALL);
		return;
	}

	if (pMultiView->pExitBtn == pObj) {
		VideoLogWarning("Exit button cliicked");

		_vp_play_multi_view_set_played_time(pMultiView);

		PlayView *pPlayView = pMultiView->pPlayView;
		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			return;
		}

		if (pPlayView->pFunc == NULL) {
			VideoLogError("pPlayView->pFunc is NULL");
			return;

		}

		VideoLogWarning("== BACK ==");

		VP_EVAS_DISABLE_SET(pMultiView->pExitBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pMultiView->pFullSizeBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pMultiView->pMinSizeBtn, EINA_TRUE);

		vp_play_util_set_unlock_power_key();
		pPlayView->pFunc->vp_play_func_exit(pPlayView);

		return;

	} else if (pMultiView->pFullSizeBtn == pObj) {
		// change window size
		VideoLogWarning("Full Size button cliicked");

		if (pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
		        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {

			if (!vp_play_util_local_file_check(pMultiView->szMediaURL)) {
				/* popup show */
				vp_play_util_status_noti_show
				(VP_PLAY_STRING_ERROR_FILE_NOT_EXIST);
				elm_exit();
				return;
			}

		}

		pMultiView->bActivate = FALSE;

		VP_EVAS_DISABLE_SET(pMultiView->pExitBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pMultiView->pFullSizeBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pMultiView->pMinSizeBtn, EINA_TRUE);

		pMultiView->bFloatingMode = TRUE;
		pMultiView->bIsResizeDone = FALSE;

		elm_win_floating_mode_set(pMultiView->pWin, EINA_FALSE);

		elm_win_norender_push(pMultiView->pWin);
	} else if (pMultiView->pMinSizeBtn == pObj) {
		VideoLogWarning("Min Size button cliicked");

		PlayView *pPlayView = pMultiView->pPlayView;
		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			return;
		}

		if (pPlayView->pFunc == NULL) {
			VideoLogError("pPlayView->pFunc is NULL");
			return;
		}
		int nCurPos = 0;
		vp_play_multi_view_get_last_position(pMultiView, &nCurPos);
		pPlayView->nStartPosition = nCurPos;
		elm_win_iconified_set(pMultiView->pWin, EINA_TRUE);
	} else if (pMultiView->pPlayFocusBtn == pObj) {
		VideoLogWarning("pPlayFocusBtn click");

		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

		if (!vp_mm_player_get_state(pMultiView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}

		if (nState == VP_MM_PLAYER_STATE_PLAYING) {
			VideoLogWarning("Pause button");
			if (pMultiView->bIsRealize) {
				if (!vp_mm_player_pause(pMultiView->pPlayerHandle)) {
					VideoLogWarning("Pause Fail");
				} else {
					pMultiView->bManualPause = TRUE;
				}
			}
		} else {
			VideoLogWarning("Resume button");
			_vp_play_multi_view_check_during_call(pMultiView);

			if (pMultiView->bSeekComplete == FALSE
			        || pMultiView->pSpeedTimer) {
				pMultiView->bManualPause = FALSE;
				VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
			}
			if (pMultiView->bIsRealize) {
				if (!vp_mm_player_play(pMultiView->pPlayerHandle)) {
					VideoLogWarning("Resume Failed, %d, seek[%d]",
					                pMultiView->nPlayerState,
					                pMultiView->bSeekComplete);
					if (!vp_play_util_check_streaming(pMultiView->szMediaURL) && pMultiView->nPlayerState != VP_MM_PLAYER_STATE_PLAYING && pMultiView->bSeekComplete == TRUE) {	/* vp_mm_player_play will be failed if it try in seeking */
						vp_play_util_status_noti_show
						(VP_PLAY_STRING_ERROR_UNABLE_PLAY);
					}
				} else {
					pMultiView->bManualPause = FALSE;
				}
			}
		}

		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
		VP_EVAS_IDLER_DEL(pMultiView->pControlIdler);
		pMultiView->pControlIdler =
		    ecore_idler_add(__vp_play_multi_view_control_idler_cb,
		                    (void *) pMultiView);
	} else if (pMultiView->pPrevBtn == pObj) {
		VideoLogWarning("Prev button cliicked");
		if (pMultiView->bIsRealize == FALSE) {
			return;
		}

		if (pMultiView->bHLSMode == FALSE) {
			if (pMultiView->nSpeedValue < 2) {
				VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
				_vp_play_multi_view_on_prev_play(pMultiView, TRUE, FALSE);
			}
		}
	} else if (pMultiView->pNextBtn == pObj) {
		VideoLogWarning("Next button cliicked");
		if (pMultiView->bIsRealize == FALSE) {
			return;
		}

		if (pMultiView->bHLSMode == FALSE) {
			if (pMultiView->nSpeedValue < 2) {
				VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
				_vp_play_multi_view_on_next_play(pMultiView, TRUE);
			}
		}
	} else {
		VideoLogError("Unknown button cliicked");
	}
	return;

}

static void __vp_play_multi_view_btn_press_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	if (!pObj) {
		VideoLogError("pObj is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	if (pObj == pMultiView->pNextBtn) {
		VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
		VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
		if (pMultiView->bHLSMode == FALSE) {
			pMultiView->bSpeedFF = TRUE;
			pMultiView->nSpeedValue = 1;
			pMultiView->bSpeedDown = TRUE;
			pMultiView->pSpeedTimer =
			    ecore_timer_add(VP_MULTI_LONG_PRESS_TIMER_INTERVAL,
			                    __vp_multi_speed_timer_cb,
			                    (void *) pMultiView);
		}
	} else if (pObj == pMultiView->pPrevBtn) {
		VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
		VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
		if (pMultiView->bHLSMode == FALSE) {
			pMultiView->bSpeedFF = FALSE;
			pMultiView->nSpeedValue = 1;
			pMultiView->bSpeedDown = TRUE;
			pMultiView->pSpeedTimer =
			    ecore_timer_add(VP_MULTI_LONG_PRESS_TIMER_INTERVAL,
			                    __vp_multi_speed_timer_cb,
			                    (void *) pMultiView);
		}
	} else if (pObj == pMultiView->pResizeBtn) {
		evas_object_size_hint_min_set(pMultiView->pWin,
		                              VP_MULTI_DEFAULT_WIDTH,
		                              VP_MULTI_DEFAULT_HEIGHT);
		int x = 0;
		int y = 0;
		ecore_x_pointer_last_xy_get(&x, &y);
		ecore_x_mouse_up_send(pMultiView->nXwinID, x, y, 1);
		ecore_x_pointer_ungrab();
		ecore_x_netwm_moveresize_request_send(pMultiView->nXwinID, x, y,
		                                      ECORE_X_NETWM_DIRECTION_SIZE_BR,
		                                      1);
	} else if (pObj == pMultiView->pExitBtn) {
		VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
		VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_TITLE_CLOSE_BTN_PRESSED,
		                       "*");
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
	} else if (pObj == pMultiView->pFullSizeBtn) {
		VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
		VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_TITLE_FULLSIZE_BTN_PRESSED,
		                       "*");
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
	} else if (pObj == pMultiView->pMinSizeBtn) {
		VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
		VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_TITLE_MINIMIZE_BTN_PRESSED,
		                       "*");
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
	}

	pMultiView->bMouseDown = FALSE;
}



static void __vp_play_multi_view_btn_unpress_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	if (!pObj) {
		VideoLogError("pObj is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	if (pObj == pMultiView->pPauseBtn) {
		_vp_play_multi_view_create_layout_hide_timer(pMultiView);
	} else if (pObj == pMultiView->pPlayBtn) {
		_vp_play_multi_view_create_layout_hide_timer(pMultiView);
	} else if (pObj == pMultiView->pPlayFocusBtn) {
		_vp_play_multi_view_create_layout_hide_timer(pMultiView);
	} else if (pObj == pMultiView->pNextBtn) {
		pMultiView->bSpeedDown = FALSE;
		_vp_play_multi_view_create_layout_hide_timer(pMultiView);
	} else if (pObj == pMultiView->pPrevBtn) {
		pMultiView->bSpeedDown = FALSE;
		_vp_play_multi_view_create_layout_hide_timer(pMultiView);
	} else if (pObj == pMultiView->pExitBtn) {
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_TITLE_CLOSE_BTN_RELEASED,
		                       "*");
	} else if (pObj == pMultiView->pFullSizeBtn) {
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_TITLE_FULLSIZE_BTN_RELEASED,
		                       "*");
	} else if (pObj == pMultiView->pMinSizeBtn) {
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_TITLE_MINIMIZE_BTN_RELEASED,
		                       "*");
	}

}

static void __vp_play_multi_view_media_key_event_cb(vp_media_key_event_t
                        nKey, bool bRelease,
                        void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	switch (nKey) {
	case VP_MEDIA_KEY_PLAY:
	case VP_MEDIA_KEY_PAUSE:
	case VP_MEDIA_KEY_PLAYPAUSE:
		if (bRelease) {
			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state
			        (pMultiView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pMultiView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				pMultiView->bManualPause = FALSE;
			} else {
				if (!vp_mm_player_pause(pMultiView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				pMultiView->bManualPause = TRUE;
			}
			_vp_play_multi_view_set_play_state(pMultiView);
		}
		break;

	case VP_MEDIA_KEY_PREVIOUS:
		if (bRelease) {
			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
			_vp_play_multi_view_on_prev_play(pMultiView, TRUE, FALSE);
		}
		break;
	case VP_MEDIA_KEY_NEXT:
		if (bRelease) {
			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
			_vp_play_multi_view_on_next_play(pMultiView, TRUE);
		}
		break;
	case VP_MEDIA_KEY_REWIND:
		if (bRelease) {
			pMultiView->bSpeedDown = FALSE;

			if (pMultiView->nSpeedValue < 2) {
				VideoLogInfo("speed up");
				_vp_play_multi_view_speed_for_steps(pMultiView, FALSE);
			}

			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");

			if (pMultiView->bManualPause == FALSE) {
				if (!vp_mm_player_play(pMultiView->pPlayerHandle)) {
					VideoLogError("play Fail");
				}
				pMultiView->bManualPause = FALSE;
			}

			_vp_play_multi_view_set_play_state(pMultiView);
		} else {
			VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");

			pMultiView->bSpeedFF = FALSE;
			pMultiView->nSpeedValue = 1;
			pMultiView->bSpeedDown = TRUE;
			pMultiView->pSpeedTimer =
			    ecore_timer_add(VP_MULTI_LONG_PRESS_TIMER_INTERVAL,
			                    __vp_multi_speed_timer_cb,
			                    (void *) pMultiView);
		}
		break;
	case VP_MEDIA_KEY_FASTFORWARD:
		if (bRelease) {
			pMultiView->bSpeedDown = FALSE;

			if (pMultiView->nSpeedValue < 2) {
				VideoLogInfo("speed up");
				_vp_play_multi_view_speed_for_steps(pMultiView, TRUE);
			}

			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");

			if (pMultiView->bManualPause == FALSE) {
				if (!vp_mm_player_play(pMultiView->pPlayerHandle)) {
					VideoLogError("play Fail");
				}
				pMultiView->bManualPause = FALSE;
			}

			_vp_play_multi_view_set_play_state(pMultiView);
		} else {
			VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");

			pMultiView->bSpeedFF = TRUE;
			pMultiView->nSpeedValue = 1;
			pMultiView->bSpeedDown = TRUE;
			pMultiView->pSpeedTimer =
			    ecore_timer_add(VP_MULTI_LONG_PRESS_TIMER_INTERVAL,
			                    __vp_multi_speed_timer_cb,
			                    (void *) pMultiView);
		}
		break;
	case VP_MEDIA_KEY_STOP:
		if (bRelease) {
			VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
			elm_object_signal_emit(pMultiView->pLayout,
			                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state
			        (pMultiView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState == VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_pause(pMultiView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
			}
			pMultiView->bManualPause = TRUE;
			_vp_play_multi_view_set_play_state(pMultiView);
		}
		break;
	default:
		VideoLogWarning("Undefined key : %d", nKey);
		break;
	}

	VP_FREE(pMultiView);

}

static void __vp_multi_view_exit_popup_yes_btn_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	VP_EVAS_DEL(pMultiView->pPopup);
	pMultiView->pPopup = NULL;
	VP_EVAS_DEL(pMultiView->pExitPopupWin);
	pMultiView->pExitPopupWin = NULL;

	if (pMultiView->pPlayView == NULL) {
		VideoLogError("pMultiView->pPlayView is NULL");
		return;
	}

	PlayView *pPlayView = pMultiView->pPlayView;

	pPlayView->pFunc->vp_play_func_exit(pPlayView);

	return;
}

static void __vp_multi_view_exit_popup_no_btn_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pUserData;

	VP_EVAS_DEL(pMultiView->pPopup);
	pMultiView->pPopup = NULL;
	VP_EVAS_DEL(pMultiView->pExitPopupWin);
	pMultiView->pExitPopupWin = NULL;

	return;
}

static void _vp_play_multi_view_check_exit_popup(MultiView *pMultiView)
{
	VP_EVAS_DEL(pMultiView->pPopup);
	VP_EVAS_DEL(pMultiView->pExitPopupWin);

	int w = 0, h = 0;
	pMultiView->pExitPopupWin =
	    elm_win_add(pMultiView->pWin, "VIDEO_MULTI_VIEW_EXIT_POPUP",
	                ELM_WIN_POPUP_MENU);
	Ecore_Evas *ee =
	    ecore_evas_ecore_evas_get(evas_object_evas_get
	                              (pMultiView->pExitPopupWin));
	ecore_evas_name_class_set(ee, "APP_POPUP", "APP_POPUP");
	elm_win_alpha_set(pMultiView->pExitPopupWin, EINA_TRUE);

	PlayView *pPlayView = pMultiView->pPlayView;
	elm_win_screen_size_get(pPlayView->pWin, NULL, NULL, &w, &h);
	evas_object_resize(pMultiView->pExitPopupWin, w, h);

	if (elm_win_wm_rotation_supported_get(pMultiView->pExitPopupWin)) {
		const int rots[4] = { APP_DEVICE_ORIENTATION_0,
		                      APP_DEVICE_ORIENTATION_90,
		                      APP_DEVICE_ORIENTATION_180,
		                      APP_DEVICE_ORIENTATION_270
		                    };
		elm_win_wm_rotation_available_rotations_set(pMultiView->
		        pExitPopupWin, rots,
		        4);
	}

	/* pass '-1' value to this API then it will unset preferred rotation angle */
	elm_win_wm_rotation_preferred_rotation_set(pMultiView->pExitPopupWin,
	        -1);

	evas_object_show(pMultiView->pExitPopupWin);

	elm_object_text_set(pMultiView->pPopup, VP_VIDEO_STRING_EXIT_POPUP);

	Evas_Object *cancel_btn = elm_button_add(pMultiView->pPopup);
	elm_object_style_set(cancel_btn, "popup_button/default");
	elm_object_text_set(cancel_btn, VP_PLAY_STRING_COM_CANCEL);
	elm_object_part_content_set(pMultiView->pPopup, "button1",
	                            cancel_btn);
	evas_object_smart_callback_add(cancel_btn, "clicked",
	                               __vp_multi_view_exit_popup_no_btn_cb,
	                               pMultiView);
	Evas_Object *ok_btn = elm_button_add(pMultiView->pPopup);
	elm_object_style_set(ok_btn, "popup_button/default");
	elm_object_text_set(ok_btn, VP_PLAY_STRING_COM_OK);
	elm_object_part_content_set(pMultiView->pPopup, "button2", ok_btn);
	evas_object_smart_callback_add(ok_btn, "clicked",
	                               __vp_multi_view_exit_popup_yes_btn_cb,
	                               pMultiView);

	evas_object_show(pMultiView->pPopup);
}

static Eina_Bool __vp_play_multi_view_back_key_event_cb(void *pUserData,
                        Elm_Object_Item *pItem)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	VideoLogError("== BACK EVENT ==");

	MultiView *pMultiView = (MultiView *) pUserData;

	if (pMultiView->bActivate) {
		_vp_play_multi_view_set_played_time(pMultiView);

		PlayView *pPlayView = pMultiView->pPlayView;
		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			return EINA_FALSE;
		}

		if (pPlayView->pFunc == NULL) {
			VideoLogError("pPlayView->pFunc is NULL");
			return EINA_FALSE;

		}

		_vp_play_multi_view_check_exit_popup(pMultiView);

		VideoLogWarning("== BACK ==");

		return EINA_FALSE;
	}

	return EINA_TRUE;
}




/* internal functions */
#if 1
#endif
static void _vp_play_multi_view_on_nocontents_mode(MultiView *pMultiView,
                        bool bShow)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return;
	}
	if (bShow) {

		if (pMultiView->pLoadingAni) {
			vp_play_loading_ani_destroy(pMultiView->pLoadingAni);
			pMultiView->pLoadingAni = NULL;
		}
		_vp_play_multi_view_hide_layout(pMultiView);

		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_SIGNAL_SHOW_NOCONTENTS, "*");
		edje_object_part_text_set(_EDJ(pMultiView->pLayout),
		                          VP_PLAY_PART_MULTI_NOCONTENTS_TXT,
		                          VP_PLAY_STRING_NO_VIDEOS);
		pMultiView->bNoContentMode = TRUE;
	} else {
		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_VIEW_SIGNAL_HIDE_NOCONTENTS, "*");
		edje_object_part_text_set(_EDJ(pMultiView->pLayout),
		                          VP_PLAY_PART_MULTI_NOCONTENTS_TXT, "");
		pMultiView->bNoContentMode = FALSE;
	}
}

static bool _vp_play_multi_view_check_during_call(MultiView *pMultiView)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return FALSE;
	}

	bool bCallOn = FALSE;
	vp_play_config_get_call_state(&bCallOn);
	if (bCallOn) {
		vp_play_util_status_noti_show
		(VP_PLAY_STRING_UNABLE_TO_PLAY_VIDEO_DURING_CALL);
	}

	return TRUE;
}

static bool _vp_play_multi_view_check_drm(MultiView *pMultiView,
                        bool *bIsAvailablePlay)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return FALSE;
	}

	PlayView *pPlayView = pMultiView->pPlayView;
	if (pPlayView == NULL) {
		VideoLogError("pPlayView is NULL");
		return FALSE;
	}

	bool bIsLicense = FALSE;
	*bIsAvailablePlay = TRUE;
#ifdef ENABLE_DRM_FEATURE
	bool bIsDrm = FALSE;

	if (!vp_drm_is_drm_file(pMultiView->szMediaURL, &bIsDrm)) {
		VideoLogError("vp_drm_is_drm_file is fail");
		return FALSE;
	}
#endif
#ifdef ENABLE_DRM_FEATURE
	if (bIsDrm == FALSE) {
		VideoLogWarning("Not DRM File");
		return TRUE;
	} else {
		VideoLogWarning
		("== IS DRM FILE ================================");
	}
#endif
#ifdef ENABLE_DRM_FEATURE
	if (!vp_drm_is_check_license(pMultiView->szMediaURL, &bIsLicense)) {
		VideoLogError("vp_drm_is_check_license is fail");
		return FALSE;
	}
#endif
	if (bIsLicense == FALSE) {
		VideoLogWarning("bIsLicense is Invalid");

		*bIsAvailablePlay = FALSE;

		char *szMessage = NULL;
		char *szTitle = NULL;

		szTitle =
		    vp_play_util_get_title_from_path((char *) pMultiView->
		                                     szMediaURL);
		szMessage =
		    g_strdup_printf(VP_PLAY_STRING_DRM_CURRENTLY_LOCKED_UNLOCK_Q,
		                    szTitle);

		vp_play_util_status_noti_show(szMessage);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);

		VP_FREE(szMessage);
		VP_FREE(szTitle);

		return TRUE;
	}
#ifdef ENABLE_DRM_FEATURE

	vp_drm_constraint_info stConstraintInfo = { 0, };

	if (!vp_drm_get_constarint_info
	        (pMultiView->szMediaURL, &stConstraintInfo)) {
		VideoLogError("vp_drm_get_constarint_info is fail");
		return FALSE;
	}
#endif
	char *szMsg = NULL;
	char *szTitle = NULL;
	szTitle =
	    vp_play_util_get_title_from_path((char *)pMultiView->szMediaURL);
	bool bNotiPopup = FALSE;
#ifdef ENABLE_DRM_FEATURE
	while (stConstraintInfo.constraints) {
		if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_COUNT) {
			if (stConstraintInfo.remaining_count == 2) {
				szMsg =
				    g_strdup_printf
				    (VP_PLAY_STRING_DRM_PS_2_MORE_TIME_START_NOW_Q,
				     szMsg);
			} else if (stConstraintInfo.remaining_count == 1) {
				szMsg =
				    g_strdup_printf
				    (VP_PLAY_STRING_DRM_PS_1_MORE_TIME_START_NOW_Q,
				     szMsg);
			} else if (stConstraintInfo.remaining_count == 0) {
				bNotiPopup = TRUE;
			}
			break;
		}

		if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_DATE_TIME) {
			if (stConstraintInfo.date_time_expired) {
				bNotiPopup = TRUE;
			}
			break;
		}

		if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_INTERVAL) {
			if (stConstraintInfo.remaining_interval_sec == 0) {
				bNotiPopup = TRUE;
			} else {
				int nDays =
				    stConstraintInfo.remaining_interval_sec /
				    VP_ACCUMULATED_DATE + 1;
				szMsg =
				    g_strdup_printf
				    (VP_PLAY_STRING_DRM_PS_FOR_PD_DAYS_START_NOW_Q, szMsg,
				     nDays);
			}
			break;
		}

		if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_TIMED_COUNT) {
			if (stConstraintInfo.remaining_timed_count == 2) {
				szMsg =
				    g_strdup_printf
				    (VP_PLAY_STRING_DRM_PS_2_MORE_TIME_START_NOW_Q,
				     szMsg);
			} else if (stConstraintInfo.remaining_timed_count == 1) {
				szMsg =
				    g_strdup_printf
				    (VP_PLAY_STRING_DRM_PS_1_MORE_TIME_START_NOW_Q,
				     szMsg);
			} else if (stConstraintInfo.remaining_timed_count == 0) {
				bNotiPopup = TRUE;
			}
			break;
		}

		if (stConstraintInfo.
		        constraints & VP_DRM_CONSTRAINT_ACCUMLATED_TIME) {
			if (stConstraintInfo.remaining_acc_sec == 0) {
				bNotiPopup = TRUE;
			}
			break;
		}
	}
#endif
	if (bNotiPopup) {
		*bIsAvailablePlay = FALSE;

		VP_EVAS_DEL(pMultiView->pPopup);
		pMultiView->pPopup = NULL;

		char *szMessage = NULL;
		szMessage =
		    g_strdup_printf(VP_PLAY_STRING_DRM_CURRENTLY_LOCKED_UNLOCK_Q,
		                    szTitle);

		pMultiView->pPopup =
		    vp_popup_create(pPlayView->pWin,
		                    POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                    VP_PLAY_STRING_COM_ERROR, szMessage, 3.0,
		                    __vp_multi_error_popup_time_out_cb, NULL,
		                    NULL, pMultiView);

		evas_object_show(pMultiView->pPopup);

		VP_FREE(szTitle);
		VP_FREE(szMessage);
		VP_FREE(szMsg);

		return TRUE;
	}

	VP_FREE(szTitle);

	if (szMsg) {
		*bIsAvailablePlay = FALSE;

		VP_EVAS_DEL(pMultiView->pPopup);
		pMultiView->pPopup = NULL;

		pMultiView->pPopup =
		    vp_popup_create(pPlayView->pWin,
		                    POPUP_STYLE_DEFAULT_WITH_CANCEL_BTN,
		                    VP_PLAY_STRING_COM_ERROR, szMsg, 0.0, NULL,
		                    NULL, NULL, pMultiView);
		evas_object_show(pMultiView->pPopup);

		Evas_Object *pButton1 = NULL;
		Evas_Object *pButton2 = NULL;

		pButton1 = elm_button_add(pMultiView->pPopup);
		elm_object_style_set(pButton1, "popup_button/default");
		elm_object_domain_translatable_text_set(pButton1,
		                                        VP_SYS_STR_PREFIX,
		                                        VP_PLAY_STRING_COM_YES_IDS);
		elm_object_part_content_set(pMultiView->pPopup, "button1",
		                            pButton1);
		evas_object_smart_callback_add(pButton1, "clicked",
		                               __vp_multi_drm_yes_button_cb,
		                               (void *) pMultiView);

		pButton2 = elm_button_add(pMultiView->pPopup);
		elm_object_style_set(pButton2, "popup_button/default");
		elm_object_domain_translatable_text_set(pButton2,
		                                        VP_SYS_STR_PREFIX,
		                                        VP_PLAY_STRING_COM_NO_IDS);
		elm_object_part_content_set(pMultiView->pPopup, "button2",
		                            pButton2);

		evas_object_smart_callback_add(pButton2, "clicked",
		                               __vp_multi_drm_no_button_cb,
		                               (void *) pMultiView);


		VP_FREE(szMsg);

		return TRUE;

	}
	return TRUE;
}

#ifndef ENABLE_SUBTITLE
static void _vp_play_multi_view_free_subtitle_list(GList *pSubtitleList)
{
	if (pSubtitleList) {
		int nCount = 0;
		int i = 0;
		nCount = g_list_length(pSubtitleList);
		for (i = 0; i < nCount; i++) {
			char *szName = NULL;
			szName = (char *) g_list_nth_data(pSubtitleList, i);
			VP_FREE(szName);
		}

		g_list_free(pSubtitleList);
	}
}
#endif

static bool _vp_play_multi_view_play_start(MultiView *pMultiView,
                        bool bCheckDRM)
{

	if (!pMultiView) {
		VideoLogError("No Exist pUserData.");
		return FALSE;
	}

	if (pMultiView->bActivate == FALSE) {
		VideoLogError("pMultiView is NOT realized. skip");
		return FALSE;
	}

	if (pMultiView->pPlayerHandle) {
		vp_mm_player_destroy(pMultiView->pPlayerHandle);
		pMultiView->pPlayerHandle = NULL;
	}

	VideoLogInfo("_vp_play_multi_view_play_start");

	VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);
	elm_object_signal_emit(pMultiView->pLayout,
	                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");

	if (pMultiView->pImageBufferObj) {
		elm_object_part_content_unset(pMultiView->pLayout,
		                              VP_PLAY_SWALLOW_MULTI_IMAGE_BUFFER);
		VP_EVAS_DEL(pMultiView->pImageBufferObj);
	}

	pMultiView->bIsExistSubtitle = FALSE;
	pMultiView->bIsRealize = FALSE;
	pMultiView->bBufferingComplete = TRUE;

	VP_EVAS_TIMER_DEL(pMultiView->pSubtitleTimer);
	pMultiView->pSubtitleTimer = NULL;
	VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

	PlayView *pPlayView = pMultiView->pPlayView;
	if (pPlayView == NULL) {
		VideoLogWarning("pPlayView is NULL");
		return FALSE;
	}

	VP_FREE(pPlayView->szMediaURL);
	VP_STRDUP(pPlayView->szMediaURL, pMultiView->szMediaURL);

	_vp_play_multi_view_on_nocontents_mode(pMultiView, FALSE);

	VP_EVAS_DEL(pMultiView->pVideoSink);
	pMultiView->pVideoSink =
	    _vp_play_multi_view_create_image_sink(pMultiView->pLayout,
	            (void *) pMultiView);
	if (pMultiView->pVideoSink == NULL) {
		VideoLogError
		("_vp_play_multi_view_create_image_sink handle is null");
		return FALSE;
	}

	if (pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {

		if (pMultiView->szMediaURL == NULL) {
			_vp_play_multi_view_on_nocontents_mode(pMultiView, TRUE);
			pMultiView->bIsRealize = TRUE;
			return TRUE;
		}

		if (strlen(pMultiView->szMediaURL) <= 0) {
			_vp_play_multi_view_on_nocontents_mode(pMultiView, TRUE);
			pMultiView->bIsRealize = TRUE;
			return TRUE;
		}

		if (!vp_play_util_local_file_check(pMultiView->szMediaURL)) {
			/* popup show */
			_vp_play_multi_view_on_nocontents_mode(pMultiView, TRUE);
			pMultiView->bIsRealize = TRUE;
			return TRUE;
		}
	}

	if (bCheckDRM) {
		if (pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
		        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {
			bool bIsAvailablePlay = TRUE;
			if (_vp_play_multi_view_check_drm
			        (pMultiView, &bIsAvailablePlay)) {
				if (bIsAvailablePlay == FALSE) {
					VideoLogWarning("Wait Drm popup result");
					return TRUE;
				}
			}
		}
	}

	if (pMultiView->nStartPosition == 0) {
		if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_STORE ||
		        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_LIST ||
		        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_OTHER) {
			int nStartPos = 0;
			if (vp_media_contents_get_played_position
			        (pMultiView->szMediaURL, &nStartPos)) {
				pMultiView->nStartPosition = nStartPos;
			}
		}
	}

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_STORE ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_LIST ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_OTHER) {
		vp_media_contents_set_played_time(pMultiView->szMediaURL);
	}

	vp_play_subtitle_set_text(pMultiView->pSubtitle, NULL);

	/* mm_player create */
	pMultiView->pPlayerHandle = vp_mm_player_create();
	if (pMultiView->pPlayerHandle == NULL) {
		VideoLogError("vp_mm_player_create fail");
		return FALSE;
	}
	if (!vp_mm_player_set_user_param
	        (pMultiView->pPlayerHandle, (void *) pMultiView)) {
		VideoLogError("vp_mm_player_set_user_param fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_REALIZE_ASYNC_CB,
	         (void *) __vp_multi_view_prepare_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_SEEK_COMPLETE_CB,
	         (void *) __vp_multi_view_seek_completed_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_END_OF_STREAM_CB,
	         (void *) __vp_multi_view_completed_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_INTERRUPT_CB,
	         (void *) __vp_multi_view_interrupted_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_ERROR_CB,
	         (void *) __vp_multi_view_error_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_BUFFERING_CB,
	         (void *) __vp_multi_view_buffering_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_SUBTITLE_UPDATE_CB,
	         (void *) __vp_multi_view_subtitle_updated_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_PD_MESSAGE_CB,
	         (void *) __vp_multi_view_pd_message_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_MISSED_PLUGIN_CB,
	         (void *) __vp_multi_view_missed_plugin_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback
	        (pMultiView->pPlayerHandle, VP_MM_PLAYER_IMAGE_BUFFER_CB,
	         (void *) __vp_multi_view_image_buffer_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}


	if (!vp_mm_player_set_scaling(pMultiView->pPlayerHandle, TRUE)) {
		VideoLogError("vp_mm_player_set_scaling fail");
		return FALSE;
	}

	if (!vp_mm_player_set_video_sink(pMultiView->pPlayerHandle,
	                                 VP_MM_PLAYER_VIDEO_TYPE_EVAS,
	                                 (void *) pMultiView->pVideoSink)) {
		VideoLogError("vp_mm_player_set_video_sink fail");
		_vp_play_multi_view_destroy_handle(pMultiView);
		return FALSE;
	}

	if (!vp_mm_player_set_hub_download_mode
	        (pMultiView->pPlayerHandle, pPlayView->bStoreDownload)) {
		VideoLogError("vp_mm_player_set_hub_download_mode fail");
	}

	char *szSubtitle = NULL;

	if (pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {
		char *szTitle =
		    vp_play_util_get_title_from_path((char *) pMultiView->
		                                     szMediaURL);
		edje_object_part_text_set(_EDJ(pMultiView->pLayout),
		                          VP_PLAY_PART_MULTI_TITLE, szTitle);
		VP_FREE(szTitle);

		if (pMultiView->szSubtitleURL) {
			VP_STRDUP(szSubtitle, pMultiView->szSubtitleURL);
		} else {
			vp_play_util_get_subtitle_path(pMultiView->szMediaURL,
			                               &szSubtitle);
		}

		if (szSubtitle) {
			pMultiView->bIsExistSubtitle = TRUE;
			vp_mm_player_set_subtitle_url(pMultiView->pPlayerHandle,
			                              szSubtitle);
		}

		VP_FREE(pMultiView->szSubtitleURL);
		VP_FREE(szSubtitle);
	} else {
		if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
			edje_object_part_text_set(_EDJ(pMultiView->pLayout),
			                          VP_PLAY_PART_MULTI_TITLE,
			                          VP_PLAY_STRING_STREAMING_PLAYER);
			if (pPlayView->szCookie) {
				if (!vp_mm_player_set_cookie
				        (pMultiView->pPlayerHandle, pPlayView->szCookie)) {
					VideoLogError("vp_mm_player_set_cookie fail");
				}
			}
			if (pPlayView->szProxy) {
				if (!vp_mm_player_set_proxy
				        (pMultiView->pPlayerHandle, pPlayView->szProxy)) {
					VideoLogError("vp_mm_player_set_proxy fail");
				}
			}
		} else if (pMultiView->nLaunchingType ==
		           VIDEO_PLAY_TYPE_MULTI_PATH) {
			char *szMultiPathURL = NULL;
			char *szMultiSubTitle = NULL;
			bool bIsSameAP = TRUE;
			int nMultiPathPosition = 0;
			int nMultiPathDuration = 0;
			char *szTitle = NULL;

			vp_multi_path_get_current_item(pMultiView->szMediaURL,
			                               &szMultiPathURL, &szTitle,
			                               &szMultiSubTitle,
			                               &nMultiPathPosition,
			                               &nMultiPathDuration,
			                               &bIsSameAP,
			                               pPlayView->pMultiPathList);
			pMultiView->nStartPosition = nMultiPathPosition;
			pMultiView->nDuration = nMultiPathDuration;

			if (szTitle == NULL) {
				szTitle =
				    vp_play_util_get_title_from_path((char *) pMultiView->
				                                     szMediaURL);
			}

			if (szTitle) {
				edje_object_part_text_set(_EDJ(pMultiView->pLayout),
				                          VP_PLAY_PART_MULTI_TITLE,
				                          szTitle);
			} else {
				edje_object_part_text_set(_EDJ(pMultiView->pLayout),
				                          VP_PLAY_PART_MULTI_TITLE,
				                          VP_PLAY_STRING_NO_TITLE);
			}

			if (szMultiSubTitle) {
				pMultiView->bIsExistSubtitle = TRUE;
				vp_mm_player_set_subtitle_url(pMultiView->pPlayerHandle,
				                              szMultiSubTitle);
			} else {
				vp_play_util_status_noti_show
				(VP_PLAY_STRING_ERROR_SUBTITLE_FAIL);
			}

			VP_FREE(szTitle);
			VP_FREE(szMultiPathURL);
			VP_FREE(szMultiSubTitle);
		}
	}

	if (pMultiView->nAudioTrackIndex > 0) {
		vp_mm_player_set_audio_track(pMultiView->pPlayerHandle,
		                             pMultiView->nAudioTrackIndex);
	}

	if (!vp_mm_player_realize_async
	        (pMultiView->pPlayerHandle, pMultiView->szMediaURL)) {
		VideoLogError("vp_mm_player_realize_async fail");
		vp_play_util_status_noti_show(VP_PLAY_STRING_ERROR_UNABLE_PLAY);
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		return TRUE;
	}

	vp_mm_player_sound_filter_t nSoundFilter = VP_MM_PLAYER_FILTER_NONE;

	if (pMultiView->nSoundAlive == VIDEO_SA_NORMAL) {
		nSoundFilter = VP_MM_PLAYER_FILTER_NONE;
	} else if (pMultiView->nSoundAlive == VIDEO_SA_VOICE) {
		nSoundFilter = VP_MM_PLAYER_FILTER_VOICE;
	} else if (pMultiView->nSoundAlive == VIDEO_SA_MOVIE) {
		nSoundFilter = VP_MM_PLAYER_FILTER_MOVIE;
	} else if (pMultiView->nSoundAlive == VIDEO_SA_7_1_CH) {
		nSoundFilter = VP_MM_PLAYER_FILTER_VITUAL_71;

		video_sound_device_type_t nSoundDevType = VP_SOUND_DEVICE_NONE;
		vp_sound_get_active_device(&nSoundDevType);

		if (nSoundDevType != VP_SOUND_DEVICE_EARJACK) {
			nSoundFilter = VP_MM_PLAYER_FILTER_NONE;
		}
	}

	if (nSoundFilter != VP_MM_PLAYER_FILTER_NONE) {
		if (!vp_mm_player_set_sound_filter
		        (pMultiView->pPlayerHandle, nSoundFilter)) {
			VideoLogWarning("vp_mm_player_set_sound_filter is fail");
		}
	}
//      _vp_play_multi_view_show_layout(pMultiView);

	return TRUE;
}

static void _vp_play_multi_view_on_next_play(MultiView *pMultiView,
        bool bManual)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return;
	}

	bool bIsExit = FALSE;

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW) {
		return;
	}

	pMultiView->nStartPosition = 0;
	pMultiView->nAudioTrackIndex = 0;

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_NONE) {
			if (bManual) {
				return;
			} else {
				bIsExit = TRUE;
			}
		} else {
			_vp_play_multi_view_play_start(pMultiView, FALSE);
		}
	} else if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		PlayView *pPlayView = pMultiView->pPlayView;

		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			bIsExit = TRUE;
		} else if (pPlayView->pMultiPathList == NULL) {
			VideoLogError("pMultiPathList is NULL");
			bIsExit = TRUE;
		} else {
			if (bManual) {
				_vp_play_multi_view_set_played_time(pMultiView);
				_vp_play_multi_view_set_preview(pMultiView);

				char *szNextURL = NULL;
				char *szSubtitle = NULL;
				int nPosition = 0;
				int nDuration = 0;
				vp_multi_path_get_next_item(pMultiView->szMediaURL,
				                            &szNextURL, &szSubtitle,
				                            &nPosition, &nDuration, TRUE,
				                            pPlayView->pMultiPathList);
				VP_FREE(szSubtitle);
				if (szNextURL) {
					VP_FREE(pMultiView->szMediaURL);
					VP_STRDUP(pMultiView->szMediaURL, szNextURL);
					VP_FREE(szNextURL);
					pMultiView->nDuration = nDuration;
					_vp_play_multi_view_play_start(pMultiView, TRUE);
				} else {
					_vp_play_multi_view_play_start(pMultiView, TRUE);
				}
				return;
			}

			if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_OFF) {
				bIsExit = TRUE;
			} else if (pMultiView->nRepeatMode ==
			           VIDEO_PLAY_REPEAT_ALL_STOP) {
				char *szNextURL = NULL;
				char *szSubtitle = NULL;
				int nPosition = 0;
				int nDuration = 0;
				vp_multi_path_get_next_item(pMultiView->szMediaURL,
				                            &szNextURL, &szSubtitle,
				                            &nPosition, &nDuration, FALSE,
				                            pPlayView->pMultiPathList);
				VP_FREE(szSubtitle);
				if (szNextURL == NULL) {
					bIsExit = TRUE;
				} else {
					VP_FREE(pMultiView->szMediaURL);
					VP_STRDUP(pMultiView->szMediaURL, szNextURL);
					VP_FREE(szNextURL);
					pMultiView->nDuration = nDuration;
					_vp_play_multi_view_play_start(pMultiView, TRUE);
				}
			} else if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE) {
				_vp_play_multi_view_play_start(pMultiView, TRUE);
			} else if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
				char *szNextURL = NULL;
				char *szSubtitle = NULL;
				int nPosition = 0;
				int nDuration = 0;
				vp_multi_path_get_next_item(pMultiView->szMediaURL,
				                            &szNextURL, &szSubtitle,
				                            &nPosition, &nDuration, TRUE,
				                            pPlayView->pMultiPathList);
				VP_FREE(szSubtitle);
				if (szNextURL == NULL) {
					bIsExit = TRUE;
				} else {
					VP_FREE(pMultiView->szMediaURL);
					VP_STRDUP(pMultiView->szMediaURL, szNextURL);
					VP_FREE(szNextURL);
					pMultiView->nDuration = nDuration;
					_vp_play_multi_view_play_start(pMultiView, TRUE);
				}
			}
		}
	} else {
		if (bManual) {
			_vp_play_multi_view_set_played_time(pMultiView);
			_vp_play_multi_view_set_preview(pMultiView);

			char *szNextURL = NULL;
			vp_media_contents_get_next_file_path(pMultiView->szMediaURL,
			                                     &szNextURL, TRUE,
			                                     pMultiView->
			                                     pMediaItemList);
			if (szNextURL) {
				VP_FREE(pMultiView->szMediaURL);
				VP_STRDUP(pMultiView->szMediaURL, szNextURL);
				VP_FREE(szNextURL);
				_vp_play_multi_view_play_start(pMultiView, TRUE);
			} else {
				_vp_play_multi_view_play_start(pMultiView, TRUE);
			}
			return;
		}

		if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_OFF) {
			bIsExit = TRUE;
		} else if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL_STOP) {
			char *szNextURL = NULL;
			vp_media_contents_get_next_file_path(pMultiView->szMediaURL,
			                                     &szNextURL, FALSE,
			                                     pMultiView->
			                                     pMediaItemList);
			if (szNextURL == NULL) {
				bIsExit = TRUE;
			} else {
				VP_FREE(pMultiView->szMediaURL);
				VP_STRDUP(pMultiView->szMediaURL, szNextURL);
				VP_FREE(szNextURL);
				_vp_play_multi_view_play_start(pMultiView, TRUE);
			}
		} else if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE) {
			_vp_play_multi_view_play_start(pMultiView, TRUE);
		} else if (pMultiView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
			char *szNextURL = NULL;
			vp_media_contents_get_next_file_path(pMultiView->szMediaURL,
			                                     &szNextURL, TRUE,
			                                     pMultiView->
			                                     pMediaItemList);
			if (szNextURL == NULL) {
				bIsExit = TRUE;
			} else {
				VP_FREE(pMultiView->szMediaURL);
				VP_STRDUP(pMultiView->szMediaURL, szNextURL);
				VP_FREE(szNextURL);
				_vp_play_multi_view_play_start(pMultiView, TRUE);
			}
		}
	}

	if (bIsExit) {

		PlayView *pPlayView = pMultiView->pPlayView;

		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			return;
		}

		if (pPlayView->pFunc == NULL) {
			VideoLogError("pPlayView pFunc is NULL");
			return;
		}

		if (pPlayView->pFunc->vp_play_func_exit == NULL) {
			VideoLogError("pPlayView exit Func is NULL");
			return;
		}

		vp_play_util_set_unlock_power_key();

		pPlayView->pFunc->vp_play_func_exit(pPlayView);
	}

}

static void _vp_play_multi_view_on_prev_play(MultiView *pMultiView,
                        bool bManual, bool bVoice)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return;
	}

	pMultiView->nAudioTrackIndex = 0;

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		_vp_play_multi_view_play_start(pMultiView, FALSE);
		return;
	}

	int nPosition = 0;
	if (!vp_mm_player_get_position(pMultiView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
	}

	pMultiView->nStartPosition = 0;
	if (bVoice == FALSE) {
		if (nPosition > 2000) {
			vp_mm_player_set_position(pMultiView->pPlayerHandle, 0);
			return;
		}
	}

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW) {
		return;
	}

	if (bManual) {
		_vp_play_multi_view_set_played_time(pMultiView);
		_vp_play_multi_view_set_preview(pMultiView);
	}

	char *szPrevURL = NULL;
	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		PlayView *pPlayView = pMultiView->pPlayView;

		if (pPlayView == NULL) {
			_vp_play_multi_view_play_start(pMultiView, TRUE);
			return;
		} else if (pPlayView->pMultiPathList == NULL) {
			_vp_play_multi_view_play_start(pMultiView, TRUE);
			return;
		} else {
			char *szSubtitle = NULL;
			int nPosition = 0;
			int nDuration = 0;
			vp_multi_path_get_next_item(pMultiView->szMediaURL,
			                            &szPrevURL, &szSubtitle,
			                            &nPosition, &nDuration, TRUE,
			                            pPlayView->pMultiPathList);
			VP_FREE(szSubtitle);
			if (szPrevURL) {
				VP_FREE(pMultiView->szMediaURL);
				VP_STRDUP(pMultiView->szMediaURL, szPrevURL);
				VP_FREE(szPrevURL);
				pMultiView->nDuration = nDuration;
			}

			_vp_play_multi_view_play_start(pMultiView, TRUE);
		}

	} else {
		vp_media_contents_get_prev_file_path(pMultiView->szMediaURL,
		                                     &szPrevURL, TRUE,
		                                     pMultiView->pMediaItemList);
		if (szPrevURL) {
			VP_FREE(pMultiView->szMediaURL);
			VP_STRDUP(pMultiView->szMediaURL, szPrevURL);
			VP_FREE(szPrevURL);
		}

		_vp_play_multi_view_play_start(pMultiView, TRUE);
	}


}

static void _vp_play_multi_view_set_preview(MultiView *pMultiView)
{
	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_LIST ||
	        pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_OTHER) {
		bool bIsCloud = FALSE;
		vp_media_contents_get_cloud_attribute(pMultiView->szMediaURL,
		                                      &bIsCloud);
		if (!bIsCloud) {
			PlayView *pPlayView = pMultiView->pPlayView;
			if (pPlayView == NULL) {
				VideoLogError("pPlayView is NULL");
				return;
			}
			if (pPlayView->bViewChange == FALSE) {
				if (vp_file_exists(pMultiView->szMediaURL)) {
					VideoSecureLogInfo("pMultiView-path = %s",
					                   pMultiView->szMediaURL);
					vp_play_config_set_preview_url_videos(pMultiView->
					                                      szMediaURL);
				}
			}
			vp_play_preference_set_preview_audio_track(pMultiView->
			        nAudioTrackIndex);
		}
	}
}

static void _vp_play_multi_view_set_played_time(MultiView *pMultiView)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return;
	}
	PlayView *pPlayView = pMultiView->pPlayView;
	int nPosition = 0;
	if (pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_PREVIEW &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_MESSAGE &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_EMAIL &&
	        pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {

		if (!vp_mm_player_get_position
		        (pMultiView->pPlayerHandle, &nPosition)) {
			VideoLogError("vp_mm_player_get_position is fail");
		} else {
			if (nPosition == pMultiView->nDuration) {
				vp_media_contents_set_played_position(pMultiView->
				                                      szMediaURL, 0);

			} else {
				vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
				if (!vp_mm_player_get_state
				        (pMultiView->pPlayerHandle, &nState)) {
					VideoLogWarning("vp_mm_player_get_state is fail");
				}

				if (nState == VP_MM_PLAYER_STATE_PLAYING) {
					nPosition -= 5000;
				}

				if (nPosition < 0) {
					nPosition = 0;
				}
				vp_media_contents_set_played_position(pMultiView->
				                                      szMediaURL,
				                                      nPosition);
			}
		}
	} else if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		int nPosition = 0;
		if (!vp_mm_player_get_position
		        (pMultiView->pPlayerHandle, &nPosition)) {
			VideoLogError("vp_mm_player_get_position is fail");
		} else {
			if (pPlayView == NULL) {
				VideoLogError("pPlayView is NULL");
				return;
			}
			if (pPlayView->pMultiPathList == NULL) {
				VideoLogError("pMultiPathList is NULL");
				return;
			}
			vp_multi_path_set_item_position(pMultiView->szMediaURL,
			                                nPosition,
			                                pPlayView->pMultiPathList);
		}
	}
}


static Evas_Object *_vp_play_multi_view_create_layout(Evas_Object *
        pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;
	Eina_Bool bRet = EINA_FALSE;

	pObj = elm_layout_add(pParent);
	if (pObj == NULL) {
		VideoLogError("elm_layout_add object is NULL");
		return NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VP_PLAY_MULTI_MAIN_EDJ);
	free(path);
	bRet =
	    elm_layout_file_set(pObj, edj_path,
	                        VP_PLAY_EDJ_GROUP_MULTI);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
	}
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);

	evas_object_show(pObj);

	return pObj;

}

static Evas_Object *_vp_play_multi_view_create_image_sink(void *pParent,
                        void *pUserData)
{

	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return NULL;
	}

	MultiView *pMultiView = (MultiView *) pUserData;
	Evas *pEvas = NULL;
	Evas_Object *pObj = NULL;

	pEvas = evas_object_evas_get(pParent);

	pObj = evas_object_image_add(pEvas);
	if (NULL == pObj) {
		VideoLogError("pObj is NULL");
		return NULL;
	}

	evas_object_image_size_set(pObj, VP_MULTI_DEFAULT_WIDTH,
	                           VP_MULTI_DEFAULT_HEIGHT);
	evas_object_resize(pObj, VP_MULTI_DEFAULT_WIDTH,
	                   VP_MULTI_DEFAULT_HEIGHT);

	void *pImageBuf = evas_object_image_data_get(pObj, EINA_TRUE);
	if (NULL == pImageBuf) {
		VideoLogError("pImageBuf is NULL");
		VP_EVAS_DEL(pObj);
		return NULL;
	}

	int nBufSize = VP_MULTI_DEFAULT_WIDTH * VP_MULTI_DEFAULT_HEIGHT * 4;

	memset(pImageBuf, 0, nBufSize);
	evas_object_image_data_set(pObj, pImageBuf);

	evas_object_event_callback_add(pObj, EVAS_CALLBACK_RESIZE,
	                               __vp_play_multi_view_imagesink_resize_cb,
	                               (void *) pMultiView);

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_SINK,
	                            pObj);

	return pObj;

}

static void _vp_play_multi_view_create_layout_hide_timer(MultiView *
                        pMultiView)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);

	pMultiView->pHideTimer =
	    ecore_timer_add(VP_MULTI_HIDE_LAYOUT_TIMER_INTERVAL,
	                    __vp_multi_hide_layout_timer_cb,
	                    (void *) pMultiView);

}

static void _vp_play_multi_view_show_layout(MultiView *pMultiView)
{
	if (!pMultiView) {
		VideoLogError("pMultiView handle is NULL");
		return;
	}

	if (!pMultiView->pLayout) {
		VideoLogError("pMultiView layout handle is NULL");
		return;
	}

	if (pMultiView->bNoContentMode) {
		VideoLogInfo("No videos show");
		return;
	}

	PlayView *pPlayView = pMultiView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	if (!pPlayView->pWin) {
		VideoLogError("pPlayView->pWin is NULL");
		return;
	}

	bool bPlaying = FALSE;

	if (pMultiView->nPlayerState == VP_MM_PLAYER_STATE_PLAYING) {
		bPlaying = TRUE;
	}

	evas_object_show(pMultiView->pExitBtn);
	evas_object_show(pMultiView->pFullSizeBtn);
	evas_object_show(pMultiView->pMinSizeBtn);

	evas_object_show(pMultiView->pPrevBtn);
	evas_object_show(pMultiView->pNextBtn);

	elm_object_part_content_unset(pMultiView->pLayout,
	                              VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE);

	if (bPlaying) {
		evas_object_hide(pMultiView->pPlayBtn);
		evas_object_show(pMultiView->pPauseBtn);
		elm_object_part_content_set(pMultiView->pLayout,
		                            VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE,
		                            pMultiView->pPauseBtn);
	} else {
		evas_object_hide(pMultiView->pPauseBtn);
		evas_object_show(pMultiView->pPlayBtn);
		elm_object_part_content_set(pMultiView->pLayout,
		                            VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE,
		                            pMultiView->pPlayBtn);
	}

	elm_object_signal_emit(pMultiView->pLayout,
	                       VP_MULTI_VIEW_SIGNAL_SHOW_CONTROL, "*");

	pMultiView->bShowLayout = TRUE;

	vp_play_multi_view_set_main_layout_focus_out((void *) pPlayView->
	        pMultiView);
	_vp_play_multi_view_set_button_focus_sequence(pMultiView);
	_vp_play_multi_view_create_layout_hide_timer(pMultiView);

}

static void _vp_play_multi_view_hide_layout(MultiView *pMultiView)
{
	if (!pMultiView) {
		VideoLogError("pMultiView handle is NULL");
		return;
	}

	if (!pMultiView->pLayout) {
		VideoLogError("pMultiView layout handle is NULL");
		return;
	}

	PlayView *pPlayView = pMultiView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	evas_object_hide(pMultiView->pPlayBtn);
	evas_object_hide(pMultiView->pPauseBtn);
	evas_object_hide(pMultiView->pPlayFocusBtn);
	evas_object_hide(pMultiView->pPrevBtn);
	evas_object_hide(pMultiView->pNextBtn);

	elm_object_signal_emit(pMultiView->pLayout,
	                       VP_MULTI_VIEW_SIGNAL_HIDE_CONTROL, "*");

	vp_play_multi_view_set_main_layout_focus_out((void *) pPlayView->
	        pMultiView);
	pMultiView->bShowLayout = FALSE;

}

static void _vp_play_multi_view_set_play_state(MultiView *pMultiView)
{
	if (!pMultiView) {
		VideoLogError("No Exist pUserData.");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pMultiView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	pMultiView->nPlayerState = nState;

	elm_object_part_content_unset(pMultiView->pLayout,
	                              VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE);
	switch (nState) {
	case VP_MM_PLAYER_STATE_NONE:
	case VP_MM_PLAYER_STATE_IDLE:
	case VP_MM_PLAYER_STATE_READY:
	case VP_MM_PLAYER_STATE_PAUSED:
		VideoLogWarning("VP_MM_PLAYER_STATE_PAUSED");
		evas_object_hide(pMultiView->pPauseBtn);
		evas_object_show(pMultiView->pPlayBtn);
		elm_object_part_content_set(pMultiView->pLayout,
		                            VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE,
		                            pMultiView->pPlayBtn);
		vp_play_util_set_unlock_power_key();
		break;
	case VP_MM_PLAYER_STATE_PLAYING:
		VideoLogWarning("VP_MM_PLAYER_STATE_PLAYING");
		evas_object_hide(pMultiView->pPlayBtn);
		evas_object_show(pMultiView->pPauseBtn);
		elm_object_part_content_set(pMultiView->pLayout,
		                            VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE,
		                            pMultiView->pPauseBtn);
		vp_play_util_set_lock_power_key();
		break;
	case VP_MM_PLAYER_STATE_STOP:
		break;
	}

	return;
}

static bool _vp_play_multi_view_init_buttons(MultiView *pMultiView)
{
	if (!pMultiView) {
		VideoLogError("play view handle is NULL");
		return FALSE;
	}

	if (!pMultiView->pLayout) {
		VideoLogError("play view layout handle is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pMultiView->pLayout;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	pMultiView->pExitBtn =
	    vp_button_create(pParent, "playview/custom/flat_94_60/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pExitBtn) {
		VideoLogError("vp_play_util_create_buttonis fail");
		return FALSE;
	}

	pMultiView->pFullSizeBtn =
	    vp_button_create(pParent, "playview/custom/flat_94_60/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pFullSizeBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pMultiView->pMinSizeBtn =
	    vp_button_create(pParent, "playview/custom/flat_94_60/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pMinSizeBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pMultiView->pResizeBtn =
	    vp_button_create(pParent, "playview/custom/flat_36_36/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pResizeBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}


	pMultiView->pPlayBtn =
	    vp_button_create(pParent, "playview/custom/round_center/default",
	                     VP_PLAY_STRING_COM_PLAY,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pPlayBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pMultiView->pPauseBtn =
	    vp_button_create(pParent, "playview/custom/round_center/default",
	                     VP_PLAY_STRING_COM_PAUSE,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pPauseBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pMultiView->pPlayFocusBtn = vp_button_create(pParent, "focus", NULL,
	                            (Evas_Smart_Cb)
	                            __vp_play_multi_view_btn_clicked_cb,
	                            (Evas_Smart_Cb)
	                            __vp_play_multi_view_btn_press_cb,
	                            (Evas_Smart_Cb)
	                            __vp_play_multi_view_btn_unpress_cb,
	                            (void *) pMultiView);
	if (!pMultiView->pPlayFocusBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pMultiView->pPrevBtn =
	    vp_button_create(pParent, "playview/custom/round_left/default",
	                     VP_PLAY_STRING_COM_PREVIOUS,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pPrevBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pMultiView->pNextBtn =
	    vp_button_create(pParent, "playview/custom/round_right/default",
	                     VP_PLAY_STRING_COM_NEXT,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_clicked_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_multi_view_btn_unpress_cb,
	                     (void *) pMultiView);
	if (!pMultiView->pNextBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	Evas_Object *pIcon = NULL;
	pIcon =
	    vp_button_create_icon(pMultiView->pResizeBtn,
	                          edj_path,
	                          VP_PLAY_MULTI_VIEW_RESIZE);
	elm_object_part_content_set(pMultiView->pResizeBtn,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pMultiView->pResizeBtn,
	                          edj_path,
	                          VP_PLAY_MULTI_VIEW_RESIZE_PRESS);
	elm_object_part_content_set(pMultiView->pResizeBtn,
	                            VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pMultiView->pResizeBtn,
	                          edj_path,
	                          VP_PLAY_MULTI_VIEW_RESIZE);
	elm_object_part_content_set(pMultiView->pResizeBtn,
	                            VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);


	pIcon =
	    vp_button_create_icon(pMultiView->pPlayBtn,
	                          edj_path,
	                          VP_PLAY_MULTI_VIEW_PLAY);
	elm_object_part_content_set(pMultiView->pPlayBtn,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pMultiView->pPauseBtn,
	                          edj_path,
	                          VP_PLAY_MULTI_VIEW_PAUSE);
	elm_object_part_content_set(pMultiView->pPauseBtn,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pMultiView->pPrevBtn,
	                          edj_path,
	                          VP_PLAY_MULTI_VIEW_REW);
	elm_object_part_content_set(pMultiView->pPrevBtn,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pMultiView->pNextBtn,
	                          edj_path,
	                          VP_PLAY_MULTI_VIEW_FF);
	elm_object_part_content_set(pMultiView->pNextBtn,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_CLOSE,
	                            pMultiView->pExitBtn);
	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_FULL_SIZE,
	                            pMultiView->pFullSizeBtn);
	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_MINIMIZE,
	                            pMultiView->pMinSizeBtn);
	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_RESIZE,
	                            pMultiView->pResizeBtn);

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE,
	                            pMultiView->pPauseBtn);
	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_PLAY_FOCUS,
	                            pMultiView->pPlayFocusBtn);
	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_REW,
	                            pMultiView->pPrevBtn);
	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_FF,
	                            pMultiView->pNextBtn);

	return TRUE;
}

static bool _vp_play_multi_view_create_gesture_layout(MultiView *
                        pMultiView)
{
	if (!pMultiView) {
		VideoLogError("pMultiView is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;
	Eina_Bool bRet = EINA_FALSE;

	pParent = pMultiView->pLayout;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VP_PLAY_GESTURE_EDJ);
	free(path);
	pMultiView->pMainEventRect = elm_layout_add(pParent);
	bRet =
	    elm_layout_file_set(pMultiView->pMainEventRect,
	                        edj_path,
	                        VP_PLAY_EDJ_GROUP_GESTURE);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail : %s [%s]",
		              edj_path, VP_PLAY_EDJ_GROUP_GESTURE);
		return FALSE;
	}
	elm_object_part_content_set(pParent, "pv.multi.event",
	                            pMultiView->pMainEventRect);

	pMultiView->pGestureRect =
	    elm_gesture_layer_add(pMultiView->pMainEventRect);
	if (pMultiView->pGestureRect == NULL) {
		VideoLogError("elm_gesture_layer_add is NULL");
		return FALSE;
	}

	elm_gesture_layer_cb_set(pMultiView->pGestureRect, ELM_GESTURE_N_TAPS,
	                         ELM_GESTURE_STATE_START,
	                         __vp_multi_view_gesture_n_tab_start_cb,
	                         (void *) pMultiView);
	elm_gesture_layer_cb_set(pMultiView->pGestureRect, ELM_GESTURE_N_TAPS,
	                         ELM_GESTURE_STATE_END,
	                         __vp_multi_view_gesture_n_tab_end_cb,
	                         (void *) pMultiView);
	elm_gesture_layer_cb_set(pMultiView->pGestureRect, ELM_GESTURE_N_TAPS,
	                         ELM_GESTURE_STATE_ABORT,
	                         __vp_multi_view_gesture_n_tab_abort_cb,
	                         (void *) pMultiView);

	elm_gesture_layer_attach(pMultiView->pGestureRect,
	                         pMultiView->pMainEventRect);

	evas_object_show(pMultiView->pMainEventRect);

	return TRUE;
}

static bool _vp_play_multi_view_create_subtitle(MultiView *pMultiView)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;
	Evas_Object *pObj = NULL;
	pParent = pMultiView->pLayout;

	pMultiView->pSubtitle =
	    vp_play_subtitle_create(pParent, VP_SUBTITLE_TYPE_MULTI);
	if (pMultiView->pSubtitle == NULL) {
		VideoLogError("pSubtitle create fail");
		return FALSE;
	}

	pObj = vp_play_subtitle_get_object(pMultiView->pSubtitle);
	if (pObj == NULL) {
		VideoLogError("vp_play_subtitle_get_object fail");
		return FALSE;
	}
	bool bShow = FALSE;
	vp_play_preference_get_subtitle_show_key(&bShow);
	if (bShow) {
		char *szFont = NULL;
		int nSize = 0;
		int nWinW = 0;
		int nWinH = 0;
		float fZoom = 1.0;
		PlayView *pPlayView = pMultiView->pPlayView;
		elm_win_screen_size_get(pPlayView->pWin, NULL, NULL, &nWinW,
		                        &nWinH);

		fZoom =
		    (float)((float)(VP_MULTI_DEFAULT_WIDTH) / (float)(nWinW));

		vp_play_subtitle_realize(pMultiView->pSubtitle);

		vp_play_preference_get_subtitle_size_key(&nSize);

		vp_play_preference_get_subtitle_size_key(&nSize);
		vp_play_preference_get_subtitle_font_name_key(&szFont);
#ifndef SUBTITLE_K_FEATURE
		video_subtitle_color_t nFontColor = VP_SUBTITLE_COLOR_BLACK;
		int nFontColorKey = VP_SUBTITLE_COLOR_BLACK;

		int nBGColorKey = VP_SUBTITLE_COLOR_BLACK;
		video_subtitle_color_t nBGColor = VP_SUBTITLE_COLOR_BLACK;

		vp_play_preference_get_subtitle_font_color_key(&nFontColorKey);
		vp_play_preference_get_subtitle_bg_color_key(&nBGColorKey);

		if (nFontColorKey == VIDEO_SUBTITLE_COLOR_BLACK) {
			nFontColor = VP_SUBTITLE_COLOR_BLACK;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_BLUE) {
			nFontColor = VP_SUBTITLE_COLOR_BLUE;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_GREEN) {
			nFontColor = VP_SUBTITLE_COLOR_GREEN;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_WHITE) {
			nFontColor = VP_SUBTITLE_COLOR_WHITE;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_RED) {
			nFontColor = VP_SUBTITLE_COLOR_RED;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_YELLOW) {
			nFontColor = VP_SUBTITLE_COLOR_YELLOW;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_MAGENTA) {
			nFontColor = VP_SUBTITLE_COLOR_MAGENTA;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_CYAN) {
			nFontColor = VP_SUBTITLE_COLOR_CYAN;
		} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_CUSTOM) {
			nFontColor = VP_SUBTITLE_COLOR_CUSTOM;
		}

		if (nBGColorKey == VIDEO_SUBTITLE_COLOR_BLACK) {
			nBGColor = VP_SUBTITLE_COLOR_BLACK;
		} else if (nBGColorKey == VIDEO_SUBTITLE_COLOR_WHITE) {
			nBGColor = VP_SUBTITLE_COLOR_WHITE;
		} else if (nBGColorKey == VIDEO_SUBTITLE_COLOR_NONE) {
			nBGColor = VP_SUBTITLE_COLOR_NONE;
		}

		vp_play_subtitle_set_color(pMultiView->pSubtitle, nFontColor);
		vp_play_subtitle_set_bg_color(pMultiView->pSubtitle, nBGColor);

#else
		int nAlignmentKey = VP_SUBTITLE_ALIGNMENT_CENTER;
		vp_subtitle_alignment_t nAlignment = VP_SUBTITLE_ALIGNMENT_CENTER;
		char *szColorHex = NULL;
		char *szColorBGHex = NULL;

		vp_play_preference_get_subtitle_alignment_key(&nAlignmentKey);
		if (nAlignmentKey == VIDEO_SUBTITLE_ALIGNMENT_LEFT) {
			nAlignment = VP_SUBTITLE_ALIGNMENT_LEFT;
		} else if (nAlignmentKey == VIDEO_SUBTITLE_ALIGNMENT_CENTER) {
			nAlignment = VP_SUBTITLE_ALIGNMENT_CENTER;
		} else if (nAlignmentKey == VIDEO_SUBTITLE_ALIGNMENT_RIGHT) {
			nAlignment = VP_SUBTITLE_ALIGNMENT_RIGHT;
		}

		vp_play_preference_get_subtitle_font_color_hex_key(&szColorHex);
		if (!szColorHex) {
			VideoLogError("szColorHex is NULL");
		}

		vp_play_preference_get_subtitle_bg_color_hex_key(&szColorBGHex);
		if (!szColorBGHex) {
			VideoLogError("szColorBGHex is NULL");
		}

		vp_play_subtitle_set_alignment(pMultiView->pSubtitle, nAlignment);
		vp_play_subtitle_set_color(pMultiView->pSubtitle, szColorHex);
		vp_play_subtitle_set_bg_color(pMultiView->pSubtitle,
		                              szColorBGHex);

#endif
		vp_play_subtitle_set_size(pMultiView->pSubtitle, nSize);
		vp_play_subtitle_set_size_zoom(pMultiView->pSubtitle, fZoom);

		vp_play_subtitle_set_font(pMultiView->pSubtitle, szFont);

		VP_FREE(szFont);
	}

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_MULTI_SUBTITLE,
	                            pObj);

	return TRUE;
}

static bool _vp_play_multi_view_init_layout(MultiView *pMultiView)
{
	if (!pMultiView) {
		VideoLogError("play view handle is NULL");
		return FALSE;
	}

	pMultiView->pLayout =
	    _vp_play_multi_view_create_layout(pMultiView->pNaviFrame);
	if (pMultiView->pLayout == NULL) {
		VideoLogError("_vp_play_multi_view_create_layout handle is null");
		return FALSE;
	}

	if (!_vp_play_multi_view_create_gesture_layout(pMultiView)) {
		VideoLogError
		("_vp_play_multi_view_create_gesture_layout is fail");
		return FALSE;
	}

	if (!_vp_play_multi_view_init_buttons(pMultiView)) {
		VideoLogError("_vp_play_multi_view_init_buttons is fail");
		return FALSE;
	}

	if (!_vp_play_multi_view_create_subtitle(pMultiView)) {
		VideoLogError("_vp_play_multi_view_create_subtitle fail");
		return FALSE;
	}

	if (!_vp_play_multi_view_create_loading_ani(pMultiView)) {
		VideoLogError("_vp_play_multi_view_create_loading_ani fail");
		return FALSE;
	}
	Elm_Object_Item *pNaviIt = NULL;

	pNaviIt =
	    elm_naviframe_item_push(pMultiView->pNaviFrame, NULL, NULL, NULL,
	                            pMultiView->pLayout, "playview/multiwin");
	elm_naviframe_item_pop_cb_set(pNaviIt,
	                              __vp_play_multi_view_back_key_event_cb,
	                              (void *) pMultiView);

	pMultiView->pNaviItem = pNaviIt;

	return TRUE;

}

static void _vp_play_multi_view_evas_focus_in_cb(void *pUserData,
        Evas *pEvas,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	Evas_Object *pLayout = pUserData;
	elm_object_signal_emit(pLayout, VP_MULTI_VIEW_TITLE_FOCUS_IN_CONTROL,
	                       "*");

	return;
}

static void _vp_play_multi_view_evas_focus_out_cb(void *pUserData,
                        Evas *pEvas,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	Evas_Object *pLayout = pUserData;
	elm_object_signal_emit(pLayout, VP_MULTI_VIEW_TITLE_FOCUS_OUT_CONTROL,
	                       "*");

	return;
}


static void _vp_play_multi_view_destroy_handle(MultiView *pMultiView)
{
	if (!pMultiView) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pMultiView->pPlayerHandle) {
		vp_mm_player_destroy(pMultiView->pPlayerHandle);
		pMultiView->pPlayerHandle = NULL;
	}

	VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
	VP_EVAS_TIMER_DEL(pMultiView->pSubtitleTimer);

	if (pMultiView->pMediaKey) {
		vp_media_key_destroy(pMultiView->pMediaKey);
		pMultiView->pMediaKey = NULL;
	}

	vp_play_loading_ani_destroy(pMultiView->pLoadingAni);
	pMultiView->pLoadingAni = NULL;

	vp_play_subtitle_destroy(pMultiView->pSubtitle);
	pMultiView->pSubtitle = NULL;

	if (pMultiView->pDownloadHandle) {
		VppDownloadDestroyItem(pMultiView->pDownloadHandle);
		pMultiView->pDownloadHandle = NULL;
	}

	VP_FREE(pMultiView->szSubtitleURL);
	VP_FREE(pMultiView->szMediaURL);

	VP_EVAS_DEL(pMultiView->pPopup);
	VP_EVAS_DEL(pMultiView->pExitPopupWin);

	VP_EVAS_DEL(pMultiView->pImageBufferObj);

	VP_EVAS_DEL(pMultiView->pExitBtn);
	VP_EVAS_DEL(pMultiView->pFullSizeBtn);
	VP_EVAS_DEL(pMultiView->pMinSizeBtn);
	VP_EVAS_DEL(pMultiView->pResizeBtn);
	VP_EVAS_DEL(pMultiView->pPlayBtn);
	VP_EVAS_DEL(pMultiView->pPauseBtn);
	VP_EVAS_DEL(pMultiView->pPlayFocusBtn);
	VP_EVAS_DEL(pMultiView->pPrevBtn);
	VP_EVAS_DEL(pMultiView->pNextBtn);
	VP_EVAS_DEL(pMultiView->pMainEventRect);
	VP_EVAS_DEL(pMultiView->pGestureRect);

	VP_EVAS_IDLER_DEL(pMultiView->pControlIdler);
	VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

	VP_EVAS_EVENT_HANDLER_DEL(pMultiView->pMouseDownHandle);
	VP_EVAS_EVENT_HANDLER_DEL(pMultiView->pMouseUpHandle);
	VP_EVAS_EVENT_HANDLER_DEL(pMultiView->pMouseMoveHandle);
	VP_EVAS_EVENT_HANDLER_DEL(pMultiView->pConfigureNotify);

	VP_EVAS_DEL(pMultiView->pVideoSink);
	VP_EVAS_DEL(pMultiView->pLayout);

	VP_EVAS_PIPE_DEL(pMultiView->pPreparePipe);

	if (pMultiView->pWin) {
		Evas *e = evas_object_evas_get(pMultiView->pWin);
		evas_event_callback_del(e, EVAS_CALLBACK_CANVAS_FOCUS_IN,
		                        _vp_play_multi_view_evas_focus_in_cb);
		evas_event_callback_del(e, EVAS_CALLBACK_CANVAS_FOCUS_OUT,
		                        _vp_play_multi_view_evas_focus_out_cb);
	}
}

static void _vp_play_multi_view_prepare_pipe_cb(void *data,
                        void *pipeData,
                        unsigned int nbyte)
{
	if (NULL == data) {
		VideoLogError("data is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) data;
	int nDuration = 0;
	if (!vp_mm_player_get_duration(pMultiView->pPlayerHandle, &nDuration)) {
		VideoLogError("vp_mm_player_get_duration is fail");
	}

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_WEB
	        && nDuration == 0) {
		pMultiView->bHLSMode = TRUE;
		pMultiView->nStartPosition = 0;
	} else {
		pMultiView->bHLSMode = FALSE;
	}

	pMultiView->nDuration = nDuration;

	if (pMultiView->nStartPosition > 0) {
		if (vp_mm_player_set_position
		        (pMultiView->pPlayerHandle, pMultiView->nStartPosition)) {
			pMultiView->bSeekComplete = FALSE;
		}
	} else {
		if (vp_play_util_check_streaming(pMultiView->szMediaURL)) {
			if (pMultiView->bBufferingComplete == FALSE) {
				VideoLogWarning("Skip for buffering state");
				return;
			}
		}

		pMultiView->bIsRealize = TRUE;
		pMultiView->bSeekComplete = TRUE;

		if (pMultiView->pLoadingAni) {
			vp_play_loading_ani_destroy(pMultiView->pLoadingAni);
			pMultiView->pLoadingAni = NULL;
		}

		if (pMultiView->bManualPause) {
			vp_mm_player_set_visible(pMultiView->pPlayerHandle, TRUE);
			vp_mm_player_pause(pMultiView->pPlayerHandle);
		} else {
			vp_mm_player_play(pMultiView->pPlayerHandle);
		}
		if (!vp_mm_player_set_subtitle_position
		        (pMultiView->pPlayerHandle,
		         pMultiView->fSubtitleSyncValue * 1000)) {
			VideoLogError("vp_mm_player_set_subtitle_position is fail");
		}
	}

	vp_mm_player_set_rate(pMultiView->pPlayerHandle,
	                      (float) pMultiView->fPlaySpeed);

	_vp_play_multi_view_set_play_state(pMultiView);
}

/* external functions */
multi_view_handle vp_play_multi_view_create(PlayView *pPlayView,
                        video_play_launching_type_t nLaunchingType)
{
	if (pPlayView == NULL) {
		VideoLogError("pPlayView is NULL");
		return NULL;
	}

	MultiView *pMultiView = NULL;

	pMultiView = calloc(1, sizeof(MultiView));

	if (pMultiView == NULL) {
		VideoLogError("pMultiView alloc fail");
		return NULL;
	}

	pMultiView->pPlayView = pPlayView;
	pMultiView->pWin = pPlayView->pWin;
	pMultiView->pNaviFrame = pPlayView->pNaviframe;
	pMultiView->nLaunchingType = nLaunchingType;
	pMultiView->fPlaySpeed = pPlayView->fPlaySpeed;
	pMultiView->fSubtitleSyncValue = pPlayView->fSubtitleSyncValue;

	if (!_vp_play_multi_view_init_layout(pMultiView)) {
		VideoLogError("_vp_play_multi_view_init_layout is fail");
		_vp_play_multi_view_destroy_handle(pMultiView);
		return NULL;

	}
	pMultiView->nXwinID = elm_win_xwindow_get(pMultiView->pWin);

	pMultiView->bMouseDown = FALSE;
	pMultiView->nMousePosX = 0;
	pMultiView->nMousePosY = 0;
	pMultiView->nMouseButtons = 0;
	pMultiView->bIsResizeDone = TRUE;
	pMultiView->bNoContentMode = FALSE;

	pMultiView->pMouseDownHandle =
	    ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN,
	                            __vp_play_multi_view_mouse_down_cb,
	                            (void *) pMultiView);
	pMultiView->pMouseUpHandle =
	    ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
	                            __vp_play_multi_view_mouse_up_cb,
	                            (void *) pMultiView);
	pMultiView->pMouseMoveHandle =
	    ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE,
	                            __vp_play_multi_view_mouse_move_cb,
	                            (void *) pMultiView);
	pMultiView->pConfigureNotify =
	    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE,
	                            __vp_play_multi_view_configure_cb,
	                            (void *) pMultiView);

	// create prepare_cb pipe
	VP_EVAS_PIPE_DEL(pMultiView->pPreparePipe);
	pMultiView->pPreparePipe =
	    ecore_pipe_add((Ecore_Pipe_Cb)
	                   _vp_play_multi_view_prepare_pipe_cb,
	                   (void *) pMultiView);

	if (pPlayView) {
		pMultiView->pMediaKey =
		    vp_media_key_create(pPlayView->pWin,
		                        __vp_play_multi_view_media_key_event_cb);
		vp_media_key_set_user_data(pMultiView->pMediaKey,
		                           (void *) pMultiView);
	}

	elm_win_indicator_mode_set(pMultiView->pWin, ELM_WIN_INDICATOR_HIDE);

	elm_win_floating_mode_set(pMultiView->pWin, EINA_TRUE);

	evas_object_resize(pMultiView->pWin, VP_MULTI_DEFAULT_WIDTH,
	                   VP_MULTI_DEFAULT_HEIGHT);

	Evas *e = evas_object_evas_get(pMultiView->pWin);
	evas_event_callback_add(e, EVAS_CALLBACK_CANVAS_FOCUS_IN,
	                        _vp_play_multi_view_evas_focus_in_cb,
	                        pMultiView->pLayout);
	evas_event_callback_add(e, EVAS_CALLBACK_CANVAS_FOCUS_OUT,
	                        _vp_play_multi_view_evas_focus_out_cb,
	                        pMultiView->pLayout);

	_vp_play_multi_view_hide_layout(pMultiView);

	bool bMultiPlay = FALSE;
	vp_play_config_get_multi_play_status(&bMultiPlay);
	if (!bMultiPlay) {
		vp_play_config_set_multi_play_status(TRUE);
	}

	return (multi_view_handle) pMultiView;
}

void vp_play_multi_view_destroy(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;
	elm_win_indicator_mode_set(pMultiView->pWin, ELM_WIN_INDICATOR_SHOW);

	elm_naviframe_item_pop(pMultiView->pNaviFrame);

	vp_media_contents_unset_update_cb();

	vp_play_multi_view_unrealize(pMultiView);

	_vp_play_multi_view_set_preview(pMultiView);

	_vp_play_multi_view_destroy_handle(pMultiView);

}

bool vp_play_multi_view_realize(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	vp_media_key_realize(pMultiView->pMediaKey);
	pMultiView->bActivate = TRUE;

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		pMultiView->nRepeatMode = VIDEO_PLAY_REPEAT_NONE;
	} else {
		int nVal = 0;
		vp_play_preference_get_repeat_mode_key(&nVal);
		pMultiView->nRepeatMode = (video_play_repeat_mode_t) nVal;
	}

	if (pMultiView->pMediaItemList) {
		vp_media_contents_free_video_items(pMultiView->pMediaItemList);
		pMultiView->pMediaItemList = NULL;
	}

	pMultiView->bIsRealize = FALSE;
	pMultiView->bSeekComplete = TRUE;

	video_play_sort_type_t nType = VIDEO_SORT_BY_NONE;
	int nSortVal = 0;
	vp_play_config_get_sort_type_key(&nSortVal);

	nType = (video_play_sort_type_t) nSortVal;

	if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_GALLERY) {
		char *szFolder =
		    vp_play_util_get_folder_from_path(pMultiView->szMediaURL);
		vp_media_contents_get_video_items_to_folder
		(VIDEO_SORT_BY_DATE_MOST_RECENT, szFolder,
		 &(pMultiView->pMediaItemList));
		VP_FREE(szFolder);
	} else if (pMultiView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
	} else {
		PlayView *pPlayView = pMultiView->pPlayView;

		if (pPlayView == NULL) {
			VideoLogError("pPlayView is NULL");
			return FALSE;
		}
		if (pPlayView->nListType == VIDEO_PLAY_LIST_TYPE_FOLDER) {
			char *szFolder =
			    vp_play_util_get_folder_from_path(pMultiView->szMediaURL);
			vp_media_contents_get_video_items_to_folder(nType, szFolder,
			        &(pMultiView->
			          pMediaItemList));
			VP_FREE(szFolder);
		} else {
			vp_media_contents_get_video_items(nType,
			                                  &(pMultiView->
			                                    pMediaItemList));
		}
	}
	if (!_vp_play_multi_view_play_start(pMultiView, TRUE)) {
		return FALSE;
	}

	_vp_play_multi_view_check_during_call(pMultiView);

	evas_object_show(pMultiView->pWin);
	elm_win_activate(pMultiView->pWin);

	return TRUE;

}

bool vp_play_multi_view_unrealize(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;
	_vp_play_multi_view_set_played_time(pMultiView);

	if (pMultiView->pPlayerHandle) {
		vp_mm_player_destroy(pMultiView->pPlayerHandle);
		pMultiView->pPlayerHandle = NULL;
	}

	VP_EVAS_DEL(pMultiView->pVideoSink);
	VP_EVAS_TIMER_DEL(pMultiView->pHideTimer);
	VP_EVAS_TIMER_DEL(pMultiView->pSubtitleTimer);
	VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

	VP_FREE(pMultiView->szSubtitleURL);

	vp_media_key_unrealize(pMultiView->pMediaKey);

	if (pMultiView->pMediaItemList) {
		vp_media_contents_free_video_items(pMultiView->pMediaItemList);
		pMultiView->pMediaItemList = NULL;
	}

	pMultiView->bIsRealize = FALSE;

	vp_play_util_set_unlock_power_key();

	return TRUE;
}

bool vp_play_multi_view_is_realize(multi_view_handle pViewHandle,
                                   bool *bIsRealize)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	*bIsRealize = pMultiView->bIsRealize;

	return TRUE;

}

bool vp_play_multi_view_pause(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	vp_mm_player_pause(pMultiView->pPlayerHandle);
	_vp_play_multi_view_set_play_state(pMultiView);

	return TRUE;
}

bool vp_play_multi_view_resume(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;
	if (pMultiView->bManualPause == FALSE) {
		vp_mm_player_play(pMultiView->pPlayerHandle);
		_vp_play_multi_view_set_play_state(pMultiView);
	}
	_vp_play_multi_view_check_during_call(pMultiView);

	return TRUE;
}

bool vp_play_multi_view_resume_or_pause(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pMultiView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	switch (nState) {
	case VP_MM_PLAYER_STATE_NONE:
	case VP_MM_PLAYER_STATE_IDLE:
	case VP_MM_PLAYER_STATE_READY:
	case VP_MM_PLAYER_STATE_PAUSED:
		vp_mm_player_play(pMultiView->pPlayerHandle);
		break;
	case VP_MM_PLAYER_STATE_PLAYING:
		vp_mm_player_pause(pMultiView->pPlayerHandle);
		break;
	case VP_MM_PLAYER_STATE_STOP:
		break;
	}

	_vp_play_multi_view_set_play_state(pMultiView);
	_vp_play_multi_view_check_during_call(pMultiView);

	return TRUE;
}

bool vp_play_multi_view_next_play(multi_view_handle pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	_vp_play_multi_view_on_next_play(pMultiView, TRUE);

	return TRUE;
}

bool vp_play_multi_view_prev_play(multi_view_handle pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	_vp_play_multi_view_on_prev_play(pMultiView, TRUE, FALSE);

	return TRUE;
}

bool vp_play_multi_view_set_url(multi_view_handle pViewHandle,
                                const char *szMediaURL)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	if (szMediaURL == NULL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	VP_FREE(pMultiView->szMediaURL);
	VP_STRDUP(pMultiView->szMediaURL, szMediaURL);

	return TRUE;

}

bool vp_play_multi_view_set_start_position(multi_view_handle pViewHandle,
        int nStartPosition)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	if (nStartPosition < 0) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	pMultiView->nStartPosition = nStartPosition;

	return TRUE;
}

bool vp_play_multi_view_get_last_position(multi_view_handle pViewHandle,
        int *nCurPosition)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	int nPosition = 0;

	if (pMultiView->bHLSMode) {
		*nCurPosition = 0;
		return TRUE;
	}

	if (!vp_mm_player_get_position(pMultiView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return FALSE;
	}

	pMultiView->nCurPosition = nPosition;

	*nCurPosition = nPosition;

	return TRUE;
}

bool vp_play_multi_view_set_subtitle(multi_view_handle pViewHandle,
                                     char *szSubtitle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	VP_FREE(pMultiView->szSubtitleURL);

	VP_STRDUP(pMultiView->szSubtitleURL, szSubtitle);

	return TRUE;

}

bool vp_play_multi_view_get_subtitle(multi_view_handle pViewHandle,
                                     char **szSubtitle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	if (!vp_mm_player_get_subtitle_url
	        (pMultiView->pPlayerHandle, szSubtitle)) {
		VideoLogError("vp_mm_player_get_subtitle_url is fail");
		return FALSE;
	}
	return TRUE;

}

bool vp_play_multi_view_get_audio_track(multi_view_handle pViewHandle,
                                        int *nAudioTrack)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	*nAudioTrack = pMultiView->nAudioTrackIndex;

	return TRUE;
}

bool vp_play_multi_view_set_audio_track(multi_view_handle pViewHandle,
                                        int nAudioTrack)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	pMultiView->nAudioTrackIndex = nAudioTrack;

	return TRUE;
}

bool vp_play_multi_view_set_manual_pause(multi_view_handle pViewHandle,
        bool bManualPause)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	pMultiView->bManualPause = bManualPause;

	return TRUE;

}

bool vp_play_multi_view_get_manual_pause(multi_view_handle pViewHandle,
        bool *bManualPause)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	*bManualPause = pMultiView->bManualPause;

	return TRUE;
}

bool vp_play_multi_view_set_rotate(multi_view_handle pViewHandle,
                                   video_play_rotate_t nRotate)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	if (pMultiView->pWin == NULL) {
		VideoLogError("Multi Win is NULL.");
		return FALSE;
	}

	if (pMultiView->nRotate == nRotate) {
		VideoLogWarning("Skip for same rotate");
		return TRUE;
	}

	if (nRotate == VIDEO_PLAY_ROTATE_NONE) {
//              elm_win_rotation_set(pMultiView->pWin, 0);
	} else if (nRotate == VIDEO_PLAY_ROTATE_90) {
//              elm_win_rotation_set(pMultiView->pWin, 90);
	} else if (nRotate == VIDEO_PLAY_ROTATE_180) {
//              elm_win_rotation_set(pMultiView->pWin, 180);
	} else if (nRotate == VIDEO_PLAY_ROTATE_270) {
//              elm_win_rotation_set(pMultiView->pWin, 270);
	}

	pMultiView->nRotate = nRotate;

	return TRUE;
}

bool vp_play_multi_view_get_sound_filter(multi_view_handle pViewHandle,
        video_sound_alive_t *
        nSoundAlive)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	*nSoundAlive = pMultiView->nSoundAlive;

	return TRUE;
}

bool vp_play_multi_view_set_sound_filter(multi_view_handle pViewHandle,
        video_sound_alive_t nSoundAlive)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	pMultiView->nSoundAlive = nSoundAlive;

	return TRUE;
}

bool vp_play_multi_view_set_launching_mode(multi_view_handle pViewHandle,
        video_play_launching_type_t
        nLaunchingType)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	pMultiView->nLaunchingType = nLaunchingType;
	pMultiView->bManualPause = FALSE;

	return TRUE;
}

bool vp_play_multi_view_update(multi_view_handle pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	return TRUE;
}

bool vp_play_multi_view_volume_increase(multi_view_handle pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	int nMaxValue = 0;
	int nCurVal = 0;
	if (!vp_sound_get_max_volume(&nMaxValue)) {
		VideoLogError("vp_sound_get_max_volume is fail");
		return FALSE;
	}

	if (!vp_sound_get_volume(&nCurVal)) {
		VideoLogError("vp_sound_get_volume is fail");
		return FALSE;
	}
	nCurVal++;

	if (nCurVal < nMaxValue) {
		if (!vp_sound_set_volume(nCurVal)) {
			VideoLogError("vp_sound_set_volume is fail");
			return FALSE;
		}
	}

	return TRUE;
}

bool vp_play_multi_view_volume_decrease(multi_view_handle pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	int nCurVal = 0;

	if (!vp_sound_get_volume(&nCurVal)) {
		VideoLogError("vp_sound_get_volume is fail");
		return FALSE;
	}

	nCurVal--;

	if (nCurVal >= 0) {
		if (!vp_sound_set_volume(nCurVal)) {
			VideoLogError("vp_sound_set_volume is fail");
			return FALSE;
		}
	}

	return TRUE;
}

void vp_play_multi_view_web_type_disconnect(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	int nPosition = 0;
	if (!vp_mm_player_get_position(pMultiView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
	}

	vp_mm_player_destroy(pMultiView->pPlayerHandle);
	pMultiView->pPlayerHandle = NULL;

	if (nPosition > 0) {
		pMultiView->nStartPosition = nPosition;
	}
}

void vp_play_multi_view_web_type_reconnect(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	if (!pMultiView->pPlayView) {
		VideoLogError("pMultiView->pPlayView is NULL");
		return;
	}

	if (pMultiView->nLaunchingType != VIDEO_PLAY_TYPE_WEB) {
		VideoLogInfo("Not WEB type");
		return;
	}

	if (pMultiView->pPlayerHandle) {
		vp_mm_player_destroy(pMultiView->pPlayerHandle);
		pMultiView->pPlayerHandle = NULL;
	}

	PlayView *pPlayView = pMultiView->pPlayView;
	if (pPlayView->nStartPosition > 0) {
		pMultiView->nStartPosition = pPlayView->nStartPosition;
	}

	VideoLogInfo("pMultiView->nStartPosition : [%d]",
	             pMultiView->nStartPosition);
	_vp_play_multi_view_play_start(pMultiView, FALSE);
}

void vp_play_multi_view_set_cancelkey_press(multi_view_handle pViewHandle,
        bool bCancelKeyPress)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	pMultiView->bCancelKeyPress = bCancelKeyPress;
}

void vp_play_multi_view_get_cancelkey_press(multi_view_handle pViewHandle,
        bool *bCancelKeyPress)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	*bCancelKeyPress = pMultiView->bCancelKeyPress;
}

void vp_play_multi_view_destroy_exit_popup(multi_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	if (pMultiView->pExitPopupWin) {
		VP_EVAS_DEL(pMultiView->pPopup);
		VP_EVAS_DEL(pMultiView->pExitPopupWin);
	}
}

bool vp_play_multi_view_get_main_layout_show_state(multi_view_handle *
        pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	return pMultiView->bShowLayout;
}


//Focus UI
static void _vp_play_multi_view_set_button_focus_sequence(MultiView *
        pMultiView)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return;
	}

	elm_object_focus_set(pMultiView->pMinSizeBtn, EINA_TRUE);

	vp_play_util_focus_next_object_set(pMultiView->pMinSizeBtn,
	                                   pMultiView->pFullSizeBtn,
	                                   ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pMultiView->pMinSizeBtn,
	                                   pMultiView->pExitBtn,
	                                   ELM_FOCUS_LEFT);
	vp_play_util_focus_next_object_set(pMultiView->pMinSizeBtn,
	                                   pMultiView->pPlayFocusBtn,
	                                   ELM_FOCUS_DOWN);

	vp_play_util_focus_next_object_set(pMultiView->pFullSizeBtn,
	                                   pMultiView->pExitBtn,
	                                   ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pMultiView->pFullSizeBtn,
	                                   pMultiView->pMinSizeBtn,
	                                   ELM_FOCUS_LEFT);
	vp_play_util_focus_next_object_set(pMultiView->pFullSizeBtn,
	                                   pMultiView->pPlayFocusBtn,
	                                   ELM_FOCUS_DOWN);

	vp_play_util_focus_next_object_set(pMultiView->pExitBtn,
	                                   pMultiView->pMinSizeBtn,
	                                   ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pMultiView->pExitBtn,
	                                   pMultiView->pFullSizeBtn,
	                                   ELM_FOCUS_LEFT);
	vp_play_util_focus_next_object_set(pMultiView->pExitBtn,
	                                   pMultiView->pPlayFocusBtn,
	                                   ELM_FOCUS_DOWN);


	vp_play_util_focus_next_object_set(pMultiView->pPlayFocusBtn,
	                                   pMultiView->pNextBtn,
	                                   ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pMultiView->pPlayFocusBtn,
	                                   pMultiView->pPrevBtn,
	                                   ELM_FOCUS_LEFT);
	vp_play_util_focus_next_object_set(pMultiView->pPlayFocusBtn,
	                                   pMultiView->pMinSizeBtn,
	                                   ELM_FOCUS_UP);

	vp_play_util_focus_next_object_set(pMultiView->pNextBtn,
	                                   pMultiView->pPrevBtn,
	                                   ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pMultiView->pNextBtn,
	                                   pMultiView->pPlayFocusBtn,
	                                   ELM_FOCUS_LEFT);
	vp_play_util_focus_next_object_set(pMultiView->pNextBtn,
	                                   pMultiView->pMinSizeBtn,
	                                   ELM_FOCUS_UP);

	vp_play_util_focus_next_object_set(pMultiView->pPrevBtn,
	                                   pMultiView->pPlayFocusBtn,
	                                   ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pMultiView->pPrevBtn,
	                                   pMultiView->pNextBtn,
	                                   ELM_FOCUS_LEFT);
	vp_play_util_focus_next_object_set(pMultiView->pPrevBtn,
	                                   pMultiView->pMinSizeBtn,
	                                   ELM_FOCUS_UP);

}

//Focus UI
void vp_play_multi_view_set_main_layout_focus_out(multi_view_handle *
        pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	VideoLogInfo("");

	MultiView *pMultiView = (MultiView *) pViewHandle;
	pMultiView->bMainFocusState = FALSE;

	elm_object_focus_set(pMultiView->pNaviFrame, EINA_FALSE);
	elm_object_tree_focus_allow_set(pMultiView->pNaviFrame, EINA_FALSE);
	elm_object_focus_allow_set(pMultiView->pNaviFrame, EINA_FALSE);

	elm_object_focus_set(pMultiView->pLayout, EINA_FALSE);
	elm_object_tree_focus_allow_set(pMultiView->pLayout, EINA_FALSE);
	elm_object_focus_allow_set(pMultiView->pLayout, EINA_FALSE);
}

void vp_play_multi_view_set_main_layout_focus_in(multi_view_handle *
        pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	VideoLogInfo("");

	MultiView *pMultiView = (MultiView *) pViewHandle;

	if (pMultiView->bMainFocusState == TRUE) {
		VideoLogInfo("Already Focus In");
		return;
	}
	pMultiView->bMainFocusState = TRUE;

	elm_object_focus_allow_set(pMultiView->pNaviFrame, EINA_TRUE);
	elm_object_tree_focus_allow_set(pMultiView->pNaviFrame, EINA_TRUE);
	elm_object_focus_set(pMultiView->pNaviFrame, EINA_TRUE);

	elm_object_focus_allow_set(pMultiView->pLayout, EINA_TRUE);
	elm_object_tree_focus_allow_set(pMultiView->pLayout, EINA_TRUE);
	elm_object_focus_set(pMultiView->pLayout, EINA_TRUE);

	elm_object_focus_set(pMultiView->pMinSizeBtn, EINA_TRUE);
}

bool vp_play_multi_view_get_main_layout_focus_state(multi_view_handle *
        pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	return pMultiView->bMainFocusState;
}

void vp_play_multi_view_show_main_layout(multi_view_handle *pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	_vp_play_multi_view_show_layout(pMultiView);
}

static void _vp_play_multi_view_speed_for_steps(MultiView *pMultiView,
        bool bSpeedFF)
{
	if (pMultiView == NULL) {
		VideoLogError("pMultiView is NULL");
		return;
	}

	VideoLogInfo("");

	if (pMultiView->bSeekComplete == FALSE) {
		VideoLogError("bSeekComplete is fail");
		return;
	}

	int nSetPosition = 0;
	int nCurPosition = 0;

	if (!vp_mm_player_get_position
	        (pMultiView->pPlayerHandle, &nCurPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return;
	}

	if (pMultiView->nDuration <= nCurPosition) {
		_vp_play_multi_view_on_next_play(pMultiView, FALSE);
		return;
	}

	VideoLogInfo("pMultiView->nDuration %d", pMultiView->nDuration);
	if (bSpeedFF) {
		nSetPosition = nCurPosition + (15 * 1000);	//15 sec
	} else {
		nSetPosition = nCurPosition - (15 * 1000);
	}

	if (pMultiView->nDuration < nSetPosition) {
		nSetPosition = pMultiView->nDuration;
	}

	if (nSetPosition < 0) {
		nSetPosition = 0;
	}
#if 1
	if (vp_mm_player_set_position
	        (pMultiView->pPlayerHandle, nSetPosition)) {
#else
	if (vp_mm_player_set_position_by_key_frame
	        (pMultiView->pPlayerHandle, nSetPosition)) {
#endif
		pMultiView->bSeekComplete = FALSE;
		pMultiView->nCurPosition = nSetPosition;
	}

	return;
}

void
vp_play_multi_view_ff_rew_keyboard_longpress_event_cb(multi_view_handle
        pViewHandle,
        bool bRelease,
        bool bFFseek)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	if (pMultiView->pSpeedTimer == NULL && bRelease == FALSE) {
		pMultiView->bSpeedDown = FALSE;

		if (bFFseek)
			__vp_play_multi_view_media_key_event_cb
			(VP_MEDIA_KEY_FASTFORWARD, bRelease, pMultiView);
		else
			__vp_play_multi_view_media_key_event_cb(VP_MEDIA_KEY_REWIND,
			                                        bRelease, pMultiView);
	} else if (pMultiView->pSpeedTimer && bRelease == TRUE) {

		if (pMultiView->nSpeedValue < 2) {
			VideoLogInfo("speed up");
			_vp_play_multi_view_speed_for_steps(pMultiView, bFFseek);
		} else {
			int nPosition = 0;
			if (vp_mm_player_get_position
			        (pMultiView->pPlayerHandle, &nPosition)) {
				if (vp_mm_player_set_position
				        (pMultiView->pPlayerHandle,
				         pMultiView->nCurPosition)) {
					pMultiView->bSeekComplete = FALSE;
				}
			}
		}

		VP_EVAS_TIMER_DEL(pMultiView->pSpeedTimer);

		elm_object_signal_emit(pMultiView->pLayout,
		                       VP_MULTI_SIGNAL_MAIN_SPEED_HIDE, "*");

		if (pMultiView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pMultiView->pPlayerHandle)) {
				VideoLogError("play Fail");
			}
			pMultiView->bManualPause = FALSE;
		}

		_vp_play_multi_view_set_play_state(pMultiView);
	}
	VP_FREE(pMultiView);
}

bool vp_play_multi_view_change_to_normal_view(multi_view_handle
        pViewHandle)
{
	if (pViewHandle == NULL) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	MultiView *pMultiView = (MultiView *) pViewHandle;

	__vp_play_multi_view_btn_clicked_cb(pViewHandle,
	                                    pMultiView->pFullSizeBtn, NULL);

	return TRUE;
}
