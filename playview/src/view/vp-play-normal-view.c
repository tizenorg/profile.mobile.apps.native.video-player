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


#include <math.h>
#include <sound_manager.h>
#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-normal-view.h"

#include "vp-play-button.h"
#include "vp-play-bookmark.h"
#include "vp-play-progressbar.h"
#include "vp-play-subtitle.h"
#include "vp-play-loading-ani.h"

#include "vp-mm-player.h"
#include "vp-play-popup.h"
#include "vp-play-preference.h"
#include "vp-play-config.h"
#include "vp-play-util.h"
#include "vp-play-button.h"
#include "vp-play-volume.h"
#include "vp-play-volume-popup.h"
#include "vp-play-brightness-popup.h"

#include "vp-image-util.h"
#include "vp-media-contents.h"

#include "vp-share.h"
#include "vp-play-ug.h"
#include "vp-sound-alive.h"
#include "vp-sound-path.h"
#include "vp-audio-track.h"

#include "vp-sound.h"
#include "vp-subtitle.h"
#include "vp-subtitle-sync.h"
#include "vp-subtitle-select.h"
#include "vp-subtitle-font.h"
#include "vp-subtitle-size.h"
#include "vp-subtitle-alignment.h"
#include "vp-subtitle-color.h"
#include "vp-subtitle-bg-color.h"
#include "vp-subtitle-track.h"
#include "vp-subtitle-edge.h"

#include "vp-repeat.h"
#include "vp-capture.h"
#include "vp-play-speed.h"
#include "vp-setting.h"
#ifdef ENABLE_DRM_FEATURE
#include "vp-drm.h"
#endif
#include "vp-device.h"
#include "vp-media-key.h"
#include "vp-detail.h"
#include "vp-zoom-guide.h"

#include "vp-multi-path.h"

//#include "VppDownload.h"

#include "vp-device-language.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-util.h"
#include "vp-file-util.h"
#include "vp-avrcp.h"


#define VP_NORMAL_PROGRESS_TIMER_INTERVAL		0.3
#define VP_NORMAL_PROGRESS_BAR_START_TERM		26 * elm_config_scale_get()
#define VP_NORMAL_PROGRESS_BAR_WIDTH_TERM		52 * elm_config_scale_get()

#define VP_NORMAL_HIDE_LAYOUT_TIMER_INTERVAL		5.0
#define VP_NORMAL_HIDE_LAYOUT_FINISH_TIMER_INTERVAL	2.0 //3.0
#define VP_NORMAL_WINDOW_LOAD_FINISH_TIMER_INTERVAL	1.5 //wait for window loaded

#define VP_NORMAL_LONG_PRESS_TIMER_INTERVAL		1.0
#define VP_NORMAL_SUBTITLE_LONG_PRESS_TIMER_INTERVAL		0.5

#define VP_NORMAL_HIDE_SCREEN_LOCK_TIMER_INTERVAL	3.0
#define VP_NORMAL_MOMENTUM_PRESS_TIMER_INTERVAL		0.2
#define VP_NORMAL_SCREENSHOT_TIMER_INTERVAL		2.0
#define VP_NORMAL_EXIT_WAIT_TIMER_INTERVAL		3.0
#define VP_NORMAL_INDICATOR_HIDE_TIMER_INTERVAL		2.0

#define VP_NORMAL_ALL_BUTTON_L					885
#define VP_NORMAL_ALL_BUTTON_VSFRP_L			680
#define VP_NORMAL_ALL_BUTTON_FRP_L				430
#define VP_NORMAL_ALL_BUTTON_P_L				140


#define VP_SUBTITLE_PORTRAIT_POSX 				360
#define VP_SUBTITLE_PORTRAIT_POSY				1335
#define VP_SUBTITLE_LANDSCAPE_POSX				640
#define VP_SUBTITLE_LANDSCAPE_POSY				765

#define VP_SUBTITLE_LANDSCAPE_SIZE				1280
#define VP_SUBTITLE_PORTRAIT_SIZE				720

#define VP_SUBTITLE_MARGIN_SIZE		30

#define VP_NORMAL_SPEED_VALUE_MAX			64

#define VP_NORMAL_VALUE_SEC				60

#define	VP_VOICE_MUTE_VALUE		0

#define VP_NORMAL_HIDE_VOLUME_TIMER_INTERVAL		0.05

#define VP_NORMAL_SET_GREP_BOX_SIZE_ID				1
#define VP_NORMAL_SET_CAPTION_WINDOW_SIZE_ID		2
#define VP_NORMAL_SET_CAPTION_WINDOW_COLOR_ID		3
#define VP_NORMAL_SET_CAPTION_WINDOW_OFFSET_ID		4

#define VP_SUBTITLE_SET_GREP_BOX_SIZE_ID			1
#define VP_SUBTITLE_SET_CAPTION_WINDOW_SIZE_ID		2

#ifdef K_FEATURE_MICROSEEK
#define VP_NORMAL_SZ_HALF_SPEED				"1/2"
#define VP_NORMAL_SZ_QUARTER_SPEED			"1/4"
#define VP_NORMAL_MICROSEEK_TEXT_HEIGHT		(46 * elm_config_scale_get())
#define VP_NORMAL_MICROSEEK_IMAGE_WIDTH		(46 * elm_config_scale_get())
#define VP_NORMAL_MICROSEEK_IMAGE_HEIGHT	(70 * elm_config_scale_get())
#endif

#ifdef _NATIVE_BUFFER_SYNC
#define VP_NORMAL_DEFAULT_WIDTH		(480 * elm_config_scale_get())
#define VP_NORMAL_DEFAULT_HEIGHT		(800 * elm_config_scale_get())
#endif

typedef struct _ProgressMouseInfo {
	int			nDownPosX;
} ProgressMouseInfo;

typedef enum {
	BUTTON_TYPE_NORMAL = 0,
	BUTTON_TYPE_PRESSED,
	BUTTON_TYPE_DIM,
} ButtonType;

#ifdef K_FEATURE_MICROSEEK
typedef enum {
	SEEK_LEVEL_NORMAL = 1,
	SEEK_LEVEL_HALF = 2,
	SEEK_LEVEL_QUARTER = 4,
} SeekLevel;
#endif

typedef struct _NormalView {
	PlayView			*pPlayView;
	Ecore_X_Window			nXwinID;

	Evas_Object			*pNaviFrame;
	Elm_Object_Item			*pNaviItem;
	Evas_Object			*pMainEventRect;
	Evas_Object			*pGestureRect;
	Evas_Object			*pMainLayout;
#ifdef _NATIVE_BUFFER_SYNC
	Evas_Object			*pVideoSink;
#endif
	Evas_Object			*pTopControlLayout;
	Evas_Object			*pControlLayout;
	Evas_Object			*pFunctionLayout;

	Evas_Object			*pNaviLeftBtn;
	Evas_Object			*pNaviRightBtn;

	Evas_Object			*pSoundPathBtn;
	Evas_Object			*pAllsharBtn;

	Evas_Object			*pMoreBtn;
	Evas_Object			*pBackBtn;

	Evas_Object			*pCtxPopup;

	Evas_Object			*pImageBufferObj;

	Evas_Object			*pRotateBtn;
	Evas_Object			*pVolumeBtn;
	Evas_Object			*pScreenSizeBtn;
	Evas_Object			*pBookmarkBtn;
	Evas_Object			*pMultiWinBtn;
	Evas_Object			*pRepeatBtn;
	Evas_Object			*pTrimBtn;
	Evas_Object			*pSoundAliveBtn;
	Evas_Object			*pShareBtn;
	Evas_Object			*pLockBtn;

	Evas_Object			*pResumeBtn;
	Evas_Object			*pPauseBtn;
	Evas_Object			*pPlayFocusBtn;
	Evas_Object			*pPrevBtn;
	Evas_Object			*pNextBtn;

	Evas_Object			*pCaptureBtn;
	Evas_Object			*pCaptureRewBtn;
	Evas_Object			*pCaptureFFBtn;
	Evas_Object			*pPopupBtn;

	Evas_Object   			*pSpeedBoxCtrl;

	Evas_Object			*pPopup;

	Evas_Object			*pLoadingAni;

	Ecore_Timer			*pHideTimer;
	Ecore_Timer			*pHideFinishTimer;
	Ecore_Timer			*pLockScreenTimer;

	Ecore_Timer			*pSpeedTimer;
	bool				bSpeedFF;
	int				nSpeedValue;
	bool				bSpeedDown;
	bool				bKeyPressed;

	Ecore_Timer			*pExitWaitTimer;

	Ecore_Idler			*pControlIdler;
	Ecore_Idler			*pMultiWinIdler;

	GList				*pMediaItemList;

	bool				bIsRealize;
	bool				bIsActive;
	bool				bShowLayout;
	bool				bMainFocusState;
	bool				bIsPopupShow;
	bool				bIsSubtitleShow;
	bool				bIsVideoUnsupport;

	/* bookmark part */
	bookmark_handle			pBookmarkHandle;
	Evas_Object			*pBookmarkObj;
	bool				bCaptureComplete;
	int				nCapturePosition;
	/* progressbar part */
	Evas_Object			*pProgressEvent;
	Ecore_Timer			*pProgressTimer;
	progressbar_handle		pProgressbar;
	bool				bProgressDown;
#ifdef K_FEATURE_MICROSEEK
	double 			nPrevCanvasX;
	double 			nPrevPositionX;
#endif
	int				nCurPosition;
	bool				bHLSMode;
	int				nWaitPos;
	bool				bSeekComplete;
	bool				bSeekPause;
	bool				bSeekResume;
#ifdef K_FEATURE_MICROSEEK
	Evas_Object 		*pMicroSeekTextLayout;
	Evas_Object 		*pMicroSeekImageLayout;
#endif
#ifdef FLICK_JUMP
	int 			nFlickPressX1;
	int 			nFlickPressY1;
	bool			bFlickRewind;
	bool			bFlickWaiting;
	Evas_Object 		*pFlickJumpSeek;
#endif
	bool				bBufferingComplete;

	char				*szMediaURL;
	char				*szSubtitleURL;
	int				nStartPosition;
	bool				bManualPause;

	mm_player_handle		pPlayerHandle;
	vp_mm_player_display_mode_t	nDisplayMode;

	int				nDuration;
	video_play_launching_type_t	nLaunchingType;
	video_play_rotate_t		nRotate;

	share_handle			pShareHandle;

	volume_handle			pVolumeHandle;
	volume_popup_handle		pVolumePopupHandle;
	brightness_handle		pBrightnessHandle;

	sound_alive_handle		pSAHandle;
	int				nSoundAlive;

	subtitle_handle			pSubtitle;
	Ecore_Timer			*pSubtitleTimer;
	bool				bIsExistSubtitle;
	subtitle_popup_handle		pSubtitlePopup;
	subtitle_select_handle 		pSubtitleSelectPopup;
#ifdef SUBTITLE_K_FEATURE
	subtitle_alignment_handle 	pSubtitleAlignmentPopup;
#endif
	subtitle_font_handle 		pSubtitleFontPopup;
	subtitle_size_handle 		pSubtitleSizePopup;
	subtitle_edge_handle		pSubtitleEdgePopup;
	subtitle_color_handle 	pSubtitleFontColorPopup;
#ifndef SUBTITLE_K_FEATURE
	subtitle_bg_color_handle 	pSubtitleBGColorPopup;
#else
	subtitle_color_handle	pSubtitleBGColorPopup;
#endif
	subtitle_color_handle	pSubtitleCaptionWinColorPopup;
	subtitle_sync_popup		pSubtitleSyncPopup;
	subtitle_track_handle		pSubtitleTrackHandle;
	int				nDefaultSubtitleTrackIndex;
	float				fSubtitleSyncValue;

	repeat_handle			pRepeatHandle;
	video_play_repeat_mode_t	nRepeatMode;

	sound_path_handle		pSoundPathHandle;
	setting_popup_handle		pSettingHandle;
	detail_handle			pDetailHandle;

	audio_track_handle		pAudioTrackHandle;
	int				nDefaultAudioTrackIndex;

	media_key_handle		pMediaKey;
	bool				bLockScreen;
	char				*szScreenMirrorMacAddr;

	//VppDownloadHandle		pDownloadHandle;
	bool				bVideoOnlyMode;
	bool				bDeviceRemoveInterrupt;

	int				dSubtitleStartTime;
	int				dSubtitleEndTime;

	Ecore_Timer			*pMomentTimer;
	int				nGestureStartPosX;
	int				nGestureStartPosY;
	int				nGestureOldPosY;
	int				nMomentDefaultVal;
	int				nMomentWeight;
	int				nMomentStep;
	int				nWindowWidth;
	int				nWindowHeight;

	Ecore_Job			*pVolumeJob;
	Ecore_Job			*pBrightnessJob;

	bool				bMomentumIncrease;

	play_speed_popup		pPlaySpeedPopup;
	double				fPlaySpeed;

	capture_popup_handle		pCapturePopup;
	Ecore_Timer			*pScreenShotTimer;
	Evas_Object			*pScreenShot;

	bool 				bCaptureMode;
	bool				bCaptureBookmark;
	bool				bEarjackConnect;

	zoom_guide_handle		pZoomGuide;
	int				nZoomPosX;
	int				nZoomPosY;
	int				nPrevZoomPosX;
	int				nPrevZoomPosY;

	double				fActiveZoom;
	double				fZoomValue;
	double				fPrevZoomValue;
	bool				bZoomEnable;

	int					nLastVolume;

	Ecore_Pipe			*pSeekPipe;

	vp_normal_view_state_cb vp_mini_played_state_cb;
	bool	bKeySpeed;
	Ecore_Pipe			*pPreparePipe;
	bool	bDestroyNavi;
	bool 	bSharepopup;
	bool	bSharePanelPause;
	bool 	bSharePanelFocusIn; //first focus in is menu popup hide, second is share panel hdie

#ifdef SUBTITLE_K_FEATURE
	bool 	bSubtitleDown;

	int 			nInitSubtitleX;
	int 			nInitSubtitleY;

	int 			nDeltaSubtitleX;
	int 			nDeltaSubtitleY;

	int 			nLongPressX1;
	int 			nLongPressY1;
	int 			nLongPressX2;
	int 			nLongPressY2;

	Ecore_Timer 	*pSubtitleMoveTimer;
	int		nSubtitleInterval;
#endif
#ifdef _SUBTITLE_MULTI_LANGUAGE
	int		nSubtitleLanguageCount;
	bool 		*pSelectedSubtitleLanguage;
#endif
} NormalView;


static void _vp_play_normal_view_destroy_handle(NormalView *pNormalView);
static bool _vp_play_normal_view_set_subtitle_font_config(NormalView *pNormalView);
static bool _vp_play_normal_view_update_progress_value(NormalView *pNormalView);
static bool _vp_play_normal_view_play_start(NormalView *pNormalView, bool bCheckDRM);
static void _vp_play_normal_view_on_next_play(NormalView *pNormalView, bool bManual);
static void _vp_play_normal_view_on_prev_play(NormalView *pNormalView, bool bManual, bool bVoice);
static bool _vp_play_normal_view_check_during_call(NormalView *pNormalView);
static void _vp_play_normal_view_show_audio_only_popup(NormalView *pNormalView);
static void _vp_play_normal_view_show_video_only_popup(NormalView *pNormalView);
static bool _vp_play_normal_view_check_audio_only(NormalView *pNormalView);
static bool _vp_play_normal_view_check_drm_lock(NormalView *pNormalView, bool *bLock);
static void _vp_play_normal_view_set_played_time(NormalView *pNormalView);
/*static void _vp_play_normal_view_screen_capture(NormalView *pNormalView);*/
static void _vp_play_normal_view_screen_move(NormalView *pNormalView, bool bFoward);

static void _vp_play_normal_view_create_progress_timer(NormalView *pNormalView);
static void _vp_play_normal_view_create_layout_hide_timer(NormalView *pNormalView);
static bool _vp_play_normal_view_create_loading_ani(NormalView *pNormalView);
static void _vp_play_normal_view_set_play_state(NormalView *pNormalView);
static double _vp_play_normal_view_get_mouse_pos_ratio(Evas_Object *pObj, int nCurX);
static void _vp_play_normal_view_all_close_popup(NormalView *pNormalView);
static void _vp_play_normal_view_show_layout(NormalView *pNormalView);
static void _vp_play_normal_view_hide_layout(NormalView *pNormalView, bool bNoEffect);

#ifdef ENABLE_GESTURE_ZOOM_FEATURE
static void _vp_play_normal_view_on_zoom_mode(NormalView *pNormalView);
#endif
static void _vp_play_normal_view_on_capture_mode(NormalView *pNormalView);
static void _vp_play_normal_view_on_share_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_sound_alive_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_sound_path_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_audio_track_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_sync_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_language_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_select_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_font_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_size_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_edge_popup(NormalView *pNormalView);

static void _vp_play_normal_view_on_subtitle_font_color_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_bg_color_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_subtitle_caption_win_color_popup(NormalView *pNormalView);

#ifdef SUBTITLE_K_FEATURE
//static void _vp_play_normal_view_on_subtitle_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
//static void _vp_play_normal_view_on_subtitle_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
//static void _vp_play_normal_view_on_subtitle_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _vp_play_normal_view_on_subtitle_alignment_popup(NormalView *pNormalView);
#endif

//static void _vp_play_normal_view_on_repeat_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_setting_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_detail_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_capture_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_play_speed_popup(NormalView *pNormalView);
static void _vp_play_normal_view_on_play_speed_popup_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _vp_play_normal_view_on_volume_popup(NormalView *pNormalView, bool bToggle);
static void _vp_play_normal_view_on_bookmark_mode(NormalView *pNormalView, bool bShow);
//static void _vp_play_normal_view_download_sdp(NormalView *pNormalView, char *szSdpPath);

static void _vp_play_normal_view_show_rotate(NormalView *pNormalView);
static void _vp_play_normal_view_show_volume(NormalView *pNormalView);
static void _vp_play_normal_view_set_sound_path_state(NormalView *pNormalView);
static void _vp_play_normal_view_set_rotate_lock_state(NormalView *pNormalView);
static void _vp_play_normal_view_set_volume_lock_state(NormalView *pNormalView);

static void _vp_normal_set_audio_only_image(NormalView *pNormalView, bool bAudioOnly);

static void __vp_play_normal_view_seek_pipe_cb(void *pUserData, void *pBuf, int nByte);
static void __vp_normal_error_popup_time_out_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo);
static void __vp_normal_error_popup_key_event_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo);
static void __vp_normal_error_popup_mouse_event_cb(void *pUserData, Evas *pEvas, Evas_Object *pObj, void *pEventInfo);
static void __vp_normal_completed_cb(void *pUserData);


//Focus UI
static void _vp_play_normal_view_set_button_focus_sequence(NormalView *pNormalView);
static void _vp_normal_view_speed_for_steps(NormalView *pNormalView, bool bSpeedFF);

static vp_mm_player_state_t pState = VP_MM_PLAYER_STATE_NONE;
static NormalView *curr_Handle = NULL;

#ifdef SUBTITLE_K_FEATURE
/*Set caption window: size, rel1, color*/
static void __vp_normal_subtitle_set_caption_window(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (!pNormalView->pSubtitle) {
		VideoLogError("pSubtitle is NULL");
		return;
	}

	Evas_Coord ww = 0;
	Evas_Coord hh = 0;
	bool bLandscape = FALSE;
	int width = 0.0;
	int offset_x = 0.0;
	vp_subtitle_alignment_t nAlignment = VP_SUBTITLE_ALIGNMENT_CENTER;

	bLandscape = vp_play_util_get_landscape_check(pNormalView->nRotate);
	vp_play_subtitle_get_size_formatted(pNormalView->pSubtitle, &ww, &hh);
	if (pNormalView->bSubtitleDown) {
		ww = bLandscape ? VP_SUBTITLE_LANDSCAPE_SIZE : VP_SUBTITLE_PORTRAIT_SIZE;
		hh += VP_SUBTITLE_MARGIN_SIZE;
	} else {
		ww += VP_SUBTITLE_MARGIN_SIZE;
		hh += VP_SUBTITLE_MARGIN_SIZE;
	}
	vp_play_subtitle_get_alignment(pNormalView->pSubtitle, &nAlignment);
	vp_play_util_set_object_size(_EDJ(vp_play_subtitle_get_object(pNormalView->pSubtitle)),
	                             ww, hh, VP_NORMAL_SET_CAPTION_WINDOW_SIZE_ID);
	width = bLandscape ? VP_SUBTITLE_LANDSCAPE_SIZE : VP_SUBTITLE_PORTRAIT_SIZE;
	if (nAlignment == VP_SUBTITLE_ALIGNMENT_LEFT) {
		offset_x = 0;
	} else if (nAlignment == VP_SUBTITLE_ALIGNMENT_CENTER) {
		offset_x = (width - ww) / 2;
	} else if (nAlignment == VP_SUBTITLE_ALIGNMENT_RIGHT) {
		offset_x = width - ww;
	}
	VideoLogInfo("nAlignment:%d,offset_x:%d", nAlignment, offset_x);
	vp_play_util_set_object_offset(_EDJ(vp_play_subtitle_get_object(pNormalView->pSubtitle)),
	                               offset_x, 0, VP_NORMAL_SET_CAPTION_WINDOW_OFFSET_ID);

	vp_play_subtitle_set_caption_win_color(pNormalView->pSubtitle, TRUE);
}
#endif

/* callback functions */

static Eina_Bool __vp_normal_exit_wait_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	pNormalView->pExitWaitTimer = NULL;

	return EINA_FALSE;
}

static void _vp_play_normal_view_set_volume_state(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	Evas_Object *pIcon = NULL;

	pIcon = elm_object_part_content_get(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_ICON);
	VP_EVAS_DEL(pIcon);
	pIcon = elm_object_part_content_get(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON);
	VP_EVAS_DEL(pIcon);
	pIcon = elm_object_part_content_get(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON);
	VP_EVAS_DEL(pIcon);

	elm_object_part_content_unset(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_ICON);
	elm_object_part_content_unset(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON);
	elm_object_part_content_unset(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	int nVolume = 0;
	bool bMute = FALSE;
	vp_sound_get_volume(&nVolume);

	VideoLogError("nVolume = %d:%d", nVolume, bMute);

	if ((nVolume == 0) || bMute) {
		pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_MUTE);
		elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_MUTE_PRESS);
		elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_MUTE_DIM);
		elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
	} else {
		pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_VOLUME);
		elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_VOLUME_PRESS);
		elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_VOLUME_DIM);
		elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
	}

}

static void __vp_normal_volume_change_value_cb(int nVolume, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VideoLogWarning("Set Volume: %d", nVolume);

	pNormalView->nLastVolume = nVolume;
	_vp_play_normal_view_set_volume_state(pNormalView);
}

/*static void __vp_normal_subtitle_popup_key_event_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pPopup);

	pNormalView->bIsPopupShow = FALSE;
}
*/

static void __vp_normal_popup_key_event_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pPopup);

	pNormalView->bIsPopupShow = FALSE;
}

static void __vp_normal_popup_mouse_event_cb(void *pUserData, Evas *pEvas, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Evas_Event_Mouse_Up *ev = pEventInfo;

	if (ev->button == 3) {
		NormalView *pNormalView = (NormalView *)pUserData;
		VP_EVAS_DEL(pNormalView->pPopup);
		pNormalView->bIsPopupShow = FALSE;
	}
}

static void __vp_normal_popup_time_out_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pPopup);
	pNormalView->bIsPopupShow = FALSE;
}

static void __vp_normal_prepare_error_popup_time_out_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pObj);
	pNormalView->bIsPopupShow = FALSE;

	if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE ||
	        pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
		PlayView *pPlayView = pNormalView->pPlayView;
		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			return;
		}
		if (!pPlayView->pFunc) {
			VideoLogError("pPlayView pFunc is NULL");
			return;
		}

		if (!pPlayView->pFunc->vp_play_func_exit) {
			VideoLogError("pPlayView exit Func is NULL");
			return;
		}
		vp_device_set_screen_off(FALSE);
		vp_play_util_set_unlock_power_key();

		// tmpory comment
		//pPlayView->pFunc->vp_play_func_exit(pPlayView);
	} else {
		//_vp_play_normal_view_on_next_play(pNormalView, FALSE);
	}

}

static void __vp_normal_error_popup_key_event_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pPopup);

	pNormalView->bIsPopupShow = FALSE;
	if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE ||
	        pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
		PlayView *pPlayView = pNormalView->pPlayView;
		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			return;
		}
		if (!pPlayView->pFunc) {
			VideoLogError("pPlayView pFunc is NULL");
			return;
		}

		if (!pPlayView->pFunc->vp_play_func_exit) {
			VideoLogError("pPlayView exit Func is NULL");
			return;
		}
		vp_device_set_screen_off(FALSE);
		vp_play_util_set_unlock_power_key();

		pPlayView->pFunc->vp_play_func_exit(pPlayView);
	} else {
		_vp_play_normal_view_on_next_play(pNormalView, FALSE);
	}

}

static void __vp_normal_error_popup_mouse_event_cb(void *pUserData, Evas *pEvas, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Evas_Event_Mouse_Up *ev = pEventInfo;

	if (ev->button == 3) {
		NormalView *pNormalView = (NormalView *)pUserData;
		VP_EVAS_DEL(pNormalView->pPopup);
		pNormalView->bIsPopupShow = FALSE;
		if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE ||
		        pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
			PlayView *pPlayView = pNormalView->pPlayView;
			if (!pPlayView) {
				VideoLogError("pPlayView is NULL");
				return;
			}
			if (!pPlayView->pFunc) {
				VideoLogError("pPlayView pFunc is NULL");
				return;
			}

			if (!pPlayView->pFunc->vp_play_func_exit) {
				VideoLogError("pPlayView exit Func is NULL");
				return;
			}
			vp_device_set_screen_off(FALSE);
			vp_play_util_set_unlock_power_key();

			pPlayView->pFunc->vp_play_func_exit(pPlayView);
		} else {
			_vp_play_normal_view_on_next_play(pNormalView, FALSE);
		}
	}
}

static void __vp_normal_error_popup_time_out_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pPopup);
	pNormalView->bIsPopupShow = FALSE;

	_vp_play_normal_view_on_next_play(pNormalView, FALSE);
}

static Eina_Bool __vp_normal_control_idler_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return ECORE_CALLBACK_CANCEL;
	}
	NormalView	*pNormalView = (NormalView *)pUserData;

	_vp_play_normal_view_set_play_state(pNormalView);
	_vp_play_normal_view_on_capture_mode(pNormalView);

	pNormalView->pControlIdler = NULL;

	return ECORE_CALLBACK_CANCEL;
}

static void __vp_normal_sound_alive_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	video_sound_alive_t nSAType = (video_sound_alive_t)nType;

	vp_sound_alive_destroy(pNormalView->pSAHandle);
	pNormalView->pSAHandle = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	} else {
		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		}
	}


	vp_mm_player_sound_filter_t nSoundFilter = VP_MM_PLAYER_FILTER_NONE;

	if (nSAType == VIDEO_SA_NORMAL) {
		nSoundFilter = VP_MM_PLAYER_FILTER_NONE;
	} else if (nSAType == VIDEO_SA_VOICE) {
		nSoundFilter = VP_MM_PLAYER_FILTER_VOICE;
	} else if (nSAType == VIDEO_SA_MOVIE) {
		nSoundFilter = VP_MM_PLAYER_FILTER_MOVIE;
	} else if (nSAType == VIDEO_SA_7_1_CH) {
		nSoundFilter = VP_MM_PLAYER_FILTER_VITUAL_71;

		video_sound_device_type_t nSoundDevType = VP_SOUND_DEVICE_NONE;
		vp_sound_get_active_device(&nSoundDevType);
		if (nSoundDevType != VP_SOUND_DEVICE_EARJACK && nSoundDevType != VP_SOUND_DEVICE_BLUETOOTH) {
			/* show popup */
			VP_EVAS_DEL(pNormalView->pPopup);
			pNormalView->pPopup = NULL;

			PlayView *pPlayView = pNormalView->pPlayView;

			if (!pPlayView) {
				VideoLogError("pPlayView is NULL");
				return;
			}
			pNormalView->bIsPopupShow = TRUE;
			char *szMsg = g_strdup_printf(VP_PLAY_STRING_71CH_ONLY_EARPHONE, "7.1");
			pNormalView->pPopup = vp_popup_create(pPlayView->pWin, POPUP_STYLE_DEFAULT_WITH_CANCEL_BTN,
			                                      NULL,
			                                      szMsg,
			                                      0.0, NULL,
			                                      __vp_normal_popup_key_event_cb,
			                                      __vp_normal_popup_mouse_event_cb,
			                                      pNormalView);
			VP_FREE(szMsg);

			Evas_Object *pBtn = NULL;
			pBtn = elm_button_add(pNormalView->pPopup);
			elm_object_style_set(pBtn, "popup_button/default");
			elm_object_domain_translatable_text_set(pBtn, VP_SYS_STR_PREFIX, VP_PLAY_STRING_COM_OK_IDS);
			elm_object_part_content_set(pNormalView->pPopup, "button1", pBtn);
			evas_object_smart_callback_add(pBtn, "clicked", __vp_normal_popup_key_event_cb, (void*)pNormalView);
			evas_object_show(pNormalView->pPopup);

			return;
		}
	} else {
		return;

	}

	pNormalView->nSoundAlive = nSAType;
	if (!vp_play_preference_set_sound_alive_status(nSAType)) {
		VideoLogError("vp_play_preference_set_sound_alive_status failed");
	}

	if (!vp_mm_player_set_sound_filter(pNormalView->pPlayerHandle, nSoundFilter)) {
		VideoLogWarning("vp_mm_player_set_sound_filter is fail");
	}
}

static void __vp_normal_sound_path_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	vp_play_volume_update_value(pNormalView->pVolumeHandle);

	vp_sound_path_destroy(pNormalView->pSoundPathHandle);
	pNormalView->pSoundPathHandle = NULL;
	pNormalView->bIsPopupShow = FALSE;

	_vp_play_normal_view_set_sound_path_state(pNormalView);

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}

		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	} else {
		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("play Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		}
	}

}

/*
static void __vp_normal_repeat_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	else {
		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		}
	}

	video_play_repeat_mode_t nRepeatMode = (video_play_repeat_mode_t)nType;

	vp_repeat_destroy(pNormalView->pRepeatHandle);
	pNormalView->pRepeatHandle = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	else {
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}

	if (nRepeatMode != VIDEO_PLAY_REPEAT_NONE) {
		pNormalView->nRepeatMode = nRepeatMode;
	}
}
*/

static void __vp_normal_setting_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	vp_setting_destroy(pNormalView->pSettingHandle);
	pNormalView->pSettingHandle = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (nType == VP_SETTING_MODE_NONE || nType == VP_SETTING_MODE_PLAY_SPEED) {
		if (bPause) {
			if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		} else {
			if (pNormalView->bManualPause == FALSE) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogWarning("Pause Fail");
				}
				_vp_play_normal_view_set_play_state(pNormalView);
				_vp_play_normal_view_on_capture_mode(pNormalView);
				_vp_play_normal_view_create_layout_hide_timer(pNormalView);
			}
		}
	}

	switch (nType) {
	case VP_SETTING_MODE_PLAY_SPEED:
		_vp_play_normal_view_on_play_speed_popup(pNormalView);
		break;
	case VP_SETTING_MODE_SOUND_ALIVE:
		_vp_play_normal_view_on_sound_alive_popup(pNormalView);
		break;
	case VP_SETTING_MODE_AUDIO_TRACK:
		_vp_play_normal_view_on_audio_track_popup(pNormalView);
		break;
	case VP_SETTING_MODE_CAPTURE:
		_vp_play_normal_view_on_capture_popup(pNormalView);
		break;
	}
}

static void __vp_normal_capture_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (nType != -1) {
		pNormalView->bCaptureMode = (bool)nType;
	}
	vp_play_preference_set_capture_on_key(pNormalView->bCaptureMode);

	vp_capture_popup_destroy(pNormalView->pCapturePopup);
	pNormalView->pCapturePopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}

	_vp_play_normal_view_on_capture_mode(pNormalView);
}

static void __vp_normal_play_speed_popup_update_cb(double fValue, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	NormalView *pNormalView = (NormalView *)pUserData;

	vp_mm_player_set_rate(pNormalView->pPlayerHandle, (float)fValue);
}

static void __vp_normal_play_speed_popup_done_cb(double fValue, bool bSetValue, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	NormalView *pNormalView = (NormalView *)pUserData;

	vp_play_speed_popup_destroy(pNormalView->pPlaySpeedPopup);
	pNormalView->pPlaySpeedPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Play Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}

	if (bSetValue) {
		pNormalView->fPlaySpeed = fValue;
		if (pNormalView->pPlayView != NULL) {
			pNormalView->pPlayView->fPlaySpeed = fValue;
		}

		if ((fValue - 0.05) > 1.000 || (fValue + 0.05) < 1.000) {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_SHOW, "*");
			elm_object_signal_callback_add(pNormalView->pMainLayout, "elm,action,click", "", _vp_play_normal_view_on_play_speed_popup_cb, pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		} else {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_HIDE, "*");
		}
		char szTxt[5] = {0,};
		snprintf(szTxt, 5, "%1.1fX", (float)pNormalView->fPlaySpeed);
		elm_object_part_text_set(pNormalView->pMainLayout, VP_PLAY_PART_NORMAL_PLAYING_SPEED_TXT, szTxt);
	}
	vp_mm_player_set_rate(pNormalView->pPlayerHandle, (float)pNormalView->fPlaySpeed);
}

static void __vp_normal_subtitle_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_mode_t nSubtitleMode = (vp_subtitle_mode_t)nType;

	if (nSubtitleMode == VP_SUBTITLE_MODE_CLOSE) {
		_vp_play_normal_view_set_subtitle_font_config(pNormalView);
		vp_play_subtitle_text_update(pNormalView->pSubtitle, NULL);
		bool bShow = FALSE;
		vp_play_preference_get_subtitle_show_key(&bShow);
		if (bShow) {
			vp_play_subtitle_realize(pNormalView->pSubtitle);
			vp_mm_player_set_deactivate_subtitle(pNormalView->pPlayerHandle, FALSE);
		} else {
			vp_play_subtitle_unrealize(pNormalView->pSubtitle);
			vp_mm_player_set_deactivate_subtitle(pNormalView->pPlayerHandle, TRUE);
		}
		vp_subtitle_destroy(pNormalView->pSubtitlePopup);
		pNormalView->pSubtitlePopup = NULL;
		pNormalView->bIsSubtitleShow = FALSE;
	}

	switch (nSubtitleMode) {
	case VP_SUBTITLE_MODE_NONE:
	case VP_SUBTITLE_MODE_CLOSE:
		break;
	case VP_SUBTITLE_MODE_ACTIVATION: {
		bool bShow = FALSE;
		vp_play_preference_get_subtitle_show_key(&bShow);
		if (bShow) {
			vp_play_subtitle_realize(pNormalView->pSubtitle);
			vp_mm_player_set_deactivate_subtitle(pNormalView->pPlayerHandle, FALSE);
		} else {
			vp_play_subtitle_unrealize(pNormalView->pSubtitle);
			vp_mm_player_set_deactivate_subtitle(pNormalView->pPlayerHandle, TRUE);
		}
	}
	break;
	case VP_SUBTITLE_MODE_SYNC:
		_vp_play_normal_view_on_subtitle_sync_popup(pNormalView);
		break;
	case VP_SUBTITLE_MODE_LANGUAGE:
		_vp_play_normal_view_on_subtitle_language_popup(pNormalView);
		break;
#ifdef SUBTITLE_K_FEATURE
	case VP_SUBTITLE_MODE_ALIGNMENT:
		_vp_play_normal_view_on_subtitle_alignment_popup(pNormalView);
		break;
#endif
	case VP_SUBTITLE_MODE_SELECT:
		_vp_play_normal_view_on_subtitle_select_popup(pNormalView);
		break;
	case VP_SUBTITLE_MODE_FONT:
		_vp_play_normal_view_on_subtitle_font_popup(pNormalView);
		break;
	case VP_SUBTITLE_MODE_SIZE:
		_vp_play_normal_view_on_subtitle_size_popup(pNormalView);
		break;
//#ifdef SUBTITLE_K_FEATURE
	case VP_SUBTITLE_MODE_EDGE:
		_vp_play_normal_view_on_subtitle_edge_popup(pNormalView);
		break;
	case VP_SUBTITLE_MODE_TEXT:
		_vp_play_normal_view_on_subtitle_font_color_popup(pNormalView);
		break;
	case VP_SUBTITLE_MODE_BG_COLOR:

		_vp_play_normal_view_on_subtitle_bg_color_popup(pNormalView);
		break;
#ifdef SUBTITLE_K_FEATURE
	case VP_SUBTITLE_MODE_CAPTION_WINDOW:
		_vp_play_normal_view_on_subtitle_caption_win_color_popup(pNormalView);
		break;
#endif
	}
}

static void __vp_normal_subtitle_sync_popup_update_cb(float fValue, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	/* set subtitle sync value */

	int nPosition = fValue * 1000;

	if (!vp_mm_player_set_subtitle_position(pNormalView->pPlayerHandle, nPosition)) {
		VideoLogError("vp_mm_player_set_subtitle_position is fail");
	}
}

static void __vp_normal_subtitle_sync_popup_done_cb(float fValue, bool bSetValue, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_sync_popup_destroy(pNormalView->pSubtitleSyncPopup);
	pNormalView->pSubtitleSyncPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bSetValue) {
		/* set subtitle sync value */
		int nPosition = fValue * 1000;
		pNormalView->fSubtitleSyncValue = fValue;
		pNormalView->pPlayView->fSubtitleSyncValue = fValue;
		if (!vp_mm_player_set_subtitle_position(pNormalView->pPlayerHandle, nPosition)) {
			VideoLogError("vp_mm_player_set_subtitle_position is fail");
		}
	} else {
		/* set subtitle sync value */
		int nPosition = pNormalView->fSubtitleSyncValue * 1000;

		if (!vp_mm_player_set_subtitle_position(pNormalView->pPlayerHandle, nPosition)) {
			VideoLogError("vp_mm_player_set_subtitle_position is fail");
		}
		_vp_play_normal_view_on_subtitle_popup(pNormalView);
	}
}


static void __vp_normal_subtitle_size_popup_done_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_size_destroy(pNormalView->pSubtitleSizePopup);
	pNormalView->pSubtitleSizePopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (nType != -1) {
		vp_play_subtitle_set_size(pNormalView->pSubtitle, nType);
		vp_play_subtitle_set_size_zoom(pNormalView->pSubtitle, 1.0);
		vp_play_preference_set_subtitle_size_key(nType);
		_vp_play_normal_view_set_subtitle_font_config(pNormalView);
#ifdef SUBTITLE_K_FEATURE
		/*Set caption window: size, rel1, color*/
		__vp_normal_subtitle_set_caption_window(pNormalView);
#endif
	}

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	_vp_play_normal_view_on_subtitle_popup(pNormalView);

	/*
		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		}
	*/
}


static void __vp_normal_subtitle_font_popup_done_cb(char *szFontName, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (szFontName) {
		vp_play_subtitle_set_font(pNormalView->pSubtitle, szFontName);
		vp_play_preference_set_subtitle_font_name_key(szFontName);
		_vp_play_normal_view_set_subtitle_font_config(pNormalView);
#ifdef SUBTITLE_K_FEATURE
		/*Set caption window: size, rel1, color*/
		__vp_normal_subtitle_set_caption_window(pNormalView);
#endif
	}

	vp_subtitle_font_destroy(pNormalView->pSubtitleFontPopup);
	pNormalView->pSubtitleFontPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	/*
		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		}
	*/
	_vp_play_normal_view_on_subtitle_popup(pNormalView);
}

static void __vp_normal_subtitle_select_button_popup_done_cb(char *option, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (!strcmp(option, "settings")) {
		_vp_play_normal_view_on_subtitle_popup(pNormalView);
	} else {
		vp_subtitle_select_destroy(pNormalView->pSubtitleSelectPopup);
		pNormalView->pSubtitleSelectPopup = NULL;
		pNormalView->bIsPopupShow = FALSE;
		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		}
	}
}

static void __vp_normal_subtitle_select_popup_done_cb(char *szFilePath, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	char *szSubtitle = NULL;

	vp_subtitle_select_destroy(pNormalView->pSubtitleSelectPopup);
	pNormalView->pSubtitleSelectPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;
	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}
	if (!szFilePath) {
		goto SELECT_DONE;
	}
	if (!strcmp(szFilePath, VP_PLAY_STRING_SUBTITLE_OFF)) {
		pNormalView->bIsExistSubtitle = FALSE;
		vp_mm_player_set_deactivate_subtitle(pNormalView->pPlayerHandle, TRUE);
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_HIDE, "*");
		goto SELECT_DONE;
	} else {
		bool bOn = FALSE;
		vp_play_preference_get_subtitle_show_key(&bOn);
		if (bOn) {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_PORTRAIT, "*");
			pNormalView->bIsExistSubtitle = TRUE;
			vp_mm_player_set_deactivate_subtitle(pNormalView->pPlayerHandle, FALSE);
		}
	}
	VP_FREE(pNormalView->szSubtitleURL);
	VP_STRDUP(pNormalView->szSubtitleURL, szFilePath);
	vp_mm_player_get_subtitle_url(pNormalView->pPlayerHandle, &szSubtitle);

	if (szSubtitle) {
		if (strcmp(szSubtitle, szFilePath) == 0) {
			goto SELECT_DONE;
		}
	}
	/*
		int nPosition = 0;
		if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
			VideoLogError("vp_mm_player_get_position is fail");
			pNormalView->nStartPosition = nPosition;
		}
	*/
	pNormalView->bIsExistSubtitle = vp_mm_player_set_subtitle_url(pNormalView->pPlayerHandle, szFilePath);

SELECT_DONE:
	VP_FREE(szSubtitle);

	/*if (pNormalView->bIsExistSubtitle) {
		_vp_play_normal_view_on_subtitle_popup(pNormalView);
	}*/
}

#ifndef SUBTITLE_K_FEATURE
static void __vp_normal_subtitle_font_color_popup_done_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_color_destroy(pNormalView->pSubtitleFontColorPopup);
	pNormalView->pSubtitleFontColorPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (nType != -1) {

		video_subtitle_color_t nColor = VP_SUBTITLE_COLOR_BLACK;
		if (nType == VIDEO_SUBTITLE_COLOR_BLACK) {
			nColor = VP_SUBTITLE_COLOR_BLACK;
		} else if (nType == VIDEO_SUBTITLE_COLOR_BLUE) {
			nColor = VP_SUBTITLE_COLOR_BLUE;
		} else if (nType == VIDEO_SUBTITLE_COLOR_GREEN) {
			nColor = VP_SUBTITLE_COLOR_GREEN;
		} else if (nType == VIDEO_SUBTITLE_COLOR_WHITE) {
			nColor = VP_SUBTITLE_COLOR_WHITE;
		}
		//need to improve....

		vp_play_subtitle_set_color(pNormalView->pSubtitle, nColor);

		vp_play_preference_set_subtitle_font_color_key(nType);

		_vp_play_normal_view_set_subtitle_font_config(pNormalView);
	}

	_vp_play_normal_view_on_subtitle_popup(pNormalView);

	/*if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}*/
}

static void __vp_normal_subtitle_bg_color_popup_done_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_bg_color_destroy(pNormalView->pSubtitleBGColorPopup);
	pNormalView->pSubtitleBGColorPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (nType != -1) {
		video_subtitle_color_t nColor = VP_SUBTITLE_COLOR_BLACK;
		if (nType == VIDEO_SUBTITLE_COLOR_BLACK) {
			nColor = VP_SUBTITLE_COLOR_BLACK;
		} else if (nType == VIDEO_SUBTITLE_COLOR_WHITE) {
			nColor = VP_SUBTITLE_COLOR_WHITE;
		} else if (nType == VIDEO_SUBTITLE_COLOR_NONE) {
			nColor = VP_SUBTITLE_COLOR_NONE;
		}
		vp_play_subtitle_set_bg_color(pNormalView->pSubtitle, nColor);
		vp_play_preference_set_subtitle_bg_color_key(nType);
		_vp_play_normal_view_set_subtitle_font_config(pNormalView);
	}
	_vp_play_normal_view_on_subtitle_popup(pNormalView);

	/*if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}*/
}

#else
static void __vp_normal_subtitle_font_color_popup_done_hex_cb(char *pColorHex, bool bSetValue, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	char *pSelectColorHex = strdup(pColorHex);

	vp_subtitle_color_destroy(pNormalView->pSubtitleFontColorPopup);
	pNormalView->pSubtitleFontColorPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bSetValue) {
		VideoLogInfo("pSelectColorHex =%s", pSelectColorHex);

		if (pSelectColorHex != NULL) {
			vp_play_subtitle_set_color(pNormalView->pSubtitle, pSelectColorHex);

			vp_play_preference_set_subtitle_font_color_hex_key(pSelectColorHex);

			_vp_play_normal_view_set_subtitle_font_config(pNormalView);

		}
	}

	VP_FREE(pSelectColorHex);

	_vp_play_normal_view_on_subtitle_popup(pNormalView);
}

static void __vp_normal_subtitle_bg_color_popup_done_hex_cb(char *pColorHex, bool bSetValue, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	char *pSelectColorHex = strdup(pColorHex);

	vp_subtitle_color_destroy(pNormalView->pSubtitleBGColorPopup);
	pNormalView->pSubtitleBGColorPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bSetValue) {
		VideoLogInfo("pSelectColorHex =%s", pSelectColorHex);
		if (pSelectColorHex != NULL) {
			vp_play_subtitle_set_bg_color(pNormalView->pSubtitle, pSelectColorHex);

			vp_play_preference_set_subtitle_bg_color_hex_key(pSelectColorHex);

			_vp_play_normal_view_set_subtitle_font_config(pNormalView);

		}
	}

	VP_FREE(pSelectColorHex);

	_vp_play_normal_view_on_subtitle_popup(pNormalView);
}

static void __vp_normal_subtitle_caption_win_color_popup_done_hex_cb(char *pColorHex, bool bSetValue, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	char *pSelectColorHex = strdup(pColorHex);

	vp_subtitle_color_destroy(pNormalView->pSubtitleCaptionWinColorPopup);
	pNormalView->pSubtitleCaptionWinColorPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bSetValue) {
		VideoLogInfo("pSelectColorHex =%s", pSelectColorHex);
		if (pSelectColorHex != NULL) {
			vp_play_preference_set_subtitle_caption_win_color_hex_key(pSelectColorHex);
			vp_play_subtitle_set_caption_win_color(pNormalView->pSubtitle, TRUE);
		}
	}

	VP_FREE(pSelectColorHex);

	_vp_play_normal_view_on_subtitle_popup(pNormalView);
}

#endif

static void __vp_normal_subtitle_track_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_track_destroy(pNormalView->pSubtitleTrackHandle);
	pNormalView->pSubtitleTrackHandle = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (nType >= 0) {

		/*if (bPause) {
			if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		}
		else {
			if (pNormalView->bManualPause == FALSE) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogWarning("Pause Fail");
				}
				_vp_play_normal_view_set_play_state(pNormalView);
				_vp_play_normal_view_on_capture_mode(pNormalView);
				_vp_play_normal_view_create_layout_hide_timer(pNormalView);
			}
		}*/

		pNormalView->nDefaultSubtitleTrackIndex = nType;
#ifndef _SUBTITLE_MULTI_LANGUAGE
		vp_mm_player_set_subtitle_track(pNormalView->pPlayerHandle, pNormalView->nDefaultSubtitleTrackIndex);
#endif
		_vp_play_normal_view_on_subtitle_popup(pNormalView);

	} else {
		_vp_play_normal_view_on_subtitle_popup(pNormalView);
	}

}



static void __vp_normal_detail_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("resume Fail");
		}

		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}

	vp_detail_destroy(pNormalView->pDetailHandle);
	pNormalView->pDetailHandle = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	} else {
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}

}


static void __vp_normal_audio_track_popup_close_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	} else {
		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
			_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		}
	}

	vp_audio_track_destroy(pNormalView->pAudioTrackHandle);
	pNormalView->pAudioTrackHandle = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	} else {
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}
	if (nType != -1) {
		pNormalView->nDefaultAudioTrackIndex = nType;

		vp_mm_player_set_audio_track(pNormalView->pPlayerHandle, pNormalView->nDefaultAudioTrackIndex);
	}

}

static Evas_Event_Flags __vp_normal_gesture_n_tab_start_cb(void *pUserData , void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags __vp_normal_gesture_n_tab_end_cb(void *pUserData , void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) pEventInfo;
	NormalView *pNormalView = (NormalView *)pUserData;
	VideoLogInfo("gesture tab = %d" , p->n);

	if (p->n == 1) {
		if (pNormalView->pVolumeHandle) {
			bool bIsRealize = FALSE;

			if (!vp_play_volume_is_realize(pNormalView->pVolumeHandle, &bIsRealize)) {
				VideoLogWarning("vp_play_volume_is_realize is fail");
			}

			if (bIsRealize) {
				if (!vp_play_volume_unrealize(pNormalView->pVolumeHandle)) {
					VideoLogWarning("vp_play_volume_unrealize is fail");
				}
			}
		}

		if (pNormalView->bShowLayout) {
			_vp_play_normal_view_hide_layout(pNormalView, FALSE);
		} else {
			_vp_play_normal_view_show_layout(pNormalView);
		}
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags __vp_normal_gesture_n_tab_abort_cb(void *pUserData , void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}


	return EVAS_EVENT_FLAG_NONE;
}

#ifdef ENABLE_VOLUME_BRIGHTNESS_POPUP
static void __vp_normal_volume_job_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bIsActive == FALSE) {
		VideoLogError("Not Active mode");
		return;
	}
	if (!pNormalView->pVolumePopupHandle) {
		if (!pNormalView->pPlayView) {
			VideoLogError("pNormalView->pPlayView is NULL");
			return;
		}

		Evas_Object *pParent = pNormalView->pPlayView->pWin;
		pNormalView->pVolumePopupHandle = vp_play_volume_popup_create(pParent);
		if (!pNormalView->pVolumePopupHandle) {
			VideoLogError("pNormalView->pVolumePopupHandle is NULL");
			return;
		}
	}

	bool bIsRealize = FALSE;

	if (!vp_play_volume_popup_is_realize(pNormalView->pVolumePopupHandle, &bIsRealize)) {
		VideoLogWarning("vp_play_volume_popup_is_realize is fail");
		return;
	}
	if (bIsRealize == FALSE) {
		bool bLandscape = FALSE;

		if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
			bLandscape = TRUE;
		}

		if (!vp_play_volume_popup_set_landscape_mode(pNormalView->pVolumePopupHandle, bLandscape)) {
			VideoLogWarning("vp_play_volume_popup_set_landscape_mode is fail");
		}

		if (!vp_play_volume_popup_realize(pNormalView->pVolumePopupHandle)) {
			VideoLogWarning("vp_play_volume_popup_realize is fail");
		}
	}
	if (pNormalView->bMomentumIncrease) {
		int nMaxVal = 0;
		if (!vp_play_volume_popup_set_value(pNormalView->pVolumePopupHandle, pNormalView->nMomentDefaultVal + pNormalView->nMomentWeight)) {
			VideoLogWarning("vp_play_volume_popup_set_value is fail");
		}

		if (vp_play_volume_popup_get_max_value(&nMaxVal)) {
			if (pNormalView->nMomentDefaultVal + pNormalView->nMomentWeight >= nMaxVal) {
				pNormalView->nGestureStartPosY = pNormalView->nGestureOldPosY;
				pNormalView->nMomentDefaultVal = nMaxVal;
			}
		}
	} else {
		if (!vp_play_volume_popup_set_value(pNormalView->pVolumePopupHandle, pNormalView->nMomentDefaultVal - pNormalView->nMomentWeight)) {
			VideoLogWarning("vp_play_volume_popup_set_value is fail");
		}

		if (pNormalView->nMomentDefaultVal - pNormalView->nMomentWeight <= 0) {
			pNormalView->nGestureStartPosY = pNormalView->nGestureOldPosY;
			pNormalView->nMomentDefaultVal = 0;
		}
	}

	VP_EVAS_JOB_DEL(pNormalView->pVolumeJob);

}

static void __vp_normal_brightness_job_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bIsActive == FALSE) {
		VideoLogError("Not Active mode");
		return;
	}
	if (!pNormalView->pBrightnessHandle) {
		if (!pNormalView->pPlayView) {
			VideoLogError("pNormalView->pPlayView is NULL");
			return;
		}

		Evas_Object *pParent = pNormalView->pPlayView->pWin;
		pNormalView->pBrightnessHandle = vp_play_brightness_create(pParent);
		if (!pNormalView->pBrightnessHandle) {
			VideoLogError("pNormalView->pBrightnessHandle is NULL");
			return;
		}
	}

	bool bIsRealize = FALSE;

	if (!vp_play_brightness_is_realize(pNormalView->pBrightnessHandle, &bIsRealize)) {
		VideoLogWarning("vp_play_brightness_is_realize is fail");
		return;
	}
	if (bIsRealize == FALSE) {
		bool bLandscape = FALSE;

		if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
			bLandscape = TRUE;
		}

		if (!vp_play_brightness_set_landscape_mode(pNormalView->pBrightnessHandle, bLandscape)) {
			VideoLogWarning("vp_play_brightness_set_landscape_mode is fail");
		}

		if (!vp_play_brightness_realize(pNormalView->pBrightnessHandle)) {
			VideoLogWarning("vp_play_brightness_realize is fail");
		}
	}

	if (pNormalView->bMomentumIncrease) {
		int nMaxVal = 0;
		if (!vp_play_brightness_set_value(pNormalView->pBrightnessHandle, pNormalView->nMomentDefaultVal + pNormalView->nMomentWeight)) {
			VideoLogWarning("vp_play_brightness_set_value is fail");
		}

		if (vp_play_brightness_get_max_value(&nMaxVal)) {
			if (pNormalView->nMomentDefaultVal + pNormalView->nMomentWeight >= nMaxVal) {
				pNormalView->nGestureStartPosY = pNormalView->nGestureOldPosY;
				pNormalView->nMomentDefaultVal = nMaxVal;
			}
		}
	} else {
		int nMinVal = 0;
		if (!vp_play_brightness_set_value(pNormalView->pBrightnessHandle, pNormalView->nMomentDefaultVal - pNormalView->nMomentWeight)) {
			VideoLogWarning("vp_play_brightness_set_value is fail");
		}

		if (vp_play_brightness_get_min_value(&nMinVal)) {
			if (pNormalView->nMomentDefaultVal - pNormalView->nMomentWeight <= nMinVal) {
				pNormalView->nGestureStartPosY = pNormalView->nGestureOldPosY;
				pNormalView->nMomentDefaultVal = nMinVal;
			}
		}
	}
	VP_EVAS_JOB_DEL(pNormalView->pBrightnessJob);

}
#endif

#ifdef ENABLE_VOLUME_BRIGHTNESS_POPUP
static Eina_Bool __vp_normal_momentum_press_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

#ifdef SUBTITLE_K_FEATURE
	if (pNormalView->bSubtitleDown) {
		VideoLogError("Skip for subtitle down");
		return EINA_FALSE;
	}
#endif

	pNormalView->pMomentTimer = NULL;
	bool bVolumeMode = FALSE;

	if (pNormalView->nGestureStartPosX < (pNormalView->nWindowWidth / 2)) {
		bVolumeMode = TRUE;
	}

	if (bVolumeMode) {
		VP_EVAS_JOB_DEL(pNormalView->pVolumeJob);
		pNormalView->pVolumeJob = ecore_job_add(__vp_normal_volume_job_cb, (void *)pNormalView);
	} else {
		VP_EVAS_JOB_DEL(pNormalView->pBrightnessJob);
		pNormalView->pBrightnessJob = ecore_job_add(__vp_normal_brightness_job_cb, (void *)pNormalView);
	}

	return EINA_FALSE;
}
#endif

static Evas_Event_Flags __vp_normal_gesture_momentum_start_cb(void *pUserData, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) pEventInfo;
	NormalView *pNormalView = (NormalView *)pUserData;
	if (pNormalView->bLockScreen) {
		return EVAS_EVENT_FLAG_NONE;
	}
#ifdef SUBTITLE_K_FEATURE
	if (pNormalView->pSubtitleMoveTimer || pNormalView->bSubtitleDown) {
		VideoLogWarning("Longpress subtitle");
		return EVAS_EVENT_FLAG_NONE;
	}
#endif
	pNormalView->nGestureOldPosY = p->y1;
	pNormalView->nGestureStartPosY = p->y1;

	int nWidth, nHeight;
	bool bLandscape = FALSE;

	if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
		bLandscape = TRUE;
	}
	elm_win_screen_size_get(pNormalView->pPlayView->pWin, NULL, NULL, &nWidth, &nHeight);
	if (bLandscape) {
		pNormalView->nWindowWidth = nHeight;
		pNormalView->nWindowHeight = nWidth;
	} else {

		pNormalView->nWindowWidth = nWidth;
		pNormalView->nWindowHeight = nHeight;
	}

	if (pNormalView->bZoomEnable == FALSE) {
#ifdef ENABLE_VOLUME_BRIGHTNESS_POPUP
		bool bVolumeMode = FALSE;
		int nMaxVal = 0;
		int nMinVal = 0;

		pNormalView->nGestureStartPosX = p->x1;
		pNormalView->nMomentWeight = 0;

		if (p->x1 < (pNormalView->nWindowWidth / 2)) {
			bVolumeMode = TRUE;
		}

		bool bIsVolumeRealize = FALSE;

		if (pNormalView->pVolumeHandle) {
			vp_play_volume_is_realize(pNormalView->pVolumeHandle, &bIsVolumeRealize);
		}

		if (bVolumeMode && bIsVolumeRealize) {
			if (pNormalView->pVolumePopupHandle) {
				if (!vp_play_volume_popup_unrealize(pNormalView->pVolumePopupHandle)) {
					VideoLogWarning("vp_play_volume_popup_unrealize is fail");
				}
			}
			return EVAS_EVENT_FLAG_NONE;
		}


		if (bVolumeMode) {
			if (!vp_play_volume_popup_get_value(&pNormalView->nMomentDefaultVal)) {
				VideoLogError("vp_play_volume_popup_get_value is fail");
			}
			if (!vp_play_volume_popup_get_max_value(&nMaxVal)) {
				VideoLogError("vp_play_volume_popup_get_max_value is fail");
			}
		} else {

			if (!vp_play_brightness_get_value(&pNormalView->nMomentDefaultVal)) {
				VideoLogError("vp_play_brightness_get_value is fail");
			}
			if (!vp_play_brightness_get_max_value(&nMaxVal)) {
				VideoLogError("vp_play_brightness_get_max_value is fail");
			}
			if (!vp_play_brightness_get_min_value(&nMinVal)) {
				VideoLogError("vp_play_brightness_get_min_value is fail");
			}
		}

		VP_EVAS_TIMER_DEL(pNormalView->pMomentTimer);
		pNormalView->pMomentTimer = NULL;

		pNormalView->pMomentTimer = ecore_timer_add(VP_NORMAL_MOMENTUM_PRESS_TIMER_INTERVAL,
		                            __vp_normal_momentum_press_timer_cb, (void *)pNormalView);

		if (nMaxVal <= 0) {
			nMaxVal = 1;
		}
		pNormalView->nMomentStep = (pNormalView->nWindowHeight / 2) / (nMaxVal - nMinVal);
#endif
	} else	{
		pNormalView->nGestureStartPosX = p->x2;
		pNormalView->nGestureStartPosY = p->y2;
	}

	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags __vp_normal_gesture_momentum_move_cb(void *pUserData, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) pEventInfo;
	NormalView *pNormalView = (NormalView *)pUserData;

#ifdef SUBTITLE_K_FEATURE
	if (pNormalView->pSubtitleMoveTimer || pNormalView->bSubtitleDown) {
		VideoLogWarning("Longpress subtitle");
		return EVAS_EVENT_FLAG_NONE;
	}
#endif

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for lock mode");
		_vp_play_normal_view_show_layout(pNormalView);
		return EVAS_EVENT_FLAG_NONE;
	}

	if (p->n != 1) {
		return EVAS_EVENT_FLAG_NONE;
	}

	if (pNormalView->bZoomEnable == FALSE) {
#ifdef ENABLE_VOLUME_BRIGHTNESS_POPUP
		if (pNormalView->pMomentTimer ||
		        pNormalView->bLockScreen) {
			return EVAS_EVENT_FLAG_NONE;
		}

		if (pNormalView->bShowLayout) {
			_vp_play_normal_view_hide_layout(pNormalView, FALSE);
		}

		int nDiff = 0;
		bool bVolumeMode = FALSE;
		if (pNormalView->nMomentStep <= 0) {
			pNormalView->nMomentStep = 1;
		}

		if (p->x1 < (pNormalView->nWindowWidth / 2)) {
			bVolumeMode = TRUE;
		}

		bool bIsVolumeRealize = FALSE;

		if (pNormalView->pVolumeHandle) {
			vp_play_volume_is_realize(pNormalView->pVolumeHandle, &bIsVolumeRealize);
		}

		if (bVolumeMode && bIsVolumeRealize) {
			if (pNormalView->pVolumePopupHandle) {
				if (!vp_play_volume_popup_unrealize(pNormalView->pVolumePopupHandle)) {
					VideoLogWarning("vp_play_volume_popup_unrealize is fail");
				}
			}
			return EVAS_EVENT_FLAG_NONE;
		}

		if (pNormalView->nGestureStartPosY < p->y2) {
			pNormalView->bMomentumIncrease = FALSE;
			nDiff = p->y2 - pNormalView->nGestureStartPosY;
		} else {
			pNormalView->bMomentumIncrease = TRUE;
			nDiff = pNormalView->nGestureStartPosY - p->y2;
		}

		pNormalView->nGestureOldPosY = p->y2;
		pNormalView->nMomentWeight = nDiff / pNormalView->nMomentStep;

		if (bVolumeMode) {
			VP_EVAS_JOB_DEL(pNormalView->pVolumeJob);
			pNormalView->pVolumeJob = ecore_job_add(__vp_normal_volume_job_cb, (void *)pNormalView);
		} else {
			VP_EVAS_JOB_DEL(pNormalView->pBrightnessJob);
			pNormalView->pBrightnessJob = ecore_job_add(__vp_normal_brightness_job_cb, (void *)pNormalView);
		}
#endif
	} else {
		int nDiffX = 0;
		int nDiffY = 0;
		int nWidth = 0;
		int nHeight = 0;
		double fX = 0.0;
		double fY = 0.0;

		nDiffX = p->x2 - pNormalView->nGestureStartPosX;
		nDiffY = p->y2 - pNormalView->nGestureStartPosY;

		vp_zoom_guide_get_move_position(pNormalView->pZoomGuide, &fX, &fY);
		Evas_Coord_Rectangle rtSrc = {0,};
		Evas_Coord_Rectangle rtDst = {0,};
		Evas_Coord_Rectangle rtResult = {0,};

		elm_win_screen_size_get(pNormalView->pPlayView->pWin, NULL, NULL, &nWidth, &nHeight);

		if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
			rtDst.w = nHeight;
			rtDst.h = nWidth;
		} else {
			rtDst.w = nWidth;
			rtDst.h = nHeight;
		}
		vp_mm_player_get_resolution(pNormalView->pPlayerHandle, &(rtSrc.w), &(rtSrc.h));

		vp_play_util_calculator_position(rtSrc, rtDst, &rtResult, pNormalView->nDisplayMode);

		fX -= (double)nDiffX / rtResult.w;
		fY -= (double)nDiffY / rtResult.h;

		pNormalView->nGestureStartPosX = p->x2;
		pNormalView->nGestureStartPosY = p->y2;

		vp_zoom_guide_set_move_position(pNormalView->pZoomGuide, fX, fY);
		//vp_zoom_guide_get_move_position(pNormalView->pZoomGuide, &fX, &fY);
		vp_zoom_guide_get_real_position(pNormalView->pZoomGuide, &fX, &fY);

		//VideoLogError("ZoomValue : [%f], offset_x : [%d], offset_y : [%d]", pNormalView->fZoomValue, (int)(fX * rtSrc.w), (int)(fY * rtSrc.h));
		vp_mm_player_set_zoom(pNormalView->pPlayerHandle, pNormalView->fZoomValue, (int)(fX * rtSrc.w), (int)(fY * rtSrc.h));
	}

	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags __vp_normal_gesture_momentum_end_cb(void *pUserData, void *pEventInfo)
{
	VideoLogInfo("");
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for lock mode");
		return EVAS_EVENT_FLAG_NONE;
	}

#ifdef SUBTITLE_K_FEATURE
	if (pNormalView->pSubtitleMoveTimer || pNormalView->bSubtitleDown) {
		VideoLogWarning("Longpress subtitle");
		return EVAS_EVENT_FLAG_NONE;
	}
#endif

	if (pNormalView->bZoomEnable == FALSE) {
#ifdef ENABLE_VOLUME_BRIGHTNESS_POPUP
		VP_EVAS_TIMER_DEL(pNormalView->pMomentTimer);
		pNormalView->pMomentTimer = NULL;

		VP_EVAS_JOB_DEL(pNormalView->pVolumeJob);
		VP_EVAS_JOB_DEL(pNormalView->pBrightnessJob);

		if (!vp_play_brightness_unrealize(pNormalView->pBrightnessHandle)) {
			VideoLogWarning("vp_play_brightness_unrealize is fail");
		}

		if (!vp_play_volume_popup_unrealize(pNormalView->pVolumePopupHandle)) {
			VideoLogWarning("vp_play_volume_popup_unrealize is fail");
		}

		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
#endif
	}

	return EVAS_EVENT_FLAG_ON_HOLD;
}

#ifdef ENABLE_GESTURE_ZOOM_FEATURE
static Evas_Event_Flags __vp_normal_gesture_zoom_start_cb(void *pUserData, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}


	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for lock mode");
		_vp_play_normal_view_show_layout(pNormalView);
		return EVAS_EVENT_FLAG_NONE;
	}

	Elm_Gesture_Zoom_Info *pZoomInfo = (Elm_Gesture_Zoom_Info *) pEventInfo;

	pNormalView->fActiveZoom = pZoomInfo->zoom;

	pNormalView->nZoomPosX = pZoomInfo->x;
	pNormalView->nZoomPosY = pZoomInfo->y;

	pNormalView->nPrevZoomPosX = 0;
	pNormalView->nPrevZoomPosY = 0;
	pNormalView->fPrevZoomValue = 0;

	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags __vp_normal_gesture_zoom_move_cb(void *pUserData, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	Elm_Gesture_Zoom_Info *pZoomInfo = (Elm_Gesture_Zoom_Info *) pEventInfo;

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for lock mode");
		return EVAS_EVENT_FLAG_NONE;
	}

	double fVal = pZoomInfo->zoom / pNormalView->fActiveZoom;

	pNormalView->fActiveZoom = pZoomInfo->zoom;

	pNormalView->fZoomValue = pNormalView->fZoomValue * fVal;

	if (pNormalView->fZoomValue < 1.0) {
		pNormalView->fZoomValue = 1.0;
	} else if (pNormalView->fZoomValue > 3.0) {
		pNormalView->fZoomValue = 3.0;
	}

	// event filtering
	bool bLandscape = FALSE;
	if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
		bLandscape = TRUE;
	}
	if (bLandscape && fabs(pNormalView->fPrevZoomValue - pNormalView->fZoomValue) < 0.05) {
//		VideoLogWarning("same ZoomValue. skip it");
		return EVAS_EVENT_FLAG_ON_HOLD;
	}

	_vp_play_normal_view_on_zoom_mode(pNormalView);

	double fX = 0.0;
	double fY = 0.0;
	int nSrcW = 0;
	int nSrcH = 0;

	vp_mm_player_get_resolution(pNormalView->pPlayerHandle, &nSrcW, &nSrcH);
	vp_zoom_guide_get_real_position(pNormalView->pZoomGuide, &fX, &fY);

	// event filtering
	if ((pNormalView->fZoomValue > 1.0 && pNormalView->fZoomValue < 3.0) &&
	        (pNormalView->nPrevZoomPosX == (int)(fX * nSrcW)) && (pNormalView->nPrevZoomPosY == (int)(fY * nSrcH))) {
//		VideoLogWarning("skip same position");
		return EVAS_EVENT_FLAG_ON_HOLD;
	}

	pNormalView->nPrevZoomPosX = (int)(fX * nSrcW);
	pNormalView->nPrevZoomPosY = (int)(fY * nSrcH);
	pNormalView->fPrevZoomValue = pNormalView->fZoomValue;

//	VideoLogError("ZoomValue : [%f], offset_x : [%d], offset_y : [%d]", pNormalView->fZoomValue, (int)(fX * nSrcW), (int)(fY * nSrcH));
	vp_mm_player_set_zoom(pNormalView->pPlayerHandle, pNormalView->fZoomValue, (int)(fX * nSrcW), (int)(fY * nSrcH));

	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags __vp_normal_gesture_zoom_end_cb(void *pUserData, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	Elm_Gesture_Zoom_Info *pZoomInfo = (Elm_Gesture_Zoom_Info *) pEventInfo;

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for lock mode");
		return EVAS_EVENT_FLAG_NONE;
	}

	double fVal = pZoomInfo->zoom / pNormalView->fActiveZoom;

	pNormalView->fActiveZoom = pZoomInfo->zoom;

	pNormalView->fZoomValue = pNormalView->fZoomValue * fVal;

	if (pNormalView->fZoomValue < 1.0) {
		pNormalView->fZoomValue = 1.0;
	} else if (pNormalView->fZoomValue > 3.0) {
		pNormalView->fZoomValue = 3.0;
	}

	_vp_play_normal_view_on_zoom_mode(pNormalView);

	pNormalView->nPrevZoomPosX = 0;
	pNormalView->nPrevZoomPosY = 0;
	pNormalView->fPrevZoomValue = 0;

	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags __vp_normal_gesture_zoom_abort_cb(void *pUserData, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	return EVAS_EVENT_FLAG_ON_HOLD;
}
#endif


static Evas_Event_Flags __vp_normal_gesture_double_click_start_cb(void *pUserData, void *pEventInfo)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags __vp_normal_gesture_double_click_end_cb(void *pUserData, void *pEventInfo)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	Elm_Gesture_Taps_Info *pEvent = (Elm_Gesture_Taps_Info *) pEventInfo;

	if (pNormalView->bZoomEnable && pEvent->n == 1) {
		pNormalView->fZoomValue = 1.0;
#ifdef ENABLE_GESTURE_ZOOM_FEATURE
		_vp_play_normal_view_on_zoom_mode(pNormalView);
#endif
	} else if (pEvent->n == 1) {

		if (pNormalView->bLockScreen) {
			_vp_play_normal_view_show_layout(pNormalView);
		} else {
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}
			if (nState == VP_MM_PLAYER_STATE_PLAYING) {
				vp_mm_player_pause(pNormalView->pPlayerHandle);
				pNormalView->bManualPause = TRUE;
			} else {
				bool bSpeedMode = FALSE;

				if (pNormalView->pSpeedTimer) {
					bSpeedMode = TRUE;
				}
				VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
				VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
				VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);

				elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

				if (bSpeedMode) {
					int nPosition = 0;
					if (vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
						VideoLogDebug("nPosition[%d], curPosition[%d]", nPosition, pNormalView->nCurPosition);

						if (vp_mm_player_set_position(pNormalView->pPlayerHandle, pNormalView->nCurPosition)) {
							pNormalView->bSeekComplete = FALSE;
							pNormalView->nWaitPos = -1;
						} else {
							pNormalView->nWaitPos = nPosition;
						}
					}
				}

				pNormalView->nSpeedValue = 1;
				pNormalView->bSpeedDown = TRUE;

				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogWarning("Resume Fail");
				}
				VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
				pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);

				pNormalView->bManualPause = FALSE;
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		}

	}

	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags __vp_normal_gesture_double_click_abort_cb(void *pUserData, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	return EVAS_EVENT_FLAG_ON_HOLD;
}

#ifdef FLICK_JUMP
static Evas_Event_Flags __vp_normal_gesture_flick_start_cb(void *pUserData, void *event_info)
{
	Elm_Gesture_Line_Info *pEvent = (Elm_Gesture_Line_Info *) event_info;

	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	VideoLogWarning("flick started");
	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bFlickWaiting == TRUE) {
		VideoLogWarning("seek is not complete. ignore signal");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (!pNormalView->pFlickJumpSeek) {
		pNormalView->nFlickPressX1 = pEvent->momentum.x1;
		pNormalView->nFlickPressY1 = pEvent->momentum.y1;
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Eina_Bool __vp_normal_flickjump_seek(void *pUserData, bool rewind)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	NormalView *pNormalView = (NormalView *)pUserData;

	int nSetPosition = 0;
	int nCurPosition = 0;

	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nCurPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return EINA_TRUE;
	}

	if (rewind) {
		if (nCurPosition - 5000 < 0) {
			nSetPosition = 0;
		} else {
			nSetPosition = nCurPosition - 5000;
		}
	} else {
		if (nCurPosition + 5000 > pNormalView->nDuration) {
			nSetPosition = pNormalView->nDuration;
		} else {
			nSetPosition = nCurPosition + 5000;
		}
	}

	pNormalView->bFlickWaiting = TRUE;

	if (vp_mm_player_set_position(pNormalView->pPlayerHandle, nSetPosition)) {
		pNormalView->nWaitPos = -1;
		pNormalView->bSeekComplete = FALSE;
		pNormalView->nCurPosition = nSetPosition;
		vp_play_progressbar_set_position(pNormalView->pProgressbar, nSetPosition);
	}

	return EINA_TRUE;
}

static void __vp_normal_flickjump_seek_del_cb(void *pUserData, Evas * e, Evas_Object *obj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, __vp_normal_flickjump_seek_del_cb);
	NormalView *pNormalView = (NormalView *)pUserData;
	pNormalView->pFlickJumpSeek = NULL;
}

static void __vp_normal_flickjump_layout_position(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	if (pNormalView->pFlickJumpSeek) {
		int nFlickSeekImageWidth = 80;
		int nFlickSeekImageHeight = 60;
		int nFlickSeekLeftMargin = 8;
		int nScreenWidth = 0;
		int nScreenHeight = 0;
		if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
			nScreenWidth = 1280;
			nScreenHeight = 720;
		} else {
			nScreenWidth = 720;
			nScreenHeight = 1280;
		}
		if (pNormalView->bFlickRewind == FALSE) {
			evas_object_move(pNormalView->pFlickJumpSeek, nScreenWidth - (nFlickSeekLeftMargin + nFlickSeekImageWidth / 2), (nScreenHeight - nFlickSeekImageHeight / 2) / 2);
		} else {
			evas_object_move(pNormalView->pFlickJumpSeek, nFlickSeekLeftMargin + nFlickSeekImageWidth / 2, (nScreenHeight - nFlickSeekImageHeight / 2) / 2);
		}
	}
}

static void __vp_normal_gesture_flick_animation_done_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	if (!data) {
		VideoLogError("data is NULL");
		return;
	}
	NormalView *pNormalView = (NormalView *)data;

	if (emission) {
		if (!g_strcmp0(emission, "rew_animation_done") || !g_strcmp0(emission, "ff_animation_done")) {
			if (pNormalView->bFlickWaiting == TRUE) {
				if (pNormalView->bFlickRewind == TRUE) {
					if (pNormalView->pFlickJumpSeek) {
						elm_object_signal_emit(pNormalView->pFlickJumpSeek, VP_NORMAL_SIGNAL_FLICKJUMP_FF, "*");
					}
				} else {
					if (pNormalView->pFlickJumpSeek) {
						elm_object_signal_emit(pNormalView->pFlickJumpSeek, VP_NORMAL_SIGNAL_FLICKJUMP_REW, "*");
					}
				}
			} else {
				VP_EVAS_DEL(pNormalView->pFlickJumpSeek);
			}
		}
	}
}

static Evas_Event_Flags __vp_normal_gesture_flick_end_cb(void *pUserData, void *event_info)
{
	Elm_Gesture_Line_Info *pEvent = (Elm_Gesture_Line_Info *) event_info;

	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EVAS_EVENT_FLAG_NONE;
	}

	VideoLogWarning("gesture flick end");
	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for lock mode");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (pNormalView->pFlickJumpSeek || pNormalView->nFlickPressX1 == 0 || pNormalView->nFlickPressY1 == 0) {
		VideoLogWarning("seek is not complete. ignore signal");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (pNormalView->fZoomValue > 1.0) {
		VideoLogWarning("Current zoom level : [%f]", pNormalView->fZoomValue);
		return EVAS_EVENT_FLAG_NONE;
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB &&
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW &&
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE &&
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL &&
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		VideoLogWarning("pNormalView->nLaunchingType : [%d]. not support gesture flick in this mode", pNormalView->nLaunchingType);
		return EVAS_EVENT_FLAG_NONE;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_NORMAL_MAIN_EDJ);
	free(path);
	pNormalView->pFlickJumpSeek = elm_layout_add(pNormalView->pMainLayout);
	elm_layout_file_set(pNormalView->pFlickJumpSeek, edj_path, VP_PLAY_EDJ_GROUP_FLICKJUMP_SEEK);
	evas_object_size_hint_weight_set(pNormalView->pFlickJumpSeek, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pNormalView->pFlickJumpSeek, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(pNormalView->pFlickJumpSeek, EVAS_CALLBACK_DEL, __vp_normal_flickjump_seek_del_cb, (void *)pNormalView);
	edje_object_signal_callback_add(_EDJ(pNormalView->pFlickJumpSeek), "*", "*", __vp_normal_gesture_flick_animation_done_cb, (void *)pNormalView);
	evas_object_hide(pNormalView->pFlickJumpSeek);

	if (pNormalView->nFlickPressX1 > pEvent->momentum.x2) {
		pNormalView->bFlickRewind = TRUE;
		elm_object_signal_emit(pNormalView->pFlickJumpSeek, VP_NORMAL_SIGNAL_FLICKJUMP_FF, "*");
		__vp_normal_flickjump_layout_position(pNormalView);
		__vp_normal_flickjump_seek(pNormalView, TRUE);
	} else if (pNormalView->nFlickPressX1 < pEvent->momentum.x2) {
		pNormalView->bFlickRewind = FALSE;
		elm_object_signal_emit(pNormalView->pFlickJumpSeek, VP_NORMAL_SIGNAL_FLICKJUMP_REW, "*");
		__vp_normal_flickjump_layout_position(pNormalView);
		__vp_normal_flickjump_seek(pNormalView, FALSE);
	}

	char *szTxt = NULL;
	char *szPdSECS = VP_PLAY_STRING_PD_SECS;
	szTxt = g_strdup_printf(szPdSECS, 5);

	if (pNormalView->bFlickRewind == FALSE) {
		elm_object_part_text_set(pNormalView->pFlickJumpSeek, VP_PLAY_PART_FLICKJUMP_SEEK_REW_TEXT, szTxt);
	} else {
		elm_object_part_text_set(pNormalView->pFlickJumpSeek, VP_PLAY_PART_FLICKJUMP_SEEK_FF_TEXT, szTxt);
	}
	VP_FREE(szTxt);

	evas_object_show(pNormalView->pFlickJumpSeek);

	pNormalView->nFlickPressX1 = 0;
	pNormalView->nFlickPressY1 = 0;


	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags __vp_normal_gesture_flick_abort_cb(void *pUserData, void *event_info)
{
	VideoLogWarning("flick abort");

	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->bFlickWaiting == FALSE) {
		pNormalView->nFlickPressX1 = 0;
		pNormalView->nFlickPressY1 = 0;
	}

	return EVAS_EVENT_FLAG_NONE;
}
#endif	// FLICK_JUMP

#ifdef K_FEATURE_MICROSEEK
static void __vp_normal_progress_microseek_textlayout_del_cb(void *pUserData, Evas * e, Evas_Object *obj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, __vp_normal_progress_microseek_textlayout_del_cb);
	NormalView *pNormalView = (NormalView *)pUserData;
	pNormalView->pMicroSeekTextLayout = NULL;
}

static void __vp_normal_progress_microseek_imagelayout_del_cb(void *pUserData, Evas * e, Evas_Object *obj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, __vp_normal_progress_microseek_imagelayout_del_cb);
	NormalView *pNormalView = (NormalView *)pUserData;
	pNormalView->pMicroSeekImageLayout = NULL;
}

#endif

static void __vp_normal_progress_mouse_down_cb(void *pUserData, Evas * e, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	NormalView *pNormalView = (NormalView *)pUserData;

	VideoLogWarning("Mouse Down");

	if (pNormalView->bIsRealize == FALSE) {
		VideoLogWarning("Skip for not realized");
		return;
	}

	if (pNormalView->bHLSMode) {
		VideoLogWarning("Skip for HLS");
		return;
	}

	Evas_Event_Mouse_Down *pMouseDownEvent = (Evas_Event_Mouse_Down *)pEvent;

	double dRatio = _vp_play_normal_view_get_mouse_pos_ratio(pNormalView->pProgressEvent, pMouseDownEvent->canvas.x);
	elm_object_signal_emit(pNormalView->pProgressEvent, VP_NORMAL_SIGNAL_PROGRESS_PRESS, "*");

	VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);
	pNormalView->pProgressTimer = NULL;

	int nCurPos = dRatio * pNormalView->nDuration;

#ifdef K_FEATURE_MICROSEEK
	pNormalView->nPrevCanvasX = (double)pMouseDownEvent->canvas.x;
	pNormalView->nPrevPositionX = (double)pNormalView->nPrevCanvasX;

	VP_EVAS_DEL(pNormalView->pMicroSeekTextLayout);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_NORMAL_MAIN_EDJ);
	free(path);
	pNormalView->pMicroSeekTextLayout = elm_layout_add(pNormalView->pMainLayout);
	elm_layout_file_set(pNormalView->pMicroSeekTextLayout, edj_path, VP_PLAY_EDJ_GROUP_MICROSEEK_TEXT);
	evas_object_size_hint_weight_set(pNormalView->pMicroSeekTextLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pNormalView->pMicroSeekTextLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(pNormalView->pMicroSeekTextLayout, EVAS_CALLBACK_DEL, __vp_normal_progress_microseek_textlayout_del_cb, (void *)pNormalView);
	evas_object_hide(pNormalView->pMicroSeekTextLayout);

	pNormalView->pMicroSeekImageLayout = elm_layout_add(pNormalView->pMainLayout);
	elm_layout_file_set(pNormalView->pMicroSeekImageLayout, edj_path, VP_PLAY_EDJ_GROUP_MICROSEEK_IMAGE);
	evas_object_size_hint_weight_set(pNormalView->pMicroSeekImageLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pNormalView->pMicroSeekImageLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(pNormalView->pMicroSeekImageLayout, EVAS_CALLBACK_DEL, __vp_normal_progress_microseek_imagelayout_del_cb, (void *)pNormalView);
	evas_object_hide(pNormalView->pMicroSeekImageLayout);

#endif

	vp_play_progressbar_set_position(pNormalView->pProgressbar, nCurPos);

	pNormalView->bProgressDown = TRUE;

	if (vp_play_util_check_streaming(pNormalView->szMediaURL)) {
		if (pNormalView->bManualPause == FALSE) {
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}
			if (nState == VP_MM_PLAYER_STATE_PLAYING) {
				vp_mm_player_pause(pNormalView->pPlayerHandle);
				_vp_play_normal_view_set_play_state(pNormalView);
				_vp_play_normal_view_on_capture_mode(pNormalView);
			}
		}

		return;
	}  else {
		if (pNormalView->bManualPause == FALSE) {
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}
			VideoLogInfo("nState:%d", nState);
			if (nState == VP_MM_PLAYER_STATE_PLAYING) {
				vp_mm_player_pause(pNormalView->pPlayerHandle);
			}
		}
	}
}

static void __vp_normal_progress_mouse_up_cb(void *pUserData, Evas * e, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	NormalView *pNormalView = (NormalView *)pUserData;
#ifdef K_FEATURE_MICROSEEK
#else
	Evas_Event_Mouse_Up *pMouseUpEvent = (Evas_Event_Mouse_Up *)pEvent;
#endif

	VideoLogWarning("Mouse Up");
	if (pNormalView->bIsRealize == FALSE || pNormalView->bProgressDown == FALSE) {
		VideoLogWarning("Skip for not realized or prgress down");
		return;
	}
	if (pNormalView->bHLSMode) {
		VideoLogWarning("Skip for HLS");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

#ifdef K_FEATURE_MICROSEEK
	double dRatio = _vp_play_normal_view_get_mouse_pos_ratio(pNormalView->pProgressEvent, pNormalView->nPrevPositionX);
#else
	double dRatio = _vp_play_normal_view_get_mouse_pos_ratio(pNormalView->pProgressEvent, pMouseUpEvent->canvas.x);
#endif
	elm_object_signal_emit(pNormalView->pProgressEvent, VP_NORMAL_SIGNAL_PROGRESS_UNPRESS, "*");
	if (fabs(dRatio - 1.0) < 1.0e-6) {
		VideoLogWarning("It is the end of the video!");
		__vp_normal_completed_cb(pNormalView);
		return;
	}

	int nCurPos = dRatio * pNormalView->nDuration;

#ifdef K_FEATURE_MICROSEEK
	VP_EVAS_DEL(pNormalView->pMicroSeekTextLayout);
	VP_EVAS_DEL(pNormalView->pMicroSeekImageLayout);

	pNormalView->nPrevCanvasX = 0.0;
	pNormalView->nPrevPositionX = 0.0;
#endif

	vp_play_progressbar_set_position(pNormalView->pProgressbar, nCurPos);

	pNormalView->bProgressDown = FALSE;

	if (vp_play_util_check_streaming(pNormalView->szMediaURL)) {
		if (pNormalView->bSeekComplete == FALSE) {
			pNormalView->nWaitPos = nCurPos;
			VideoLogWarning("== REAL SEEK SET == [%d]", pNormalView->nWaitPos);
		} else {
			VideoLogWarning("== REAL SEEK == [%d]", nCurPos);

			pNormalView->bBufferingComplete = TRUE;

			if (vp_mm_player_set_position(pNormalView->pPlayerHandle, nCurPos)) {
				pNormalView->nWaitPos = -1;
				pNormalView->bSeekComplete = FALSE;
				if (pNormalView->pLoadingAni) {
					VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
					VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
				}
			} else {
				_vp_play_normal_view_update_progress_value(pNormalView);
			}
		}

		return;
	}

	if (pNormalView->bSeekComplete == FALSE) {
		pNormalView->nWaitPos = nCurPos;
		VideoLogWarning("== REAL SEEK SET == [%d]", pNormalView->nWaitPos);
	} else {
		VideoLogWarning("== REAL SEEK == [%d]", nCurPos);
		if (vp_mm_player_set_position(pNormalView->pPlayerHandle, nCurPos)) {
			pNormalView->nWaitPos = -1;
			pNormalView->bSeekComplete = FALSE;
			if (pNormalView->pLoadingAni) {
				VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
				VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
			}
		} else {
			_vp_play_normal_view_update_progress_value(pNormalView);
		}
	}

}

static void __vp_normal_progress_del_cb(void *pUserData, Evas * e, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogError("pNormalView->pProgressEvent is free");
	NormalView *pNormalView = (NormalView *)pUserData;
	pNormalView->pProgressEvent = NULL;
}


static void __vp_normal_progress_mouse_move_cb(void *pUserData, Evas * e, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	Evas_Event_Mouse_Move *pMouseMoveEvent = (Evas_Event_Mouse_Move *)pEvent;
	if (pNormalView->bHLSMode) {
		VideoLogWarning("Skip for HLS");
		return;
	}

	if (pNormalView->bIsRealize == FALSE || pNormalView->bProgressDown == FALSE) {
		VideoLogWarning("Skip for not realized or prgress down");
		return;
	}

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for lock mode");
		__vp_normal_progress_mouse_up_cb(pUserData, NULL, NULL, pEvent);
		return;
	}

	if (pNormalView->bProgressDown == FALSE || pNormalView->bIsRealize == FALSE) {
		VideoLogWarning("Skip for not realized or prgress down");
		return;
	}

#ifdef K_FEATURE_MICROSEEK
	bool bLandscape = FALSE;
	int nScreenWidth = 0;
	int nScreenHeight = 0;
	int nControllerHeight = 0;
	int nSeek_Height = 0;
	if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
		bLandscape = TRUE;
		nScreenWidth = 1280;
		nScreenHeight = 720;
		nControllerHeight = 180;
		nSeek_Height = (VP_NORMAL_MICROSEEK_TEXT_HEIGHT + ((6 + 70 + 40) * elm_config_scale_get()));
	} else {
		nScreenWidth = 720;
		nScreenHeight = 1280;
		nControllerHeight = 293;
		nSeek_Height = (VP_NORMAL_MICROSEEK_TEXT_HEIGHT + ((6 + 70 + 20) * elm_config_scale_get()));
	}

//	VideoLogWarning("canvas.x[%d] canvas.y[%d]", pMouseMoveEvent->cur.canvas.x, pMouseMoveEvent->cur.canvas.y);

	double dDistance = 0;
	SeekLevel Level = SEEK_LEVEL_NORMAL;
	dDistance = (pMouseMoveEvent->cur.canvas.x - pNormalView->nPrevCanvasX);

	if (pMouseMoveEvent->cur.canvas.y > (nScreenHeight - nControllerHeight)) {
		Level = SEEK_LEVEL_NORMAL;
	} else if (pMouseMoveEvent->cur.canvas.y > (nScreenHeight - nControllerHeight - nSeek_Height)) {
		Level = SEEK_LEVEL_HALF;
		dDistance = dDistance / 2;
	} else {
		Level = SEEK_LEVEL_QUARTER;
		dDistance = dDistance / 4;
	}

	pNormalView->nPrevCanvasX = (double)pMouseMoveEvent->cur.canvas.x;

	// Calc position
	if (Level == SEEK_LEVEL_NORMAL) {
		pNormalView->nPrevPositionX = (double)pMouseMoveEvent->cur.canvas.x;
	} else {
		pNormalView->nPrevPositionX = pNormalView->nPrevPositionX + dDistance;
	}


	if (Level == SEEK_LEVEL_NORMAL) {
		if (pNormalView->pMicroSeekTextLayout) {
			elm_object_signal_emit(pNormalView->pMicroSeekTextLayout, VP_NORMAL_SIGNAL_MICROSEEK_TEXT_HIDE, "*");
		}
		if (pNormalView->pMicroSeekImageLayout) {
			elm_object_signal_emit(pNormalView->pMicroSeekImageLayout, VP_NORMAL_SIGNAL_MICROSEEK_IMAGE_HIDE, "*");
		}
	} else {
		Evas_Coord nTextWidth = -1, nTextHeight = -1;
		// Text  Layout move
		if (pNormalView->pMicroSeekTextLayout) {
			char *szTxt = NULL;
			char *szPsSpeed = VP_PLAY_STRING_SCRUBBING_AT_PSX_SPEED;

			if (Level == SEEK_LEVEL_HALF) {
				szTxt = g_strdup_printf(szPsSpeed, VP_NORMAL_SZ_HALF_SPEED);
			} else if (Level == SEEK_LEVEL_QUARTER) {
				szTxt = g_strdup_printf(szPsSpeed, VP_NORMAL_SZ_QUARTER_SPEED);
			}

			elm_object_part_text_set(pNormalView->pMicroSeekTextLayout, VP_NORMAL_SIGNAL_MICROSEEK_SPEED_TEXT, szTxt);
			edje_object_size_min_restricted_calc(_EDJ(pNormalView->pMicroSeekTextLayout), &nTextWidth, &nTextHeight, nTextWidth, nTextHeight);
			evas_object_resize(pNormalView->pMicroSeekTextLayout, nTextWidth, nTextHeight);

			int nTextPositionX = (int)pNormalView->nPrevPositionX - nTextWidth / 2;

			if (nTextPositionX < 0) {
				nTextPositionX = 0;
			} else if (nTextPositionX + nTextWidth > nScreenWidth) {
				nTextPositionX = nScreenWidth - nTextWidth;
			}

			if (Level == SEEK_LEVEL_HALF) {
				evas_object_move(pNormalView->pMicroSeekTextLayout, nTextPositionX, nScreenHeight - nControllerHeight - nSeek_Height);
			} else if (Level == SEEK_LEVEL_QUARTER) {
				evas_object_move(pNormalView->pMicroSeekTextLayout, nTextPositionX, nScreenHeight - nControllerHeight - nSeek_Height * 2);
			}
			elm_object_signal_emit(pNormalView->pMicroSeekTextLayout, VP_NORMAL_SIGNAL_MICROSEEK_TEXT_SHOW, "*");
			evas_object_show(pNormalView->pMicroSeekTextLayout);

			VP_FREE(szTxt);

			// Image  Layout move
			if (pNormalView->pMicroSeekImageLayout) {
				Evas_Coord nImageWidth = -1, nImageHeight = -1;
				edje_object_size_min_restricted_calc(_EDJ(pNormalView->pMicroSeekImageLayout), &nImageWidth, &nImageHeight, nImageWidth, nImageHeight);
				evas_object_resize(pNormalView->pMicroSeekImageLayout, nImageWidth, nImageHeight);

				int nImagePositionX = (int)pNormalView->nPrevPositionX - nImageWidth / 2;

				int nImageLeftPadding = 0;
				int nImageRightPadding = 0;
				if (bLandscape) {
					nImageLeftPadding = 28 + 112;
					nImageRightPadding = 28 + 162;
				} else {
					nImageLeftPadding = 6;
					nImageRightPadding = 26;
				}

				if (nImagePositionX < nImageLeftPadding) {
					nImagePositionX = nImageLeftPadding;
				} else if (nImagePositionX > nScreenWidth - nImageRightPadding) {
					nImagePositionX = nScreenWidth - nImageRightPadding;
				}

				if (Level == SEEK_LEVEL_HALF) {
					evas_object_move(pNormalView->pMicroSeekImageLayout, nImagePositionX, nScreenHeight - nControllerHeight - nSeek_Height + nTextHeight + 6);
				} else if (Level == SEEK_LEVEL_QUARTER) {
					evas_object_move(pNormalView->pMicroSeekImageLayout, nImagePositionX, nScreenHeight - nControllerHeight - nSeek_Height * 2 + nTextHeight + 6);
				}
				elm_object_signal_emit(pNormalView->pMicroSeekImageLayout, VP_NORMAL_SIGNAL_MICROSEEK_IMAGE_SHOW, "*");
				evas_object_show(pNormalView->pMicroSeekImageLayout);
			}
		}
	}

	double dRatio = _vp_play_normal_view_get_mouse_pos_ratio(pNormalView->pProgressEvent, (int)pNormalView->nPrevPositionX , bLandscape);

#else

	double dRatio = _vp_play_normal_view_get_mouse_pos_ratio(pNormalView->pProgressEvent, pMouseMoveEvent->cur.canvas.x);
#endif
	int nCurPos = dRatio * pNormalView->nDuration;

	vp_play_progressbar_set_position(pNormalView->pProgressbar, nCurPos);

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		return;
	}

	if (pNormalView->bSeekComplete == FALSE) {
		pNormalView->nWaitPos = nCurPos;
	} else {
		VideoLogWarning("== I FRMAE SEEK == [%d]", nCurPos);
		if (vp_mm_player_set_position_by_key_frame(pNormalView->pPlayerHandle, nCurPos)) {
			pNormalView->nWaitPos = -1;
			pNormalView->bSeekComplete = FALSE;
		}
	}
}

static void __vp_normal_focus_key_down_cb(void *pUserData, Evas * e, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	Evas_Event_Key_Down *ev = pEvent;

	if (!ev) {
		return;
	}
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		return;
	}

	VideoLogInfo("KEY: %s", ev->keyname);

	if (!strcmp(ev->keyname, "Left")) {
		if (pNormalView->bKeySpeed == FALSE) {
			vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, FALSE, FALSE);
			pNormalView->bKeySpeed = TRUE;
		}
	} else if (!strcmp(ev->keyname, "Right")) {
		if (pNormalView->bKeySpeed == FALSE) {
			vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, FALSE, TRUE);
			pNormalView->bKeySpeed = TRUE;
		}
	}
}

static void __vp_normal_focus_key_up_cb(void *pUserData, Evas * e, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	Evas_Event_Key_Down *ev = pEvent;

	if (!ev) {
		return;
	}
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		return;
	}

	VideoLogInfo("KEY: %s", ev->keyname);

	if (!strcmp(ev->keyname, "Left")) {
		if (pNormalView->bKeySpeed == TRUE) {
			vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, TRUE, FALSE);
			pNormalView->bKeySpeed = FALSE;
		}
	} else if (!strcmp(ev->keyname, "Right")) {
		if (pNormalView->bKeySpeed == TRUE) {
			vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(pPlayView->pNormalView, TRUE, TRUE);
			pNormalView->bKeySpeed = FALSE;
		}
	}
}
/*
static void __vp_normal_bookmark_item_select_cb(int nPos, const char *szPath, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (nPos == -1 && szPath == NULL) {
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		return;
	}

	if (pNormalView->bSeekComplete == FALSE) {
		pNormalView->nWaitPos = nPos;
	} else {
		if (vp_mm_player_set_position(pNormalView->pPlayerHandle, nPos)) {
			pNormalView->nWaitPos = -1;
			pNormalView->bSeekComplete = FALSE;

			if (pNormalView->pLoadingAni) {
				VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
				VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
			}
			vp_play_progressbar_set_position(pNormalView->pProgressbar, nPos);
		}
	}

	_vp_play_normal_view_show_layout(pNormalView);
}
*/

bool vp_normal_rotate_icon_timer_cb(void *pUserData)
{
	VideoLogError("vp_normal_rotate_icon_timer_cb");
	_vp_play_normal_view_hide_layout(curr_Handle, TRUE);
	return TRUE;
}

static Eina_Bool __vp_normal_hide_layout_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	NormalView *pNormalView = (NormalView *)pUserData;

	pNormalView->pHideTimer = NULL;
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if ((nState != VP_MM_PLAYER_STATE_PLAYING) && (nState != VP_MM_PLAYER_STATE_PAUSED)) {
		return EINA_FALSE;
	}

	_vp_play_normal_view_hide_layout(pNormalView, FALSE);

	return EINA_FALSE;
}

static Eina_Bool __vp_normal_hide_layout_finish_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");
	NormalView *pNormalView = (NormalView *)pUserData;
	PlayView *pPlayView = pNormalView->pPlayView;

	if (pNormalView->pSubtitleTimer) {
		VideoLogError("Skip for Subtitle");
		return EINA_TRUE;
	}

	if (pNormalView->pVolumeJob) {
		VideoLogError("Skip for pVolumeJob");
		return EINA_TRUE;
	}

	if (pNormalView->pBrightnessJob) {
		VideoLogError("Skip for pBrightnessJob");
		return EINA_TRUE;
	}

	pNormalView->pHideFinishTimer = NULL;

	if (pNormalView->pImageBufferObj == NULL &&
	        pNormalView->bIsPopupShow == FALSE &&
	        pNormalView->bIsSubtitleShow == FALSE &&
	        pPlayView->bMouseOut == FALSE &&
	        pNormalView->bVideoOnlyMode == FALSE &&
	        pNormalView->bCaptureMode == FALSE) {
		vp_play_util_fb_visible_off(pPlayView->pWin, FALSE);
	}
	vp_device_set_frame_rate(40);

	return EINA_FALSE;
}

static Eina_Bool __vp_normal_subtitle_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	int nPosition = 0;
	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
	}

	if (nPosition > pNormalView->dSubtitleStartTime &&
	        nPosition < pNormalView->dSubtitleEndTime) {
		return EINA_TRUE;
	}

	pNormalView->pSubtitleTimer = NULL;

	vp_play_subtitle_set_text(pNormalView->pSubtitle, NULL);

	return EINA_FALSE;
}
/*
static Eina_Bool __vp_normal_screenshot_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pScreenShot);
	pNormalView->pScreenShot = NULL;

	elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_PORTRAIT);
	elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_LANDSCAPE);

	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SCREEN_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_ROTATE_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_VOLUME_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_HIDE, "*");

	pNormalView->pScreenShotTimer = NULL;

	return EINA_FALSE;
}
*/

static Eina_Bool __vp_normal_dlna_speed_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pPopup);
	pNormalView->pPopup = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");

		pNormalView->pSpeedTimer = NULL;
		return EINA_FALSE;
	}
	pNormalView->bIsPopupShow = TRUE;

	pNormalView->pPopup = vp_popup_create(pPlayView->pWin, POPUP_STYLE_DEFAULT_WITH_CANCEL_BTN,
	                                      NULL,
	                                      VP_PLAY_STRING_UNABLE_TO_FF_REW,
	                                      0.0, NULL,
	                                      __vp_normal_popup_key_event_cb,
	                                      __vp_normal_popup_mouse_event_cb,
	                                      pNormalView);

	Evas_Object *pBtn = NULL;
	pBtn = elm_button_add(pNormalView->pPopup);
	elm_object_style_set(pBtn, "popup_button/default");
	elm_object_domain_translatable_text_set(pBtn, VP_SYS_STR_PREFIX, VP_PLAY_STRING_COM_OK_IDS);
	elm_object_part_content_set(pNormalView->pPopup, "button1", pBtn);
	evas_object_smart_callback_add(pBtn, "clicked", __vp_normal_popup_key_event_cb, (void*)pNormalView);
	evas_object_show(pNormalView->pPopup);

	pNormalView->pSpeedTimer = NULL;

	return EINA_FALSE;
}


static Eina_Bool __vp_normal_speed_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}
	VideoLogWarning("");

	NormalView *pNormalView = (NormalView *)pUserData;


	if (pNormalView->bSpeedDown) {
		pNormalView->nSpeedValue++;
	}

	//SPEED: *2, *8, *32
	//int nSeedVal = pNormalView->nSpeedValue - (pNormalView->nSpeedValue%2);

	//SPEED: *2, *4, *8, *16, *32
	int nSeedVal = pNormalView->nSpeedValue;

	int nStep = (int) pow(2.0, (double)(nSeedVal - 1));

	VideoLogWarning("pNormalView->nSpeedValue : %d", pNormalView->nSpeedValue);
	if (pNormalView->nSpeedValue >= 2) {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_SHOW, "*");
	} else {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
		if (pNormalView->bManualPause == FALSE) {
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				_vp_play_normal_view_set_play_state(pNormalView);
				_vp_play_normal_view_on_capture_mode(pNormalView);
			}
			pNormalView->pSpeedTimer = NULL;
		}

		return EINA_FALSE;
	}

	if (nStep > VP_NORMAL_SPEED_VALUE_MAX) {
		nStep = VP_NORMAL_SPEED_VALUE_MAX;
	}

	char szTxt[5] = {0,};
	snprintf(szTxt, 5, "%d X", nStep);
	elm_object_part_text_set(pNormalView->pMainLayout, VP_PLAY_PART_NORMAL_SPEED_TXT, szTxt);

	if (pNormalView->bSeekComplete == FALSE) {
		VideoLogError("bSeekComplete is fail");
		return EINA_TRUE;
	}

	if (pNormalView->nSpeedValue > 1) {
		// during FAST FF/REW moving(player state is paused). don't allow sleep mode
		vp_play_util_set_lock_power_key();
	}

	int nSetPosition = 0;
	int nCurPosition = 0;

	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nCurPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return EINA_TRUE;
	}

	if (pNormalView->nDuration <= nCurPosition) {
		VideoLogWarning("nCurPosition : %d, Duration : %d", nCurPosition, pNormalView->nDuration);
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
		_vp_play_normal_view_on_next_play(pNormalView, FALSE);
		pNormalView->pSpeedTimer = NULL;
		return EINA_FALSE;
	}

	if (nCurPosition <= 0 && pNormalView->bSpeedFF == FALSE) {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
		if (pNormalView->bManualPause == FALSE && pNormalView->bSpeedDown == FALSE) {
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				_vp_play_normal_view_set_play_state(pNormalView);
				_vp_play_normal_view_on_capture_mode(pNormalView);
			}
			pNormalView->pSpeedTimer = NULL;
			return EINA_FALSE;
		}

		return EINA_TRUE;
	}

	if (pNormalView->bSpeedFF) {
		nSetPosition = nCurPosition  + (nStep * 1000);
	} else {
		nSetPosition = nCurPosition  - (nStep * 1000);
	}

	if (pNormalView->nDuration < nSetPosition) {
		nSetPosition = pNormalView->nDuration;
		VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
	}

	if (nSetPosition < 0) {
		nSetPosition = 0;
//		VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
	}
	if (pNormalView->pProgressTimer) {
		VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);
	}

	if (vp_mm_player_set_position_by_key_frame(pNormalView->pPlayerHandle, nSetPosition)) {
		pNormalView->bSeekComplete = FALSE;
		pNormalView->nCurPosition = nSetPosition;
		vp_play_progressbar_set_position(pNormalView->pProgressbar, nSetPosition);
	}

	return EINA_TRUE;
}



static void  __vp_normal_show_finish_cb(void *pUserData, Evas_Object *pObj, const char *emission, const char *source)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("");

	//NormalView *pNormalView = (NormalView *)pUserData;
}

static void  __vp_normal_hide_finish_cb(void *pUserData, Evas_Object *pObj, const char *emission, const char *source)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("");

	NormalView *pNormalView = (NormalView *)pUserData;
	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	if (!pPlayView->pWin) {
		VideoLogError("pPlayView->pWin is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

	pNormalView->pHideFinishTimer = ecore_timer_add(VP_NORMAL_HIDE_LAYOUT_FINISH_TIMER_INTERVAL,
	                                __vp_normal_hide_layout_finish_timer_cb, (void *)pNormalView);

#ifndef SUBTITLE_K_FEATURE
	if (pNormalView->nDisplayMode != VP_MM_PLAYER_DISPLAY_FULL_SCREEN) {
		if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_NONE || pNormalView->nRotate == VIDEO_PLAY_ROTATE_180) {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_PORTRAIT, "*");
		} else {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_HIDE, "*");
		}
	} else {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_HIDE, "*");
	}
#endif
}



static Eina_Bool __vp_normal_progressbar_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (!_vp_play_normal_view_update_progress_value(pNormalView)) {
		pNormalView->pProgressTimer = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}


static Eina_Bool __vp_normal_lockscreen_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_LOCKSCREEN_HIDE, "*");

	pNormalView->pLockScreenTimer = NULL;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

	return EINA_FALSE;
}

static void __vp_normal_prepare_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("");

	NormalView *pNormalView = (NormalView *)pUserData;
	if (pNormalView->pPreparePipe != NULL) {
		ecore_pipe_write(pNormalView->pPreparePipe, pNormalView, sizeof(NormalView));
	}
}

static void __vp_normal_completed_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("");

	NormalView	*pNormalView = (NormalView *)pUserData;
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	if(pPlayView->szAutoPlay != NULL) {
	if(!strcmp(pPlayView->szAutoPlay,"TRUE")) {
		VideoLogError("===========================vp_is_auto_play_on is True");
		_vp_play_normal_view_on_next_play(pNormalView, TRUE);
	}
	else
	{
		VideoLogError("===========================vp_is_auto_play_on is False");
		_vp_play_normal_view_on_next_play(pNormalView, FALSE);
	}
	}
	else
	{
		VideoLogError("===========================vp_is_auto_play_on is False");
				_vp_play_normal_view_on_next_play(pNormalView, FALSE);
	}

#ifdef FLICK_JUMP
	pNormalView->bFlickWaiting = FALSE;
	VP_EVAS_DEL(pNormalView->pFlickJumpSeek);
#endif

	if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_PREVIEW &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MESSAGE &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_EMAIL &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {
		vp_media_contents_set_played_position(pNormalView->szMediaURL, 0);
	} else if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		vp_multi_path_set_item_position(pNormalView->szMediaURL, 0, pPlayView->pMultiPathList);
	}

	vp_mm_player_unrealize(pNormalView->pPlayerHandle);

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW) {
		if (!pPlayView->pFunc) {
			VideoLogError("pPlayView pFunc is NULL");
			return;
		}

		if (!pPlayView->pFunc->vp_play_func_exit) {
			VideoLogError("pPlayView exit Func is NULL");
			return;
		}
		vp_device_set_screen_off(FALSE);
		vp_play_util_set_unlock_power_key();

		VideoLogWarning("== Auto Off ==");

		pPlayView->pFunc->vp_play_func_exit(pPlayView);

		return;
	}

}


static void __vp_play_normal_view_seek_pipe_cb(void *pUserData, void *pBuf, int nByte)
{
	if (!pUserData) {
		VideoLogError("NormalView handle is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (pNormalView->nWaitPos >= 0) {
		if (pNormalView->bProgressDown == TRUE) {
			VideoLogWarning("== I-FRAME SEEK ==[%d]", pNormalView->nWaitPos);
			if (vp_mm_player_set_position_by_key_frame(pNormalView->pPlayerHandle, pNormalView->nWaitPos)) {
				pNormalView->bSeekComplete = FALSE;
				pNormalView->nWaitPos = -1;
				return;
			}
		} else {
			VideoLogWarning("== REAL SEEK ==[%d]", pNormalView->nWaitPos);
			if (vp_mm_player_set_position(pNormalView->pPlayerHandle, pNormalView->nWaitPos)) {
				pNormalView->bSeekComplete = FALSE;
				pNormalView->nWaitPos = -1;
				return;
			}
		}
		pNormalView->nWaitPos = -1;
	}
}

static void __vp_normal_seek_completed_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	NormalView *pNormalView = (NormalView *)pUserData;
	VideoLogWarning("%d", pNormalView->nWaitPos);

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView handle is NULL");
		return;
	}

	if (!pPlayView->pWin) {
		VideoLogError("pPlayView->pWin handle is NULL");
		return;
	}

	pNormalView->bSeekComplete = TRUE;

	if (vp_play_util_check_streaming(pNormalView->szMediaURL)) {
		if (pNormalView->bBufferingComplete == FALSE) {
			VideoLogError("Skip for buffering state : buffering complete(false) so, set StartPosition 0");
			if (pNormalView->nStartPosition > 0) {
				pNormalView->nStartPosition = 0;
				pNormalView->bIsRealize = TRUE;
			}
			return;
		}
	}
#ifdef FLICK_JUMP
	pNormalView->bFlickWaiting = FALSE;
#endif

	if (pNormalView->pLoadingAni) {
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		vp_play_loading_ani_destroy(pNormalView->pLoadingAni);
		pNormalView->pLoadingAni = NULL;
	}

	if (pPlayView->bAVRCP) {
		int nPosition = 0;
		if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
			VideoLogError("vp_mm_player_get_position is fail");
		} else {
			vp_avrcp_noti_track_position(nPosition);
		}
	}

	if (pNormalView->nStartPosition > 0) {
		bool bLCDOffStatus = FALSE;

		vp_play_config_get_lcd_off_state(&bLCDOffStatus);
		if (bLCDOffStatus && (pNormalView->bVideoOnlyMode == FALSE)) {
			VideoLogError("Invalid LCD Status");
			//elm_exit();

			pPlayView->pFunc->vp_play_func_exit(pPlayView);

			return;
		}

		if (pNormalView->bManualPause || pNormalView->bSharepopup) {
			vp_mm_player_pause(pNormalView->pPlayerHandle);
		} else {
			vp_mm_player_play(pNormalView->pPlayerHandle);
		}

		if (!vp_mm_player_set_subtitle_position(pNormalView->pPlayerHandle, pNormalView->fSubtitleSyncValue * 1000)) {
			VideoLogError("vp_mm_player_set_subtitle_position is fail");
		}

		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		pNormalView->nStartPosition = 0;
		pNormalView->bIsRealize = TRUE;

		return;
	}

	if (pNormalView->bProgressDown == TRUE) {
		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
		if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}
		if (nState == VP_MM_PLAYER_STATE_PLAYING) {
			vp_mm_player_pause(pNormalView->pPlayerHandle);
			_vp_play_normal_view_set_play_state(pNormalView);
		}
	}

	if (pNormalView->bIsRealize == FALSE) {
		pNormalView->nStartPosition = 0;
		pNormalView->bIsRealize = TRUE;
		vp_mm_player_play(pNormalView->pPlayerHandle);
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		//_vp_play_normal_view_show_layout(pNormalView);
		return;
	}

	if (pNormalView->nWaitPos >= 0) {
		if (pNormalView->pSeekPipe) {
			ecore_pipe_write(pNormalView->pSeekPipe, pNormalView, sizeof(NormalView));
		}
	}

	if (pNormalView->bManualPause == FALSE &&
	        pNormalView->bProgressDown == FALSE &&
	        !pNormalView->pSpeedTimer) {
		vp_mm_player_play(pNormalView->pPlayerHandle);
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}

}
static void __vp_normal_interrupted_cb(vp_mm_player_interrupt_t nCode, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VideoLogWarning("== %d ==", nCode);

	if (nCode == VP_MM_PLAYER_INTERRUPTED_COMPLETED) {
		if (pNormalView->bManualPause == FALSE) {
			vp_mm_player_play(pNormalView->pPlayerHandle);
		}
	} else if (nCode == VP_MM_PLAYER_INTERRUPTED_BY_MEDIA) {
		pNormalView->bManualPause = TRUE;
	} else if (nCode == VP_MM_PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG) {
		if (pNormalView->bDeviceRemoveInterrupt) {
			if (pNormalView->bManualPause == FALSE) {
				vp_mm_player_play(pNormalView->pPlayerHandle);
			}
			pNormalView->bDeviceRemoveInterrupt = FALSE;
		}
	} else if (nCode == VP_MM_PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT) {

		PlayView *pPlayView = pNormalView->pPlayView;
		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			return;
		}

		if (!pPlayView->pFunc) {
			VideoLogError("pPlayView->pFunc is NULL");
			return;

		}
		elm_exit();
		return;
	}

	_vp_play_normal_view_set_play_state(pNormalView);
	_vp_play_normal_view_on_capture_mode(pNormalView);
}

static void __vp_normal_error_cb(vp_mm_player_error_t nError, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	VideoLogWarning("Error triggered: 0x%x", nError);

	NormalView *pNormalView = (NormalView *)pUserData;
	PlayView *pPlayView = pNormalView->pPlayView;

	VP_EVAS_DEL(pNormalView->pPopup);
	pNormalView->pPopup = NULL;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	Evas_Object *pParent = pPlayView->pWin;
	switch (nError) {
	case VP_MM_PLAYER_ERR_OUT_OF_MEMORY:
	case VP_MM_PLAYER_ERR_INVALID_OPERATION:
	case VP_MM_PLAYER_ERR_FILE_NO_SPACE_ON_DEVICE:
	case VP_MM_PLAYER_ERR_INVALID_PARAMETER:
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_ERROR_UNABLE_PLAY,
		                                      3.0, __vp_normal_error_popup_time_out_cb,
		                                      __vp_normal_error_popup_key_event_cb,
		                                      __vp_normal_error_popup_mouse_event_cb,
		                                      pNormalView);
		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;

		break;
	case VP_MM_PLAYER_ERR_NO_SUCH_FILE:
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_ERROR_FILE_NOT_EXIST,
		                                      3.0, __vp_normal_error_popup_time_out_cb,
		                                      __vp_normal_error_popup_key_event_cb,
		                                      __vp_normal_error_popup_mouse_event_cb,
		                                      pNormalView);
		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;

		break;


	case VP_MM_PLAYER_ERR_SEEK_FAILED:
		VideoLogWarning("VP_MM_PLAYER_ERR_SEEK_FAILED");
		break;
	case VP_MM_PLAYER_ERR_INVALID_STATE:
		VideoLogWarning("VP_MM_PLAYER_ERR_INVALID_STATE");
		break;
	case VP_MM_PLAYER_ERR_SOUND_POLICY:
		VideoLogWarning("VP_MM_PLAYER_ERR_SOUND_POLICY");
		break;
	case VP_MM_PLAYER_ERR_VIDEO_CAPTUR_FAILED:
		VideoLogWarning("VP_MM_PLAYER_ERR_VIDEO_CAPTUR_FAILED");
		break;
	case VP_MM_PLAYER_ERR_NOT_SUPPORTED_FILE:
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_ERROR_UNSUPPORTED_FORMAT,
		                                      3.0, __vp_normal_error_popup_time_out_cb,
		                                      __vp_normal_error_popup_key_event_cb,
		                                      __vp_normal_error_popup_mouse_event_cb,
		                                      pNormalView);
		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
		break;
	case VP_MM_PLAYER_ERR_INVALID_URI:
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_ERROR_INVALID_URL,
		                                      3.0, __vp_normal_error_popup_time_out_cb,
		                                      __vp_normal_error_popup_key_event_cb,
		                                      __vp_normal_error_popup_mouse_event_cb,
		                                      pNormalView);
		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
		break;

	case VP_MM_PLAYER_ERR_CONNECTION_FAILED:
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_ERROR_CONNECTION_FAILED,
		                                      3.0, __vp_normal_error_popup_time_out_cb,
		                                      __vp_normal_error_popup_key_event_cb,
		                                      __vp_normal_error_popup_mouse_event_cb,
		                                      pNormalView);
		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
		break;
	case VP_MM_PLAYER_ERR_DRM_EXPIRED:
	case VP_MM_PLAYER_ERR_DRM_FUTURE_USE:
		VideoLogWarning("DRM BUG");
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_ERROR_UNABLE_PLAY,
		                                      3.0, __vp_normal_error_popup_time_out_cb,
		                                      __vp_normal_error_popup_key_event_cb,
		                                      __vp_normal_error_popup_mouse_event_cb,
		                                      pNormalView);
		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
		break;
	case VP_MM_PLAYER_ERR_DRM_NO_LICENSE:
		pNormalView->pPopup = vp_popup_create(pPlayView->pWin, POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_DIVX_DRM_AUTHORIZATION_ERROR,
		                                      3.0, __vp_normal_error_popup_time_out_cb,
		                                      __vp_normal_error_popup_key_event_cb,
		                                      __vp_normal_error_popup_mouse_event_cb,
		                                      pNormalView);
		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
		break;
	default:
		VideoLogWarning("UNKNOWN TYPE : %d", nError);
		break;
	}

}

static void _vp_normal_view_loading_ani_layout_del_cb(void *data, Evas *e,
        Evas_Object *obj,
        void *event_info)
{
	if (!data) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)data;
	pNormalView->pLoadingAni = NULL;
}

static bool _vp_play_normal_view_create_loading_ani(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;
	pParent = pNormalView->pMainLayout;

	if (pNormalView->pLoadingAni) {
		vp_play_loading_ani_destroy(pNormalView->pLoadingAni);
		pNormalView->pLoadingAni = NULL;
	}

	pNormalView->pLoadingAni = vp_play_loading_ani_create(pParent, VIDEO_LOADING_SIZE_LARGE);
	if (!pNormalView->pLoadingAni) {
		VideoLogError("pLoadingAni create fail");
		return FALSE;
	}
	evas_object_event_callback_add(pNormalView->pLoadingAni, EVAS_CALLBACK_DEL, _vp_normal_view_loading_ani_layout_del_cb, (void *)pNormalView);

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_LOADING_ANI, pNormalView->pLoadingAni);

	return TRUE;
}

static void __vp_normal_buffering_cb(int nPercent, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	if (nPercent == 100) {
		VideoLogError("Buffering complete");
		pNormalView->bBufferingComplete = TRUE;
		if (pNormalView->pLoadingAni) {
			vp_play_loading_ani_destroy(pNormalView->pLoadingAni);
			pNormalView->pLoadingAni = NULL;
		}

		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

		if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}

		if (nState != VP_MM_PLAYER_STATE_PLAYING) {
			PlayView *pPlayView = pNormalView->pPlayView;
			if (!pPlayView) {
				VideoLogError("pPlayView handle is NULL");
				return;
			}

			if (pNormalView->nSpeedValue > 1) {
				VideoLogWarning("pNormalView->nSpeedValue : [%d]", pNormalView->nSpeedValue);
			} else {
				if (pNormalView->bManualPause || pPlayView->bActivate == FALSE) {
					vp_mm_player_set_visible(pNormalView->pPlayerHandle, TRUE);
					vp_mm_player_pause(pNormalView->pPlayerHandle);
				} else {
					vp_mm_player_play(pNormalView->pPlayerHandle);
				}
				_vp_play_normal_view_set_play_state(pNormalView);
				_vp_play_normal_view_on_capture_mode(pNormalView);
			}
		}
	} else {
		pNormalView->bBufferingComplete = FALSE;
		if (pNormalView->bManualPause == FALSE) {
			vp_play_util_set_lock_power_key();
		}
		// Remove loading animation during speed playback.
		if (pNormalView->pSpeedTimer) {
			//
		} else if (!pNormalView->pLoadingAni) {
			if (!_vp_play_normal_view_create_loading_ani(pNormalView)) {
				VideoLogError("_vp_play_normal_view_create_loading_ani fail");
				return;
			}
		}
	}
}

static void __vp_normal_subtitle_updated_cb(unsigned long nDuration, char *text, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("%s", text);

	NormalView *pNormalView = (NormalView *)pUserData;

	int nPosition = 0;
	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
	}
	int tDuration = 0;
	vp_mm_player_get_duration(pNormalView->pPlayerHandle, &tDuration);
	if (nPosition >= tDuration) {
		__vp_normal_completed_cb(pNormalView);
	}

	bool bIsWhiteSpace = vp_play_util_check_white_space(text);

	VP_EVAS_TIMER_DEL(pNormalView->pSubtitleTimer);

	if (pNormalView->bVideoOnlyMode) {
		return;
	}

	if (bIsWhiteSpace) {
		vp_play_subtitle_set_text(pNormalView->pSubtitle, "");
		elm_object_signal_emit(vp_play_subtitle_get_object(pNormalView->pSubtitle), VP_SUBTITLE_SIGNAL_CAPTION_WINDOW_HIDE, "*");
		elm_object_signal_emit(vp_play_subtitle_get_object(pNormalView->pSubtitle), VP_SUBTITLE_SIGNAL_GREP_BOX_HIDE, "*");
		return;
	}

	double dTimeInterval = (double)nDuration / (double)1000;

	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
	}

	pNormalView->dSubtitleStartTime = nPosition;
	pNormalView->dSubtitleEndTime = nPosition + nDuration;

	pNormalView->pSubtitleTimer = ecore_timer_add(dTimeInterval,
	                              __vp_normal_subtitle_timer_cb, (void *)pNormalView);

	vp_play_subtitle_set_text(pNormalView->pSubtitle, text);
#ifdef SUBTITLE_K_FEATURE
	/*Set caption window: size, rel1, color*/
	__vp_normal_subtitle_set_caption_window(pNormalView);
#endif
}


static void __vp_normal_pd_message_cb(vp_mm_player_pd_message_t nType, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("");
}

static void __vp_normal_missed_plugin_cb(vp_mm_player_missed_plugin_t nType, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("nType=%d", nType);

	NormalView *pNormalView = (NormalView *)pUserData;
	PlayView *pPlayView = pNormalView->pPlayView;

	VP_EVAS_DEL(pNormalView->pPopup);
	pNormalView->pPopup = NULL;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
#if 1
	if (nType == VP_MM_PLAYER_MISSED_PLUGIN_AUDIO) {
		_vp_play_normal_view_show_video_only_popup(pNormalView);
	} else if (nType == VP_MM_PLAYER_MISSED_PLUGIN_VIDEO) {
		_vp_play_normal_view_show_audio_only_popup(pNormalView);
	}
#else /* Disabled. please use ecore_pipe if need to use popup. */
	Evas_Object *pParent = pPlayView->pWin;
	if (nType == VP_MM_PLAYER_MISSED_PLUGIN_AUDIO) {
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT,
		                                      NULL,
		                                      VP_PLAY_STRING_ERROR_UNSUPPORTED_AUDIO,
		                                      3.0, __vp_normal_popup_time_out_cb,
		                                      __vp_normal_popup_key_event_cb,
		                                      __vp_normal_popup_mouse_event_cb,
		                                      pNormalView);

		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
	} else if (nType == VP_MM_PLAYER_MISSED_PLUGIN_VIDEO) {
		pNormalView->pPopup = vp_popup_create(pParent,
		                                      POPUP_STYLE_DEFAULT,
		                                      NULL,
		                                      VP_PLAY_STRING_ERROR_UNSUPPORTED_VIDEO,
		                                      3.0, __vp_normal_popup_time_out_cb,
		                                      __vp_normal_popup_key_event_cb,
		                                      __vp_normal_popup_mouse_event_cb,
		                                      pNormalView);

		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
		pNormalView->bIsVideoUnsupport = TRUE;

		VP_EVAS_DISABLE_SET(pNormalView->pBookmarkBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pNormalView->pMultiWinBtn, EINA_TRUE);
	}
#endif
}

static void __vp_normal_image_buffer_cb(char *pBuffer, int nSize, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	VideoLogWarning("%p, %d", pBuffer, nSize);

	NormalView *pNormalView = (NormalView *)pUserData;
	if (pNormalView->pImageBufferObj) {
		elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_IMAGE_BUFFER);
		VP_EVAS_DEL(pNormalView->pImageBufferObj);
	}
	vp_play_util_save_file(VP_PLAY_IMAGE_BUFFER_PATH, pBuffer, nSize);

	pNormalView->pImageBufferObj = elm_image_add(pNormalView->pMainLayout);
	elm_image_file_set(pNormalView->pImageBufferObj, VP_PLAY_IMAGE_BUFFER_PATH, NULL);

	evas_object_size_hint_weight_set(pNormalView->pImageBufferObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pNormalView->pImageBufferObj, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_IMAGE_BUFFER, pNormalView->pImageBufferObj);
	evas_object_show(pNormalView->pImageBufferObj);
}
#if 1
static void _vp_normal_set_audio_only_image(NormalView *pNormalView, bool bAudioOnly)
{
	if (pNormalView->pImageBufferObj) {
		elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_IMAGE_BUFFER);
		VP_EVAS_DEL(pNormalView->pImageBufferObj);
	}

	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	if (bAudioOnly == TRUE) {
		pNormalView->pImageBufferObj = vp_button_create_image(pNormalView->pMainLayout,
		                               edj_path,
		                               VP_PLAY_NORMAL_AUDIO_ERROR_IMG);
		evas_object_size_hint_weight_set(pNormalView->pImageBufferObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(pNormalView->pImageBufferObj, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_color_set(pNormalView->pImageBufferObj, 255, 255, 255, 255);
		elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_ERROR_IMAGE, pNormalView->pImageBufferObj);
		evas_object_show(pNormalView->pImageBufferObj);
		/*double dImageAspectRatio = (double)270/214;

		pNormalView->pImageBufferObj = vp_button_create_image(pNormalView->pMainLayout,
																edj_path,
																VP_PLAY_NORMAL_AUDIO_ONLY_IMG);
		evas_object_size_hint_weight_set(pNormalView->pImageBufferObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(pNormalView->pImageBufferObj, EVAS_HINT_FILL, EVAS_HINT_FILL);

		evas_object_size_hint_max_set(pNormalView->pImageBufferObj, 720, 720/dImageAspectRatio);
		elm_image_aspect_fixed_set(pNormalView->pImageBufferObj, EINA_TRUE);

		elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_IMAGE_BUFFER, pNormalView->pImageBufferObj);
		evas_object_show(pNormalView->pImageBufferObj);*/
	}
}
#endif

static void __vp_normal_ctx_popup_dismiss_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pObj) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	_vp_play_normal_view_create_layout_hide_timer(pNormalView);
}

static void __vp_normal_ctx_popup_share_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);
	if (pNormalView->bSharepopup) {
		VideoLogWarning("Share popup is creating, ignore this event");
		return;
	}

	_vp_play_normal_view_on_share_popup(pNormalView);
}

static void __vp_normal_ctx_popup_subtitle_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	/*If it is playing video, pause video*/
	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
	}

	_vp_play_normal_view_set_play_state(pNormalView);

	_vp_play_normal_view_on_subtitle_select_popup(pNormalView);
}


static void __vp_normal_del_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	NormalView	*pNormalView	= (NormalView*)data;

	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	VP_EVAS_DEL(pNormalView->pPopup);

	pNormalView->bIsPopupShow = FALSE;

	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}
}

static void __vp_normal_del_popup_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	NormalView	*pNormalView	= (NormalView*)data;
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (pNormalView->szMediaURL) {
		char *szPreviewURL = NULL;
		vp_play_config_get_preview_url_videos(&szPreviewURL);

		if (szPreviewURL) {
			if (!g_strcmp0(szPreviewURL, pNormalView->szMediaURL)) {
				VideoLogInfo("Erase preview url.");
				vp_play_config_set_preview_url_videos("");
			}
		}
		VP_FREE(szPreviewURL);

		vp_media_contents_del_video(pNormalView->szMediaURL);

		VP_EVAS_DEL(pNormalView->pPopup);
		pNormalView->bIsPopupShow = FALSE;


		PlayView *pPlayView = pNormalView->pPlayView;
		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			return;
		}
		if (!pPlayView->pFunc) {
			VideoLogError("pPlayView pFunc is NULL");
			return;
		}

		if (!pPlayView->pFunc->vp_play_func_exit) {
			VideoLogError("pPlayView exit Func is NULL");
			return;
		}
		vp_device_set_screen_off(FALSE);
		vp_play_util_set_unlock_power_key();

		pPlayView->pFunc->vp_play_func_exit(pPlayView);

	}
}

static void __vp_normal_ctx_popup_delete_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
	}

	VP_EVAS_DEL(pNormalView->pPopup);
	pNormalView->pPopup	= vp_two_button_popup_create(pPlayView->pWin,
	                      VP_PLAY_STRING_POPUP_DELETE,
	                      VP_PLAY_STRING_POPUP_DELETE_CONTENT,
	                      VP_PLAY_STRING_CANCEL,
	                      __vp_normal_del_popup_cancel_cb,
	                      VP_PLAY_STRING_POPUP_DELETE,
	                      __vp_normal_del_popup_delete_cb,
	                      (const void*)pNormalView);

	pNormalView->bIsPopupShow = TRUE;
	eext_object_event_callback_add(pNormalView->pPopup, EEXT_CALLBACK_BACK, __vp_normal_del_popup_cancel_cb, (void*)pNormalView);

}

/*
static void __vp_normal_ctx_popup_repeat_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
	}

	pNormalView->pRepeatHandle = vp_repeat_create(pPlayView->pWin,
							pNormalView->szMediaURL,
							__vp_normal_repeat_popup_close_cb);


	if (!pNormalView->pRepeatHandle) {
		VideoLogError("vp_repeat_create fail");
		return;
	}
	if (!vp_repeat_set_user_data(pNormalView->pRepeatHandle, (void *)pNormalView)) {
		VideoLogWarning("vp_repeat_set_user_data is fail");
	}

	if (!vp_repeat_realize(pNormalView->pRepeatHandle)) {
		VideoLogWarning("vp_repeat_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}
*/

static void __vp_normal_ctx_popup_setting_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	_vp_play_normal_view_on_setting_popup(pNormalView);
}

/*
static void __vp_normal_ctx_popup_sound_alive_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	_vp_play_normal_view_on_sound_alive_popup(pNormalView);
}*/

static void __vp_normal_ctx_popup_play_speed_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	VideoLogInfo("");
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
	}

	_vp_play_normal_view_on_play_speed_popup(pNormalView);
}

static void __vp_normal_ctx_popup_detail_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	_vp_play_normal_view_on_detail_popup(pNormalView);
}

static bool _vp_play_normal_view_show_lock_screen(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (pNormalView->bLockScreen == FALSE) {
		return FALSE;
	}

	VP_EVAS_TIMER_DEL(pNormalView->pLockScreenTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_LOCKSCREEN_SHOW, "*");

	pNormalView->pLockScreenTimer =
	    ecore_timer_add(VP_NORMAL_HIDE_SCREEN_LOCK_TIMER_INTERVAL,
	                    __vp_normal_lockscreen_timer_cb, (void *)pNormalView);

	return TRUE;
}

static void __vp_normal_more_btn_clicked_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pUserData;
	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	VideoLogInfo("");

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for bLockScreen");
		_vp_play_normal_view_show_lock_screen(pNormalView);
		return;
	}

	if (pNormalView->bIsRealize == FALSE) {
		VideoLogWarning("Skip for not realized");
		return;
	}

	if (!vp_play_volume_unrealize(pNormalView->pVolumeHandle)) {
		VideoLogWarning("vp_play_volume_unrealize is fail");
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		VideoLogWarning("Skip for VIDEO_PLAY_TYPE_MULTI_PATH mode.");
		if (vp_play_util_check_streaming(pNormalView->szMediaURL)) {
			VideoLogWarning("url is streaming. don't need to show detail");
			return;
		}
	}
	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW) {
		VideoLogInfo("current launching type not support menu.");
		return;
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		VideoLogInfo("current launching type not support menu.");
		return;
	}
	if (strstr(pPlayView->szMediaURL, "/.") != NULL) {
		VideoLogInfo("hidden file not suport menu.");
		return;
	}

	int val = 0;
	system_settings_get_value_int(SYSTEM_SETTINGS_KEY_LOCK_STATE, &val);
	if (val == SYSTEM_SETTINGS_LOCK_STATE_LOCK) {
		return;
	}

	pNormalView->pCtxPopup = elm_ctxpopup_add(pPlayView->pWin);

	if (!pNormalView->pCtxPopup) {
		VideoLogError("pNormalView->pCtxPopup is NULL");
		return;
	}
	eext_object_event_callback_add(pNormalView->pCtxPopup, EEXT_CALLBACK_BACK, __vp_normal_ctx_popup_dismiss_cb, (void *)pNormalView);

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

	elm_object_style_set(pNormalView->pCtxPopup , "more/default");
	evas_object_smart_callback_add(pNormalView->pCtxPopup , "dismissed", __vp_normal_ctx_popup_dismiss_cb, (void *)pNormalView);
	elm_ctxpopup_auto_hide_disabled_set(pNormalView->pCtxPopup, EINA_TRUE);

#ifdef ENABLE_DRM_FEATURE
	bool bDRMLock = FALSE;
	bool bPlayReady = FALSE;
#endif
	char *szMediaID = NULL;

	if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {
		vp_media_contents_get_video_id(pNormalView->szMediaURL, &szMediaID);
		if (!szMediaID) {
		}
	}
	VP_FREE(szMediaID);
#ifdef ENABLE_DRM_FEATURE
	_vp_play_normal_view_check_drm_lock(pNormalView, &bDRMLock);

	vp_drm_is_playready_drm_file(pNormalView->szMediaURL, &bPlayReady);
#endif
	//bool bIsPersonal = FALSE;

	switch (pNormalView->nLaunchingType) {
	case VIDEO_PLAY_TYPE_STORE:
	case VIDEO_PLAY_TYPE_LIST:
	case VIDEO_PLAY_TYPE_GALLERY:
	case VIDEO_PLAY_TYPE_MYFILE:
	case VIDEO_PLAY_TYPE_OTHER:

		/* Share via */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_SHARE, NULL, __vp_normal_ctx_popup_share_cb, (void *)pNormalView);

		/* delete */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_DELETE, NULL, __vp_normal_ctx_popup_delete_cb, (void *)pNormalView);

		/* Play speed */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_PLAY_SPEED, NULL, __vp_normal_ctx_popup_play_speed_cb, (void *)pNormalView);

		/* subtitles */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_SUBTITLE, NULL, __vp_normal_ctx_popup_subtitle_cb, (void *)pNormalView);

		/* repeat mode */
		//elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_REPEAT_SETTING, NULL, __vp_normal_ctx_popup_repeat_cb, (void *)pNormalView);

		/* Sound alive */
		//elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_SOUND_ALIVE, NULL, __vp_normal_ctx_popup_sound_alive_cb, (void *)pNormalView);

		/* Details */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_COM_DETAILS, NULL, __vp_normal_ctx_popup_detail_cb, (void *)pNormalView);
		break;

	case VIDEO_PLAY_TYPE_MESSAGE:
	case VIDEO_PLAY_TYPE_EMAIL:

		/* Play speed */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_PLAY_SPEED, NULL, __vp_normal_ctx_popup_play_speed_cb, (void *)pNormalView);

		/* subtitles */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_SUBTITLE, NULL, __vp_normal_ctx_popup_subtitle_cb, (void *)pNormalView);
		break;

	case VIDEO_PLAY_TYPE_WEB:

		/* Settings */
		if (FALSE) {
			elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_POPUP_SETTINGS, NULL, __vp_normal_ctx_popup_setting_cb, (void *)pNormalView);
		}
		break;

	case VIDEO_PLAY_TYPE_MULTI_PATH:
		/* Details */
		elm_ctxpopup_item_append(pNormalView->pCtxPopup, VP_PLAY_STRING_COM_DETAILS, NULL, __vp_normal_ctx_popup_detail_cb, (void *)pNormalView);
		break;

	case VIDEO_PLAY_TYPE_PREVIEW:
		break;
	}

	int nW = 0;
	int nH = 0;

	Evas_Coord_Rectangle rect = {0,};
	elm_win_screen_size_get(pPlayView->pWin,  &rect.x, &rect.y, &rect.w, &rect.h);
	if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_NONE) {
		nW = rect.w / 2;
		nH = rect.h;
	} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90) {
		nW = rect.h / 2;
		nH = rect.w;
	} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_180) {
		nW = rect.w / 2;
		nH = rect.h;
	} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
		nW = rect.h / 2;
		nH = rect.w;
	}

	evas_object_move(pNormalView->pCtxPopup, nW, nH);


	evas_object_show(pNormalView->pCtxPopup);
}

void vp_play_normal_view_pause_player(void *pPlayview)
{
	if (!pPlayview) {
		return;
	}
	PlayView *vPlayview = (PlayView *)pPlayview;
	NormalView *pNormalView = vPlayview->pNormalView;
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}
	if (nState == VP_MM_PLAYER_STATE_PLAYING) {
		vp_mm_player_pause(pNormalView->pPlayerHandle);
		pNormalView->bManualPause = TRUE;
	}
	_vp_play_normal_view_set_play_state(pNormalView);
	_vp_play_normal_view_on_capture_mode(pNormalView);
}

void vp_play_normal_view_play_player(void *pPlayview)
{
	if (!pPlayview) {
		VideoLogError("Invalid player View)");
		return;
	}
	PlayView *vPlayview = (PlayView *)pPlayview;
	NormalView *pNormalView = vPlayview->pNormalView;
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}
	if (nState == VP_MM_PLAYER_STATE_PAUSED) {
		vp_mm_player_play(pNormalView->pPlayerHandle);
		pNormalView->bManualPause = FALSE;
	}
	_vp_play_normal_view_set_play_state(pNormalView);
	_vp_play_normal_view_on_capture_mode(pNormalView);
}

void vp_play_normal_view_show_volume_popup(void *pPlayview)
{
	if (!pPlayview) {
		return;
	}
	PlayView *vPlayview = (PlayView *)pPlayview;
	NormalView *pNormalView = vPlayview->pNormalView;
	pNormalView->pVolumePopupHandle = NULL;
	vp_play_volume_update_value(pNormalView->pVolumeHandle);
	_vp_play_normal_view_on_volume_popup(pNormalView, FALSE);
	vp_play_volume_set_focus_in(pNormalView->pVolumeHandle);
}

static void __vp_normal_function_btn_clicked_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pUserData;

	if (pObj != pNormalView->pVolumeBtn && pObj != pNormalView->pScreenSizeBtn && pObj != pNormalView->pLockBtn) {
		return;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	if (pObj == pNormalView->pVolumeBtn) {
		VideoLogWarning("pVolumeBtn button");
		vp_play_volume_update_value(pNormalView->pVolumeHandle);
		_vp_play_normal_view_on_volume_popup(pNormalView, TRUE);
		vp_play_volume_set_focus_in(pNormalView->pVolumeHandle);
	} else if (pObj == pNormalView->pBookmarkBtn) {
		VideoLogWarning("pBookmarkBtn button");

		bool bIsRealize = FALSE;
		if (!vp_play_bookmark_is_realize(pNormalView->pBookmarkHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_bookmark_is_realize fail");
			return;
		}
		if (bIsRealize) {
			_vp_play_normal_view_on_bookmark_mode(pNormalView, FALSE);
		} else {
			_vp_play_normal_view_on_bookmark_mode(pNormalView, TRUE);
		}
	} else if (pObj == pNormalView->pShareBtn) {
		VideoLogWarning("pShareBtn button");
		_vp_play_normal_view_on_share_popup(pNormalView);
	} else if (pObj == pNormalView->pLockBtn) {
		VideoLogWarning("pLockBtn button");
		if (!pNormalView->bLockScreen) {
			pNormalView->bLockScreen = TRUE;
			vp_play_normal_view_set_lock_screen(pNormalView, pNormalView->bLockScreen);
		} else {
			pNormalView->bLockScreen = FALSE;
			vp_play_normal_view_set_lock_screen(pNormalView, pNormalView->bLockScreen);
		}
	} else if (pObj == pNormalView->pSoundAliveBtn) {
		VideoLogWarning("pSoundAliveBtn button");
		_vp_play_normal_view_on_sound_alive_popup(pNormalView);
	} else if (pObj == pNormalView->pRepeatBtn) {
		VideoLogWarning("pRepeatBtn button");
		if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_NONE) {
			pNormalView->nRepeatMode = VIDEO_PLAY_REPEAT_ONE;

			Evas_Object *pIcon = NULL;
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_ONE);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_ONE_PRESS);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_ONE_DIM);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
		} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE) {
			pNormalView->nRepeatMode = VIDEO_PLAY_REPEAT_ALL;

			Evas_Object *pIcon = NULL;
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_ALL);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_ALL_PRESS);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_ALL_DIM);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

		} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
			pNormalView->nRepeatMode = VIDEO_PLAY_REPEAT_NONE;
			Evas_Object *pIcon = NULL;
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_OFF);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_OFF_PRESS);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pRepeatBtn, edj_path, VP_PLAY_NORMAL_RES_REPEAT_OFF_DIM);
			elm_object_part_content_set(pNormalView->pRepeatBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
		}
	} else if (pObj == pNormalView->pScreenSizeBtn) {
		VideoLogWarning("pScreenSizeBtn button");

		if (pNormalView->nDisplayMode == VP_MM_PLAYER_DISPLAY_FULL_SCREEN) {
			VideoLogWarning(" VP_MM_PLAYER_DISPLAY_LETTER_BOX");
			pNormalView->nDisplayMode = VP_MM_PLAYER_DISPLAY_LETTER_BOX;
			vp_play_preference_set_screen_mode_key(pNormalView->nDisplayMode);

			vp_mm_player_set_display_mode(pNormalView->pPlayerHandle, pNormalView->nDisplayMode);

			Evas_Object *pIcon = NULL;
			pIcon = elm_object_part_content_get(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}

			pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE);
			elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE);
			elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			//pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE_DIM);
			//elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

			return;
		} else if (pNormalView->nDisplayMode == VP_MM_PLAYER_DISPLAY_LETTER_BOX) {
			VideoLogWarning("VP_MM_PLAYER_DISPLAY_FULL_SCREEN");
			pNormalView->nDisplayMode = VP_MM_PLAYER_DISPLAY_FULL_SCREEN;
			vp_play_preference_set_screen_mode_key(pNormalView->nDisplayMode);

			vp_mm_player_set_display_mode(pNormalView->pPlayerHandle, pNormalView->nDisplayMode);

			Evas_Object *pIcon = NULL;
			pIcon = elm_object_part_content_get(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}
			pIcon = elm_object_part_content_get(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON);
			if (pIcon) {
				evas_object_del(pIcon);
				pIcon = NULL;
			}

			pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_2_MODE);
			elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_2_MODE_PRESS);
			elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			//pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_2_MODE_DIM);
			//elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

			return;
		}
	}
}

static void __vp_normal_control_btn_clicked_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pUserData;
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView handle is NULL");
		return;
	}

	if (pNormalView->bIsRealize == FALSE) {
		VideoLogWarning("Skip for not realized");
		return;
	}

	if (pObj == pNormalView->pPauseBtn) {
		VideoLogWarning("Pause button");
		vp_util_release_cpu();

		if (pNormalView->bBufferingComplete == FALSE) {
			VideoLogWarning("Bufferring is not completed. ignore pause signal");
		} else {
			if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}
		}

		VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
		pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);

		pNormalView->bManualPause = TRUE;
	} else if (pObj == pNormalView->pResumeBtn) {
		VideoLogWarning("Resume button");
		vp_util_lock_cpu();
		if(_vp_play_normal_view_check_during_call(pNormalView)) {
			return;
		}

		VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
		pNormalView->nSpeedValue = 1;
		pNormalView->bSpeedDown = TRUE;

		if (pNormalView->bBufferingComplete == FALSE) {
			VideoLogWarning("Bufferring is not completed. ignore pause signal");
		} else {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Resume Fail");
			}
		}

		VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
		pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);

		pNormalView->bManualPause = FALSE;
	} else if (pObj == pNormalView->pPlayFocusBtn) {
		VideoLogWarning("pPlayFocusBtn click");
		Evas_Object *pObj = elm_object_part_content_get(pNormalView->pControlLayout, VP_PLAY_SWALLOW_NORMAL_PLAY);
		if (pNormalView->pPauseBtn == pObj) {

			if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Pause Fail");
			}

			VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
			pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);

			pNormalView->bManualPause = TRUE;
		} else {
			if(_vp_play_normal_view_check_during_call(pNormalView)) {
				return;
			}

			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Resume Fail");
			}
			VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
			pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);

			pNormalView->bManualPause = FALSE;
		}
	} else if (pObj == pNormalView->pPrevBtn) {
		VideoLogWarning("Prev button");
		if (pNormalView->bHLSMode == FALSE) {
			if (pNormalView->nSpeedValue < 2) {
				VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
				_vp_play_normal_view_on_prev_play(pNormalView, TRUE, FALSE);
			}
		}
	} else if (pObj == pNormalView->pNextBtn) {
		VideoLogWarning("Next button");
		if (pNormalView->bHLSMode == FALSE) {
			if (pNormalView->nSpeedValue < 2 && strstr(pPlayView->szMediaURL, "/.") == NULL) {
				VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
				_vp_play_normal_view_on_next_play(pNormalView, TRUE);
			}
		}
	} else if (pObj == pNormalView->pRotateBtn) {
		VideoLogWarning("Rotate button");

		bool bLandscape = vp_play_util_get_landscape_check(pNormalView->nRotate);
		PlayView *pPlayView = pNormalView->pPlayView;

		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			return;
		}

		if (bLandscape) {
			pPlayView->nRotate = VIDEO_PLAY_ROTATE_NONE;
			elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, 0);
			vp_play_normal_view_set_rotate((normal_view_handle) pNormalView, VIDEO_PLAY_ROTATE_NONE, TRUE);
		} else {
			pPlayView->nRotate = VIDEO_PLAY_ROTATE_270;
			elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, 270);
			vp_play_normal_view_set_rotate((normal_view_handle) pNormalView, VIDEO_PLAY_ROTATE_270, TRUE);
		}
	} else if (pObj == pNormalView->pSoundPathBtn) {
		VideoLogWarning("SoundPath button");
		_vp_play_normal_view_on_sound_path_popup(pNormalView);
	} else if (pObj == pNormalView->pCaptureRewBtn) {
		_vp_play_normal_view_screen_move(pNormalView, FALSE);
	} else if (pObj == pNormalView->pCaptureFFBtn) {
		_vp_play_normal_view_screen_move(pNormalView, TRUE);
	} else {
		VideoLogWarning("Unknown button");
	}
}

static void __vp_normal_control_btn_press_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	if (!pObj) {
		VideoLogError("pObj is NULL");
		return;
	}
	bool bSpeedMode = FALSE;
	NormalView	*pNormalView = (NormalView *)pUserData;
	if (pNormalView->bKeyPressed == TRUE) {
		VideoLogInfo("Key has been pressed");
		return;
	}
	pNormalView->bKeyPressed = TRUE;

	if (pNormalView->pSpeedTimer) {
		bSpeedMode = TRUE;
	}

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);

	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

	if (pObj == pNormalView->pNextBtn) {
		VideoLogInfo("pNextBtn Press");
		if (pNormalView->bHLSMode == FALSE) {
			pNormalView->bSpeedFF = TRUE;
			pNormalView->nSpeedValue = 1;
			pNormalView->bSpeedDown = TRUE;
			pNormalView->pSpeedTimer = ecore_timer_add(VP_NORMAL_LONG_PRESS_TIMER_INTERVAL,
			                           __vp_normal_speed_timer_cb,
			                           (void *)pNormalView);
		} else {
			pNormalView->pSpeedTimer = ecore_timer_add(VP_NORMAL_LONG_PRESS_TIMER_INTERVAL,
			                           __vp_normal_dlna_speed_timer_cb,
			                           (void *)pNormalView);
		}

	} else if (pObj == pNormalView->pPrevBtn) {
		VideoLogInfo("pPrevBtn Press");
		if (pNormalView->bHLSMode == FALSE) {
			pNormalView->bSpeedFF = FALSE;
			pNormalView->nSpeedValue = 1;
			pNormalView->bSpeedDown = TRUE;
			pNormalView->pSpeedTimer = ecore_timer_add(VP_NORMAL_LONG_PRESS_TIMER_INTERVAL,
			                           __vp_normal_speed_timer_cb,
			                           (void *)pNormalView);
		} else {
			pNormalView->pSpeedTimer = ecore_timer_add(VP_NORMAL_LONG_PRESS_TIMER_INTERVAL,
			                           __vp_normal_dlna_speed_timer_cb,
			                           (void *)pNormalView);
		}
	} else if (pObj == pNormalView->pResumeBtn) {
		if (bSpeedMode) {
			int nPosition = 0;
			if (vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
				VideoLogDebug("nPosition[%d], curPosition[%d]", nPosition, pNormalView->nCurPosition);

				if (vp_mm_player_set_position(pNormalView->pPlayerHandle, pNormalView->nCurPosition)) {
					pNormalView->bSeekComplete = FALSE;
					pNormalView->nWaitPos = -1;
				} else {
					pNormalView->nWaitPos = nPosition;
				}
			}
		}
	} else if (pObj == pNormalView->pPlayFocusBtn) {
		VideoLogInfo("pPlayFocusBtn Press");
		Evas_Object *pObj = elm_object_part_content_get(pNormalView->pControlLayout, VP_PLAY_SWALLOW_NORMAL_PLAY);
		if (pNormalView->pResumeBtn == pObj) {
			if (bSpeedMode) {
				int nPosition = 0;
				if (vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
					if (vp_mm_player_set_position(pNormalView->pPlayerHandle, pNormalView->nCurPosition)) {
						pNormalView->bSeekComplete = FALSE;
						pNormalView->nWaitPos = -1;
					} else {
						pNormalView->nWaitPos = nPosition;
					}
				}
			}
		}
	}
}

static void __vp_normal_control_btn_unpress_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (!pObj) {
		VideoLogError("pObj is NULL");
		return;
	}
	VideoLogInfo("");

	NormalView	*pNormalView = (NormalView *)pUserData;

	pNormalView->bKeyPressed = FALSE;

	if (pObj == pNormalView->pPauseBtn) {
	} else if (pObj == pNormalView->pResumeBtn) {
	} else if (pObj == pNormalView->pNextBtn) {
		pNormalView->bSpeedDown = FALSE;
		VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
		if (!pNormalView->pProgressTimer) {
			_vp_play_normal_view_create_progress_timer(pNormalView);
		}
	} else if (pObj == pNormalView->pPrevBtn) {
		pNormalView->bSpeedDown = FALSE;
		VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
		if (!pNormalView->pProgressTimer) {
			_vp_play_normal_view_create_progress_timer(pNormalView);
		}
	} else if (pObj == pNormalView->pAllsharBtn) {
		VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
		pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);
	}
	_vp_play_normal_view_create_layout_hide_timer(pNormalView);
}

static void __vp_normal_media_key_event_cb(vp_media_key_event_t nKey, bool bRelease, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	if (pNormalView->bSharepopup) {
		VideoLogWarning("bSharepopup is TRUE");
		return;
	}

	switch (nKey) {
	case VP_MEDIA_KEY_PLAY:
	case VP_MEDIA_KEY_PAUSE:
	case VP_MEDIA_KEY_PLAYPAUSE:
		if (bRelease) {
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);

			if (pNormalView->pDetailHandle) {
				vp_detail_destroy(pNormalView->pDetailHandle);
				pNormalView->pDetailHandle = NULL;
			}

			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				pNormalView->bManualPause = FALSE;
			} else {
				if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
				pNormalView->bManualPause = TRUE;
			}

			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		}
		break;

	case VP_MEDIA_KEY_PREVIOUS:
		if (bRelease) {
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
			_vp_play_normal_view_on_prev_play(pNormalView, TRUE, FALSE);
		}
		break;
	case VP_MEDIA_KEY_NEXT:
		if (bRelease) {
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
			_vp_play_normal_view_on_next_play(pNormalView, TRUE);
		}
		break;
	case VP_MEDIA_KEY_REWIND:
		if (bRelease) {
			pNormalView->bSpeedDown = FALSE;

			if (pNormalView->nSpeedValue < 2) {
				VideoLogInfo("speed up");
				_vp_normal_view_speed_for_steps(pNormalView, FALSE);
			}

			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
			if (pNormalView->pDetailHandle) {
				vp_detail_destroy(pNormalView->pDetailHandle);
				pNormalView->pDetailHandle = NULL;
			}

			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

			if (pNormalView->bManualPause == FALSE) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogError("play Fail");
				}
				pNormalView->bManualPause = FALSE;
			}

			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		} else {
			if (pNormalView->pDetailHandle) {
				vp_detail_destroy(pNormalView->pDetailHandle);
				pNormalView->pDetailHandle = NULL;
			}
			VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
			VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);

			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

			pNormalView->bSpeedFF = FALSE;
			pNormalView->nSpeedValue = 1;
			pNormalView->bSpeedDown = TRUE;
			pNormalView->pSpeedTimer = ecore_timer_add(VP_NORMAL_LONG_PRESS_TIMER_INTERVAL,
			                           __vp_normal_speed_timer_cb,
			                           (void *)pNormalView);
		}
		break;
	case VP_MEDIA_KEY_FASTFORWARD:
		if (bRelease) {
			pNormalView->bSpeedDown = FALSE;

			if (pNormalView->nSpeedValue < 2) {
				VideoLogInfo("speed up");
				_vp_normal_view_speed_for_steps(pNormalView, TRUE);
			}

			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
			if (pNormalView->pDetailHandle) {
				vp_detail_destroy(pNormalView->pDetailHandle);
				pNormalView->pDetailHandle = NULL;
			}

			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

			if (pNormalView->bManualPause == FALSE) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogError("play Fail");
				}
				pNormalView->bManualPause = FALSE;
			}

			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		} else {
			VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
			VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);

			if (pNormalView->pDetailHandle) {
				vp_detail_destroy(pNormalView->pDetailHandle);
				pNormalView->pDetailHandle = NULL;
			}

			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

			pNormalView->bSpeedFF = TRUE;
			pNormalView->nSpeedValue = 1;
			pNormalView->bSpeedDown = TRUE;
			pNormalView->pSpeedTimer = ecore_timer_add(VP_NORMAL_LONG_PRESS_TIMER_INTERVAL,
			                           __vp_normal_speed_timer_cb,
			                           (void *)pNormalView);
		}
		break;
	case VP_MEDIA_KEY_STOP:
		if (bRelease) {
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}

			if (nState == VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
					VideoLogError("Pause Fail");
				}
			}
			pNormalView->bManualPause = TRUE;
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		}
		break;
	default:
		VideoLogWarning("Undefined key : %d", nKey);
		break;
	}

}

/*
   this function is for only for keyboard FF/REW key longpress case
   because of different FF/REW long press scenario in keyboard
	video -> play a file -> pause -> key down FF button(long press) -> move seek -> key up FF button -> seek stop -> stay pause
	video -> play a file -> playing -> key down FF button(long press) -> move seek -> key up FF button -> seek stop -> resume
*/
void vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(normal_view_handle pViewHandle, bool bRelease, bool bFFseek)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (pNormalView->pSpeedTimer == NULL && bRelease == FALSE) {
		if (bFFseek) {
			__vp_normal_media_key_event_cb(VP_MEDIA_KEY_FASTFORWARD, bRelease, pNormalView);
		} else {
			__vp_normal_media_key_event_cb(VP_MEDIA_KEY_REWIND, bRelease, pNormalView);
		}
	} else if (pNormalView->pSpeedTimer && bRelease == TRUE) {
		if (pNormalView->nSpeedValue < 2) {
			VideoLogInfo("speed up");
			_vp_normal_view_speed_for_steps(pNormalView, bFFseek);
		} else {
			int nPosition = 0;
			if (vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
				if (vp_mm_player_set_position(pNormalView->pPlayerHandle, pNormalView->nCurPosition)) {
					pNormalView->bSeekComplete = FALSE;
					pNormalView->nWaitPos = -1;
				} else {
					pNormalView->nWaitPos = nPosition;
				}
			}
		}

		VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
		if (pNormalView->pDetailHandle) {
			vp_detail_destroy(pNormalView->pDetailHandle);
			pNormalView->pDetailHandle = NULL;
		}

		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");

		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogError("play Fail");
			}
			pNormalView->bManualPause = FALSE;
		}

		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
}

static void _vp_normal_view_speed_for_steps(NormalView *pNormalView, bool bSpeedFF)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	VideoLogInfo("");

	if (pNormalView->bSeekComplete == FALSE) {
		VideoLogError("bSeekComplete is fail");
		return;
	}

	int nSetPosition = 0;
	int nCurPosition = 0;

	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nCurPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return;
	}

	if (pNormalView->nDuration <= nCurPosition) {
		_vp_play_normal_view_on_next_play(pNormalView, FALSE);
		return;
	}

	VideoLogInfo("pNormalView->nDuration %d", pNormalView->nDuration);
	if (bSpeedFF) {
		nSetPosition = nCurPosition  + (15 * 1000);//15 sec
	} else {
		nSetPosition = nCurPosition  - (15 * 1000);
	}

	if (pNormalView->nDuration < nSetPosition) {
		nSetPosition = pNormalView->nDuration;
	}

	if (nSetPosition < 0) {
		nSetPosition = 0;
	}

#if 1
	if (vp_mm_player_set_position(pNormalView->pPlayerHandle, nSetPosition)) {
#else
	if (vp_mm_player_set_position_by_key_frame(pNormalView->pPlayerHandle, nSetPosition)) {
#endif
		pNormalView->bSeekComplete = FALSE;
		pNormalView->nCurPosition = nSetPosition;
		vp_play_progressbar_set_position(pNormalView->pProgressbar, nSetPosition);
	}

	return;
}

//Focus UI
void vp_play_normal_view_set_main_layout_focus_out(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	VideoLogInfo("");

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->bMainFocusState = FALSE;

	elm_object_focus_set(pNormalView->pNaviFrame, EINA_FALSE);
	elm_object_tree_focus_allow_set(pNormalView->pNaviFrame, EINA_FALSE);
	elm_object_focus_allow_set(pNormalView->pNaviFrame, EINA_FALSE);

	elm_object_focus_set(pNormalView->pMainLayout, EINA_FALSE);
	elm_object_tree_focus_allow_set(pNormalView->pMainLayout, EINA_FALSE);
	elm_object_focus_allow_set(pNormalView->pMainLayout, EINA_FALSE);
}

void vp_play_normal_view_set_main_layout_focus_in(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	VideoLogInfo("");

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (pNormalView->bMainFocusState == TRUE) {
		VideoLogInfo("Already Focus In");
		return;
	}
	pNormalView->bMainFocusState = TRUE;

	elm_object_focus_allow_set(pNormalView->pNaviFrame, EINA_TRUE);
	elm_object_tree_focus_allow_set(pNormalView->pNaviFrame, EINA_TRUE);
	elm_object_focus_set(pNormalView->pNaviFrame, EINA_TRUE);

	elm_object_focus_allow_set(pNormalView->pMainLayout, EINA_TRUE);
	elm_object_tree_focus_allow_set(pNormalView->pMainLayout, EINA_TRUE);
	elm_object_focus_set(pNormalView->pMainLayout, EINA_TRUE);

	elm_object_focus_set(pNormalView->pAllsharBtn, EINA_TRUE);
}

void vp_play_normal_view_set_resume_or_pause(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}
	if (nState == VP_MM_PLAYER_STATE_PLAYING) {
		vp_mm_player_pause(pNormalView->pPlayerHandle);
		pNormalView->bManualPause = TRUE;
	} else {
		vp_mm_player_play(pNormalView->pPlayerHandle);
		pNormalView->bManualPause = FALSE;
	}
	_vp_play_normal_view_set_play_state(pNormalView);
	_vp_play_normal_view_on_capture_mode(pNormalView);
}

void vp_play_normal_view_show_main_layout(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	_vp_play_normal_view_show_layout(pNormalView);
}

void vp_play_normal_view_create_main_layout_hide_timer(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	_vp_play_normal_view_create_layout_hide_timer(pNormalView);
}

static void _vp_play_normal_view_set_button_focus_sequence(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	bool bLandscape = vp_play_util_get_landscape_check(pNormalView->nRotate);

	elm_object_focus_set(pNormalView->pVolumeBtn, EINA_TRUE);

	vp_play_util_focus_next_object_set(pNormalView->pPrevBtn, pNormalView->pPlayFocusBtn, ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pNormalView->pPlayFocusBtn, pNormalView->pPrevBtn, ELM_FOCUS_LEFT);

	vp_play_util_focus_next_object_set(pNormalView->pPlayFocusBtn, pNormalView->pNextBtn, ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pNormalView->pNextBtn, pNormalView->pPlayFocusBtn, ELM_FOCUS_LEFT);

	vp_play_util_focus_next_object_set(pNormalView->pAllsharBtn, pNormalView->pPlayFocusBtn, ELM_FOCUS_DOWN);
	if (pNormalView->pSoundPathBtn) {
		vp_play_util_focus_next_object_set(pNormalView->pAllsharBtn, pNormalView->pSoundPathBtn, ELM_FOCUS_NEXT);
	} else {
		vp_play_util_focus_next_object_set(pNormalView->pAllsharBtn, pNormalView->pPlayFocusBtn, ELM_FOCUS_NEXT);
	}
	vp_play_util_focus_next_object_set(pNormalView->pSoundPathBtn, pNormalView->pPlayFocusBtn, ELM_FOCUS_DOWN);
	vp_play_util_focus_next_object_set(pNormalView->pSoundPathBtn, pNormalView->pPlayFocusBtn, ELM_FOCUS_NEXT);

	if (bLandscape == TRUE) {
		vp_play_util_focus_next_object_set(pNormalView->pVolumeBtn, pNormalView->pBookmarkBtn, ELM_FOCUS_RIGHT);
		vp_play_util_focus_next_object_set(pNormalView->pBookmarkBtn, pNormalView->pVolumeBtn, ELM_FOCUS_LEFT);

		vp_play_util_focus_next_object_set(pNormalView->pBookmarkBtn, pNormalView->pPrevBtn, ELM_FOCUS_RIGHT);
		vp_play_util_focus_next_object_set(pNormalView->pPrevBtn, pNormalView->pBookmarkBtn, ELM_FOCUS_LEFT);

		vp_play_util_focus_next_object_set(pNormalView->pNextBtn, pNormalView->pMultiWinBtn, ELM_FOCUS_RIGHT);
		vp_play_util_focus_next_object_set(pNormalView->pMultiWinBtn, pNormalView->pNextBtn, ELM_FOCUS_LEFT);

		vp_play_util_focus_next_object_set(pNormalView->pMultiWinBtn, pNormalView->pScreenSizeBtn, ELM_FOCUS_RIGHT);
		vp_play_util_focus_next_object_set(pNormalView->pScreenSizeBtn, pNormalView->pMultiWinBtn, ELM_FOCUS_LEFT);
	} else {
		vp_play_util_focus_next_object_set(pNormalView->pBookmarkBtn, pNormalView->pMultiWinBtn, ELM_FOCUS_RIGHT);
		vp_play_util_focus_next_object_set(pNormalView->pMultiWinBtn, pNormalView->pBookmarkBtn, ELM_FOCUS_LEFT);

		vp_play_util_focus_next_object_set(pNormalView->pVolumeBtn, pNormalView->pPrevBtn, ELM_FOCUS_RIGHT);
		vp_play_util_focus_next_object_set(pNormalView->pPrevBtn, pNormalView->pVolumeBtn, ELM_FOCUS_LEFT);

		vp_play_util_focus_next_object_set(pNormalView->pNextBtn, pNormalView->pScreenSizeBtn, ELM_FOCUS_RIGHT);
		vp_play_util_focus_next_object_set(pNormalView->pScreenSizeBtn, pNormalView->pNextBtn, ELM_FOCUS_LEFT);

		vp_play_util_focus_next_object_set(pNormalView->pVolumeBtn, pNormalView->pBookmarkBtn, ELM_FOCUS_UP);
		vp_play_util_focus_next_object_set(pNormalView->pBookmarkBtn, pNormalView->pVolumeBtn, ELM_FOCUS_DOWN);
	}
}

bool  vp_play_normal_view_get_volume_state(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	bool bIsRealize = FALSE;

	if (pNormalView->pVolumeHandle) {
		if (!vp_play_volume_is_realize(pNormalView->pVolumeHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_volume_is_realize is fail");
		}
	}

	return bIsRealize;
}

bool vp_play_normal_view_get_main_layout_focus_state(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	return pNormalView->bMainFocusState;
}

bool vp_play_normal_view_get_main_layout_show_state(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	return pNormalView->bShowLayout;
}

vp_normalview_popup_style_e vp_play_normal_view_get_popup_show_state(normal_view_handle *pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return VP_PLAY_NORMAL_VIEW_POPUP_NONE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	vp_normalview_popup_style_e nPopUpShow = VP_PLAY_NORMAL_VIEW_POPUP_NONE;

	if (pNormalView->pCtxPopup) {
		nPopUpShow = VP_PLAY_NORMAL_VIEW_CTX_POPUP;
	} else if (pNormalView->bIsPopupShow || pNormalView->bIsSubtitleShow) {
		nPopUpShow = VP_PLAY_NORMAL_VIEW_OTHER_POPUP;
	}

	return nPopUpShow;
}

static void __vp_normal_naviframe_back_btn_cb(void *pUserData, Evas_Object *obj, void *event_info)
{
	NormalView *pNormalView = (NormalView *)pUserData;

	_vp_play_normal_view_set_played_time(pNormalView);

	pNormalView->pExitWaitTimer = ecore_timer_add(VP_NORMAL_EXIT_WAIT_TIMER_INTERVAL,
				                              __vp_normal_exit_wait_timer_cb, (void *)pNormalView);

	elm_naviframe_item_pop(pNormalView->pNaviFrame);

	return;
}

static Eina_Bool __vp_normal_naviframe_pop_cb(void *pUserData, Elm_Object_Item *pItem)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pUserData;

	if (pNormalView->bLockScreen) {
		VideoLogError("Skip for bLockScreen");
		_vp_play_normal_view_show_lock_screen(pNormalView);
		return EINA_FALSE;
	}

	if (pNormalView->bIsActive) {
		if (pNormalView->pExitWaitTimer) {
			_vp_play_normal_view_set_played_time(pNormalView);

			PlayView *pPlayView = pNormalView->pPlayView;

			if (!pPlayView) {
				VideoLogError("pPlayView is NULL");
				return EINA_FALSE;
			}

			if (!pPlayView->pFunc) {
				VideoLogError("pPlayView pFunc is NULL");
				return EINA_FALSE;
			}

			if (!pPlayView->pFunc->vp_play_func_exit) {
				VideoLogError("pPlayView exit Func is NULL");
				return EINA_FALSE;
			}

			vp_device_set_screen_off(FALSE);
			vp_play_util_set_unlock_power_key();

			VideoLogWarning("== BACK ==");
			pPlayView->pFunc->vp_play_func_exit(pPlayView);

			return EINA_FALSE;
		} else {
			PlayView *pPlayView = pNormalView->pPlayView;

			if (!pPlayView) {
				VideoLogError("pPlayView is NULL");
				return EINA_FALSE;
			}

			if (pPlayView->bDestroy) {
				pNormalView->pNaviFrame = NULL; // it will be deleted when return TRUE
				return EINA_TRUE;
			}

			vp_play_util_status_noti_show(VP_PLAY_STRING_COM_WAIT_BACK);

			pNormalView->pExitWaitTimer = ecore_timer_add(VP_NORMAL_EXIT_WAIT_TIMER_INTERVAL,
			                              __vp_normal_exit_wait_timer_cb, (void *)pNormalView);

			return EINA_FALSE;
		}
	}

	if (pNormalView->bDestroyNavi) {
		VideoLogWarning("bDestroyNavi is TRUE.");
		pNormalView->pNaviFrame = NULL; // it will be deleted when return TRUE

		return EINA_TRUE;
	}

	return EINA_FALSE;
}

//static void __vp_normal_sdp_download_state_change_cb(VppDownloadHandle pDownloadHandle, VppDownloadState eState, const char *szPath, void *pUserData)
//{
//	VideoLogWarning("eState : [%d]", eState);
//
//	if (!pUserData) {
//		VideoLogError("pUserData is NULL");
//		return;
//	}
//
//	NormalView *pNormalView = (NormalView *)pUserData;
//
//	if (!pDownloadHandle) {
//		VideoLogError("pDownloadHandle == NULL!!!");
//		return;
//	}
//
//	if (eState == VPP_DOWNLOAD_STATE_COMPLETED) {
//		char *szDownloadedFilePath = VppDownloadGetDownloadedFilePath(pDownloadHandle);
//
//		if (szDownloadedFilePath) {
//			VideoLogWarning("szDownloadedFilePath : %s", szDownloadedFilePath);
//			VP_FREE(pNormalView->szMediaURL);
//			pNormalView->szMediaURL =  g_strdup(szDownloadedFilePath);
//		}
//
//		VP_FREE(szDownloadedFilePath);
//		if (!vp_mm_player_realize_async(pNormalView->pPlayerHandle, pNormalView->szMediaURL)) {
//			VideoLogError("vp_mm_player_realize_async fail");
//			VppDownloadDestroyItem(pNormalView->pDownloadHandle);
//			pNormalView->pDownloadHandle = NULL;
//			return;
//		}
//
//		VppDownloadDestroyItem(pNormalView->pDownloadHandle);
//		pNormalView->pDownloadHandle = NULL;
//	} else if (eState == VPP_DOWNLOAD_STATE_FAILED || eState == VPP_DOWNLOAD_STATE_CANCELED) {
//		if (!vp_mm_player_realize_async(pNormalView->pPlayerHandle, pNormalView->szMediaURL)) {
//
//			VppDownloadDestroyItem(pNormalView->pDownloadHandle);
//			pNormalView->pDownloadHandle = NULL;
//
//			VideoLogError("vp_mm_player_realize_async fail");
//			return;
//		}
//		VppDownloadDestroyItem(pNormalView->pDownloadHandle);
//		pNormalView->pDownloadHandle = NULL;
//	}
//}

static void __vp_normal_main_layout_resize_cb(void *pUserData, Evas *pEvas, Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return;
	}
	//if needed, to do
}

/*szColorHex:#RGBA ---> nColor:ARGB*/
static unsigned int _vp_play_normal_view_convert_color_to_argb(char *szColorHex)
{
	unsigned int nColor = 0;
	char *szColor = g_strdup_printf("%s%6.6s", (szColorHex + 7), (szColorHex + 1));
	nColor = strtoul(szColor, NULL, 16);
	VideoLogInfo("szColorHex:%s, szColor: %s, nColor:%0x", szColorHex, szColor, nColor);
	VP_FREE(szColor);

	return nColor;
}

/* internal functions */
static bool _vp_play_normal_view_set_subtitle_font_config(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	if (!pNormalView->pPlayerHandle) {
		VideoLogError("pNormalView pPlayerHandle is NULL");
		return FALSE;
	}

	/* set subtitle option */
	char *szFontFamilyName = NULL;
	int nSubtitleSize = VIDEO_SUBTITLE_SIZE_MEDIUM;
	int nSubtitleSizeValue = VP_SUBTITLE_SIZE_MEDIUM_VALUE;
	unsigned int nFontColor = 0;
	unsigned int nBGColor = 0;
	int nAlignment = VIDEO_SUBTITLE_ALIGNMENT_CENTER;

	vp_play_preference_get_subtitle_font_name_key(&szFontFamilyName);
	vp_play_preference_get_subtitle_size_key(&nSubtitleSize);

	if (nSubtitleSize == VIDEO_SUBTITLE_SIZE_SMALL) {
		nSubtitleSizeValue = VP_SUBTITLE_SIZE_SMALL_VALUE;
	} else if (nSubtitleSize == VIDEO_SUBTITLE_SIZE_MEDIUM) {
		nSubtitleSizeValue = VP_SUBTITLE_SIZE_MEDIUM_VALUE;
	} else if (nSubtitleSize == VIDEO_SUBTITLE_SIZE_LARGE) {
		nSubtitleSizeValue = VP_SUBTITLE_SIZE_LARGE_VALUE;
	}
#ifndef SUBTITLE_K_FEATURE
	int nFontColorKey = VP_SUBTITLE_COLOR_BLACK;
	int nBGColorKey = VP_SUBTITLE_COLOR_BLACK;

	vp_play_preference_get_subtitle_font_color_key(&nFontColorKey);

	if (nFontColorKey == VIDEO_SUBTITLE_COLOR_BLACK) {
		nFontColor = 0xff000000;
	} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_BLUE) {
		nFontColor = 0xff0000ff;
	} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_GREEN) {
		nFontColor = 0xff00ff00;
	} else if (nFontColorKey == VIDEO_SUBTITLE_COLOR_WHITE) {
		nFontColor = 0xffffffff;
	}

	vp_play_preference_get_subtitle_bg_color_key(&nBGColorKey);

	if (nBGColorKey == VIDEO_SUBTITLE_COLOR_BLACK) {
		nBGColor = 0xff000000;
	} else if (nBGColorKey == VIDEO_SUBTITLE_COLOR_WHITE) {
		nBGColor = 0xffffffff;
	} else if (nBGColorKey == VIDEO_SUBTITLE_COLOR_NONE) {
		nBGColor = 0x00000000;
	}

#else
	char *szColorHex = NULL;
	vp_play_preference_get_subtitle_font_color_hex_key(&szColorHex);
	if (szColorHex != NULL) {
		nFontColor = _vp_play_normal_view_convert_color_to_argb(szColorHex);
	}

	char *szBGColorHex = NULL;
	vp_play_preference_get_subtitle_bg_color_hex_key(&szBGColorHex);
	if (szBGColorHex != NULL)  {
		nBGColor = _vp_play_normal_view_convert_color_to_argb(szBGColorHex);

	}

	vp_play_preference_get_subtitle_alignment_key(&nAlignment);

#endif
	if (!vp_mm_player_set_subtitle_font(pNormalView->pPlayerHandle, szFontFamilyName, NULL, nSubtitleSizeValue)) {
		VideoLogError("vp_mm_player_set_subtitle_font fail");
	}
	if (!vp_mm_player_set_subtitle_font_color(pNormalView->pPlayerHandle, nFontColor, nBGColor)) {
		VideoLogError("vp_mm_player_set_subtitle_font_color fail");
	}
	VP_FREE(szFontFamilyName);

	if (!vp_mm_player_subtitle_ignore_markup_tags(pNormalView->pPlayerHandle, FALSE)) {
		VideoLogError("vp_mm_player_subtitle_ignore_markup_tags fail");
	}

	if (!vp_mm_player_subtitle_set_alignment_in_line(pNormalView->pPlayerHandle, nAlignment)) {
		VideoLogError("vp_mm_player_subtitle_set_alignment_in_line fail");
	}

	if (!vp_mm_player_subtitle_set_alignment_horizontal(pNormalView->pPlayerHandle, nAlignment)) {
		VideoLogError("vp_mm_player_subtitle_set_alignment_horizontal fail");
	}

	if (!vp_mm_player_subtitle_apply_alignment_right_away(pNormalView->pPlayerHandle)) {
		VideoLogError("vp_mm_player_subtitle_apply_alignment_right_away fail");
	}

	return TRUE;
}

static bool _vp_play_normal_view_update_progress_value(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	int nPosition = 0;
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (pNormalView->bHLSMode) {
		VideoLogWarning("Skip for HLS");
		VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);
		return FALSE;
	}

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}
	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
	}
	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		int nBufferingPosition = 0;
		vp_mm_player_get_buffering_position(pNormalView->pPlayerHandle, &nBufferingPosition);
		vp_play_progressbar_set_buffering_position(pNormalView->pProgressbar, nBufferingPosition);
	}
	if (nState == VP_MM_PLAYER_STATE_PLAYING) {
		nPosition += 1000;
	}
	if (nPosition > pNormalView->nDuration) {
		nPosition = pNormalView->nDuration;
	}
	vp_play_progressbar_set_position(pNormalView->pProgressbar, nPosition);

	pNormalView->nCurPosition = nPosition;

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		PlayView *pPlayView = pNormalView->pPlayView;
		pPlayView->nStartPosition = nPosition;
	}

	return TRUE;

}

static bool _vp_play_normal_view_check_during_call(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	PlayView *pPlayView = pNormalView->pPlayView;
	bool bCallOn = FALSE;

	if(pPlayView->telinit) {
		vp_play_config_get_call_state(&bCallOn);
	} else {
		VideoLogError("telephony was not initialised hence returning call state as default =  FALSE");
	}
	if (bCallOn) {
		VP_EVAS_DEL(pNormalView->pPopup);
		pNormalView->pPopup = NULL;

		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			return FALSE;
		}
		pNormalView->pPopup = vp_popup_create(pPlayView->pWin,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      NULL,
		                                      VP_PLAY_STRING_UNABLE_TO_PLAY_VIDEO_DURING_CALL,
		                                      2.0, __vp_normal_popup_time_out_cb,
		                                      __vp_normal_popup_key_event_cb,
		                                      __vp_normal_popup_mouse_event_cb,
		                                      pNormalView);

		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
	}

	return bCallOn;
}

static void _vp_play_normal_view_show_audio_only_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	VideoLogInfo("Show toast popup: Only audio data supported by this file. Now playing video.");
	_vp_normal_set_audio_only_image(pNormalView, TRUE);
	vp_play_util_status_noti_show(VP_PLAY_STRING_ERROR_ONLY_AUDIO_DATA_SUPPORTED);
}

static void _vp_play_normal_view_show_video_only_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	VideoLogInfo("Show toast popup: Only video data supported by this file. Now playing video.");
	_vp_normal_set_audio_only_image(pNormalView, FALSE);
	vp_play_util_status_noti_show(VP_PLAY_STRING_ERROR_ONLY_VIDEO_DATA_SUPPORTED);
}

static bool _vp_play_normal_view_check_audio_only(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	/* Audio only */
	char *audio_codec = NULL;
	char *video_codec = NULL;
	bool b_audio_only = FALSE;
	bool b_video_only = FALSE;
	if (vp_mm_player_get_codecinfo(pNormalView->pPlayerHandle, &audio_codec, &video_codec)) {
		VideoLogWarning("audio_codec=%s, video_codec=%s", audio_codec, video_codec);
		if (video_codec == NULL || strlen(video_codec) <= 1) {
			if (audio_codec != NULL && strlen(audio_codec) > 1) {
				b_audio_only = TRUE;
			}
		}

		if (audio_codec == NULL || strlen(audio_codec) <= 1) {
			if (video_codec != NULL && strlen(video_codec) > 1) {
				b_video_only = TRUE;
			}
		}

		VP_FREE(audio_codec);
		VP_FREE(video_codec);
	}

	VideoLogWarning("b_audio_only=%d, b_video_only=%d", b_audio_only, b_video_only);

	if (b_audio_only) {
		VideoLogInfo("Only audio data supported.");
		_vp_play_normal_view_show_audio_only_popup(pNormalView);
	} else {
		_vp_normal_set_audio_only_image(pNormalView, FALSE);
	}

	if (b_video_only) {
		VideoLogInfo("Only video data supported.");
		_vp_play_normal_view_show_video_only_popup(pNormalView);
	}

	return TRUE;
}


static bool _vp_play_normal_view_check_drm_lock(NormalView *pNormalView, bool *bLock)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}


	if (!pNormalView->szMediaURL) {
		return FALSE;
	}

	*bLock = FALSE;
#ifdef ENABLE_DRM_FEATURE
	bool bIsDrm = FALSE;

	if (vp_drm_is_drm_file(pNormalView->szMediaURL, &bIsDrm)) {
		if (bIsDrm) {
			bool bIsFWLock = FALSE;
			bool bIsDivX = FALSE;
			bool bIsPlayReadyDRM = FALSE;

			if (vp_drm_is_check_forward_lock(pNormalView->szMediaURL, &bIsFWLock)) {
				if (bIsFWLock) {
					*bLock = TRUE;
				}
			}
			if (vp_drm_is_divx_drm_file(pNormalView->szMediaURL, &bIsDivX)) {
				if (bIsDivX) {
					*bLock = TRUE;
				}
			}

			if (vp_drm_is_playready_drm_file(pNormalView->szMediaURL, &bIsPlayReadyDRM)) {
				if (bIsPlayReadyDRM) {
					*bLock = TRUE;
				}
			}
		}
	}
#endif
	return TRUE;
}

#ifdef ENABLE_DRM_FEATURE
static bool _vp_play_normal_view_check_drm(NormalView *pNormalView, bool *bIsAvailablePlay)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return FALSE;
	}

	*bIsAvailablePlay = TRUE;

	bool bIsDrm = FALSE;
	//bool bIsLicense = FALSE;
	bool bIsDivXDRM = FALSE;

	if (!vp_drm_is_drm_file(pNormalView->szMediaURL, &bIsDrm)) {
		VideoLogError("vp_drm_is_drm_file is fail");
		return FALSE;
	}

	if (bIsDrm == FALSE) {
		VideoLogWarning("Not DRM File");
		return TRUE;
	} else {
		VideoLogWarning("== IS DRM FILE ================================");
	}

	if (pNormalView->pPlayView->bStoreDownload == TRUE) {
		bool	bCheckLicense	= FALSE;
		if (!vp_drm_is_check_license(pNormalView->szMediaURL, &bCheckLicense)) {
			VideoLogError("vp_drm_is_check_license is fail");

			*bIsAvailablePlay = FALSE;

			return FALSE;
		}

		if (bCheckLicense == FALSE) {
			if (!vp_store_license_acquisition(pNormalView->szMediaURL,
			                                  pNormalView->pPlayView->pStoreAppId,
			                                  pNormalView->pPlayView->pStoreOrderId,
			                                  pNormalView->pPlayView->pStoreMvId,
			                                  pNormalView->pPlayView->pStoreServerId,
			                                  _vp_play_normal_view_store_la_cb,
			                                  (const void*)pNormalView)) {
				VideoLogError("vp_store_license_acquisition is fail");

				*bIsAvailablePlay = FALSE;

				return FALSE;
			}

			return FALSE;
		}
	}

	if (!vp_drm_is_divx_drm_file(pNormalView->szMediaURL, &bIsDivXDRM)) {
		VideoLogError("vp_drm_is_divx_drm_file is fail");
		return FALSE;
	}

	if (bIsDivXDRM == FALSE) {
		return TRUE;

		if (!vp_drm_is_check_license(pNormalView->szMediaURL, &bIsLicense)) {
			VideoLogError("vp_drm_is_check_license is fail");
			*bIsAvailablePlay = FALSE;
			return FALSE;
		}


		if (bIsLicense == FALSE) {
			VideoLogWarning("bIsLicense is Invalid");

			*bIsAvailablePlay = FALSE;


			VP_EVAS_DEL(pNormalView->pPopup);
			pNormalView->pPopup = NULL;

			char *szMessage = NULL;
			char *szTitleTemp = NULL;

			szTitleTemp = vp_play_util_get_title_from_path(pNormalView->szMediaURL);
			szMessage = g_strdup_printf(VP_PLAY_STRING_DRM_CURRENTLY_LOCKED_UNLOCK_Q, szTitleTemp);

			pNormalView->pPopup = vp_popup_create(pPlayView->pWin, POPUP_STYLE_DEFAULT,
			                                      VP_PLAY_STRING_COM_ERROR,
			                                      szMessage,
			                                      3.0, __vp_normal_error_popup_time_out_cb,
			                                      NULL, NULL, pNormalView);
			evas_object_show(pNormalView->pPopup);
			pNormalView->bIsPopupShow = TRUE;

			VP_FREE(szMessage);
			VP_FREE(szTitleTemp);

			return TRUE;
		}
	}

	vp_drm_constraint_info stConstraintInfo = {0,};

	if (!vp_drm_get_constarint_info(pNormalView->szMediaURL, &stConstraintInfo)) {
		VideoLogError("vp_drm_get_constarint_info is fail");
		return FALSE;
	}

	char *szMsg = NULL;
	char *szTitle = NULL;
	szTitle = vp_play_util_get_title_from_path(pNormalView->szMediaURL);

	if (bIsDivXDRM) {
		if (stConstraintInfo.status == VP_DRM_CONSTRAINT_TYPE_NOT_AUTHORIZED) {
			VP_EVAS_DEL(pNormalView->pPopup);
			pNormalView->pPopup = NULL;
			pNormalView->pPopup = vp_popup_create(pPlayView->pWin, POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
			                                      VP_PLAY_STRING_COM_ERROR,
			                                      VP_PLAY_STRING_DIVX_DRM_AUTHORIZATION_ERROR,
			                                      3.0, __vp_normal_error_popup_time_out_cb,
			                                      NULL, NULL, pNormalView);

			evas_object_show(pNormalView->pPopup);
			pNormalView->bIsPopupShow = TRUE;

			VP_FREE(szTitle);

			return TRUE;
		}

		while (stConstraintInfo.constraints) {
			if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_COUNT) {
				if (stConstraintInfo.remaining_count == 0) {
					szMsg = g_strdup_printf(VP_PLAY_STRING_DIVX_DRM_RENTAL_EXPIRED_ERROR, stConstraintInfo.original_count - stConstraintInfo.remaining_count, stConstraintInfo.original_count);
				} else {
					szMsg = g_strdup_printf(VP_PLAY_STRING_DIVX_DRM_RENTAL_CONFIRMATION_CHOICE, stConstraintInfo.original_count - stConstraintInfo.remaining_count, stConstraintInfo.original_count);
				}
				break;
			}
		}
	} else {
		bool bNotiPopup = FALSE;

		while (stConstraintInfo.constraints) {
			if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_COUNT) {
				if (stConstraintInfo.remaining_count == 2) {
					szMsg = g_strdup_printf(VP_PLAY_STRING_DRM_PS_2_MORE_TIME_START_NOW_Q, szTitle);
				} else if (stConstraintInfo.remaining_count == 1) {
					szMsg = g_strdup_printf(VP_PLAY_STRING_DRM_PS_1_MORE_TIME_START_NOW_Q, szTitle);
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
					int nDays = stConstraintInfo.remaining_interval_sec / VP_ACCUMULATED_DATE + 1;
					szMsg = g_strdup_printf(VP_PLAY_STRING_DRM_PS_FOR_PD_DAYS_START_NOW_Q, szTitle, nDays);
				}
				break;
			}

			if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_TIMED_COUNT) {
				if (stConstraintInfo.remaining_timed_count == 2) {
					szMsg = g_strdup_printf(VP_PLAY_STRING_DRM_PS_2_MORE_TIME_START_NOW_Q, szTitle);
				} else if (stConstraintInfo.remaining_timed_count == 1) {
					szMsg = g_strdup_printf(VP_PLAY_STRING_DRM_PS_1_MORE_TIME_START_NOW_Q, szTitle);
				} else if (stConstraintInfo.remaining_timed_count == 0) {
					bNotiPopup = TRUE;
				}
				break;
			}

			if (stConstraintInfo.constraints & VP_DRM_CONSTRAINT_ACCUMLATED_TIME) {
				if (stConstraintInfo.remaining_acc_sec == 0) {
					bNotiPopup = TRUE;
				}
				break;
			}
		}
	}

	VP_FREE(szMsg);
	VP_FREE(szTitle);

	return TRUE;
}
#endif

#ifndef ENABLE_SUBTITLE

static void _vp_play_normal_view_free_subtitle_list(GList *pSubtitleList)
{
	if (pSubtitleList) {
		int nCount = 0;
		int i = 0;
		nCount = g_list_length(pSubtitleList);
		for (i = 0; i < nCount ; i++) {
			char *szName = NULL;
			szName = (char *)g_list_nth_data(pSubtitleList, i);
			VP_FREE(szName);
		}

		g_list_free(pSubtitleList);
	}
}
#endif

#ifdef _NATIVE_BUFFER_SYNC
static Evas_Object *_vp_play_normal_view_create_image_sink(void *pParent, void *pUserData)
{

	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return NULL;
	}

//	NormalView *pNormalView = (NormalView *)pUserData;
	Evas *pEvas = NULL;
	Evas_Object *pObj = NULL;

	pEvas = evas_object_evas_get(pParent);

	pObj = evas_object_image_add(pEvas);
	if (NULL == pObj) {
		VideoLogError("pObj is NULL");
		return NULL;
	}

	evas_object_image_size_set(pObj, VP_NORMAL_DEFAULT_WIDTH, VP_NORMAL_DEFAULT_HEIGHT);
	evas_object_image_fill_set(pObj, 0, 0, VP_NORMAL_DEFAULT_WIDTH, VP_NORMAL_DEFAULT_HEIGHT);
	evas_object_resize(pObj, VP_NORMAL_DEFAULT_WIDTH, VP_NORMAL_DEFAULT_HEIGHT);

	void *pImageBuf = evas_object_image_data_get(pObj, EINA_TRUE);
	if (NULL == pImageBuf) {
		VideoLogError("pImageBuf is NULL");
		VP_EVAS_DEL(pObj);
		return NULL;
	}

	int nBufSize = VP_NORMAL_DEFAULT_WIDTH * VP_NORMAL_DEFAULT_HEIGHT * 4;

	memset(pImageBuf, 0, nBufSize);
	evas_object_image_data_set(pObj, pImageBuf);

//	evas_object_event_callback_add(pObj, EVAS_CALLBACK_RESIZE,
//					__vp_play_multi_view_imagesink_resize_cb, (void *)pNormalView);

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_SINK, pObj);

	return pObj;

}
#endif

bool vp_play_normal_view_play_start(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;
	_vp_play_normal_view_play_start(pNormalView, FALSE);
	return TRUE;
}

static bool _vp_play_normal_view_play_start(NormalView *pNormalView, bool bCheckDRM)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}
	VideoLogWarning(">>");

	if (pNormalView->bIsActive == FALSE) {
		VideoLogError("pNormalView is NOT realized. skip");
		return FALSE;
	}


	VP_EVAS_TIMER_DEL(pNormalView->pSubtitleTimer);
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogWarning("pPlayView is NULL");
		return FALSE;
	}
	if (pNormalView->pZoomGuide) {
		vp_zoom_guide_destroy(pNormalView->pZoomGuide);
		pNormalView->pZoomGuide = NULL;
	}

	if (pNormalView->pImageBufferObj) {
		elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_IMAGE_BUFFER);
		VP_EVAS_DEL(pNormalView->pImageBufferObj);
	}

	pNormalView->fZoomValue = 1.0;
	pNormalView->bIsVideoUnsupport = FALSE;
	pNormalView->bBufferingComplete = TRUE;
	pNormalView->bCaptureComplete = TRUE;
	pNormalView->bDeviceRemoveInterrupt = FALSE;

	_vp_play_normal_view_all_close_popup(pNormalView);

	VP_FREE(pPlayView->szMediaURL);
	VP_STRDUP(pPlayView->szMediaURL, pNormalView->szMediaURL);
	VideoSecureLogInfo("== PLAY START : %s ==", pNormalView->szMediaURL);

#ifdef _NATIVE_BUFFER_SYNC
	VP_EVAS_DEL(pNormalView->pVideoSink);
	pNormalView->pVideoSink = _vp_play_normal_view_create_image_sink(pNormalView->pMainLayout, (void *)pNormalView);
	if (!pNormalView->pVideoSink) {
		VideoLogError("_vp_play_normal_view_create_image_sink handle is null");
		return FALSE;
	}
#endif

	pNormalView->pSubtitleTimer = NULL;
	pNormalView->bIsExistSubtitle = FALSE;
	pNormalView->fPlaySpeed = pNormalView->pPlayView->fPlaySpeed;

	VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);

	if (pNormalView->pDetailHandle) {
		vp_detail_destroy(pNormalView->pDetailHandle);
		pNormalView->pDetailHandle = NULL;
	}

	if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {

		if (!vp_play_util_local_file_check(pNormalView->szMediaURL)) {
			/* popup show */
			VP_EVAS_DEL(pNormalView->pPopup);
			pNormalView->pPopup = NULL;
			PlayView *pPlayView = pNormalView->pPlayView;
			if (!pPlayView) {
				VideoLogError("pPlayView is NULL");
				return FALSE;
			}
			pNormalView->pPopup = vp_popup_create(pPlayView->pWin, POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
			                                      VP_PLAY_STRING_COM_ERROR,
			                                      VP_PLAY_STRING_ERROR_FILE_NOT_EXIST,
			                                      3.0, __vp_normal_error_popup_time_out_cb,
			                                      NULL, NULL, pNormalView);

			evas_object_show(pNormalView->pPopup);
			pNormalView->bIsPopupShow = TRUE;

			return TRUE;
		}
	}

	if (bCheckDRM) {
		if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
		        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {
			bool bIsAvailablePlay = TRUE;
#ifdef ENABLE_DRM_FEATURE
			if (_vp_play_normal_view_check_drm(pNormalView, &bIsAvailablePlay)) {
				if (bIsAvailablePlay == FALSE) {
					VideoLogWarning("Wait Drm popup result");
					return TRUE;
				}
			} else
#endif
			{
				if (pNormalView->pPlayView->bStoreDownload == TRUE) {
					if (bIsAvailablePlay == FALSE) {
						VideoLogWarning("license acquisition failed!!!");

						Evas_Object *pPopup = NULL;
						pPopup = vp_popup_create(pNormalView->pPlayView->pWin,
						                         POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
						                         VP_PLAY_STRING_COM_ERROR,
						                         VP_PLAY_STRING_ERROR_UNABLE_PLAY,
						                         3.0, __vp_normal_prepare_error_popup_time_out_cb,
						                         NULL, NULL, pNormalView);

						evas_object_show(pPopup);

						pNormalView->bIsPopupShow = TRUE;

						return FALSE;
					} else {
						VideoLogWarning("need license acquisition");
						return FALSE;
					}
				}
			}
		}
	}

	if (pNormalView->nStartPosition == 0) {
		if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_STORE ||
		        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_LIST ||
		        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_OTHER) {
			int nStartPos = 0;
			if (vp_media_contents_get_played_position(pNormalView->szMediaURL, &nStartPos)) {
				pNormalView->nStartPosition = nStartPos;
			}
		}
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_STORE ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_LIST ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_OTHER) {
		vp_media_contents_set_played_time(pNormalView->szMediaURL);
	}

	vp_play_subtitle_set_text(pNormalView->pSubtitle, NULL);

	if (bCheckDRM) {
		if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
		        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {

			bool bDRMLock = FALSE;
			_vp_play_normal_view_check_drm_lock(pNormalView, &bDRMLock);
			if (bDRMLock) {
				VP_EVAS_DISABLE_SET(pNormalView->pBookmarkBtn, EINA_TRUE);
				VP_EVAS_DISABLE_SET(pNormalView->pMultiWinBtn, EINA_TRUE);
			} else {
				VP_EVAS_DISABLE_SET(pNormalView->pBookmarkBtn, EINA_FALSE);
				VP_EVAS_DISABLE_SET(pNormalView->pMultiWinBtn, EINA_FALSE);
			}
		} else {
			if (pNormalView->bVideoOnlyMode == FALSE) {
				VP_EVAS_DISABLE_SET(pNormalView->pMultiWinBtn, EINA_FALSE);
			} else {
				VP_EVAS_DISABLE_SET(pNormalView->pMultiWinBtn, EINA_TRUE);
			}
		}
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL) {
		char *szMediaID = NULL;
		bool bReturn = FALSE;
		bReturn = vp_media_contents_get_video_id(pNormalView->szMediaURL, &szMediaID);
		if (!szMediaID || bReturn == FALSE) {
			VP_EVAS_DISABLE_SET(pNormalView->pTrimBtn, EINA_TRUE);
		} else {
			VP_EVAS_DISABLE_SET(pNormalView->pTrimBtn, EINA_FALSE);
		}
		VP_FREE(szMediaID);
	}
	char *szSubtitle = NULL;
	vp_mm_player_get_subtitle_url(pNormalView->pPlayerHandle, &szSubtitle);

	if (szSubtitle) {
		VideoLogWarning("subtitle value : %s", szSubtitle);
		VP_STRDUP(pNormalView->szSubtitleURL, szSubtitle);
		VP_FREE(szSubtitle);
	}
	if (pNormalView->pPlayerHandle) {
		vp_mm_player_destroy(pNormalView->pPlayerHandle);
		pNormalView->pPlayerHandle = NULL;
	}

	if (pNormalView->pLoadingAni) {
		VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
		VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
	}

	/* mm_player create */
	if (!vp_mm_player_set_hw_decoder(TRUE)) {
		VideoLogError("vp_mm_player_set_user_param fail");
		_vp_play_normal_view_destroy_handle(pNormalView);
		return FALSE;
	}

	pNormalView->pPlayerHandle = vp_mm_player_create();
	if (!pNormalView->pPlayerHandle) {
		VideoLogError("vp_mm_player_create fail");
		return FALSE;
	}
	MMPlayer *handle = (MMPlayer *)pNormalView->pPlayerHandle;
	int nRet;
	nRet = player_set_audio_policy_info(handle->pPlayer, pNormalView->pPlayView->stream_info);
	if (nRet != PLAYER_ERROR_NONE) {
	        VideoLogError("unable to set sound policy [%x]", nRet);
	}

	if (!vp_mm_player_set_stream_info(pNormalView->pPlayerHandle, (void *)(pNormalView->pPlayView->stream_info))) {
		VideoLogError("vp_mm_player_set_stream_info fail");
		return FALSE;
	}
	if (!vp_mm_player_set_user_param(pNormalView->pPlayerHandle, (void *) pNormalView)) {
		VideoLogError("vp_mm_player_set_user_param fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_REALIZE_ASYNC_CB, (void *)__vp_normal_prepare_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_SEEK_COMPLETE_CB, (void *)__vp_normal_seek_completed_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_END_OF_STREAM_CB, (void *)__vp_normal_completed_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_INTERRUPT_CB, (void *)__vp_normal_interrupted_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_ERROR_CB, (void *)__vp_normal_error_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_BUFFERING_CB, (void *)__vp_normal_buffering_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_SUBTITLE_UPDATE_CB, (void *)__vp_normal_subtitle_updated_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_PD_MESSAGE_CB, (void *)__vp_normal_pd_message_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}
	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_MISSED_PLUGIN_CB, (void *)__vp_normal_missed_plugin_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (!vp_mm_player_set_callback(pNormalView->pPlayerHandle, VP_MM_PLAYER_IMAGE_BUFFER_CB, (void *)__vp_normal_image_buffer_cb)) {
		VideoLogError("vp_mm_player_set_callback fail");
		return FALSE;
	}

	if (pNormalView->pBookmarkHandle) {
		vp_play_bookmark_set_media_url(pNormalView->pBookmarkHandle, pNormalView->szMediaURL);
	}

	if (pNormalView->szSubtitleURL) {
		pNormalView->bIsExistSubtitle = vp_mm_player_set_subtitle_url(pNormalView->pPlayerHandle, pNormalView->szSubtitleURL);
		bool bOn = FALSE;
		vp_play_preference_get_subtitle_show_key(&bOn);
		if (bOn) {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_PORTRAIT, "*");
			pNormalView->bIsExistSubtitle = TRUE;
			vp_mm_player_set_deactivate_subtitle(pNormalView->pPlayerHandle, FALSE);
		}

	}

#ifdef _NATIVE_BUFFER_SYNC
	if (!vp_mm_player_set_video_sink(pNormalView->pPlayerHandle,
	                                 VP_MM_PLAYER_VIDEO_TYPE_EVAS,
	                                 (void *)pNormalView->pVideoSink)) {
		VideoLogError("vp_mm_player_set_video_sink fail");
		_vp_play_normal_view_destroy_handle(pNormalView);
		return FALSE;
	}
#else
	//if (!vp_mm_player_set_video_sink(pNormalView->pPlayerHandle, VP_MM_PLAYER_VIDEO_TYPE_X11, (void *)GET_DISPLAY(pNormalView->pPlayView->pWin))) {
	if (!vp_mm_player_set_video_sink(pNormalView->pPlayerHandle, VP_MM_PLAYER_VIDEO_TYPE_X11, (void *)(pNormalView->pPlayView->pWin))) {
		VideoLogError("vp_mm_player_set_video_sink fail");
		return FALSE;
	}
#endif

	if (!vp_mm_player_set_hub_download_mode(pNormalView->pPlayerHandle, pPlayView->bStoreDownload)) {
		VideoLogError("vp_mm_player_set_hub_download_mode fail");
	}

	if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_NONE) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_0);
	} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_90);
	} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_180) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_180);
	} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_270);
	}

	vp_mm_player_sound_filter_t nSoundFilter = VP_MM_PLAYER_FILTER_NONE;

	if (pNormalView->nSoundAlive == VIDEO_SA_NORMAL) {
		nSoundFilter = VP_MM_PLAYER_FILTER_NONE;
	} else if (pNormalView->nSoundAlive == VIDEO_SA_VOICE) {
		nSoundFilter = VP_MM_PLAYER_FILTER_VOICE;
	} else if (pNormalView->nSoundAlive == VIDEO_SA_MOVIE) {
		nSoundFilter = VP_MM_PLAYER_FILTER_MOVIE;
	} else if (pNormalView->nSoundAlive == VIDEO_SA_7_1_CH) {
		nSoundFilter = VP_MM_PLAYER_FILTER_VITUAL_71;

		video_sound_device_type_t nSoundDevType = VP_SOUND_DEVICE_NONE;
		vp_sound_get_active_device(&nSoundDevType);

		if (nSoundDevType != VP_SOUND_DEVICE_EARJACK) {
			nSoundFilter = VP_MM_PLAYER_FILTER_NONE;
		}
	}

	vp_mm_player_set_display_mode(pNormalView->pPlayerHandle, pNormalView->nDisplayMode);

	if (nSoundFilter != VP_MM_PLAYER_FILTER_NONE) {
		if (!vp_mm_player_set_sound_filter(pNormalView->pPlayerHandle, nSoundFilter)) {
			VideoLogWarning("vp_mm_player_set_sound_filter is fail");
		}
	}

	if (pNormalView->nDefaultAudioTrackIndex > 0) {
		vp_mm_player_set_audio_track(pNormalView->pPlayerHandle, pNormalView->nDefaultAudioTrackIndex);
	}

	if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {
		if (pPlayView->szMediaTitle) {
			elm_object_item_text_set(pNormalView->pNaviItem, pPlayView->szMediaTitle);
		} else {
			char *szTitle = NULL;
			szTitle = vp_play_util_get_title_from_path(pNormalView->szMediaURL);
			if (szTitle) {
				elm_object_item_text_set(pNormalView->pNaviItem, szTitle);
			}
			VP_FREE(szTitle);
		}
		if (!vp_mm_player_realize_async(pNormalView->pPlayerHandle, pNormalView->szMediaURL)) {
			VideoLogError("vp_mm_player_realize_async fail");

			//_vp_play_normal_view_show_layout(pNormalView);
			Evas_Object *pPopup = NULL;
			pPopup = vp_popup_create(pPlayView->pWin,
			                         POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
			                         VP_PLAY_STRING_COM_ERROR,
			                         VP_PLAY_STRING_ERROR_UNABLE_PLAY,
			                         3.0, __vp_normal_prepare_error_popup_time_out_cb,
			                         NULL, NULL, pNormalView);


			evas_object_show(pPopup);

			pNormalView->bIsPopupShow = TRUE;

			return TRUE;
		}
	} else {
		if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
			VideoLogInfo("VIDEO_PLAY_TYPE_WEB:%s", pNormalView->szMediaURL);
			if (pPlayView->szMediaTitle) {
				elm_object_item_text_set(pNormalView->pNaviItem, pPlayView->szMediaTitle);
			} else {
				elm_object_item_text_set(pNormalView->pNaviItem, VP_PLAY_STRING_STREAMING_PLAYER);
			}
			if (pPlayView->szCookie) {
				if (!vp_mm_player_set_cookie(pNormalView->pPlayerHandle, pPlayView->szCookie)) {
					VideoLogError("vp_mm_player_set_cookie fail");
				}
			}
			if (pPlayView->szProxy) {
				if (!vp_mm_player_set_proxy(pNormalView->pPlayerHandle, pPlayView->szProxy)) {
					VideoLogError("vp_mm_player_set_proxy fail");
				}
			}
			char *szSdpPath = NULL;
			szSdpPath = vp_play_util_get_sdp_url((const char *)pNormalView->szMediaURL);
			if (vp_play_util_check_sdp_url((const char *)szSdpPath)) {
				//_vp_play_normal_view_download_sdp(pNormalView, pNormalView->szMediaURL);
				//_vp_play_normal_view_show_layout(pNormalView);

				return TRUE;
			}
			VP_FREE(szSdpPath);

			if (!vp_mm_player_realize_async(pNormalView->pPlayerHandle, pNormalView->szMediaURL)) {
				VideoLogError("vp_mm_player_realize_async fail");
				//_vp_play_normal_view_show_layout(pNormalView);
				Evas_Object *pPopup = NULL;
				pPopup = vp_popup_create(pPlayView->pWin,
				                         POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
				                         VP_PLAY_STRING_COM_ERROR,
				                         VP_PLAY_STRING_ERROR_UNABLE_PLAY,
				                         3.0, __vp_normal_prepare_error_popup_time_out_cb,
				                         NULL, NULL, pNormalView);

				evas_object_show(pPopup);

				pNormalView->bIsPopupShow = TRUE;

				return TRUE;
			}
		} else if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {

			char *szTitle = NULL;
			char *szMultiPathURL = NULL;
			char *szSubtitleForMultiPath = NULL;
			int nMultiPathPosition = 0;
			int nMultiPathDuration = 0;
			bool bIsSameAP = TRUE;


			vp_multi_path_get_current_item(pNormalView->szMediaURL, &szMultiPathURL, &szTitle, &szSubtitleForMultiPath, &nMultiPathPosition, &nMultiPathDuration, &bIsSameAP, pPlayView->pMultiPathList);
			if (szSubtitleForMultiPath) {
				if (strlen(szSubtitleForMultiPath) > 0) {
					if (vp_file_exists(szSubtitleForMultiPath)) {
						pNormalView->bIsExistSubtitle = TRUE;
						VideoSecureLogInfo("SUBTITLE : %s", szSubtitleForMultiPath);
						vp_mm_player_set_subtitle_url(pNormalView->pPlayerHandle, szSubtitleForMultiPath);
						VP_FREE(pNormalView->szSubtitleURL);
					} else {
						pNormalView->pPopup = vp_popup_create(pPlayView->pWin,
						                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
						                                      VP_PLAY_STRING_COM_ERROR,
						                                      VP_PLAY_STRING_ERROR_SUBTITLE_FAIL,
						                                      3.0, __vp_normal_error_popup_time_out_cb,
						                                      __vp_normal_error_popup_key_event_cb,
						                                      __vp_normal_error_popup_mouse_event_cb,
						                                      pNormalView);
						evas_object_show(pNormalView->pPopup);
						pNormalView->bIsPopupShow = TRUE;
					}
				}
			}

			if (!szTitle) {
				szTitle = vp_play_util_get_title_from_path(pNormalView->szMediaURL);
			}

			if (szTitle) {
				elm_object_item_text_set(pNormalView->pNaviItem, szTitle);
			} else {
				elm_object_item_text_set(pNormalView->pNaviItem, VP_PLAY_STRING_NO_TITLE);
			}

			if (pNormalView->nStartPosition == 0) {
				pNormalView->nStartPosition = nMultiPathPosition;
			}

			if (nMultiPathDuration > 0) {
				pNormalView->nDuration = nMultiPathDuration;
			}

			VP_FREE(szMultiPathURL);
			VP_FREE(szSubtitleForMultiPath);
			VP_FREE(szTitle);

			if (!vp_mm_player_realize_async(pNormalView->pPlayerHandle, pNormalView->szMediaURL)) {
				VideoLogError("vp_mm_player_realize_async fail");
				Evas_Object *pPopup = NULL;
				pPopup = vp_popup_create(pPlayView->pWin,
				                         POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
				                         VP_PLAY_STRING_COM_ERROR,
				                         VP_PLAY_STRING_ERROR_UNABLE_PLAY,
				                         3.0, __vp_normal_prepare_error_popup_time_out_cb,
				                         NULL, NULL, pNormalView);

				evas_object_show(pPopup);

				pNormalView->bIsPopupShow = TRUE;

				return TRUE;
			}
		}
	}
	//_vp_play_normal_view_show_layout(pNormalView);
	vp_play_util_set_lock_power_key();

	VideoLogWarning("<<");

	return TRUE;
}


static void _vp_play_normal_view_on_next_play(NormalView *pNormalView, bool bManual)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW) {
		return;
	}

	bool bIsExit = FALSE;

	pNormalView->nStartPosition = 0;
	pNormalView->nDefaultAudioTrackIndex = 0;
	pNormalView->nSpeedValue = 1;

	_vp_play_normal_view_on_bookmark_mode(pNormalView, FALSE);

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_NONE) {
			if (bManual) {
				return;
			} else {
				bIsExit = TRUE;
			}
		} else {
			_vp_play_normal_view_play_start(pNormalView, FALSE);
		}
	} else if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		PlayView *pPlayView = pNormalView->pPlayView;

		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			bIsExit = TRUE;
		} else if (!pPlayView->pMultiPathList) {
			VideoLogError("pMultiPathList is NULL");
			bIsExit = TRUE;
		} else {
			if (bManual) {
				_vp_play_normal_view_set_played_time(pNormalView);

				char *szNextURL = NULL;
				char *szSubtitle = NULL;
				int nPosition = 0;
				int nDuration = 0;
				vp_multi_path_get_next_item(pNormalView->szMediaURL, &szNextURL, &szSubtitle, &nPosition, &nDuration, TRUE, pPlayView->pMultiPathList);
				VP_FREE(szSubtitle);
				if (szNextURL) {
					VP_FREE(pNormalView->szMediaURL);
					VP_STRDUP(pNormalView->szMediaURL, szNextURL);
					VP_FREE(szNextURL);
					pNormalView->nStartPosition = nPosition;
					pNormalView->nDuration = nDuration;
					_vp_play_normal_view_play_start(pNormalView, TRUE);
				} else {
					_vp_play_normal_view_play_start(pNormalView, TRUE);
				}
				return;
			}

			if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_NONE) {
				bIsExit = TRUE;
			} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_OFF) {
				bIsExit = TRUE;
			} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL_STOP) {
				char *szNextURL = NULL;
				char *szSubtitle = NULL;
				int nPosition = 0;
				int nDuration = 0;
				vp_multi_path_get_next_item(pNormalView->szMediaURL, &szNextURL, &szSubtitle, &nPosition, &nDuration, FALSE, pPlayView->pMultiPathList);

				VP_FREE(szSubtitle);

				if (!szNextURL) {
					bIsExit = TRUE;
				} else {
					VP_FREE(pNormalView->szMediaURL);
					VP_STRDUP(pNormalView->szMediaURL, szNextURL);
					VP_FREE(szNextURL);
					pNormalView->nDuration = nDuration;
					_vp_play_normal_view_play_start(pNormalView, TRUE);
				}
			} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE) {
				_vp_play_normal_view_play_start(pNormalView, TRUE);
			} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
				char *szNextURL = NULL;
				char *szSubtitle = NULL;
				int nPosition = 0;
				int nDuration = 0;
				vp_multi_path_get_next_item(pNormalView->szMediaURL, &szNextURL, &szSubtitle, &nPosition, &nDuration, TRUE, pPlayView->pMultiPathList);
				VP_FREE(szSubtitle);
				if (!szNextURL) {
					bIsExit = TRUE;
				} else {
					VP_FREE(pNormalView->szMediaURL);
					VP_STRDUP(pNormalView->szMediaURL, szNextURL);
					VP_FREE(szNextURL);
					pNormalView->nDuration = nDuration;
					_vp_play_normal_view_play_start(pNormalView, TRUE);
				}
			}
		}
	} else {
		if (bManual) {
			_vp_play_normal_view_set_played_time(pNormalView);
			char *szNextURL = NULL;
			vp_media_contents_get_next_file_path(pNormalView->szMediaURL, &szNextURL, TRUE, pNormalView->pMediaItemList);

			VideoSecureLogInfo("pNormalView->szMediaURL is %s", pNormalView->szMediaURL);
			if (szNextURL) {
				VP_FREE(pNormalView->szMediaURL);
				VP_STRDUP(pNormalView->szMediaURL, szNextURL);
				VP_FREE(szNextURL);
				_vp_play_normal_view_play_start(pNormalView, TRUE);
				vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
				if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
					VideoLogWarning("vp_mm_player_get_state is fail");
				}
				if (nState != VP_MM_PLAYER_STATE_PLAYING) {
					vp_mm_player_play(pNormalView->pPlayerHandle);
					pNormalView->bManualPause = FALSE;
				}
				_vp_play_normal_view_set_play_state(pNormalView);
				_vp_play_normal_view_on_capture_mode(pNormalView);
			} else {
				_vp_play_normal_view_play_start(pNormalView, TRUE);
			}
			return;
		}

		if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_NONE) {
			bIsExit = TRUE;
		} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_OFF) {
			bIsExit = TRUE;
		} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL_STOP) {
			char *szNextURL = NULL;
			vp_media_contents_get_next_file_path(pNormalView->szMediaURL, &szNextURL, FALSE, pNormalView->pMediaItemList);
			if (!szNextURL) {
				bIsExit = TRUE;
			} else {
				VP_FREE(pNormalView->szMediaURL);
				VP_STRDUP(pNormalView->szMediaURL, szNextURL);
				VP_FREE(szNextURL);
				_vp_play_normal_view_play_start(pNormalView, TRUE);
			}
		} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ONE) {
			_vp_play_normal_view_play_start(pNormalView, TRUE);
		} else if (pNormalView->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
			char *szNextURL = NULL;
			vp_media_contents_get_next_file_path(pNormalView->szMediaURL, &szNextURL, TRUE, pNormalView->pMediaItemList);
			if (!szNextURL) {
				bIsExit = TRUE;
			} else {
				VP_FREE(pNormalView->szMediaURL);
				VP_STRDUP(pNormalView->szMediaURL, szNextURL);
				VP_FREE(szNextURL);
				_vp_play_normal_view_play_start(pNormalView, TRUE);
			}
		}
	}

	if (bIsExit) {
		if (!pPlayView->pFunc) {
			VideoLogError("pPlayView pFunc is NULL");
			return;
		}

		if (!pPlayView->pFunc->vp_play_func_exit) {
			VideoLogError("pPlayView exit Func is NULL");
			return;
		}
		vp_device_set_screen_off(FALSE);
		vp_play_util_set_unlock_power_key();

		pPlayView->pFunc->vp_play_func_exit(pPlayView);
	}
}

static void _vp_play_normal_view_on_prev_play(NormalView *pNormalView, bool bManual, bool bVoice)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}
	PlayView *pPlayView = pNormalView->pPlayView;

	pNormalView->nDefaultAudioTrackIndex = 0;
	pNormalView->nSpeedValue = 1;

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		_vp_play_normal_view_play_start(pNormalView, FALSE);
		return;
	}

	int nPosition = 0;
	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
	}

	pNormalView->nStartPosition = 0;
	if (bVoice == FALSE) {
		if (nPosition > 2000) {
			vp_mm_player_set_position(pNormalView->pPlayerHandle, 0);

			if (pNormalView->bManualPause) {
				vp_play_progressbar_set_position(pNormalView->pProgressbar, 0);
			}
			return;
		}
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MESSAGE ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_PREVIEW ||
	        (pPlayView && (strstr(pPlayView->szMediaURL, "/.") != NULL))) {
		return;
	}

	if (bManual) {
		_vp_play_normal_view_set_played_time(pNormalView);
	}

	char *szPrevURL = NULL;
	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		if (!pPlayView) {
			_vp_play_normal_view_play_start(pNormalView, TRUE);
			return;
		} else if (pPlayView->pMultiPathList == NULL) {
			_vp_play_normal_view_play_start(pNormalView, TRUE);
			return;
		} else {
			char *szSubtitle = NULL;
			int nPosition = 0;
			int nDuration = 0;
			vp_multi_path_get_next_item(pNormalView->szMediaURL, &szPrevURL, &szSubtitle, &nPosition, &nDuration, TRUE, pPlayView->pMultiPathList);
			VP_FREE(szSubtitle);

			if (szPrevURL) {
				VP_FREE(pNormalView->szMediaURL);
				VP_STRDUP(pNormalView->szMediaURL, szPrevURL);
				VP_FREE(szPrevURL);
				pNormalView->nDuration = nDuration;
			}

			_vp_play_normal_view_play_start(pNormalView, TRUE);
		}

	} else {
		vp_media_contents_get_prev_file_path(pNormalView->szMediaURL, &szPrevURL, TRUE, pNormalView->pMediaItemList);
		VideoSecureLogInfo("pNormalView->szMediaURL pre is %s", pNormalView->szMediaURL);
		if (szPrevURL) {
			VP_FREE(pNormalView->szMediaURL);
			VP_STRDUP(pNormalView->szMediaURL, szPrevURL);
			VP_FREE(szPrevURL);
		}

		_vp_play_normal_view_on_bookmark_mode(pNormalView, FALSE);
		_vp_play_normal_view_play_start(pNormalView, TRUE);
		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
		if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}
		if (nState != VP_MM_PLAYER_STATE_PLAYING) {
			vp_mm_player_play(pNormalView->pPlayerHandle);
			pNormalView->bManualPause = FALSE;
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
}

static void _vp_play_normal_view_set_played_time(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	//PlayView *pPlayView = pNormalView->pPlayView;
	int nPosition = 0;
	if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_PREVIEW &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MESSAGE &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_EMAIL &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {
		if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
			VideoLogError("vp_mm_player_get_position is fail");
		} else {
			if (nPosition == pNormalView->nDuration) {
				vp_media_contents_set_played_position(pNormalView->szMediaURL, 0);

			} else {
				vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
				if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
					VideoLogWarning("vp_mm_player_get_state is fail");
				}

				if (nState == VP_MM_PLAYER_STATE_PLAYING) {
					nPosition -= 5000;
				}

				if (nPosition < 0) {
					nPosition = 0;
				}
				vp_media_contents_set_played_position(pNormalView->szMediaURL, nPosition);
			}
		}
	} else if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
			VideoLogError("vp_mm_player_get_position is fail");
		} else {
			PlayView *pPlayView = pNormalView->pPlayView;
			if (!pPlayView) {
				VideoLogError("pPlayView is NULL");
				return;
			}
			if (!pPlayView->pMultiPathList) {
				VideoLogError("pMultiPathList is NULL");
				return;
			}
			if (nPosition >= pNormalView->nDuration) {
				nPosition = 0;
			}
			vp_multi_path_set_item_position(pNormalView->szMediaURL, nPosition, pPlayView->pMultiPathList);
		}

	}
	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_LIST ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_OTHER) {
		bool bIsCloud = FALSE;
		vp_media_contents_get_cloud_attribute(pNormalView->szMediaURL, &bIsCloud);
		if (!bIsCloud) {
			PlayView *pPlayView = pNormalView->pPlayView;
			if (!pPlayView) {
				VideoLogError("pPlayView is NULL");
				return;
			}
			if (pPlayView->bViewChange == FALSE) {
				if (vp_file_exists(pNormalView->szMediaURL)) {
					vp_play_config_set_preview_url_videos(pNormalView->szMediaURL);
				}
			}
			vp_play_preference_set_preview_audio_track(pNormalView->nDefaultAudioTrackIndex);
		}
	}
}
/*
static void _vp_play_normal_view_screen_capture(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	vp_mm_player_get_position(pNormalView->pPlayerHandle, &(pNormalView->nCapturePosition));

	if (pNormalView->bCaptureComplete == FALSE) {
		VideoLogWarning("Not Complete capture");
		return;
	}

	pNormalView->bCaptureComplete = FALSE;
	pNormalView->bCaptureBookmark = FALSE;

	if (!vp_mm_player_capture_start(pNormalView->pPlayerHandle)) {
		pNormalView->bCaptureComplete = TRUE;
		VideoLogError("vp_mm_player_capture_start fail");
	}

}
*/

static void _vp_play_normal_view_screen_move(NormalView *pNormalView, bool bFoward)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	int nSetPosition = 0;
	int nCurPosition = 0;

	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nCurPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return;
	}


	if (bFoward) {
		nSetPosition = nCurPosition  + 1000;
	} else {
		nSetPosition = nCurPosition  - 1000;
	}

	if (pNormalView->nDuration < nSetPosition) {
		nSetPosition = pNormalView->nDuration;
	}

	if (nSetPosition < 0) {
		nSetPosition = 0;
	}

	if (vp_mm_player_set_position(pNormalView->pPlayerHandle, nSetPosition)) {
		pNormalView->nWaitPos = -1;
		pNormalView->bSeekComplete = FALSE;
		vp_play_progressbar_set_position(pNormalView->pProgressbar, nSetPosition);
	}
}

#ifdef ENABLE_GESTURE_ZOOM_FEATURE
static void _vp_play_normal_view_on_zoom_mode(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (!pNormalView->pPlayerHandle) {
		VideoLogWarning("Player Handle is NULL");
		return;
	}

	if (vp_play_util_check_streaming(pNormalView->szMediaURL)) {
		VideoLogWarning("Skip for streaming");
		return;
	}

	if (pNormalView->fZoomValue > 1.0) {
		pNormalView->bZoomEnable = TRUE;
		if (!pNormalView->pZoomGuide) {
			VP_EVAS_TIMER_DEL(pNormalView->pMomentTimer);

			VP_EVAS_JOB_DEL(pNormalView->pVolumeJob);
			VP_EVAS_JOB_DEL(pNormalView->pBrightnessJob);

			if (!vp_play_brightness_unrealize(pNormalView->pBrightnessHandle)) {
				VideoLogWarning("vp_play_brightness_unrealize is fail");
			}

			if (!vp_play_volume_popup_unrealize(pNormalView->pVolumePopupHandle)) {
				VideoLogWarning("vp_play_volume_popup_unrealize is fail");
			}

			pNormalView->pZoomGuide = vp_zoom_guide_create(pNormalView->pMainLayout);
			vp_zoom_guide_realize(pNormalView->pZoomGuide);
			Evas_Object *pZoomGuideLayout = vp_zoom_guide_get_object(pNormalView->pZoomGuide);
			if (pZoomGuideLayout) {
				elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_ZOOM_GUIDE, pZoomGuideLayout);
			} else {
				VideoLogError("pZoomGuideLayout is NULL");
			}

			Evas_Coord_Rectangle rtSrc = {0,};
			Evas_Coord_Rectangle rtDst = {0,};
			Evas_Coord_Rectangle rtResult = {0,};
			int nWidth = 0;
			int nHeight = 0;

			elm_win_screen_size_get(pNormalView->pPlayView->pWin, NULL, NULL, &nWidth, &nHeight);

			if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
				rtDst.w = nHeight;
				rtDst.h = nWidth;
			} else {
				rtDst.w = nWidth;
				rtDst.h = nHeight;
			}
			vp_mm_player_get_resolution(pNormalView->pPlayerHandle, &(rtSrc.w), &(rtSrc.h));
			vp_play_util_calculator_position(rtSrc, rtDst, &rtResult, pNormalView->nDisplayMode);
			vp_zoom_guide_set_move_position(pNormalView->pZoomGuide, (double)pNormalView->nZoomPosX / rtDst.w, (double)pNormalView->nZoomPosY / rtDst.h);
			float fPosX = pNormalView->nZoomPosX / rtResult.w;
			float fPosY = pNormalView->nZoomPosY / rtResult.h;
			if (fPosX > 1.0) {
				fPosX = 1.0;
			}

			if (fPosY > 1.0) {
				fPosY = 1.0;
			}

			if (fPosX < 0.0) {
				fPosX = 0.0;
			}

			if (fPosY < 0.0) {
				fPosY = 0.0;
			}
		}
		// event filtering
		bool bLandscape = FALSE;
		if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90 || pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
			bLandscape = TRUE;
		}
		if (bLandscape && fabs(pNormalView->fPrevZoomValue - pNormalView->fZoomValue) < 0.05) {
//			VideoLogWarning("same ZoomValue. skip it");
			return;
		}

		vp_zoom_guide_set_zoom_value(pNormalView->pZoomGuide, pNormalView->fZoomValue);
		pNormalView->fPrevZoomValue = pNormalView->fZoomValue;
	} else {
		pNormalView->fZoomValue = 1.0;
		pNormalView->bZoomEnable = FALSE;
		vp_zoom_guide_set_zoom_value(pNormalView->pZoomGuide, pNormalView->fZoomValue);
		elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_ZOOM_GUIDE);

		//VideoLogError("ZoomValue : [%f], offset_x : [%d], offset_y : [%d]", pNormalView->fZoomValue, 0, 0);
		vp_mm_player_set_zoom(pNormalView->pPlayerHandle, pNormalView->fZoomValue, 0, 0);

		vp_zoom_guide_destroy(pNormalView->pZoomGuide);
		pNormalView->pZoomGuide = NULL;
	}
}
#endif


static void _vp_play_normal_view_on_capture_mode(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	Evas_Object *pParent = NULL;
	pParent = pNormalView->pTopControlLayout;
	if (!pParent) {
		VideoLogError("pParent is NULL");
		return;
	}
	vp_play_preference_get_capture_on_key(&pNormalView->bCaptureMode);

	elm_object_part_content_unset(pParent, VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE);
	elm_object_part_content_unset(pParent, VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE_REW);
	elm_object_part_content_unset(pParent, VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE_FF);

	VP_EVAS_DEL(pNormalView->pCaptureBtn);
	VP_EVAS_DEL(pNormalView->pCaptureRewBtn);
	VP_EVAS_DEL(pNormalView->pCaptureFFBtn);

	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	if (!pNormalView->szMediaURL) {
		return;
	}

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		return;
	}

	if (pNormalView->bCaptureMode) {
		pNormalView->pCaptureBtn = vp_button_create(
		                               pNormalView->pNaviFrame, "custom/flat_63_63/default",
		                               NULL,
		                               (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
		                               (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
		                               (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
		                               (void *)pNormalView);
		Evas_Object *pIcon = NULL;
		pIcon = vp_button_create_icon(pNormalView->pCaptureBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE);
		elm_object_part_content_set(pNormalView->pCaptureBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pCaptureBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_PRESS);
		elm_object_part_content_set(pNormalView->pCaptureBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pCaptureBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_DIM);
		elm_object_part_content_set(pNormalView->pCaptureBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE, pNormalView->pCaptureBtn);

		evas_object_show(pNormalView->pCaptureBtn);

		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
		if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}
		if (nState == VP_MM_PLAYER_STATE_PAUSED ||
		        (nState != VP_MM_PLAYER_STATE_PLAYING && pNormalView->bManualPause)) {
			pNormalView->pCaptureRewBtn = vp_button_create(
			                                  pNormalView->pNaviFrame, "custom/flat_63_63/default",
			                                  NULL,
			                                  (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
			                                  (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
			                                  (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
			                                  (void *)pNormalView);

			pNormalView->pCaptureFFBtn = vp_button_create(
			                                 pNormalView->pNaviFrame, "custom/flat_63_63/default",
			                                 NULL,
			                                 (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
			                                 (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
			                                 (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
			                                 (void *)pNormalView);

			pIcon = vp_button_create_icon(pNormalView->pCaptureRewBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_REW);
			elm_object_part_content_set(pNormalView->pCaptureRewBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pCaptureRewBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_REW_PRESS);
			elm_object_part_content_set(pNormalView->pCaptureRewBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pCaptureRewBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_REW_DIM);
			elm_object_part_content_set(pNormalView->pCaptureRewBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);


			pIcon = vp_button_create_icon(pNormalView->pCaptureFFBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_FF);
			elm_object_part_content_set(pNormalView->pCaptureFFBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pCaptureFFBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_FF_PRESS);
			elm_object_part_content_set(pNormalView->pCaptureFFBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pCaptureFFBtn, edj_path, VP_PLAY_NORMAL_RES_CAPTURE_FF_DIM);
			elm_object_part_content_set(pNormalView->pCaptureFFBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

			elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE_REW, pNormalView->pCaptureRewBtn);
			elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE_FF, pNormalView->pCaptureFFBtn);

			evas_object_show(pNormalView->pCaptureRewBtn);
			evas_object_show(pNormalView->pCaptureFFBtn);
		}

	}
}

static void _vp_play_normal_view_on_share_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}
	pPlayView->bIsSharePopup = TRUE;
	if(nState != VP_MM_PLAYER_STATE_PLAYING)
	{
		pPlayView->bIsPlayBeforeShare = false;
	}
	else
	{
		pPlayView->bIsPlayBeforeShare = true;
	}

	bool bDRMLock = FALSE;

	_vp_play_normal_view_check_drm_lock(pNormalView, &bDRMLock);
	if (bDRMLock) {
		VP_EVAS_DEL(pNormalView->pPopup);
		pNormalView->pPopup = NULL;

		pNormalView->pPopup = vp_popup_create(pPlayView->pWin,
		                                      POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
		                                      VP_PLAY_STRING_COM_ERROR,
		                                      VP_PLAY_STRING_DRM_UNABLE_TO_SHARE_DRM_FILE,
		                                      3.0, __vp_normal_popup_time_out_cb,
		                                      NULL, NULL, pNormalView);

		evas_object_show(pNormalView->pPopup);
		pNormalView->bIsPopupShow = TRUE;
		return;
	}

	if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
		VideoLogWarning("Pause Fail");
	}
	_vp_play_normal_view_set_play_state(pNormalView);

	pNormalView->bSharePanelPause = FALSE;
	pNormalView->bSharePanelFocusIn = FALSE;
	VideoLogInfo("share popup: bSharePanelPause = %d", pNormalView->bManualPause);
	if (!pNormalView->bManualPause) {
		pNormalView->bSharePanelPause = TRUE;
		//pNormalView->bManualPause = TRUE;
	}

	if (vp_play_app_launch_share_panel(pPlayView->pWin, pNormalView->szMediaURL, (void *)pNormalView)) {
		pNormalView->bSharepopup = TRUE;

	} else {
		pNormalView->bSharepopup = FALSE;
		VideoLogError("Launch share app failed");
	}

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_sound_alive_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}

		_vp_play_normal_view_set_play_state(pNormalView);
	}

	vp_sound_alive_destroy(pNormalView->pSAHandle);

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	pNormalView->pSAHandle = vp_sound_alive_create(pPlayView->pWin,
	                         pNormalView->szMediaURL,
	                         __vp_normal_sound_alive_popup_close_cb,
	                         pNormalView->nSoundAlive);
	if (!pNormalView->pSAHandle) {
		VideoLogError("vp_sound_alive_create fail");
		return;
	}
	if (!vp_sound_alive_set_user_data(pNormalView->pSAHandle, (void *)pNormalView)) {
		VideoLogWarning("vp_share_set_user_data is fail");
	}

	if (!vp_sound_alive_realize(pNormalView->pSAHandle)) {
		VideoLogWarning("vp_share_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}


static void _vp_play_normal_view_on_sound_path_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	vp_sound_path_destroy(pNormalView->pSoundPathHandle);
	pNormalView->pSoundPathHandle = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
	}

	pNormalView->pSoundPathHandle = vp_sound_path_create(pPlayView->pWin,
	                                __vp_normal_sound_path_popup_close_cb);
	if (!pNormalView->pSoundPathHandle) {
		VideoLogError("vp_sound_path_create fail");
		return;
	}
	if (!vp_sound_path_set_user_data(pNormalView->pSoundPathHandle, (void *)pNormalView)) {
		VideoLogWarning("vp_sound_path_set_user_data is fail");
	}

	if (!vp_sound_path_realize(pNormalView->pSoundPathHandle)) {
		VideoLogWarning("vp_sound_path_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_audio_track_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);

	}

	vp_audio_track_destroy(pNormalView->pAudioTrackHandle);
	pNormalView->pAudioTrackHandle = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	pNormalView->pAudioTrackHandle = vp_audio_track_create(pPlayView->pWin,
	                                 __vp_normal_audio_track_popup_close_cb,
	                                 pNormalView->nDefaultAudioTrackIndex);
	if (!pNormalView->pAudioTrackHandle) {
		VideoLogError("vp_audio_track_create fail");
		return;
	}

	if (!vp_audio_track_set_user_data(pNormalView->pAudioTrackHandle, (void *)pNormalView)) {
		VideoLogWarning("vp_audio_track_set_user_data is fail");
	}

	int nCount = 0;
	if (vp_mm_player_get_audio_track_count(pNormalView->pPlayerHandle, &nCount)) {
		if (nCount > 1) {
			int nIdx = 0;
			for (nIdx = 0; nIdx < nCount; nIdx++) {
				char *szCode = NULL;

				if (vp_mm_player_get_audio_track_language_code(pNormalView->pPlayerHandle, nIdx, &szCode)) {
					vp_audio_track_add_Item(pNormalView->pAudioTrackHandle, szCode, nIdx);
				} else {
					vp_audio_track_add_Item(pNormalView->pAudioTrackHandle, VP_PLAY_STRING_COM_UNKNOWN, nIdx);
				}
				VP_FREE(szCode);
			}
		} else {
			char *szTrack = g_strdup_printf("%s %d", VP_PLAY_STRING_AUDIO_TRACK, 0);
			vp_audio_track_add_Item(pNormalView->pAudioTrackHandle, szTrack, 0);
			VP_FREE(szTrack);
		}
	} else {
		char *szTrack = g_strdup_printf("%s %d", VP_PLAY_STRING_AUDIO_TRACK, 0);
		vp_audio_track_add_Item(pNormalView->pAudioTrackHandle, szTrack, 0);
		VP_FREE(szTrack);
	}

	if (!vp_audio_track_realize(pNormalView->pAudioTrackHandle)) {
		VideoLogWarning("vp_audio_track_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_subtitle_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	SubtitleInfo *pSubtitleInfo = NULL;
	pSubtitleInfo = calloc(1, sizeof(SubtitleInfo));
	if (!pSubtitleInfo) {
		VideoLogError("pSubtitleInfo alloc fail");
		return;
	}
	bool bIsClosedCaption = FALSE;
	bIsClosedCaption = vp_mm_player_is_closed_caption_exist(pNormalView->pPlayerHandle);

	pSubtitleInfo->bClosedCaption = bIsClosedCaption;
	pSubtitleInfo->fSync = pNormalView->fSubtitleSyncValue;
	int nCount = 0;
	char *szSubtitle = NULL;

	if (vp_mm_player_get_subtitle_track_count(pNormalView->pPlayerHandle, &nCount)) {
		if (nCount > 0) {
			char *szCode = NULL;
#ifdef _SUBTITLE_MULTI_LANGUAGE
			if (pNormalView->pSelectedSubtitleLanguage && nCount == pNormalView->nSubtitleLanguageCount) {
				int index = 0;
				VP_FREE(pSubtitleInfo->szLanguage);
				for (; index < nCount; ++index) {
					if (pNormalView->pSelectedSubtitleLanguage[index]) {
						char *szName = NULL;
						vp_mm_player_get_subtitle_track_language_code(pNormalView->pPlayerHandle, index, &szCode);
						vp_subtitle_track_get_name(szCode, &szName);
						if (!pSubtitleInfo->szLanguage) {
							pSubtitleInfo->szLanguage = g_strdup(szName);
						} else {
							char *szTmp =  g_strdup(pSubtitleInfo->szLanguage);
							VP_FREE(pSubtitleInfo->szLanguage);
							pSubtitleInfo->szLanguage = g_strdup_printf("%s,%s", szTmp, szName);
							VP_FREE(szTmp);
						}
						VP_FREE(szName);
						VP_FREE(szCode);
					}
				}
			} else
#endif
			{
				vp_mm_player_get_subtitle_track_language_code(pNormalView->pPlayerHandle, pNormalView->nDefaultSubtitleTrackIndex, &szCode);
				vp_subtitle_track_get_name(szCode, &(pSubtitleInfo->szLanguage));
				VP_FREE(szCode);
			}
		} else {
			pSubtitleInfo->szLanguage = g_strdup_printf("%s", VP_PLAY_STRING_COM_UNKNOWN);
		}
	}

	if (!pSubtitleInfo->szLanguage) {
		pSubtitleInfo->szLanguage = g_strdup_printf("%s", VP_PLAY_STRING_COM_UNKNOWN);
	}

	vp_play_preference_get_subtitle_font_name_key(&(pSubtitleInfo->szFontName));
	if (pSubtitleInfo->szFontName) {
		if (strcmp(pSubtitleInfo->szFontName, "Tizen") == 0) {
			VP_FREE(pSubtitleInfo->szFontName);
			pSubtitleInfo->szFontName = g_strdup_printf("%s", VP_PLAY_STRING_FONT_DEFAULT);
		}
	}
	vp_play_preference_get_subtitle_size_key(&(pSubtitleInfo->nFontSize));
	vp_play_preference_get_subtitle_edge_key(&(pSubtitleInfo->nEdge));

#ifndef SUBTITLE_K_FEATURE
	vp_play_preference_get_subtitle_font_color_key(&(pSubtitleInfo->nTextColor));
	vp_play_preference_get_subtitle_bg_color_key(&(pSubtitleInfo->nBGColor));
#else
	vp_play_preference_get_subtitle_font_color_hex_key(&(pSubtitleInfo->pTextColorHex));
	vp_play_preference_get_subtitle_bg_color_hex_key(&(pSubtitleInfo->pBGColorHex));
	vp_play_preference_get_subtitle_caption_win_color_hex_key(&(pSubtitleInfo->pCaptionWinColorHex));

	int nAlignment = VIDEO_SUBTITLE_ALIGNMENT_CENTER;
	vp_play_preference_get_subtitle_alignment_key(&nAlignment);
	pSubtitleInfo->eAlignment = nAlignment;
#endif
	vp_mm_player_get_subtitle_url(pNormalView->pPlayerHandle, &szSubtitle);
	if (szSubtitle) {
		VP_STRDUP(pSubtitleInfo->szURL, szSubtitle);
	}

	VP_FREE(szSubtitle);

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		pSubtitleInfo->bSupportSelect = FALSE;
	} else {
		pSubtitleInfo->bSupportSelect = TRUE;
	}
	vp_play_subtitle_get_text(pNormalView->pSubtitle, &pSubtitleInfo->szText);
	vp_play_subtitle_get_size_zoom(pNormalView->pSubtitle, &pSubtitleInfo->fZoom);

	if (pNormalView->pSubtitlePopup) {
		VideoLogWarning("pSubtitlePopup has been created");
		vp_subtitle_update(pNormalView->pSubtitlePopup, pSubtitleInfo);
	} else {
		pNormalView->pSubtitlePopup = vp_subtitle_create(pPlayView->pWin, pSubtitleInfo,
		                              __vp_normal_subtitle_popup_close_cb);

		if (!pNormalView->pSubtitlePopup) {
			VideoLogError("vp_subtitle_create fail");

			VP_FREE(pSubtitleInfo->szURL);
			VP_FREE(pSubtitleInfo->szText);
			VP_FREE(pSubtitleInfo->szLanguage);
			VP_FREE(pSubtitleInfo->szFontName);

			VP_FREE(pSubtitleInfo);
			return;
		}
		if (!vp_subtitle_set_user_data(pNormalView->pSubtitlePopup, (void *)pNormalView)) {
			VideoLogWarning("vp_subtitle_set_user_data is fail");
		}

		if (!vp_subtitle_realize(pNormalView->pSubtitlePopup)) {
			VideoLogWarning("vp_subtitle_realize is fail");
		}
	}

	VP_FREE(pSubtitleInfo->szLanguage);
	VP_FREE(pSubtitleInfo->szFontName);
	VP_FREE(pSubtitleInfo->szText);
	VP_FREE(pSubtitleInfo->szURL);
	VP_FREE(pSubtitleInfo);

	pNormalView->bIsSubtitleShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_subtitle_sync_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	pNormalView->pSubtitleSyncPopup = vp_subtitle_sync_popup_create(pPlayView->pWin,
	                                  pNormalView->fSubtitleSyncValue,
	                                  __vp_normal_subtitle_sync_popup_update_cb,
	                                  __vp_normal_subtitle_sync_popup_done_cb);

	if (!pNormalView->pSubtitleSyncPopup) {
		VideoLogError("vp_subtitle_sync_popup_create fail");
		return;
	}
	if (!vp_subtitle_sync_popup_set_user_data(pNormalView->pSubtitleSyncPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_sync_popup_set_user_data is fail");
	}

	if (!vp_subtitle_sync_popup_realize(pNormalView->pSubtitleSyncPopup)) {
		VideoLogWarning("vp_subtitle_sync_popup_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}
#ifdef _SUBTITLE_MULTI_LANGUAGE
/*static bool _vp_play_normal_view_selected_subtitle_language_cb(int nTrackNum, void *pUserData)
{
       VideoLogInfo("nTrackNum is %d", nTrackNum);
       NormalView *pNormalView = (NormalView *)pUserData;
       if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	if (!pNormalView->pPlayerHandle) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (nTrackNum >= 0)
		pNormalView->pSelectedSubtitleLanguage[nTrackNum] = true;

	return TRUE;
}*/

static void _vp_play_normal_view_selected_subtitle_language(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (!pNormalView->pPlayerHandle) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	VP_FREE(pNormalView->pSelectedSubtitleLanguage);
	pNormalView->pSelectedSubtitleLanguage = calloc(pNormalView->nSubtitleLanguageCount, sizeof(int));

//	if (!vp_mm_player_track_foreach_selected_subtitle_language(pNormalView->pPlayerHandle,
//								_vp_play_normal_view_selected_subtitle_language_cb,
//								pNormalView)) {
//		VideoLogError("vp_mm_player_track_foreach_selected_subtitle_language failed");
//		return;
//	}
}

static void _vp_play_normal_view_subtitle_add_language_cb(int index, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	if (!pNormalView->pPlayerHandle) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	if (!pNormalView->pSelectedSubtitleLanguage) {
		VideoLogError("pSelectedSubtitleLanguage is NULL");
		return;
	}

	if (index < 0 || index > pNormalView->nSubtitleLanguageCount) {
		VideoLogError("Invalid index");
		return;
	}

	if (pNormalView->pSelectedSubtitleLanguage[index]) {
		VideoLogWarning("index:%d has been added", index);
	} else {
		if (!vp_mm_player_add_subtitle_language(pNormalView->pPlayerHandle, index)) {
			VideoLogError("vp_mm_player_add_subtitle_language:%d failed", index);
		} else {
			VideoLogInfo("vp_mm_player_add_subtitle_language:%d successed", index);
			pNormalView->pSelectedSubtitleLanguage[index] = true;
		}
	}
}

static void _vp_play_normal_view_subtitle_remove_language_cb(int index, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pUserData;
	if (!pNormalView->pPlayerHandle) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	if (!pNormalView->pSelectedSubtitleLanguage) {
		VideoLogError("pSelectedSubtitleLanguage is NULL");
		return;
	}

	if (index < 0 || index > pNormalView->nSubtitleLanguageCount) {
		VideoLogError("Invalid index");
		return;
	}

	if (pNormalView->pSelectedSubtitleLanguage[index]) {
		if (!vp_mm_player_remove_subtitle_language(pNormalView->pPlayerHandle, index)) {
			VideoLogError("vp_mm_player_remove_subtitle_language:%d failed", index);
		} else {
			VideoLogInfo("vp_mm_player_remove_subtitle_language:%d successed", index);
			pNormalView->pSelectedSubtitleLanguage[index] = false;
		}
	} else {
		VideoLogWarning("index:%d has not been selected", index);
	}
}

#endif
static void _vp_play_normal_view_on_subtitle_language_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);

	}

	vp_subtitle_track_destroy(pNormalView->pSubtitleTrackHandle);
	pNormalView->pSubtitleTrackHandle = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
#ifdef _SUBTITLE_MULTI_LANGUAGE
	pNormalView->pSubtitleTrackHandle = vp_subtitle_track_create(pPlayView->pWin,
	                                    __vp_normal_subtitle_track_popup_close_cb);
#else
	pNormalView->pSubtitleTrackHandle = vp_subtitle_track_create(pPlayView->pWin,
	                                    __vp_normal_subtitle_track_popup_close_cb,
	                                    pNormalView->nDefaultSubtitleTrackIndex);
#endif
	if (!pNormalView->pSubtitleTrackHandle) {
		VideoLogError("vp_subtitle_track_create fail");
		return;
	}

	if (!vp_subtitle_track_set_user_data(pNormalView->pSubtitleTrackHandle, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_track_set_user_data is fail");
	}

	int nCount = 0;
	if (vp_mm_player_get_subtitle_track_count(pNormalView->pPlayerHandle, &nCount)) {
		VideoLogInfo("== TRACK : %d ==", nCount);
		if (nCount > 0) {
#ifdef _SUBTITLE_MULTI_LANGUAGE
			pNormalView->nSubtitleLanguageCount = nCount;
			_vp_play_normal_view_selected_subtitle_language(pNormalView);
#endif
			int nIdx = 0;
			for (nIdx = 0; nIdx < nCount; nIdx++) {
				char *szCode = NULL;
				if (vp_mm_player_get_subtitle_track_language_code(pNormalView->pPlayerHandle, nIdx, &szCode)) {
					VideoLogWarning("== language : %s ==", szCode);
#ifdef _SUBTITLE_MULTI_LANGUAGE
					vp_subtitle_track_add_Item(pNormalView->pSubtitleTrackHandle, szCode, nIdx, pNormalView->pSelectedSubtitleLanguage[nIdx]);
#else
					vp_subtitle_track_add_Item(pNormalView->pSubtitleTrackHandle, szCode, nIdx);
#endif
				}
				VP_FREE(szCode);
			}
#ifdef _SUBTITLE_MULTI_LANGUAGE
			vp_subtitle_track_set_add_language_cb(pNormalView->pSubtitleTrackHandle, _vp_play_normal_view_subtitle_add_language_cb);
			vp_subtitle_track_set_remove_language_cb(pNormalView->pSubtitleTrackHandle, _vp_play_normal_view_subtitle_remove_language_cb);
#endif
		} else {
			char *szTrack = g_strdup_printf("%s %d", VP_PLAY_STRING_COM_DEFAULT, 0);
#ifdef _SUBTITLE_MULTI_LANGUAGE
			vp_subtitle_track_add_Item(pNormalView->pSubtitleTrackHandle, szTrack, 0, true);
#else
			vp_subtitle_track_add_Item(pNormalView->pSubtitleTrackHandle, szTrack, 0);
#endif
			VP_FREE(szTrack);
		}
	} else {
		char *szTrack = g_strdup_printf("%s %d", VP_PLAY_STRING_COM_DEFAULT, 0);
#ifdef _SUBTITLE_MULTI_LANGUAGE
		vp_subtitle_track_add_Item(pNormalView->pSubtitleTrackHandle, szTrack, 0, true);
#else
		vp_subtitle_track_add_Item(pNormalView->pSubtitleTrackHandle, szTrack, 0);
#endif
		VP_FREE(szTrack);
	}

	if (!vp_subtitle_track_realize(pNormalView->pSubtitleTrackHandle)) {
		VideoLogWarning("vp_subtitle_track_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
}

#ifdef SUBTITLE_K_FEATURE
static void __vp_normal_subtitle_alignment_popup_done_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_alignment_destroy(pNormalView->pSubtitleAlignmentPopup);
	pNormalView->pSubtitleAlignmentPopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (nType != -1) {

		vp_subtitle_alignment_t nAlignment = VP_SUBTITLE_ALIGNMENT_CENTER;
		if (nType == VIDEO_SUBTITLE_ALIGNMENT_LEFT) {
			nAlignment = VP_SUBTITLE_ALIGNMENT_LEFT;
		} else if (nType == VIDEO_SUBTITLE_ALIGNMENT_CENTER) {
			nAlignment = VP_SUBTITLE_ALIGNMENT_CENTER;
		} else if (nType == VIDEO_SUBTITLE_ALIGNMENT_RIGHT) {
			nAlignment = VP_SUBTITLE_ALIGNMENT_RIGHT;
		}

		vp_play_subtitle_set_alignment(pNormalView->pSubtitle, nAlignment);

		vp_play_preference_set_subtitle_alignment_key(nType);

		_vp_play_normal_view_set_subtitle_font_config(pNormalView);

		/*Set caption window: size, rel1, color*/
		__vp_normal_subtitle_set_caption_window(pNormalView);
	}

	_vp_play_normal_view_on_subtitle_popup(pNormalView);

	/*if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}*/
}

static void _vp_play_normal_view_on_subtitle_alignment_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	int nAlignment = 0;
	vp_play_preference_get_subtitle_alignment_key(&nAlignment);

	pNormalView->pSubtitleAlignmentPopup = vp_subtitle_alignment_create(pPlayView->pWin,
	                                       __vp_normal_subtitle_alignment_popup_done_cb,
	                                       nAlignment);

	if (!pNormalView->pSubtitleAlignmentPopup) {
		VideoLogError("vp_subtitle_alignment_create fail");
		return;
	}
	if (!vp_subtitle_alignment_set_user_data(pNormalView->pSubtitleAlignmentPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_alignment_set_user_data is fail");
	}

	if (!vp_subtitle_alignment_realize(pNormalView->pSubtitleAlignmentPopup)) {
		VideoLogWarning("vp_subtitle_alignment_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
}
#endif

static void _vp_play_normal_view_on_subtitle_select_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	char *szSubtitle = NULL;
	GList *pItemList = NULL;
	char *szDir = NULL;
	szDir = vp_play_util_get_folder_from_path(pNormalView->szMediaURL);
#ifdef ENABLE_SUBTITLE
	bool bIsExistSubtitlesInFolder = FALSE;
//	bIsExistSubtitlesInFolder = vp_play_util_is_exist_subtitle_from_path(szDir);
	bIsExistSubtitlesInFolder = vp_play_util_get_subtitle_path_list(&pItemList);
	if (bIsExistSubtitlesInFolder && pItemList) {

		if (g_list_length(pItemList) > 0) {
			bIsExistSubtitlesInFolder = TRUE;
		} else {
			g_list_free(pItemList);
			pItemList = NULL;
		}

	} else {
		VideoLogInfo("bIsExistSubtitlesInFolder && pItemList failed");
		bIsExistSubtitlesInFolder = FALSE;
	}

	/*if (bIsExistSubtitlesInFolder == FALSE) {
		VP_EVAS_DEL(pNormalView->pPopup);

		pNormalView->bIsPopupShow = TRUE;
		pNormalView->pPopup = vp_popup_create(pPlayView->pWin, POPUP_STYLE_DEFAULT_WITH_CANCEL_BTN,
							NULL,
							VP_PLAY_STRING_ERROR_NO_SUBTITLE,
							0.0, NULL,
							__vp_normal_popup_key_event_cb,
							__vp_normal_popup_mouse_event_cb,
							pNormalView);

		Evas_Object *pBtn = NULL;
		pBtn = elm_button_add(pNormalView->pPopup);
		elm_object_style_set (pBtn, "popup_button/default");
		elm_object_domain_translatable_text_set(pBtn, VP_SYS_STR_PREFIX, VP_PLAY_STRING_COM_OK_IDS);
		elm_object_part_content_set(pNormalView->pPopup, "button1", pBtn);
		evas_object_smart_callback_add(pBtn, "clicked", __vp_normal_subtitle_popup_key_event_cb, (void*)pNormalView);
		evas_object_show(pNormalView->pPopup);

		VP_FREE(szDir);

		return;
	}*/
#endif
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);

	}

#ifdef ENABLE_SUBTITLE
	//vp_play_util_get_subtitles_from_path(szDir, &pItemList);
#else
	vp_play_util_get_subtitle_path_list(&pItemList);
#endif
	VP_FREE(szDir);
	vp_mm_player_get_subtitle_url(pNormalView->pPlayerHandle, &szSubtitle);
	pNormalView->pSubtitleSelectPopup = vp_subtitle_select_create(pPlayView->pWin, __vp_normal_subtitle_select_popup_done_cb, __vp_normal_subtitle_select_button_popup_done_cb,
	                                    pItemList, szSubtitle, !(pNormalView->bIsExistSubtitle));
	VP_FREE(szSubtitle);
	if (pItemList) {

		int nCount = 0;
		int i = 0;
		nCount = g_list_length(pItemList);
		for (i = 0; i < nCount; i++) {
			char *szName = NULL;
			szName = (char *)g_list_nth_data(pItemList, i);
			VP_FREE(szName);
		}

		g_list_free(pItemList);
		pItemList = NULL;
	}

	if (!pNormalView->pSubtitleSelectPopup) {
		VideoLogError("vp_subtitle_select_create fail");
		return;
	}
	if (!vp_subtitle_select_set_user_data(pNormalView->pSubtitleSelectPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_select_set_user_data is fail");
	}

	if (!vp_subtitle_select_realize(pNormalView->pSubtitleSelectPopup)) {
		VideoLogWarning("vp_subtitle_select_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_subtitle_font_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	char *szFont = NULL;
	vp_play_preference_get_subtitle_font_name_key(&szFont);

	pNormalView->pSubtitleFontPopup = vp_subtitle_font_create(pPlayView->pWin,
	                                  __vp_normal_subtitle_font_popup_done_cb,
	                                  szFont);
	VP_FREE(szFont);

	if (!pNormalView->pSubtitleFontPopup) {
		VideoLogError("vp_subtitle_font_create fail");
		return;
	}
	if (!vp_subtitle_font_set_user_data(pNormalView->pSubtitleFontPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_font_set_user_data is fail");
	}

	if (!vp_subtitle_font_realize(pNormalView->pSubtitleFontPopup)) {
		VideoLogWarning("vp_subtitle_font_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);


}


static void _vp_play_normal_view_on_subtitle_size_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	int nSize = 0;
	vp_play_preference_get_subtitle_size_key(&nSize);

	pNormalView->pSubtitleSizePopup = vp_subtitle_size_create(pPlayView->pWin,
	                                  __vp_normal_subtitle_size_popup_done_cb,
	                                  nSize);

	if (!pNormalView->pSubtitleSizePopup) {
		VideoLogError("vp_subtitle_size_create fail");
		return;
	}
	if (!vp_subtitle_size_set_user_data(pNormalView->pSubtitleSizePopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_size_set_user_data is fail");
	}

	if (!vp_subtitle_size_realize(pNormalView->pSubtitleSizePopup)) {
		VideoLogWarning("vp_subtitle_size_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void __vp_normal_subtitle_edge_popup_done_cb(int nType, bool bPause, void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	// to do
	NormalView *pNormalView = (NormalView *)pUserData;

	vp_subtitle_edge_destroy(pNormalView->pSubtitleEdgePopup);
	pNormalView->pSubtitleEdgePopup = NULL;
	pNormalView->bIsPopupShow = FALSE;

	if (nType != -1) {
		vp_play_subtitle_set_edge(pNormalView->pSubtitle, nType);
		vp_play_preference_set_subtitle_edge_key(nType);
		//_vp_play_normal_view_set_subtitle_font_config(pNormalView);
		/*Set caption window: size, rel1, color*/
		__vp_normal_subtitle_set_caption_window(pNormalView);
	}

	if (bPause) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
	}
	_vp_play_normal_view_on_subtitle_popup(pNormalView);

	/*if (pNormalView->bManualPause == FALSE) {
		if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
			VideoLogWarning("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
		_vp_play_normal_view_on_capture_mode(pNormalView);
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
	}*/
}

static void _vp_play_normal_view_on_subtitle_edge_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	int nEdge = 0;
	vp_play_preference_get_subtitle_edge_key(&nEdge);

	pNormalView->pSubtitleEdgePopup = vp_subtitle_edge_create(pPlayView->pWin,
	                                  __vp_normal_subtitle_edge_popup_done_cb,
	                                  nEdge);

	if (!pNormalView->pSubtitleEdgePopup) {
		VideoLogError("vp_subtitle_size_create fail");
		return;
	}
	if (!vp_subtitle_edge_set_user_data(pNormalView->pSubtitleEdgePopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_size_set_user_data is fail");
	}

	if (!vp_subtitle_edge_realize(pNormalView->pSubtitleEdgePopup)) {
		VideoLogWarning("vp_subtitle_size_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_subtitle_font_color_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	int nColor = 0;
	vp_play_preference_get_subtitle_font_color_key(&nColor);

#ifndef SUBTITLE_K_FEATURE
	pNormalView->pSubtitleFontColorPopup = vp_subtitle_color_create(pPlayView->pWin,
	                                       __vp_normal_subtitle_font_color_popup_done_cb,
	                                       nColor);
#else
	char *szColorHex = NULL;
	vp_play_preference_get_subtitle_font_color_hex_key(&szColorHex);
	if (!szColorHex) {
		VideoLogError("szColorHex is NULL");
	} else {
		pNormalView->pSubtitleFontColorPopup = vp_subtitle_color_hex_create(pPlayView->pWin,
		                                       __vp_normal_subtitle_font_color_popup_done_hex_cb,
		                                       szColorHex);
	}
	VP_FREE(szColorHex);
#endif

	if (!pNormalView->pSubtitleFontColorPopup) {
		VideoLogError("vp_subtitle_color_create fail");
		return;
	}
	if (!vp_subtitle_color_set_user_data(pNormalView->pSubtitleFontColorPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_font_color_set_user_data is fail");
	}

	if (!vp_subtitle_color_realize(pNormalView->pSubtitleFontColorPopup)) {
		VideoLogWarning("vp_subtitle_color_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}


static void _vp_play_normal_view_on_subtitle_bg_color_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

#ifndef SUBTITLE_K_FEATURE
	int nColor = 0;
	vp_play_preference_get_subtitle_bg_color_key(&nColor);

	pNormalView->pSubtitleBGColorPopup = vp_subtitle_bg_color_create(pPlayView->pWin,
	                                     __vp_normal_subtitle_bg_color_popup_done_cb,
	                                     nColor);
#else
	char *szColorHex = NULL;
	vp_play_preference_get_subtitle_bg_color_hex_key(&szColorHex);
	if (!szColorHex) {
		VideoLogError("szColorHex is NULL");
	} else { //prevent issue fix
		pNormalView->pSubtitleBGColorPopup = vp_subtitle_color_hex_create(pPlayView->pWin,
		                                     __vp_normal_subtitle_bg_color_popup_done_hex_cb,
		                                     szColorHex);
		VP_FREE(szColorHex);
	}
#endif

	if (!pNormalView->pSubtitleBGColorPopup) {
		VideoLogError("vp_subtitle_bg_color_create fail");
		return;
	}
	if (!vp_subtitle_color_set_user_data(pNormalView->pSubtitleBGColorPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_color_set_user_data is fail");
	}

	if (!vp_subtitle_color_realize(pNormalView->pSubtitleBGColorPopup)) {
		VideoLogWarning("vp_subtitle_color_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_subtitle_caption_win_color_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	char *szColorHex = NULL;
	vp_play_preference_get_subtitle_caption_win_color_hex_key(&szColorHex);
	if (!szColorHex) {
		VideoLogError("szColorHex is NULL");
	} else {
		pNormalView->pSubtitleCaptionWinColorPopup = vp_subtitle_color_hex_create(pPlayView->pWin,
		        __vp_normal_subtitle_caption_win_color_popup_done_hex_cb,
		        szColorHex);
	}
	VP_FREE(szColorHex);


	if (!pNormalView->pSubtitleCaptionWinColorPopup) {
		VideoLogError("vp_subtitle_color_hex_create fail");
		return;
	}
	if (!vp_subtitle_color_set_user_data(pNormalView->pSubtitleCaptionWinColorPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_subtitle_color_set_user_data is fail");
	}

	if (!vp_subtitle_color_realize(pNormalView->pSubtitleCaptionWinColorPopup)) {
		VideoLogWarning("vp_subtitle_color_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
}

static void _vp_play_normal_view_on_capture_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	vp_capture_popup_destroy(pNormalView->pCapturePopup);
	pNormalView->pCapturePopup = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_play_preference_get_capture_on_key(&pNormalView->bCaptureMode);

	pNormalView->pCapturePopup = vp_capture_popup_create(pPlayView->pWin,
	                             pNormalView->bCaptureMode,
	                             __vp_normal_capture_popup_close_cb);
	if (!pNormalView->pCapturePopup) {
		VideoLogError("vp_capture_popup_create fail");
		return;
	}
	if (!vp_capture_popup_set_user_data(pNormalView->pCapturePopup, (void *)pNormalView)) {
		VideoLogWarning("vp_capture_popup_set_user_data is fail");
	}

	if (!vp_capture_popup_realize(pNormalView->pCapturePopup)) {
		VideoLogWarning("vp_capture_popup_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}

static void _vp_play_normal_view_on_play_speed_popup_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	NormalView *pNormalView = (NormalView *)data;
	_vp_play_normal_view_on_play_speed_popup(pNormalView);
}

static void _vp_play_normal_view_on_play_speed_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	vp_play_speed_popup_destroy(pNormalView->pPlaySpeedPopup);
	pNormalView->pPlaySpeedPopup = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	pNormalView->pPlaySpeedPopup = vp_play_speed_popup_create(pPlayView->pWin,
	                               pNormalView->fPlaySpeed,
	                               __vp_normal_play_speed_popup_update_cb,
	                               __vp_normal_play_speed_popup_done_cb);

	if (!pNormalView->pPlaySpeedPopup) {
		VideoLogError("vp_play_speed_popup_create fail");
		return;
	}
	if (!vp_play_speed_popup_set_user_data(pNormalView->pPlaySpeedPopup, (void *)pNormalView)) {
		VideoLogWarning("vp_play_speed_popup_set_user_data is fail");
	}

	if (!vp_play_speed_popup_realize(pNormalView->pPlaySpeedPopup)) {
		VideoLogWarning("vp_play_speed_popup_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

}


static void _vp_play_normal_view_on_setting_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	vp_setting_destroy(pNormalView->pSettingHandle);
	pNormalView->pSettingHandle = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
	}

	if (!vp_play_preference_get_sound_alive_status(&pNormalView->nSoundAlive)) {
		VideoLogError("vp_play_preference_get_sound_alive_status failed");
	}
	bool bEarjack = vp_sound_device_is_enable(VP_SOUND_DEVICE_EARJACK, pNormalView->pPlayView);
	if (bEarjack == FALSE && pNormalView->nSoundAlive == VIDEO_SA_7_1_CH) {
		pNormalView->nSoundAlive = VIDEO_SA_NORMAL;
		if (!vp_play_preference_set_sound_alive_status(pNormalView->nSoundAlive)) {
			VideoLogError("vp_play_preference_set_sound_alive_status failed");
		}
		if (!vp_mm_player_set_sound_filter(pNormalView->pPlayerHandle, VP_MM_PLAYER_FILTER_NONE)) {
			VideoLogWarning("vp_mm_player_set_sound_filter is fail");
		}
	}

	SettingInfo *pSettingInfo = NULL;
	pSettingInfo = calloc(1, sizeof(SettingInfo));
	if (!pSettingInfo) {
		VideoLogError("failed to allocate memory");
		return;
	}

	char *szDir = NULL;
	szDir = vp_play_util_get_folder_from_path(pNormalView->szMediaURL);

	pSettingInfo->fSpeed = pNormalView->fPlaySpeed;
	pSettingInfo->nSoundAlive = pNormalView->nSoundAlive;
	pSettingInfo->nRepeatMode = pNormalView->nRepeatMode;
	pSettingInfo->bExternalMode = pNormalView->bVideoOnlyMode;
	if (pNormalView->bIsExistSubtitle == FALSE) {
		pSettingInfo->bSelectSubtitle = vp_play_util_is_exist_subtitle_from_path(szDir);
	} else {
		pSettingInfo->bSelectSubtitle = TRUE;
	}

	VP_FREE(szDir);

	pSettingInfo->bStreaming = FALSE;

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB ||
	        pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		pSettingInfo->bStreaming = TRUE;
	}
#ifdef ENABLE_DRM_FEATURE
	vp_drm_is_drm_file(pNormalView->szMediaURL, &(pSettingInfo->bDrm));
#endif
	char *szCode = NULL;
	int nCount = 0;
	if (vp_mm_player_get_audio_track_count(pNormalView->pPlayerHandle, &nCount)) {
		if (nCount > 1) {
			if (vp_mm_player_get_audio_track_language_code(pNormalView->pPlayerHandle, pNormalView->nDefaultAudioTrackIndex, &szCode)) {
				vp_audio_track_get_name(szCode, &(pSettingInfo->szAudioTrack));
			} else {
				pSettingInfo->szAudioTrack = g_strdup_printf("%s", VP_PLAY_STRING_COM_UNKNOWN);
			}
		} else {
			pSettingInfo->szAudioTrack = g_strdup_printf("%s %d", VP_PLAY_STRING_AUDIO_TRACK, 0);
		}
	} else {
		pSettingInfo->szAudioTrack = g_strdup_printf("%s %d", VP_PLAY_STRING_AUDIO_TRACK, 0);
	}
	VP_FREE(szCode);

	vp_play_preference_get_capture_on_key(&pSettingInfo->bCapture);

	pNormalView->pSettingHandle = vp_setting_create(pPlayView->pWin, pSettingInfo,
	                              __vp_normal_setting_popup_close_cb);
	if (!pNormalView->pSettingHandle) {
		VideoLogError("vp_setting_create fail");

		VP_FREE(pSettingInfo->szAudioTrack);
		VP_FREE(pSettingInfo);
		return;
	}

	VP_FREE(pSettingInfo->szAudioTrack);
	VP_FREE(pSettingInfo);

	if (!vp_setting_set_user_data(pNormalView->pSettingHandle, (void *)pNormalView)) {
		VideoLogWarning("vp_setting_set_user_data is fail");
	}

	if (!vp_setting_realize(pNormalView->pSettingHandle)) {
		VideoLogWarning("vp_setting_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
}

static void _vp_play_normal_view_on_detail_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	vp_detail_destroy(pNormalView->pDetailHandle);
	pNormalView->pDetailHandle = NULL;

	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pNormalView->bVideoOnlyMode == FALSE && nState == VP_MM_PLAYER_STATE_PLAYING) {
		if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
			VideoLogError("Pause Fail");
		}
		_vp_play_normal_view_set_play_state(pNormalView);
	}

	_vp_play_normal_view_set_play_state(pNormalView);

	vp_detail_info *pDetailInfo = NULL;

	pDetailInfo = calloc(1, sizeof(vp_detail_info));
	if (!pDetailInfo) {
		VideoLogError("pDetailInfo is alloc fail");
		return;
	}
	int nWidth = 0;
	int nHeight = 0;
	double dLatitude = 0.0;
	double dLongitude = 0.0;
	char *szModifiedTime = NULL;
	char *szTitle = NULL;
	char *szFileDate = NULL;
	int nDuration = 0;

#ifdef ENABLE_DRM_FEATURE
	vp_drm_is_drm_file(pNormalView->szMediaURL, &(pDetailInfo->bDrm));
	vp_drm_is_playready_drm_file(pNormalView->szMediaURL, &(pDetailInfo->bPlayready));
	_vp_play_normal_view_check_drm_lock(pNormalView, &(pDetailInfo->bForwardLock));
#endif
	if (pDetailInfo->bPlayready == FALSE) { /* Just check it for performance */
		vp_media_contents_get_content_info(pNormalView->szMediaURL, &szTitle, &nWidth, &nHeight, &nDuration, &dLatitude, &dLongitude, &szModifiedTime);
		if (szTitle) {
			pDetailInfo->szTitle = vp_play_util_get_title_from_path(szTitle);
		} else {
			pDetailInfo->szTitle = vp_play_util_get_title_from_path(pNormalView->szMediaURL);
		}

		if (nWidth == 0) {
			vp_media_metadata_get_width(pNormalView->szMediaURL, &nWidth);
		}
		if (nHeight == 0) {
			vp_media_metadata_get_height(pNormalView->szMediaURL, &nHeight);
		}
	} else {
		pDetailInfo->szTitle = vp_play_util_get_title_from_path(pNormalView->szMediaURL);
	}

	vp_play_util_get_file_info(pNormalView->szMediaURL, &pDetailInfo->szSize, &pDetailInfo->szDate, &pDetailInfo->szFormat);

	VP_FREE(szTitle);
	VP_FREE(szFileDate);
	if (!szModifiedTime && pDetailInfo->szDate) {
		pDetailInfo->szLastModified = g_strdup(pDetailInfo->szDate);
	} else {
		pDetailInfo->szLastModified = g_strdup(szModifiedTime);
	}
	pDetailInfo->szResolution = g_strdup_printf("%d X %d", nWidth, nHeight);
	pDetailInfo->szLatitude = g_strdup_printf("%f", dLatitude);
	pDetailInfo->szLongitude = g_strdup_printf("%f", dLongitude);

	if (vp_file_exists((const char*)pNormalView->szMediaURL)) {
		pDetailInfo->szLocation = vp_dir_get((const char*)pNormalView->szMediaURL);
	}

	pNormalView->pDetailHandle = vp_detail_page_create((void *)pPlayView->pNaviframe,__vp_normal_detail_popup_close_cb, pDetailInfo);
	//pNormalView->pDetailHandle = vp_detail_create(pPlayView->pWin, __vp_normal_detail_popup_close_cb, pDetailInfo);
	if (!pNormalView->pDetailHandle) {
		VideoLogError("vp_detail_create fail");

		VP_FREE(pDetailInfo->szTitle);
		VP_FREE(pDetailInfo->szFormat);
		VP_FREE(pDetailInfo->szDate);
		VP_FREE(pDetailInfo->szSize);
		VP_FREE(pDetailInfo->szLastModified);
		VP_FREE(pDetailInfo->szResolution);
		VP_FREE(pDetailInfo->szLatitude);
		VP_FREE(pDetailInfo->szLongitude);
		VP_FREE(pDetailInfo->szLocation);
		VP_FREE(pDetailInfo->szLastModified);
		VP_FREE(pDetailInfo);
		return;
	}
	if (!vp_detail_set_user_data(pNormalView->pDetailHandle, (void *)pNormalView)) {
		VideoLogWarning("vp_detail_set_user_data is fail");
	}
/*
	if (!vp_detail_realize(pNormalView->pDetailHandle)) {
		VideoLogWarning("vp_detail_realize is fail");
	}
	pNormalView->bIsPopupShow = TRUE;
*/
	VP_FREE(pDetailInfo->szTitle);
	VP_FREE(pDetailInfo->szFormat);
	VP_FREE(pDetailInfo->szDate);
	VP_FREE(pDetailInfo->szSize);
	VP_FREE(pDetailInfo->szResolution);
	VP_FREE(pDetailInfo->szLatitude);
	VP_FREE(pDetailInfo->szLongitude);
	VP_FREE(pDetailInfo->szLocation);
	VP_FREE(pDetailInfo->szLastModified);
	VP_FREE(pDetailInfo);

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
}

static void _vp_play_normal_view_on_volume_popup(NormalView *pNormalView, bool bToggle)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	bool bLandscape = vp_play_util_get_landscape_check(pNormalView->nRotate);
	bool bIsRealize = FALSE;

	if (pNormalView->pVolumePopupHandle) {
		if (!vp_play_volume_popup_unrealize(pNormalView->pVolumePopupHandle)) {
			VideoLogWarning("vp_play_volume_popup_unrealize is fail");
		}
	}
	if (!pNormalView->pVolumeHandle) {
		if (!pNormalView->pPlayView) {
			VideoLogError("pNormalView->pPlayView is NULL");
			return;
		}

		Evas_Object *pParent = pNormalView->pPlayView->pWin;

		pNormalView->pVolumeHandle = vp_play_volume_create(pParent, pNormalView->pPlayerHandle, __vp_normal_volume_change_value_cb);
		if (!pNormalView->pVolumeHandle) {
			VideoLogError("pNormalView->pVolumeHandle  is NULL");
			return;
		}
		if (!vp_play_volume_set_user_data(pNormalView->pVolumeHandle, (void *)pNormalView)) {
			VideoLogWarning("vp_play_volume_set_user_data is fail");
			return;
		}
		if (!pNormalView->bLockScreen) {
			vp_play_volume_set_mouse_callbacks(pNormalView->pVolumeHandle, pNormalView->pPlayView->pWin);
		} else {
			vp_play_volume_unset_mouse_callbacks(pNormalView->pVolumeHandle, pNormalView->pPlayView->pWin);
		}
	}
	Evas_Object *pParent = pNormalView->pPlayView->pWin;
	if (!vp_play_volume_set_landscape_mode(pNormalView->pVolumeHandle, bLandscape, pParent)) {
		VideoLogWarning("vp_play_volume_set_landscape_mode is fail");
		return;
	}

	if (!vp_play_volume_is_realize(pNormalView->pVolumeHandle, &bIsRealize)) {
		VideoLogWarning("vp_play_volume_is_realize is fail");
		return;
	}

	if (bIsRealize && bToggle) {
		if (!vp_play_volume_unrealize(pNormalView->pVolumeHandle)) {
			VideoLogWarning("vp_play_volume_unrealize is fail");
		}
	} else {
		if (!vp_play_volume_update_value(pNormalView->pVolumeHandle)) {
			VideoLogWarning("vp_play_volume_update_value is fail");
		}

		VP_EVAS_DEL(pNormalView->pCtxPopup);

		if (!vp_play_volume_realize(pNormalView->pVolumeHandle, pNormalView->pPlayView->pWin)) {
			VideoLogWarning("vp_play_volume_realize is fail");
		}
	}
}

static void _vp_play_normal_view_on_bookmark_mode(NormalView *pNormalView, bool bShow)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (bShow) {
		_vp_play_normal_view_show_layout(pNormalView);
	}

	if (bShow) {
		pNormalView->bCaptureComplete = TRUE;
		vp_play_bookmark_realize(pNormalView->pBookmarkHandle);
#ifndef SUBTITLE_K_FEATURE
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_BOOKMARK_ON, "*");
#endif
		elm_object_part_content_set(pNormalView->pControlLayout, VP_PLAY_SWALLOW_NORMAL_BOOKMARK, pNormalView->pBookmarkObj);
	} else {
		pNormalView->bCaptureComplete = TRUE;
#ifndef SUBTITLE_K_FEATURE
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_BOOKMARK_OFF, "*");
#endif
		elm_object_part_content_unset(pNormalView->pControlLayout, VP_PLAY_SWALLOW_NORMAL_BOOKMARK);
		vp_play_bookmark_unrealize(pNormalView->pBookmarkHandle);
	}
}

//static void _vp_play_normal_view_download_sdp(NormalView *pNormalView, char *szSdpPath)
//{
//	if (!pNormalView) {
//		VideoLogError("pNormalView is NULL");
//		return;
//	}
//
//	if (!szSdpPath) {
//		VideoLogError("szSubtitlePath is NULL");
//		return;
//	}
//
//	if (pNormalView->pDownloadHandle) {
//		VppDownloadDestroyItem(pNormalView->pDownloadHandle);
//		pNormalView->pDownloadHandle = NULL;
//	}
//
//	VppDownloadCallback pFunc = {0,};
//
//	pFunc.stateChangedCb = __vp_normal_sdp_download_state_change_cb;
//
//	VideoSecureLogDebug(" ############### %s ##########################", szSdpPath);
//
//	char *app_path = app_get_data_path();
//	if (!app_path) {
//		VideoLogError("cannot retrieve app install path");
//		return;
//	}
//	char db_path[1024] = {0,};
//	snprintf(db_path, 1024, "%s%s", app_path, "sdp");
//	VideoLogError("db_path: %s", db_path);
//
//
//	pNormalView->pDownloadHandle = VppDownloadCreateItem(szSdpPath, db_path, NULL, VPP_DOWNLOAD_ITEM_TYPE_VIDEO_FILE, pFunc, (void *)pNormalView);
//
//	if (!pNormalView->pDownloadHandle) {
//		VideoLogError("pDownloadHandle is NULL");
//		return;
//	}
//
//	if (!VppDownloadRequestAppend(pNormalView->pDownloadHandle)) {
//		VideoLogError("VppDownloadRequestAppend is fail");
//		return;
//	}
//
//}

static void _vp_play_normal_view_all_close_popup(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (pNormalView->pPopup) {
		VP_EVAS_DEL(pNormalView->pPopup);
		pNormalView->pPopup = NULL;
	}

	if (pNormalView->pCtxPopup) {
		VP_EVAS_DEL(pNormalView->pCtxPopup);
		pNormalView->pCtxPopup = NULL;
	}

	/*if (pNormalView->pDetailHandle) {
		vp_detail_destroy(pNormalView->pDetailHandle);
		pNormalView->pDetailHandle = NULL;
	}*/

	if (pNormalView->pSubtitlePopup) {
		vp_subtitle_destroy(pNormalView->pSubtitlePopup);
		pNormalView->pSubtitlePopup = NULL;
		pNormalView->bIsSubtitleShow = FALSE;
	}

	if (pNormalView->pSubtitleSelectPopup) {
		vp_subtitle_select_destroy(pNormalView->pSubtitleSelectPopup);
		pNormalView->pSubtitleSelectPopup = NULL;
	}

	if (pNormalView->pSubtitleFontPopup) {
		vp_subtitle_font_destroy(pNormalView->pSubtitleFontPopup);
		pNormalView->pSubtitleFontPopup = NULL;
	}

	if (pNormalView->pSubtitleAlignmentPopup) {
		vp_subtitle_alignment_destroy(pNormalView->pSubtitleAlignmentPopup);
		pNormalView->pSubtitleAlignmentPopup = NULL;
	}

	if (pNormalView->pSubtitleFontColorPopup) {
		vp_subtitle_color_destroy(pNormalView->pSubtitleFontColorPopup);
		pNormalView->pSubtitleFontColorPopup = NULL;
	}

	if (pNormalView->pSubtitleBGColorPopup) {
		vp_subtitle_bg_color_destroy(pNormalView->pSubtitleBGColorPopup);
		pNormalView->pSubtitleBGColorPopup = NULL;
	}

	if (pNormalView->pSubtitleEdgePopup) {
		vp_subtitle_edge_destroy(pNormalView->pSubtitleEdgePopup);
		pNormalView->pSubtitleEdgePopup = NULL;
	}

	if (pNormalView->pSubtitleSizePopup) {
		vp_subtitle_size_destroy(pNormalView->pSubtitleSizePopup);
		pNormalView->pSubtitleSizePopup = NULL;
	}

	if (pNormalView->pSubtitleSyncPopup) {
		vp_subtitle_sync_popup_destroy(pNormalView->pSubtitleSyncPopup);
		pNormalView->pSubtitleSyncPopup = NULL;
	}

	if (pNormalView->pRepeatHandle) {
		vp_repeat_destroy(pNormalView->pRepeatHandle);
		pNormalView->pRepeatHandle = NULL;
	}

	if (pNormalView->pSAHandle) {
		vp_sound_alive_destroy(pNormalView->pSAHandle);
		pNormalView->pSAHandle = NULL;
	}

	if (pNormalView->pSoundPathHandle) {
		vp_sound_path_destroy(pNormalView->pSoundPathHandle);
		pNormalView->pSoundPathHandle = NULL;
	}

	if (pNormalView->pAudioTrackHandle) {
		vp_audio_track_destroy(pNormalView->pAudioTrackHandle);
		pNormalView->pAudioTrackHandle = NULL;
	}

	if (pNormalView->pSettingHandle) {
		vp_setting_destroy(pNormalView->pSettingHandle);
		pNormalView->pSettingHandle = NULL;
	}

	if (pNormalView->pCapturePopup) {
		vp_capture_popup_destroy(pNormalView->pCapturePopup);
		pNormalView->pCapturePopup = NULL;
	}

	if (pNormalView->pPlaySpeedPopup) {
		vp_mm_player_set_rate(pNormalView->pPlayerHandle, (float)pNormalView->fPlaySpeed);
		vp_play_speed_popup_destroy(pNormalView->pPlaySpeedPopup);
		pNormalView->pPlaySpeedPopup = NULL;
	}

	pNormalView->bIsPopupShow = FALSE;
}

static void _vp_play_normal_view_show_layout(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (!pNormalView->pNaviItem) {
		VideoLogError("pNaviItem is NULL");
		return;
	}

	if (pNormalView->bLockScreen) {
		VideoLogError("bLockScreen is TRUE");
		vp_play_normal_view_set_lock_screen(pNormalView, pNormalView->bLockScreen);
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	if (!pPlayView->pWin) {
		VideoLogError("pPlayView->pWin is NULL");
		return;
	}

	vp_device_set_frame_rate(60);

	//_vp_play_normal_view_update_progress_value(pNormalView);
	_vp_play_normal_view_set_rotate_lock_state(pNormalView);

	VP_EVAS_TIMER_DEL(pNormalView->pScreenShotTimer);

	elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_PORTRAIT);
	elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_LANDSCAPE);

	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SCREEN_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_ROTATE_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_VOLUME_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_HIDE, "*");

	VP_EVAS_DEL(pNormalView->pScreenShot);
	pNormalView->pScreenShot = NULL;

	elm_object_item_signal_emit(pNormalView->pNaviItem, VP_NORMAL_SIGNAL_NAVIFRAME_TITLE_SHOW, "elm");
	_vp_play_normal_view_show_rotate(pNormalView);
	_vp_play_normal_view_show_volume(pNormalView);

	bool bLandscape = vp_play_util_get_landscape_check(pNormalView->nRotate);
	if (bLandscape) {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_LANDSCAPE_SHOW, "*");
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_LANDSCAPE_SHOW, "*");
		elm_object_signal_emit(pNormalView->pControlLayout, VP_NORMAL_SIGNAL_CONTROL_LANDSCAPE_MODE, "*");
		elm_object_signal_emit(pNormalView->pFunctionLayout, VP_NORMAL_SIGNAL_FUNCTION_LANDSCAPE_MODE, "*");
		vp_play_progressbar_set_landscape_mode(pNormalView->pProgressbar, TRUE);
	} else {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PORTRAIT_SHOW, "*");
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_PORTRAIT_SHOW, "*");
		elm_object_signal_emit(pNormalView->pControlLayout, VP_NORMAL_SIGNAL_CONTROL_PORTRAIT_MODE, "*");
		elm_object_signal_emit(pNormalView->pFunctionLayout, VP_NORMAL_SIGNAL_FUNCTION_PORTRAIT_MODE, "*");
		vp_play_progressbar_set_landscape_mode(pNormalView->pProgressbar, FALSE);
	}

	bool bIsRealize = FALSE;
	if (pNormalView->pBookmarkHandle) {
		vp_play_bookmark_is_realize(pNormalView->pBookmarkHandle, &bIsRealize);
		if (bIsRealize) {
#ifndef SUBTITLE_K_FEATURE
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_BOOKMARK_ON, "*");
#endif
		}
	}

	_vp_play_normal_view_set_play_state(pNormalView);
	_vp_play_normal_view_on_capture_mode(pNormalView);

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

	pNormalView->bShowLayout = TRUE;
	vp_play_normal_view_set_main_layout_focus_out((void *)pPlayView->pNormalView);
	_vp_play_normal_view_set_button_focus_sequence(pNormalView);
	_vp_play_normal_view_create_layout_hide_timer(pNormalView);
}

static void _vp_play_normal_view_hide_layout(NormalView *pNormalView, bool bNoEffect)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	if (!pNormalView->pNaviItem) {
		VideoLogError("pNaviItem is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	if (pNormalView->pBookmarkHandle) {
		bool bIsRealize = FALSE;
		bool bEditMode = FALSE;
		bool bPressed = FALSE;
		if (!vp_play_bookmark_is_realize(pNormalView->pBookmarkHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_bookmark_is_realize fail");
		}
		if (bIsRealize) {
			if (!vp_play_bookmark_get_edit_mode(pNormalView->pBookmarkHandle, &bEditMode)) {
				VideoLogWarning("vp_play_bookmark_get_edit_mode fail");
			}
			if (bEditMode) {
				_vp_play_normal_view_create_layout_hide_timer(pNormalView);
				return;
			}

			if (!vp_play_bookmark_get_pressed_status(pNormalView->pBookmarkHandle, &bPressed)) {
				VideoLogWarning("vp_play_bookmark_get_pressed_status fail");
			}

			if (bPressed) {
				_vp_play_normal_view_create_layout_hide_timer(pNormalView);
				return;
			}
		}
	}

	VP_EVAS_TIMER_DEL(pNormalView->pScreenShotTimer);

	elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_PORTRAIT);
	elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_LANDSCAPE);

	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SCREEN_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_ROTATE_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_VOLUME_HIDE, "*");
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_HIDE, "*");

	VP_EVAS_DEL(pNormalView->pScreenShot);
	pNormalView->pScreenShot = NULL;

	VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);

	if (bNoEffect) {
		elm_object_item_signal_emit(pNormalView->pNaviItem, VP_NORMAL_SIGNAL_NAVIFRAME_TITLE_NO_EFFECT_HIDE, "elm");
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_HIDE, "*");
	} else {
		elm_object_item_signal_emit(pNormalView->pNaviItem, VP_NORMAL_SIGNAL_NAVIFRAME_TITLE_HIDE, "elm");
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_HIDE, "*");
	}
	vp_play_normal_view_set_main_layout_focus_out((void *)pPlayView->pNormalView);
	pNormalView->bShowLayout = FALSE;
}

static double _vp_play_normal_view_get_mouse_pos_ratio(Evas_Object *pObj, int nCurX)
{
	if (!pObj) {
		VideoLogError("pObj is NULL");
		return 0.0;
	}

	int nWidth = 0;
	int nCurrent = 0;
	double dRatio = 0.0;

	evas_object_geometry_get(pObj, NULL, NULL, &nWidth, NULL);

	nCurrent = nCurX - (VP_NORMAL_PROGRESS_BAR_START_TERM);
	nWidth = nWidth - (VP_NORMAL_PROGRESS_BAR_WIDTH_TERM);

	if (nCurrent < 0) {
		nCurrent = 0;
	} else if (nCurrent > nWidth) {
		nCurrent = nWidth;
	}

	dRatio = (double)nCurrent / nWidth;

	return dRatio;
}

static void _vp_play_normal_view_create_progress_timer(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);

	pNormalView->pProgressTimer =
	    ecore_timer_add(VP_NORMAL_PROGRESS_TIMER_INTERVAL,
	                    __vp_normal_progressbar_timer_cb, (void *)pNormalView);
}

static void _vp_play_normal_view_create_layout_hide_timer(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
	pNormalView->pHideTimer = ecore_timer_add(VP_NORMAL_HIDE_LAYOUT_TIMER_INTERVAL,
	                          __vp_normal_hide_layout_timer_cb, (void *)pNormalView);
}

static void _vp_play_normal_view_show_rotate(NormalView *pNormalView)
{
	bool bLockState = FALSE;

	if (!vp_play_config_get_rotate_lock_status(&bLockState)) {
		VideoLogError("vp_play_config_get_rotate_lock_status is fail");
		return;
	}

	if (!bLockState) {
		if (pNormalView->pMainLayout) {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_ROTATE_SHOW, "*");
		}
	}
}

static void _vp_play_normal_view_show_volume(NormalView *pNormalView)
{
	bool bLockState = FALSE;

	/*if (!vp_play_config_get_rotate_lock_status(&bLockState)) {
		VideoLogError("vp_play_config_get_rotate_lock_status is fail");
		return;
	}*/

	if (!bLockState) {
		if (pNormalView->pMainLayout) {
			if ((pNormalView->fPlaySpeed - 0.05) > 1.000 || (pNormalView->fPlaySpeed + 0.05) < 1.000) {
				elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_SHOW, "*");
				elm_object_signal_callback_add(pNormalView->pMainLayout, "elm,action,click", "", _vp_play_normal_view_on_play_speed_popup_cb, pNormalView);
				_vp_play_normal_view_create_layout_hide_timer(pNormalView);
			} else {
				elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_HIDE, "*");
			}
			char szTxt[5] = {0,};
			snprintf(szTxt, 5, "%1.1fX", (float)pNormalView->fPlaySpeed);
			elm_object_part_text_set(pNormalView->pMainLayout, VP_PLAY_PART_NORMAL_PLAYING_SPEED_TXT, szTxt);
		}
		bool bLandscape = vp_play_util_get_landscape_check(pNormalView->nRotate);
		if (bLandscape) {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_VOLUME_LANDSCAPE_SHOW, "*");
		} else {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_VOLUME_PORTRAIT_SHOW, "*");
		}
	}
}

static void _vp_play_normal_view_set_rotate_lock_state(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	bool bLockState = FALSE;

	if (!vp_play_config_get_rotate_lock_status(&bLockState)) {
		VideoLogError("vp_play_config_get_rotate_lock_status is fail");
		return;
	}

	/*if (!pNormalView->pSoundPathBtn) {
		elm_object_item_part_content_unset(pNormalView->pNaviItem, "title_right_btn");
	}
	else*/
	{
		if (pNormalView->pMainLayout) {
			elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_ROTATE);
		}
	}

	VP_EVAS_DEL(pNormalView->pRotateBtn);
	pNormalView->pRotateBtn = NULL;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	Evas_Object *pParent = NULL;
	if (!pNormalView->pSoundPathBtn) {
		pParent = pNormalView->pMainLayout;

	} else if (pNormalView->pFunctionLayout) {

		pParent = pNormalView->pFunctionLayout;

	} else {

		pParent = pNormalView->pMainLayout;
	}

	if (!bLockState) {
		pNormalView->pRotateBtn = vp_button_create(
		                              pParent, "custom/flat_46_46/default",
		                              VP_PLAY_STRING_COM_ROTATE,
		                              (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
		                              (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
		                              (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
		                              (void *)pNormalView);

		Evas_Object *pIcon = NULL;
		pIcon = vp_button_create_icon(pNormalView->pRotateBtn, edj_path, VP_PLAY_NORMAL_RES_ROTATE);
		elm_object_part_content_set(pNormalView->pRotateBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pRotateBtn, edj_path, VP_PLAY_NORMAL_RES_ROTATE_PRESS);
		elm_object_part_content_set(pNormalView->pRotateBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pRotateBtn, edj_path, VP_PLAY_NORMAL_RES_ROTATE_DIM);
		elm_object_part_content_set(pNormalView->pRotateBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

		/*if (!pNormalView->pSoundPathBtn) {
			elm_object_item_part_content_set(pNormalView->pNaviItem, "title_right_btn", pNormalView->pRotateBtn);
		}
		else */
		{
			if (pNormalView->pMainLayout) {
				elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_ROTATE, pNormalView->pRotateBtn);
			}
		}
	}

}

static void _vp_play_normal_view_set_volume_lock_state(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	bool bLockState = FALSE;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	/*if (!vp_play_config_get_rotate_lock_status(&bLockState)) {
		VideoLogError("vp_play_config_get_rotate_lock_status is fail");
		return;
	}*/

	/*if (!pNormalView->pSoundPathBtn) {
		elm_object_item_part_content_unset(pNormalView->pNaviItem, "title_right_btn");
	}
	else*/
	{
		if (pNormalView->pMainLayout) {
			elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_VOLUME);
		}
	}

	VP_EVAS_DEL(pNormalView->pVolumeBtn);
	pNormalView->pVolumeBtn = NULL;


	Evas_Object *pParent = NULL;
	if (pNormalView->pSoundPathBtn == NULL) {
		pParent = pNormalView->pMainLayout;

	} else if (pNormalView->pFunctionLayout) {

		pParent = pNormalView->pFunctionLayout;

	} else {

		pParent = pNormalView->pMainLayout;
	}

	if (!bLockState) {
		pNormalView->pVolumeBtn = vp_button_create(
		                              pParent, "custom/flat_46_46/default",  VP_PLAY_STRING_COM_VOLUME,
		                              (Evas_Smart_Cb)__vp_normal_function_btn_clicked_cb,
		                              (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
		                              (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
		                              (void *)pNormalView);
		if (!pNormalView->pVolumeBtn) {
			VideoLogError("elm_button_add is fail");
			return;
		}
		Evas_Object *pIcon = NULL;
		int nVolume = 0;
		vp_sound_get_volume(&nVolume);

		VideoLogError("nVolume = %d:", nVolume);

		if ((nVolume == 0)) {
			pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_MUTE);
			elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_MUTE_PRESS);
			elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_MUTE_DIM);
			elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
		} else {
			pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_VOLUME);
			elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_VOLUME_PRESS);
			elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

			pIcon = vp_button_create_icon(pNormalView->pVolumeBtn, edj_path, VP_PLAY_NORMAL_RES_VOLUME_DIM);
			elm_object_part_content_set(pNormalView->pVolumeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
		}

		if (pNormalView->pMainLayout) {
			elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_VOLUME, pNormalView->pVolumeBtn);
		}
	}
}

static void _vp_play_normal_view_set_sound_path_state(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}
	VideoLogInfo("set_sound_path");

	bool bSoundPath = FALSE;
	bool bEarjack = FALSE;

	vp_sound_path_destroy(pNormalView->pSoundPathHandle);
	pNormalView->pSoundPathHandle = NULL;

	vp_sound_is_sound_path_enable(&bSoundPath);

	bEarjack = vp_sound_device_is_enable(VP_SOUND_DEVICE_EARJACK, pNormalView->pPlayView);

	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	if (pNormalView->bEarjackConnect != bEarjack) {
		if (bEarjack == FALSE) {
			pNormalView->bManualPause = TRUE;
		}
	}

	pNormalView->bEarjackConnect = bEarjack;

	if (!vp_play_preference_get_sound_alive_status(&pNormalView->nSoundAlive)) {
		VideoLogError("vp_play_preference_get_sound_alive_status failed");
	}
	if (bEarjack == FALSE && pNormalView->nSoundAlive == VIDEO_SA_7_1_CH) {
		pNormalView->nSoundAlive = VIDEO_SA_NORMAL;
		if (!vp_play_preference_set_sound_alive_status(pNormalView->nSoundAlive)) {
			VideoLogError("vp_play_preference_set_sound_alive_status failed");
		}
		if (!vp_mm_player_set_sound_filter(pNormalView->pPlayerHandle, VP_MM_PLAYER_FILTER_NONE)) {
			VideoLogWarning("vp_mm_player_set_sound_filter is fail");
		}
	}

	elm_object_item_part_content_unset(pNormalView->pNaviItem, "title_right_btn");
	VideoLogInfo("rotate_lock_state_sound:%d", bSoundPath);

	VP_EVAS_DEL(pNormalView->pSoundPathBtn);
	pNormalView->pSoundPathBtn = NULL;

	if (bSoundPath) {
		VP_EVAS_DEL(pNormalView->pRotateBtn);
		pNormalView->pRotateBtn = NULL;

		video_sound_device_type_t nType = VP_SOUND_DEVICE_NONE;
		vp_sound_get_active_device(&nType);
		char *szIconPath = VP_PLAY_NORMAL_RES_SPEAKER;
		switch (nType) {
		case VP_SOUND_DEVICE_SPEAKER:
			szIconPath = VP_PLAY_NORMAL_RES_SPEAKER;
			break;

		case VP_SOUND_DEVICE_EARJACK:
			szIconPath = VP_PLAY_NORMAL_RES_EARPHONE;
			break;

		case VP_SOUND_DEVICE_BLUETOOTH:
			szIconPath = VP_PLAY_NORMAL_RES_BLUETOOTH_HEADSET;
			break;

		case VP_SOUND_DEVICE_HDMI:
			szIconPath = VP_PLAY_NORMAL_RES_ICON_HDMI;
			break;

		case VP_SOUND_DEVICE_MIRRORING:
			szIconPath = VP_PLAY_NORMAL_RES_MIRRORING;
			break;

		case VP_SOUND_DEVICE_USB_AUDIO:
			szIconPath = VP_PLAY_NORMAL_RES_USB_AUDIOE;
			break;

		default:
			szIconPath = VP_PLAY_NORMAL_RES_SPEAKER;
			break;
		}

		pNormalView->pSoundPathBtn = vp_navirame_button_create(pNormalView->pNaviFrame, "custom/navi_button/default", edj_path,
		                             (const char *)szIconPath, (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb, (void *)pNormalView);

		elm_object_item_part_content_set(pNormalView->pNaviItem, "title_right_btn", pNormalView->pSoundPathBtn);
	}
}

static void _vp_play_normal_view_set_play_state(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}
	if (!pPlayView->pWin) {
		VideoLogError("pPlayView->pWin is NULL");
		return;
	}

	Evas_Object *pParent = NULL;
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	pParent = pNormalView->pControlLayout;

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (pPlayView->bAVRCP && nState != VP_MM_PLAYER_STATE_NONE && nState != VP_MM_PLAYER_STATE_IDLE) {
		vp_avrcp_noti_player_state(nState);
	}

	elm_object_part_content_unset(pParent, VP_PLAY_SWALLOW_NORMAL_PLAY);
	evas_object_hide(pNormalView->pPauseBtn);
	evas_object_hide(pNormalView->pResumeBtn);

	if (nState == VP_MM_PLAYER_STATE_PLAYING) {
		VideoLogWarning("VP_MM_PLAYER_STATE_PLAYING");
		evas_object_show(pNormalView->pPauseBtn);
//		elm_object_focus_set(pNormalView->pPauseBtn, EINA_TRUE);
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_PLAY, pNormalView->pPauseBtn);
		evas_object_hide(pNormalView->pResumeBtn);

		if (!pNormalView->pProgressTimer) {
			_vp_play_normal_view_create_progress_timer(pNormalView);
		}
		vp_play_util_set_lock_power_key();
		vp_play_progressbar_set_opacity(pNormalView->pProgressbar, FALSE);
	} else {
		VideoLogWarning("Play_Status[%d]", nState);
		evas_object_show(pNormalView->pResumeBtn);
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_PLAY, pNormalView->pResumeBtn);
		evas_object_hide(pNormalView->pPauseBtn);
		VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);

		if (nState == VP_MM_PLAYER_STATE_PAUSED) {
			vp_device_set_screen_off(FALSE);
		}
		vp_play_util_set_unlock_power_key();
	}

	VideoLogWarning("STATUS[%d]", nState);
}

static Evas_Object *_vp_play_normal_view_create_main_layout(Evas_Object *pParent)
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
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_NORMAL_MAIN_EDJ);
	free(path);
	bRet = elm_layout_file_set(pObj, edj_path, VP_PLAY_EDJ_GROUP_NORMAL_MAIN);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail : %s [%s]", edj_path, VP_PLAY_EDJ_GROUP_NORMAL_MAIN);
	}

	evas_object_show(pObj);

	return pObj;
}

static bool _vp_play_normal_view_create_gesture_layout(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("Parent is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;
	PlayView *pPlayView = pNormalView->pPlayView;
	Eina_Bool bRet = EINA_FALSE;

	pParent = pPlayView->pMainLayout;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_GESTURE_EDJ);
	free(path);
	pNormalView->pMainEventRect = elm_layout_add(pParent);
	bRet = elm_layout_file_set(pNormalView->pMainEventRect, edj_path, VP_PLAY_EDJ_GROUP_GESTURE);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail : %s [%s]", edj_path, VP_PLAY_EDJ_GROUP_GESTURE);
		return FALSE;
	}
	elm_object_part_content_set(pParent, "elm.swallow.event", pNormalView->pMainEventRect);

	pNormalView->pGestureRect = elm_gesture_layer_add(pNormalView->pMainEventRect);
	if (!pNormalView->pGestureRect) {
		VideoLogError("elm_gesture_layer_add is NULL");
		return FALSE;
	}
	elm_gesture_layer_hold_events_set(pNormalView->pGestureRect, EINA_FALSE);

	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_TAPS,
	                         ELM_GESTURE_STATE_START, __vp_normal_gesture_n_tab_start_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_TAPS,
	                         ELM_GESTURE_STATE_END, __vp_normal_gesture_n_tab_end_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_TAPS,
	                         ELM_GESTURE_STATE_ABORT, __vp_normal_gesture_n_tab_abort_cb, (void *)pNormalView);

	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_MOMENTUM,
	                         ELM_GESTURE_STATE_START, __vp_normal_gesture_momentum_start_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_MOMENTUM,
	                         ELM_GESTURE_STATE_END, __vp_normal_gesture_momentum_end_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_MOMENTUM,
	                         ELM_GESTURE_STATE_ABORT, __vp_normal_gesture_momentum_end_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_MOMENTUM,
	                         ELM_GESTURE_STATE_MOVE, __vp_normal_gesture_momentum_move_cb, (void *)pNormalView);

#ifdef ENABLE_GESTURE_ZOOM_FEATURE
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_ZOOM,
	                         ELM_GESTURE_STATE_START, __vp_normal_gesture_zoom_start_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_ZOOM,
	                         ELM_GESTURE_STATE_END, __vp_normal_gesture_zoom_end_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_ZOOM,
	                         ELM_GESTURE_STATE_ABORT, __vp_normal_gesture_zoom_abort_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_ZOOM,
	                         ELM_GESTURE_STATE_MOVE, __vp_normal_gesture_zoom_move_cb, (void *)pNormalView);
#endif

	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_DOUBLE_TAPS,
	                         ELM_GESTURE_STATE_START, __vp_normal_gesture_double_click_start_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_DOUBLE_TAPS,
	                         ELM_GESTURE_STATE_END, __vp_normal_gesture_double_click_end_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_DOUBLE_TAPS,
	                         ELM_GESTURE_STATE_ABORT, __vp_normal_gesture_double_click_abort_cb, (void *)pNormalView);

#ifdef FLICK_JUMP
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_FLICKS,
	                         ELM_GESTURE_STATE_START, __vp_normal_gesture_flick_start_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_FLICKS,
	                         ELM_GESTURE_STATE_END, __vp_normal_gesture_flick_end_cb, (void *)pNormalView);
	elm_gesture_layer_cb_set(pNormalView->pGestureRect, ELM_GESTURE_N_FLICKS,
	                         ELM_GESTURE_STATE_ABORT, __vp_normal_gesture_flick_abort_cb, (void *)pNormalView);
#endif

	elm_gesture_layer_line_angular_tolerance_set(pNormalView->pGestureRect, 60);	// 60 degree ??

	elm_gesture_layer_attach(pNormalView->pGestureRect, pNormalView->pMainEventRect);

	evas_object_show(pNormalView->pMainEventRect);

	return TRUE;
}

static Evas_Object *_vp_play_normal_view_create_top_control_layout(Evas_Object *pParent)
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
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_NORMAL_TOP_CONTROL_EDJ);
	free(path);
	bRet = elm_layout_file_set(pObj, edj_path, VP_PLAY_EDJ_GROUP_NORMAL_TOP_CONTROL);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
	}

	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_play_normal_view_create_control_layout(Evas_Object *pParent)
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
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_NORMAL_CONTROL_EDJ);
	free(path);
	bRet = elm_layout_file_set(pObj, edj_path, VP_PLAY_EDJ_GROUP_NORMAL_CONTROL);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
	}

	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_play_normal_view_create_function_layout(Evas_Object *pParent)
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
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_NORMAL_FUNCTION_EDJ_PATH);
	free(path);
	bRet = elm_layout_file_set(pObj, edj_path, VP_PLAY_EDJ_GROUP_NORMAL_FUNCTION);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
	}

	evas_object_show(pObj);

	return pObj;
}

static void _vp_normal_view_top_layout_del_cb(void *data, Evas *e,
        Evas_Object *obj,
        void *event_info)
{
	if (!data) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)data;
	pNormalView->pTopControlLayout = NULL;
}

static bool _vp_play_normal_view_create_top_control(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	VideoLogInfo("create top control");

	pNormalView->pTopControlLayout = _vp_play_normal_view_create_top_control_layout(pNormalView->pMainLayout);
	if (!pNormalView->pTopControlLayout) {
		VideoLogError("pNormalView->pTopControlLayout is NULL");
		return FALSE;
	}
	evas_object_event_callback_add(pNormalView->pTopControlLayout, EVAS_CALLBACK_DEL, _vp_normal_view_top_layout_del_cb, (void *)pNormalView);

	elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_TOP_CONTROL, pNormalView->pTopControlLayout);

	_vp_play_normal_view_on_capture_mode(pNormalView);
	_vp_play_normal_view_set_sound_path_state(pNormalView);

	return TRUE;
}

static void  _vp_play_normal_view_control_layout_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	if (!data) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)data;
	if (pNormalView->pControlLayout) {
		evas_object_event_callback_del(pNormalView->pControlLayout, EVAS_CALLBACK_DEL,  _vp_play_normal_view_control_layout_del_cb);
		pNormalView->pControlLayout = NULL;
		pNormalView->pResumeBtn = NULL;
		pNormalView->pPauseBtn = NULL;
		pNormalView->pPlayFocusBtn = NULL;
		pNormalView->pPrevBtn = NULL;
		pNormalView->pNextBtn = NULL;
		pNormalView->pVolumeBtn = NULL;
		pNormalView->pScreenSizeBtn = NULL;
	}
}
/*
static void  _vp_play_normal_view_bookmark_object_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	if (!data) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)data;
	if (pNormalView->pBookmarkObj) {
		evas_object_event_callback_del(pNormalView->pBookmarkObj, EVAS_CALLBACK_DEL,  _vp_play_normal_view_bookmark_object_del_cb);
		pNormalView->pBookmarkObj = NULL;
	}
}
*/
static bool _vp_play_normal_view_create_main_control(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;

	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	pNormalView->pControlLayout = _vp_play_normal_view_create_control_layout(pNormalView->pMainLayout);
	if (!pNormalView->pControlLayout) {
		VideoLogError("pNormalView->pControlLayout is NULL");
		return FALSE;
	}

	elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pControlLayout, NULL);

	elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_CONTROL, pNormalView->pControlLayout);
	evas_object_event_callback_add(pNormalView->pControlLayout, EVAS_CALLBACK_DEL,
	                               _vp_play_normal_view_control_layout_del_cb,
	                               (void *)pNormalView);

	pParent = pNormalView->pControlLayout;

	pNormalView->pResumeBtn = vp_button_create(
	                              pParent, "custom/circle_1/default", VP_PLAY_STRING_COM_PLAY,
	                              (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
	                              (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
	                              (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
	                              (void *)pNormalView);
	if (!pNormalView->pResumeBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pNormalView->pPauseBtn = vp_button_create(
	                             pParent, "custom/circle_1_focus/default", VP_PLAY_STRING_COM_PAUSE,
	                             (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
	                             (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
	                             (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
	                             (void *)pNormalView);
	if (!pNormalView->pPauseBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pNormalView->pPlayFocusBtn = vp_button_create(
	                                 pParent, "focus", NULL,
	                                 (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
	                                 (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
	                                 (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
	                                 (void *)pNormalView);
	if (!pNormalView->pPlayFocusBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pNormalView->pPrevBtn = vp_button_create(
	                            pParent, "custom/circle_2/default", VP_PLAY_STRING_COM_PREVIOUS,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
	                            (void *)pNormalView);
	if (!pNormalView->pPrevBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pNormalView->pNextBtn = vp_button_create(
	                            pParent, "custom/circle_3/default", VP_PLAY_STRING_COM_NEXT,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_clicked_cb,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
	                            (void *)pNormalView);
	if (!pNormalView->pNextBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	pNormalView->pScreenSizeBtn = vp_button_create(
	                                  pParent, "custom/flat_56_56/default", NULL,
	                                  (Evas_Smart_Cb)__vp_normal_function_btn_clicked_cb,
	                                  (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
	                                  (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
	                                  (void *)pNormalView);
	evas_object_color_set(pNormalView->pScreenSizeBtn, 0, 166, 191, 255);
	if (!pNormalView->pScreenSizeBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	/*
	pNormalView->pPopupBtn = vp_button_create(
	                             pParent, "playview/custom/flat_56_56/default", NULL,
	                             NULL,
	                             NULL,
	                             NULL,
	                             (void *)pNormalView);
	evas_object_color_set(pNormalView->pPopupBtn, 0, 166, 191, 255);
	if (!pNormalView->pPopupBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}
	 */
	pNormalView->pLockBtn = vp_button_create(
	                            pNormalView->pMainLayout, "custom/flat_46_46/default", NULL,
	                            (Evas_Smart_Cb)__vp_normal_function_btn_clicked_cb,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
	                            (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
	                            (void *)pNormalView);
	if (!pNormalView->pLockBtn) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	Evas_Object *pIcon = NULL;

	//elm_object_focus_allow_set(pNormalView->pPauseBtn, EINA_TRUE);
	//elm_object_focus_allow_set(pNormalView->pResumeBtn, EINA_TRUE);
	//elm_object_focus_set(pNormalView->pPauseBtn, EINA_TRUE);
	//elm_object_focus_set(pNormalView->pResumeBtn, EINA_TRUE);

	//elm_object_focus_custom_chain_append(pParent, pNormalView->pPlayFocusBtn, NULL);

	pIcon = vp_button_create_icon(pNormalView->pResumeBtn, edj_path, VP_PLAY_NORMAL_RES_RESUME);
	elm_object_part_content_set(pNormalView->pResumeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon = vp_button_create_icon(pNormalView->pPauseBtn, edj_path, VP_PLAY_NORMAL_RES_PAUSE);
	elm_object_part_content_set(pNormalView->pPauseBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	//pIcon = vp_button_create_icon(pNormalView->pPrevBtn, edj_path, VP_PLAY_NORMAL_RES_PREV);
	//elm_object_part_content_set(pNormalView->pPrevBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	//pIcon = vp_button_create_icon(pNormalView->pNextBtn, edj_path, VP_PLAY_NORMAL_RES_NEXT);
	//elm_object_part_content_set(pNormalView->pNextBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	/*
	pIcon = vp_button_create_icon(pNormalView->pPopupBtn, edj_path, VP_PLAY_NORMAL_RES_POPUP_PLAYER);
	elm_object_part_content_set(pNormalView->pPopupBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon = vp_button_create_icon(pNormalView->pPopupBtn, edj_path, VP_PLAY_NORMAL_RES_POPUP_PLAYER);
	elm_object_part_content_set(pNormalView->pPopupBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);
	*/

	if (pNormalView->nDisplayMode == VP_MM_PLAYER_DISPLAY_FULL_SCREEN) {
		pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_2_MODE);
		elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_2_MODE);
		elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		//pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_2_MODE_DIM);
		//elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
	} else if (pNormalView->nDisplayMode == VP_MM_PLAYER_DISPLAY_ORIGIN_SIZE) {
		pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE);
		elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE);
		elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);
	} else if (pNormalView->nDisplayMode == VP_MM_PLAYER_DISPLAY_ORIGIN_OR_LETTER) {
		pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_3_MODE);
		elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		//pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_3_MODE_PRESS);
		//elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		//pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_3_MODE_DIM);
		//elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
	} else {
		pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE);
		elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		//pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE_PRESS);
		//elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		//pIcon = vp_button_create_icon(pNormalView->pScreenSizeBtn, edj_path, VP_PLAY_NORMAL_RES_SCREEN_1_MODE_DIM);
		//elm_object_part_content_set(pNormalView->pScreenSizeBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);
	}


	if (pNormalView->pResumeBtn) {
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_PLAY, pNormalView->pResumeBtn);

		elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pResumeBtn, NULL);
	}

	if (pNormalView->pPlayFocusBtn) {
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_PLAY_FOCUS, pNormalView->pPlayFocusBtn);
		elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pPlayFocusBtn, NULL);
	}

	if (pNormalView->pPrevBtn) {
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_REW, pNormalView->pPrevBtn);
		elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pPrevBtn, NULL);
	}

	if (pNormalView->pNextBtn) {
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_FF, pNormalView->pNextBtn);
		elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pNextBtn, NULL);
	}

	if (pNormalView->pScreenSizeBtn) {
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_SCREENSIZE, pNormalView->pScreenSizeBtn);
		elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pScreenSizeBtn, NULL);
	}
	/*
	if (pNormalView->pPopupBtn) {
		elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_POPUP_PLAYER, pNormalView->pPopupBtn);
		elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pPopupBtn, NULL);
	}
	*/
	vp_play_util_focus_next_object_set(pNormalView->pVolumeBtn, pNormalView->pPrevBtn, ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pNormalView->pPrevBtn, pNormalView->pPlayFocusBtn, ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pNormalView->pPlayFocusBtn, pNormalView->pNextBtn, ELM_FOCUS_RIGHT);
	vp_play_util_focus_next_object_set(pNormalView->pNextBtn, pNormalView->pScreenSizeBtn, ELM_FOCUS_RIGHT);

	_vp_play_normal_view_set_rotate_lock_state(pNormalView);
	_vp_play_normal_view_set_volume_lock_state(pNormalView);

	return TRUE;
}

static void _vp_normal_view_function_layout_del_cb(void *data, Evas *e,
        Evas_Object *obj,
        void *event_info)
{
	if (!data) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)data;
	pNormalView->pTopControlLayout = NULL;
	pNormalView->pBookmarkBtn = NULL;
	pNormalView->pMultiWinBtn = NULL;
	pNormalView->pRepeatBtn = NULL;
	pNormalView->pTrimBtn = NULL;
	pNormalView->pSoundAliveBtn = NULL;
	pNormalView->pShareBtn = NULL;
}

static bool _vp_play_normal_view_create_function_control(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	pNormalView->pFunctionLayout = _vp_play_normal_view_create_function_layout(pNormalView->pMainLayout);
	if (!pNormalView->pFunctionLayout) {
		VideoLogError("pNormalView->pFunctionLayout is NULL");
		return FALSE;
	}
	evas_object_event_callback_add(pNormalView->pFunctionLayout, EVAS_CALLBACK_DEL, _vp_normal_view_function_layout_del_cb, (void *)pNormalView);
	elm_object_part_content_set(pNormalView->pControlLayout, VP_PLAY_SWALLOW_NORMAL_FUNCTION, pNormalView->pFunctionLayout);
	elm_object_focus_custom_chain_append(pNormalView->pMainLayout, pNormalView->pFunctionLayout, NULL);

	return TRUE;
}

static bool _vp_play_normal_view_create_progressbar(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;

	pParent = pNormalView->pControlLayout;

	pNormalView->pProgressbar = vp_play_progressbar_create(pParent, VIDEO_PROGRESSBAR_TYPE_NORMAL);
	if (!pNormalView->pProgressbar) {
		VideoLogError("pProgressbar create fail");
		return FALSE;
	}

	pNormalView->pProgressEvent = vp_play_progressbar_get_object(pNormalView->pProgressbar);
	if (!pNormalView->pProgressEvent) {
		VideoLogError("vp_play_progressbar_get_object fail");
		return FALSE;
	}

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_PROGRESS, pNormalView->pProgressEvent);

	evas_object_event_callback_add(pNormalView->pProgressEvent, EVAS_CALLBACK_MOUSE_DOWN,
	                               __vp_normal_progress_mouse_down_cb, (void *)pNormalView);

	evas_object_event_callback_add(pNormalView->pProgressEvent, EVAS_CALLBACK_MOUSE_UP,
	                               __vp_normal_progress_mouse_up_cb, (void *)pNormalView);

	evas_object_event_callback_add(pNormalView->pProgressEvent, EVAS_CALLBACK_MOUSE_MOVE,
	                               __vp_normal_progress_mouse_move_cb, (void *)pNormalView);

	evas_object_event_callback_add(pNormalView->pProgressEvent, EVAS_CALLBACK_DEL,
	                               __vp_normal_progress_del_cb, (void *)pNormalView);

	Evas_Object *pProgressFocus = vp_play_progressbar_get_focus_object(pNormalView->pProgressbar);

	evas_object_event_callback_add(pProgressFocus, EVAS_CALLBACK_KEY_DOWN,
	                               __vp_normal_focus_key_down_cb, (void *)pNormalView);
	evas_object_event_callback_add(pProgressFocus, EVAS_CALLBACK_KEY_UP,
	                               __vp_normal_focus_key_up_cb, (void *)pNormalView);


	return TRUE;
}

static bool _vp_play_normal_view_create_subtitle(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}

	Evas_Object *pParent = NULL;
	Evas_Object *pObj = NULL;

#ifdef SUBTITLE_K_FEATURE
	//PlayView *pPlayView = pNormalView->pPlayView;
	//pParent = pPlayView->pMainLayout;
	pParent = pNormalView->pMainLayout;
	VideoLogError("1");
#else
	pParent = pNormalView->pMainLayout;
	VideoLogError("2");
#endif


	pNormalView->pSubtitle = vp_play_subtitle_create(pParent, VP_SUBTITLE_TYPE_NORMAL);
	if (!pNormalView->pSubtitle) {
		VideoLogError("pSubtitle create fail");
		return FALSE;
	}

	pObj = vp_play_subtitle_get_object(pNormalView->pSubtitle);
	if (!pObj) {
		VideoLogError("vp_play_subtitle_get_object fail");
		return FALSE;
	}

	vp_play_subtitle_realize(pNormalView->pSubtitle);
	char *szFont = NULL;
	int nSize = 0;

#ifdef SUBTITLE_K_FEATURE
	int nAlignmentKey = VP_SUBTITLE_ALIGNMENT_CENTER;
	vp_subtitle_alignment_t nAlignment = VP_SUBTITLE_ALIGNMENT_CENTER;
#else
	video_subtitle_color_t nFontColor = VP_SUBTITLE_COLOR_BLACK;
	video_subtitle_color_t nBGColor = VP_SUBTITLE_COLOR_BLACK;
#endif

	vp_play_preference_get_subtitle_size_key(&nSize);
	vp_play_preference_get_subtitle_font_name_key(&szFont);
	vp_play_subtitle_set_size(pNormalView->pSubtitle, nSize);
	vp_play_subtitle_set_size_zoom(pNormalView->pSubtitle, 1.0);
	vp_play_subtitle_set_font(pNormalView->pSubtitle, szFont);

	VP_FREE(szFont);
#ifndef SUBTITLE_K_FEATURE
	int nFontColorKey = VP_SUBTITLE_COLOR_BLACK;
	int nBGColorKey = VP_SUBTITLE_COLOR_BLACK;
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
	}

	if (nBGColorKey == VIDEO_SUBTITLE_COLOR_BLACK) {
		nBGColor = VP_SUBTITLE_COLOR_BLACK;
	} else if (nBGColorKey == VIDEO_SUBTITLE_COLOR_WHITE) {
		nBGColor = VP_SUBTITLE_COLOR_WHITE;
	} else if (nBGColorKey == VIDEO_SUBTITLE_COLOR_NONE) {
		nBGColor = VP_SUBTITLE_COLOR_NONE;
	}
	vp_play_subtitle_set_color(pNormalView->pSubtitle, nFontColor);
	vp_play_subtitle_set_bg_color(pNormalView->pSubtitle, nBGColor);

	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_SUBTITLE, pObj);

#else
	vp_play_preference_get_subtitle_alignment_key(&nAlignmentKey);

	char *szColorHex = NULL;
	char *szColorBGHex = NULL;
	vp_play_preference_get_subtitle_font_color_hex_key(&szColorHex);
	if (!szColorHex) {
		VideoLogError("szColorHex is NULL");
	}

	vp_play_preference_get_subtitle_bg_color_hex_key(&szColorBGHex);
	if (!szColorBGHex) {
		VideoLogError("szColorBGHex is NULL");
	}


	if (nAlignmentKey == VIDEO_SUBTITLE_ALIGNMENT_LEFT) {
		nAlignment = VP_SUBTITLE_ALIGNMENT_LEFT;
	} else if (nAlignmentKey == VIDEO_SUBTITLE_ALIGNMENT_CENTER) {
		nAlignment = VP_SUBTITLE_ALIGNMENT_CENTER;
	} else if (nAlignmentKey == VIDEO_SUBTITLE_ALIGNMENT_RIGHT) {
		nAlignment = VP_SUBTITLE_ALIGNMENT_RIGHT;
	}

	vp_play_subtitle_set_color(pNormalView->pSubtitle, szColorHex);
	vp_play_subtitle_set_bg_color(pNormalView->pSubtitle, szColorBGHex);

	vp_play_subtitle_set_alignment(pNormalView->pSubtitle, nAlignment);
	elm_object_part_content_set(pParent, VP_PLAY_SWALLOW_NORMAL_SUBTITLE, pObj);
	evas_object_show(pObj);
#endif

	return TRUE;
}

#ifdef _PERF_TEST_
static Evas_Object *_vp_play_normal_view_create_conformant(Evas_Object *pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_conformant_add(pParent);
	if (!pObj) {
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

static Evas_Object *_vp_play_normal_view_create_layout(Evas_Object *pParent)
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

static Evas_Object *_vp_play_normal_view_create_naviframe(Evas_Object *pParent)
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
#endif

static void _vp_play_normal_view_del_cb(void *data, Evas *e,
                                        Evas_Object *obj,
                                        void *event_info)
{
	if (!data) {
		VideoLogError("pNormalView is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)data;
	if (pNormalView->pMainLayout) {
		evas_object_event_callback_del(pNormalView->pMainLayout, EVAS_CALLBACK_DEL, _vp_play_normal_view_del_cb);
		elm_object_focus_custom_chain_unset(pNormalView->pMainLayout);
		pNormalView->pMainLayout = NULL;
		//pNormalView->pNaviItem = NULL;
		pNormalView->pAllsharBtn = NULL;
		//if (pNormalView->pPlayView)
		//pNormalView->pPlayView->pNormalView = NULL;
	}
}

static bool _vp_play_normal_view_init_layout(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}
#ifdef _PERF_TEST_
	if (pNormalView->pMainLayout) {
		VideoLogError("Already exists MainLayout: %p", pNormalView->pMainLayout);
		return TRUE;
	}
//////////
	PlayView *pPlayView = pNormalView->pPlayView;


	pPlayView->pConformant = _vp_play_normal_view_create_conformant(pPlayView->pWin);
	if (!pPlayView->pConformant) {
		VideoLogError("pConformant is null");
		return FALSE;
	}

	pPlayView->pMainLayout = _vp_play_normal_view_create_layout(pPlayView->pConformant);
	if (!pPlayView->pMainLayout) {
		VideoLogError("pMainLayout is null");
		return FALSE;
	}
	pPlayView->pNaviframe = _vp_play_normal_view_create_naviframe(pPlayView->pMainLayout);
	if (!pPlayView->pNaviframe) {
		VideoLogError("pNaviframe is null");
		return FALSE;
	}

	pNormalView->pNaviFrame = pPlayView->pNaviframe;
	pNormalView->bDestroyNavi = FALSE;
	pNormalView->nXwinID = elm_win_xwindow_get(pPlayView->pWin);

//////////
#endif

	if (!_vp_play_normal_view_create_gesture_layout(pNormalView)) {
		VideoLogError("_vp_play_normal_view_create_gesture_layout is fail");
		return FALSE;
	}

	pNormalView->pMainLayout = _vp_play_normal_view_create_main_layout(pNormalView->pNaviFrame);
	if (!pNormalView->pMainLayout) {
		VideoLogError("pLayout create fail");
		return FALSE;
	}

	evas_object_event_callback_add(pNormalView->pMainLayout, EVAS_CALLBACK_DEL, _vp_play_normal_view_del_cb, (void *)pNormalView);
	evas_object_event_callback_add(pNormalView->pMainLayout, EVAS_CALLBACK_RESIZE, __vp_normal_main_layout_resize_cb, (void *)pNormalView);
	elm_access_highlight_set(pNormalView->pMainLayout);

	pNormalView->pNaviItem = elm_naviframe_item_push(pNormalView->pNaviFrame , NULL, NULL, NULL, pNormalView->pMainLayout, "playview/1line");

	Evas_Object * pLeftbtn = elm_button_add(pNormalView->pNaviFrame);
	elm_object_style_set(pLeftbtn, "back");
	evas_object_smart_callback_add(pLeftbtn, "clicked", __vp_normal_naviframe_back_btn_cb, (void *)pNormalView);
	elm_object_item_part_content_set(pNormalView->pNaviItem, "title_left_btn", pLeftbtn);
	evas_object_show(pLeftbtn);

	elm_naviframe_item_pop_cb_set(pNormalView->pNaviItem, __vp_normal_naviframe_pop_cb, (void *)pNormalView);

	if (!pNormalView->pNaviItem) {
		VideoLogError("elm_naviframe_item_push fail");
		return FALSE;
	}

	elm_object_signal_callback_add(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SHOW_FINISH, "*", __vp_normal_show_finish_cb, (void *)pNormalView);
	elm_object_signal_callback_add(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_HIDE_FINISH, "*", __vp_normal_hide_finish_cb, (void *)pNormalView);

	if (!_vp_play_normal_view_create_top_control(pNormalView)) {
		VideoLogError("_vp_play_normal_view_create_top_control fail");
		return FALSE;
	}

	if (!_vp_play_normal_view_create_main_control(pNormalView)) {
		VideoLogError("_vp_play_normal_view_create_main_control fail");
		return FALSE;
	}

	if (!_vp_play_normal_view_create_function_control(pNormalView)) {
		VideoLogError("_vp_play_normal_view_create_function_control fail");
		return FALSE;
	}

	if (!_vp_play_normal_view_create_progressbar(pNormalView)) {
		VideoLogError("_vp_play_normal_view_create_progressbar fail");
		return FALSE;
	}

	if (!_vp_play_normal_view_create_subtitle(pNormalView)) {
		VideoLogError("_vp_play_normal_view_create_subtitle fail");
		return FALSE;
	}

	if (!_vp_play_normal_view_create_loading_ani(pNormalView)) {
		VideoLogError("_vp_play_normal_view_create_loading_ani fail");
		return FALSE;
	}
	return TRUE;
}

static void _vp_play_normal_view_destroy_handle(NormalView *pNormalView)
{
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return;
	}
	VideoLogError("_vp_play_normal_view_destroy Start");
	vp_language_list_destroy();
	if (pNormalView->pMainLayout) {
		elm_object_focus_custom_chain_unset(pNormalView->pMainLayout);
	}

	if (pNormalView->pPlayerHandle) {
		vp_mm_player_destroy(pNormalView->pPlayerHandle);
		pNormalView->pPlayerHandle = NULL;
	}

	if (pNormalView->pVolumeHandle) {
		vp_play_volume_destroy(pNormalView->pVolumeHandle);
		pNormalView->pVolumeHandle = NULL;
	}

	if (pNormalView->pVolumePopupHandle) {
		vp_play_volume_popup_destroy(pNormalView->pVolumePopupHandle);
		pNormalView->pVolumePopupHandle = NULL;
	}

	if (pNormalView->pBrightnessHandle) {
		vp_play_brightness_destroy(pNormalView->pBrightnessHandle);
		pNormalView->pBrightnessHandle = NULL;
	}

//	if (pNormalView->pDownloadHandle) {
//		VppDownloadDestroyItem(pNormalView->pDownloadHandle);
//		pNormalView->pDownloadHandle = NULL;
//	}

	VP_EVAS_PIPE_DEL(pNormalView->pPreparePipe);
	VP_EVAS_PIPE_DEL(pNormalView->pSeekPipe);

	VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pLockScreenTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pExitWaitTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pMomentTimer);

	if (pNormalView->pProgressEvent) {
		evas_object_event_callback_del(pNormalView->pProgressEvent,
		                               EVAS_CALLBACK_MOUSE_DOWN,
		                               __vp_normal_progress_mouse_down_cb);

		evas_object_event_callback_del(pNormalView->pProgressEvent,
		                               EVAS_CALLBACK_MOUSE_UP,
		                               __vp_normal_progress_mouse_up_cb);

		evas_object_event_callback_del(pNormalView->pProgressEvent,
		                               EVAS_CALLBACK_MOUSE_MOVE,
		                               __vp_normal_progress_mouse_move_cb);
		pNormalView->pProgressEvent = NULL;
	}
#ifdef K_FEATURE_MICROSEEK
	VP_EVAS_DEL(pNormalView->pMicroSeekTextLayout);
	VP_EVAS_DEL(pNormalView->pMicroSeekImageLayout);
#endif
#ifdef FLICK_JUMP
	VP_EVAS_DEL(pNormalView->pFlickJumpSeek);
#endif

	vp_audio_track_destroy(pNormalView->pAudioTrackHandle);
	pNormalView->pAudioTrackHandle = NULL;

	vp_share_destroy(pNormalView->pShareHandle);
	pNormalView->pShareHandle = NULL;

	vp_sound_alive_destroy(pNormalView->pSAHandle);
	pNormalView->pSAHandle = NULL;

	vp_subtitle_destroy(pNormalView->pSubtitlePopup);
	pNormalView->pSubtitlePopup = NULL;

	vp_subtitle_sync_popup_destroy(pNormalView->pSubtitleSyncPopup);
	pNormalView->pSubtitleSyncPopup = NULL;

	vp_subtitle_select_destroy(pNormalView->pSubtitleSelectPopup);
	pNormalView->pSubtitleSelectPopup = NULL;

	vp_subtitle_font_destroy(pNormalView->pSubtitleFontPopup);
	pNormalView->pSubtitleFontPopup = NULL;

	vp_subtitle_size_destroy(pNormalView->pSubtitleSizePopup);
	pNormalView->pSubtitleSizePopup = NULL;

	vp_subtitle_color_destroy(pNormalView->pSubtitleFontColorPopup);
	pNormalView->pSubtitleFontColorPopup = NULL;
#ifndef SUBTITLE_K_FEATURE
	vp_subtitle_bg_color_destroy(pNormalView->pSubtitleBGColorPopup);
	pNormalView->pSubtitleBGColorPopup = NULL;
#else
	vp_subtitle_color_destroy(pNormalView->pSubtitleBGColorPopup);
	pNormalView->pSubtitleBGColorPopup = NULL;
#endif
	vp_subtitle_edge_destroy(pNormalView->pSubtitleEdgePopup);
	pNormalView->pSubtitleEdgePopup = NULL;

	VP_EVAS_TIMER_DEL(pNormalView->pSubtitleTimer);

	VP_EVAS_TIMER_DEL(pNormalView->pScreenShotTimer);
	VP_EVAS_DEL(pNormalView->pScreenShot);

	vp_repeat_destroy(pNormalView->pRepeatHandle);
	pNormalView->pRepeatHandle = NULL;

	vp_play_loading_ani_destroy(pNormalView->pLoadingAni);

	vp_sound_path_destroy(pNormalView->pSoundPathHandle);
	pNormalView->pSoundPathHandle = NULL;

	vp_setting_destroy(pNormalView->pSettingHandle);
	pNormalView->pSettingHandle = NULL;

	vp_capture_popup_destroy(pNormalView->pCapturePopup);
	pNormalView->pCapturePopup = NULL;

	vp_play_speed_popup_destroy(pNormalView->pPlaySpeedPopup);
	pNormalView->pPlaySpeedPopup = NULL;

	vp_detail_destroy(pNormalView->pDetailHandle);
	pNormalView->pDetailHandle = NULL;

	vp_zoom_guide_destroy(pNormalView->pZoomGuide);
	pNormalView->pZoomGuide = NULL;

	VP_EVAS_JOB_DEL(pNormalView->pVolumeJob);
	VP_EVAS_JOB_DEL(pNormalView->pBrightnessJob);

	if (pNormalView->pMediaKey) {
		vp_media_key_destroy(pNormalView->pMediaKey);
		pNormalView->pMediaKey = NULL;
	}
	if (pNormalView->pSubtitle) {
		vp_play_subtitle_destroy(pNormalView->pSubtitle);
		pNormalView->pSubtitle = NULL;
	}

	if (pNormalView->pBookmarkHandle) {
		vp_play_bookmark_destroy(pNormalView->pBookmarkHandle);
		pNormalView->pBookmarkHandle = NULL;
	}

	if (pNormalView->pProgressbar) {
		vp_play_progressbar_destroy(pNormalView->pProgressbar);
		pNormalView->pProgressbar = NULL;
	}

	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
	VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
	VP_EVAS_IDLER_DEL(pNormalView->pMultiWinIdler);

	VP_EVAS_DEL(pNormalView->pImageBufferObj);

	VP_EVAS_DEL(pNormalView->pSoundPathBtn);
	VP_EVAS_DEL(pNormalView->pLockBtn);

	VP_EVAS_DEL(pNormalView->pPopup);
	VP_EVAS_DEL(pNormalView->pSpeedBoxCtrl);
	VP_EVAS_DEL(pNormalView->pNextBtn);
	VP_EVAS_DEL(pNormalView->pPrevBtn);
	VP_EVAS_DEL(pNormalView->pPauseBtn);
	VP_EVAS_DEL(pNormalView->pResumeBtn);
	VP_EVAS_DEL(pNormalView->pPlayFocusBtn);

	VP_EVAS_DEL(pNormalView->pRotateBtn);
	VP_EVAS_DEL(pNormalView->pVolumeBtn);
	VP_EVAS_DEL(pNormalView->pScreenSizeBtn);
	VP_EVAS_DEL(pNormalView->pBookmarkBtn);
	VP_EVAS_DEL(pNormalView->pMultiWinBtn);
	VP_EVAS_DEL(pNormalView->pRepeatBtn);
	VP_EVAS_DEL(pNormalView->pTrimBtn);
	VP_EVAS_DEL(pNormalView->pSoundAliveBtn);
	VP_EVAS_DEL(pNormalView->pShareBtn);

	VP_EVAS_DEL(pNormalView->pCtxPopup);

	VP_EVAS_DEL(pNormalView->pBackBtn);
	VP_EVAS_DEL(pNormalView->pMoreBtn);
	VP_EVAS_DEL(pNormalView->pCaptureRewBtn);
	VP_EVAS_DEL(pNormalView->pCaptureBtn);
	VP_EVAS_DEL(pNormalView->pCaptureFFBtn);

	VP_EVAS_DEL(pNormalView->pMainEventRect);
	VP_EVAS_DEL(pNormalView->pGestureRect);

	VP_EVAS_DEL(pNormalView->pFunctionLayout);
	VP_EVAS_DEL(pNormalView->pTopControlLayout);
	VP_EVAS_DEL(pNormalView->pControlLayout);
	VP_EVAS_DEL(pNormalView->pNaviRightBtn);
	VP_EVAS_DEL(pNormalView->pNaviLeftBtn);
#ifdef _NATIVE_BUFFER_SYNC
	VP_EVAS_DEL(pNormalView->pVideoSink);
#endif
	VP_EVAS_DEL(pNormalView->pMainLayout);

	VP_FREE(pNormalView->szMediaURL);
	VP_FREE(pNormalView->szSubtitleURL);
	VP_FREE(pNormalView->szScreenMirrorMacAddr);
#ifdef _SUBTITLE_MULTI_LANGUAGE
	VP_FREE(pNormalView->pSelectedSubtitleLanguage);
#endif
	VP_FREE(pNormalView);
	VideoLogError("_vp_play_normal_view_destroy End");
}

static void _vp_play_normal_view_prepare_pipe_cb(void *data, void *pipeData, unsigned int nbyte)
{
	if (!data) {
		VideoLogError("data is NULL");
		return;
	}
	VideoLogInfo("");

	NormalView *pNormalView = (NormalView*)data;
	bool bLCDOffStatus = FALSE;

	vp_mm_player_set_rate(pNormalView->pPlayerHandle, (float)pNormalView->fPlaySpeed);

	vp_play_config_get_lcd_off_state(&bLCDOffStatus);
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView handle is NULL");
		return;
	}

	if (!pPlayView->pWin) {
		VideoLogError("pPlayView->pWin handle is NULL");
		return;
	}

	if (bLCDOffStatus && (pNormalView->bVideoOnlyMode == FALSE)) {
		VideoLogError("Invalid LCD Status");
		//elm_exit();

		pPlayView->pFunc->vp_play_func_exit(pPlayView);
		return;
	}

	int nDuration = 0;
	if (!vp_mm_player_get_duration(pNormalView->pPlayerHandle, &nDuration)) {
		VideoLogError("vp_mm_player_get_duration is fail");
	}
	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB && nDuration == 0) {
		pNormalView->bHLSMode = TRUE;
	} else {
		pNormalView->bHLSMode = FALSE;
	}

	if (pPlayView->bAVRCP) {
		const char *navi_title = elm_object_item_text_get(pNormalView->pNaviItem);
		vp_avrcp_noti_track(navi_title, "Unknown", "Unknown", "Unknown", nDuration);
	}

	if (pNormalView->bHLSMode == FALSE) {
		pNormalView->nDuration = nDuration;
		vp_play_progressbar_set_duration(pNormalView->pProgressbar, nDuration);
	}

	if (vp_mm_player_is_closed_caption_exist(pNormalView->pPlayerHandle)) {
		pNormalView->bIsExistSubtitle = TRUE;
	}

	if (pNormalView->bIsExistSubtitle) {
		_vp_play_normal_view_set_subtitle_font_config(pNormalView);
	}

	vp_play_progressbar_set_position(pNormalView->pProgressbar, pNormalView->nStartPosition);
	_vp_play_normal_view_check_audio_only(pNormalView);

	if (pNormalView->nStartPosition > 0) {
		pNormalView->bSeekComplete = FALSE;
		if (vp_mm_player_set_position(pNormalView->pPlayerHandle, pNormalView->nStartPosition) == FALSE) {
			VideoLogWarning("vp_mm_player_set_position error");
			return;
		}
	} else {
		if (pPlayView->bAVRCP) {
			vp_avrcp_noti_track_position(0);
		}
	}

	pNormalView->bIsRealize = TRUE;
	pNormalView->bSeekComplete = TRUE;
#if 1
	if (vp_play_util_check_streaming(pNormalView->szMediaURL)) {
		if (pNormalView->bBufferingComplete == FALSE) {
			VideoLogWarning("Skip for buffering state");
			return;
		}
	}
#endif
	if (pNormalView->pLoadingAni) {
		_vp_play_normal_view_create_layout_hide_timer(pNormalView);
		vp_play_loading_ani_destroy(pNormalView->pLoadingAni);
		pNormalView->pLoadingAni = NULL;
	}

	bool bXwincheck = TRUE;
	if (vp_play_util_is_Xwindow_focused(pNormalView->nXwinID) == FALSE) {
		VideoLogWarning("CASE : Not a splite view and Not focused state in mainwindow[%p]", pPlayView->pWin);
		bXwincheck = FALSE;
	}

	if (pNormalView->bManualPause || bXwincheck == FALSE || _vp_play_normal_view_check_during_call(pNormalView)) {
		vp_mm_player_set_visible(pNormalView->pPlayerHandle, TRUE);
		vp_mm_player_pause(pNormalView->pPlayerHandle);
	} else {
		vp_mm_player_play(pNormalView->pPlayerHandle);
	}

	if (!vp_mm_player_set_subtitle_position(pNormalView->pPlayerHandle, pNormalView->fSubtitleSyncValue * 1000)) {
		VideoLogError("vp_mm_player_set_subtitle_position is fail");
	}

	_vp_play_normal_view_set_play_state(pNormalView);
	_vp_play_normal_view_on_capture_mode(pNormalView);

}

normal_view_handle vp_play_normal_view_create(PlayView *pPlayView, video_play_launching_type_t nLaunchingType)
{
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return NULL;
	}

	NormalView	*pNormalView = NULL;

	pNormalView = calloc(1, sizeof(NormalView));

	if (!pNormalView) {
		VideoLogError("pNormalView alloc fail");
		return NULL;
	}

	int nScreenMode = VP_MM_PLAYER_DISPLAY_LETTER_BOX;
	vp_play_preference_get_screen_mode_key(&nScreenMode);
	if ((nScreenMode != VP_MM_PLAYER_DISPLAY_LETTER_BOX) && (nScreenMode != VP_MM_PLAYER_DISPLAY_FULL_SCREEN)) {
		nScreenMode = VP_MM_PLAYER_DISPLAY_LETTER_BOX;
		vp_play_preference_set_screen_mode_key(VP_MM_PLAYER_DISPLAY_LETTER_BOX);
	}
	pNormalView->nDisplayMode = nScreenMode;

	pNormalView->pPlayView = pPlayView;
	pNormalView->pNaviFrame = pPlayView->pNaviframe;
	pNormalView->nXwinID = elm_win_xwindow_get(pPlayView->pWin);
	pNormalView->nLaunchingType = nLaunchingType;
	pNormalView->nStartPosition = 0;
	pNormalView->nRepeatMode = VIDEO_PLAY_REPEAT_NONE;
	pNormalView->bManualPause = FALSE;
	pNormalView->nWaitPos = -1;
	pNormalView->fPlaySpeed = pPlayView->fPlaySpeed;
	pNormalView->fSubtitleSyncValue = pPlayView->fSubtitleSyncValue;
	curr_Handle = pNormalView;
	elm_object_focus_allow_set(pNormalView->pNaviFrame, EINA_TRUE);
	elm_object_tree_focus_allow_set(pNormalView->pNaviFrame, EINA_TRUE);
	elm_object_focus_set(pNormalView->pNaviFrame, EINA_TRUE);

	// create prepare_cb pipe
	VP_EVAS_PIPE_DEL(pNormalView->pPreparePipe);
	pNormalView->pPreparePipe = ecore_pipe_add((Ecore_Pipe_Cb)_vp_play_normal_view_prepare_pipe_cb, (void*)pNormalView);

	VP_EVAS_PIPE_DEL(pNormalView->pSeekPipe);
	pNormalView->pSeekPipe = ecore_pipe_add((Ecore_Pipe_Cb)__vp_play_normal_view_seek_pipe_cb, (void*)pNormalView);
#ifndef _PERF_TEST_
	if (!_vp_play_normal_view_init_layout(pNormalView)) {
		VideoLogError("_vp_play_normal_view_init_layout is fail");
		return NULL;
	}
#endif
	bool bMultiPlay = TRUE;
	vp_play_preference_get_multi_play_status(&bMultiPlay);
	if (bMultiPlay) {
		vp_play_preference_set_multi_play_status(FALSE);
	}

	return (normal_view_handle)pNormalView;
}

void vp_play_normal_view_key_create(PlayView *pPlayView, normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;
	pNormalView->pMediaKey = vp_media_key_create(pPlayView->pWin, __vp_normal_media_key_event_cb);
	vp_media_key_set_user_data(pNormalView->pMediaKey, (void *)pNormalView);
	vp_media_key_realize(pNormalView->pMediaKey);
}

void vp_play_normal_view_destroy(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	elm_access_object_unregister(pNormalView->pMainLayout);
	elm_object_focus_allow_set(pNormalView->pNaviFrame, EINA_FALSE);
	elm_object_tree_focus_allow_set(pNormalView->pNaviFrame, EINA_FALSE);
	elm_object_focus_set(pNormalView->pNaviFrame, EINA_FALSE);

	pNormalView->bDestroyNavi = TRUE;

	elm_naviframe_item_pop(pNormalView->pNaviFrame);

	_vp_play_normal_view_destroy_handle(pNormalView);
	curr_Handle = NULL;
}

bool vp_play_normal_view_realize(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return FALSE;
	}

	if (!pNormalView->pLoadingAni) {
		if (!_vp_play_normal_view_create_loading_ani(pNormalView)) {
			VideoLogWarning("_vp_play_normal_view_create_loading_ani fail");
		}
	}

	/* There is no repeat setting for these modes */
	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_WEB) {
		pNormalView->nRepeatMode = VIDEO_PLAY_REPEAT_NONE;
	} else if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
		if (pPlayView->bStopAfterRepeat == TRUE) {
			pNormalView->nRepeatMode = VIDEO_PLAY_REPEAT_ALL_STOP;
		} else {
			pNormalView->nRepeatMode = VIDEO_PLAY_REPEAT_NONE;
		}
	} else {
		int nVal = 0;
		vp_play_preference_get_repeat_mode_key(&nVal);
		pNormalView->nRepeatMode = (video_play_repeat_mode_t)nVal;
	}

	int nScreenMode = 0;
	vp_play_preference_get_screen_mode_key(&nScreenMode);
	pNormalView->nDisplayMode = (vp_mm_player_display_mode_t)nScreenMode;

	pNormalView->bSeekComplete = TRUE;
	pNormalView->bIsActive = TRUE;
	VideoLogInfo("vp_play_normal_view_realize");

	if (pNormalView->pMediaItemList) {
		vp_media_contents_free_video_items(pNormalView->pMediaItemList);
		pNormalView->pMediaItemList = NULL;
	}

	video_play_sort_type_t nType = VIDEO_SORT_BY_NONE;
	int nSortVal = 0;
	vp_play_config_get_sort_type_key(&nSortVal);
	nType = (video_play_sort_type_t)nSortVal;

	_vp_play_normal_view_set_volume_state(pNormalView);

	if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_GALLERY || pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MYFILE) {
		char *szFolder = vp_play_util_get_folder_from_path(pNormalView->szMediaURL);
		vp_media_contents_get_video_items_to_folder(VIDEO_SORT_BY_DATE_MOST_RECENT, szFolder, &(pNormalView->pMediaItemList));
		VP_FREE(szFolder);
	} else if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
	} else {
		if (pPlayView->nListType == VIDEO_PLAY_LIST_TYPE_FOLDER) {
			char *szFolder = vp_play_util_get_folder_from_path(pNormalView->szMediaURL);
			vp_media_contents_get_video_items_to_folder(nType, szFolder, &(pNormalView->pMediaItemList));
			VP_FREE(szFolder);
		} else {
			vp_media_contents_get_video_items(nType, &(pNormalView->pMediaItemList));
		}
	}
#ifdef _PERF_TEST_
	if (!_vp_play_normal_view_play_start(pNormalView, TRUE)) {
		VideoLogError("_vp_play_normal_view_play_start is Fail");
		return FALSE;
	}

	_vp_play_normal_view_init_layout(pNormalView);
	_vp_play_normal_view_hide_layout(pNormalView, TRUE);
#else
	_vp_play_normal_view_hide_layout(pNormalView, TRUE);

	if (!_vp_play_normal_view_play_start(pNormalView, TRUE)) {
		VideoLogError("_vp_play_normal_view_play_start is Fail");
		return FALSE;
	}
#endif
	_vp_play_normal_view_set_rotate_lock_state(pNormalView);
	_vp_play_normal_view_set_volume_lock_state(pNormalView);

	vp_play_preference_set_allow_dock_connect_key(FALSE);

	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_VIDEO_ON, "*");

	return TRUE;
}

bool vp_play_normal_view_unrealize(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (pNormalView->pNaviItem) {
		elm_object_item_text_set(pNormalView->pNaviItem, "");
	}
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_VIDEO_OFF, "*");

	_vp_play_normal_view_set_played_time(pNormalView);

	pNormalView->bIsActive = FALSE;
	pNormalView->bIsExistSubtitle = FALSE;
	VP_FREE(pNormalView->szSubtitleURL);
	VideoLogInfo("vp_play_normal_view_unrealize");

	if (pNormalView->pMediaItemList) {
		vp_media_contents_free_video_items(pNormalView->pMediaItemList);
		pNormalView->pMediaItemList = NULL;
	}

	if (pNormalView->pDetailHandle) {
		vp_detail_destroy(pNormalView->pDetailHandle);
		pNormalView->pDetailHandle = NULL;
	}

	vp_play_preference_set_allow_dock_connect_key(TRUE);

	vp_device_set_screen_off(FALSE);
	vp_play_util_set_unlock_power_key();
	vp_device_set_frame_rate(60);

	VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pSubtitleTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pProgressTimer);
	VP_EVAS_TIMER_DEL(pNormalView->pLockScreenTimer);
#ifdef _NATIVE_BUFFER_SYNC
	VP_EVAS_DEL(pNormalView->pVideoSink);
#endif

	if (pNormalView->pPlayerHandle) {
		vp_mm_player_destroy(pNormalView->pPlayerHandle);
		pNormalView->pPlayerHandle = NULL;
	}

	pNormalView->bIsRealize = FALSE;

	return TRUE;
}

bool vp_play_normal_view_is_realize(normal_view_handle pViewHandle, bool *bIsRealize)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	*bIsRealize = pNormalView->bIsRealize;

	return TRUE;
}

bool vp_play_normal_view_pause(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	vp_mm_player_pause(pNormalView->pPlayerHandle);
	_vp_play_normal_view_set_play_state(pNormalView);

	return TRUE;
}

bool vp_play_normal_view_resume(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (pNormalView->pPlayerHandle && pNormalView->bIsRealize) {
		_vp_play_normal_view_all_close_popup(pNormalView);
	}
	if (pNormalView->bManualPause == FALSE && pNormalView->bSharepopup == FALSE) {

		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

		if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}
		if(!_vp_play_normal_view_check_during_call(pNormalView)) {
			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogError("Unable to play!");
				}
				_vp_play_normal_view_set_play_state(pNormalView);
			}
		}
	}

	return TRUE;
}

bool vp_play_normal_view_resume_or_pause(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("[ERR] No Exist pUserData.");
		return FALSE;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	switch (nState) {
	case VP_MM_PLAYER_STATE_NONE:
	case VP_MM_PLAYER_STATE_IDLE:
	case VP_MM_PLAYER_STATE_READY:
	case VP_MM_PLAYER_STATE_PAUSED:
		_vp_play_normal_view_all_close_popup(pNormalView);
		if(!_vp_play_normal_view_check_during_call(pNormalView))
			vp_mm_player_play(pNormalView->pPlayerHandle);
		break;
	case VP_MM_PLAYER_STATE_PLAYING:
		vp_mm_player_pause(pNormalView->pPlayerHandle);
		break;
	case VP_MM_PLAYER_STATE_STOP:
		break;
	}

	_vp_play_normal_view_set_play_state(pNormalView);

	return TRUE;
}

bool vp_play_normal_view_next_play(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;

	_vp_play_normal_view_on_next_play(pNormalView, TRUE);

	return TRUE;
}

bool vp_play_normal_view_prev_play(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	_vp_play_normal_view_on_prev_play(pNormalView, TRUE, FALSE);

	return TRUE;
}

bool vp_play_normal_view_set_url(normal_view_handle pViewHandle, const char *szMediaURL)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;

	VP_FREE(pNormalView->szMediaURL);
	VP_STRDUP(pNormalView->szMediaURL, szMediaURL);

	return TRUE;

}

bool vp_play_normal_view_player_state_changed_get(normal_view_handle pViewHandle)
{
	NormalView *pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (nState == pState) {
		return FALSE;
	} else {
		pState = nState;
	}

	return TRUE;
}

bool vp_play_normal_view_stop_player(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;
	if (!(vp_mm_player_stop(pNormalView->pPlayerHandle))) {
		VideoLogError("player_stop failed");
		return FALSE;
	}
	return TRUE;
}

bool vp_play_normal_view_set_video_only(normal_view_handle pViewHandle, bool bVideoOnly)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;

	PlayView *pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return FALSE;
	}

	pNormalView->bVideoOnlyMode = bVideoOnly;

	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}

	if (nState != VP_MM_PLAYER_STATE_NONE && nState != VP_MM_PLAYER_STATE_IDLE) {
		vp_play_subtitle_set_text(pNormalView->pSubtitle, "");
	}

	if (bVideoOnly == TRUE) {
		if (pNormalView->pImageBufferObj) {
			elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_IMAGE_BUFFER);
			VP_EVAS_DEL(pNormalView->pImageBufferObj);
		}

		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_NO_CONTNES_SHOW, "*");

		VP_EVAS_DISABLE_SET(pNormalView->pSoundPathBtn, EINA_TRUE);

		//VP_EVAS_DISABLE_SET(pNormalView->pRotateBtn, EINA_TRUE);
		if (pNormalView->pRotateBtn) {
			_vp_play_normal_view_set_rotate_lock_state(pNormalView);
			_vp_play_normal_view_set_volume_lock_state(pNormalView);
		}

		VP_EVAS_DISABLE_SET(pNormalView->pScreenSizeBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pNormalView->pBookmarkBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pNormalView->pMultiWinBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pNormalView->pRepeatBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pNormalView->pTrimBtn, EINA_TRUE);
		VP_EVAS_DISABLE_SET(pNormalView->pShareBtn, EINA_TRUE);
	} else {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_NO_CONTNES_HIDE, "*");

		VP_FREE(pNormalView->szScreenMirrorMacAddr);
		//VP_EVAS_DISABLE_SET(pNormalView->pRotateBtn, EINA_FALSE);
		if (pNormalView->pRotateBtn) {
			_vp_play_normal_view_set_rotate_lock_state(pNormalView);
			_vp_play_normal_view_set_volume_lock_state(pNormalView);
		}
		VP_EVAS_DISABLE_SET(pNormalView->pScreenSizeBtn, EINA_FALSE);
		VP_EVAS_DISABLE_SET(pNormalView->pRepeatBtn, EINA_FALSE);
		if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_EMAIL) {
			char *szMediaID = NULL;
			bool bReturn = FALSE;
			bReturn = vp_media_contents_get_video_id(pNormalView->szMediaURL, &szMediaID);

			if (szMediaID && bReturn) {
				VP_EVAS_DISABLE_SET(pNormalView->pTrimBtn, EINA_FALSE);
			} else {
				VP_EVAS_DISABLE_SET(pNormalView->pTrimBtn, EINA_TRUE);
			}
			VP_FREE(szMediaID);
		} else {
			VP_EVAS_DISABLE_SET(pNormalView->pTrimBtn, EINA_FALSE);
		}
		VP_EVAS_DISABLE_SET(pNormalView->pSoundAliveBtn, EINA_FALSE);

		if (pNormalView->nLaunchingType == VIDEO_PLAY_TYPE_MULTI_PATH) {
			if (vp_play_util_check_streaming(pNormalView->szMediaURL) == TRUE) {
				VP_EVAS_DISABLE_SET(pNormalView->pShareBtn, EINA_TRUE);
			} else {
				VP_EVAS_DISABLE_SET(pNormalView->pShareBtn, EINA_FALSE);
			}
		} else {
			VP_EVAS_DISABLE_SET(pNormalView->pShareBtn, EINA_FALSE);
		}

		VP_EVAS_DISABLE_SET(pNormalView->pBookmarkBtn, EINA_FALSE);
		VP_EVAS_DISABLE_SET(pNormalView->pMultiWinBtn, EINA_FALSE);

		vp_play_progressbar_set_opacity(pNormalView->pProgressbar, FALSE);
	}

	_vp_play_normal_view_on_capture_mode(pNormalView);
	_vp_play_normal_view_set_play_state(pNormalView);

	return TRUE;
}

bool vp_play_normal_view_get_video_only(normal_view_handle pViewHandle, bool *bVideoOnly)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	*bVideoOnly = pNormalView->bVideoOnlyMode;

	return TRUE;

}

bool vp_play_normal_view_set_device_unplug_interrupt(normal_view_handle pViewHandle, bool bDeviceUnplugInterrupt)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->bDeviceRemoveInterrupt = bDeviceUnplugInterrupt;

	return TRUE;
}

bool vp_play_normal_view_set_start_position(normal_view_handle pViewHandle, int nStartPosition)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	if (nStartPosition < 0) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->nStartPosition = nStartPosition;

	return TRUE;
}

bool vp_play_normal_view_get_last_position(normal_view_handle pViewHandle, int *nCurPos)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	int nPosition = 0;

	if (pNormalView->bHLSMode) {
		*nCurPos = 0;
		return TRUE;
	}

	if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
		VideoLogError("vp_mm_player_get_position is fail");
		return FALSE;
	}

	pNormalView->nCurPosition = nPosition;

	*nCurPos = pNormalView->nCurPosition;

	return TRUE;
}

bool vp_play_normal_view_set_update_last_position(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	_vp_play_normal_view_set_played_time(pNormalView);

	return TRUE;

}

bool vp_play_normal_view_set_subtitle(normal_view_handle pViewHandle, char *szSubtitle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	VP_FREE(pNormalView->szSubtitleURL);

	VP_STRDUP(pNormalView->szSubtitleURL, szSubtitle);

	return TRUE;
}

bool vp_play_normal_view_get_subtitle(normal_view_handle pViewHandle, char **szSubtitle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (!vp_mm_player_get_subtitle_url(pNormalView->pPlayerHandle, szSubtitle)) {
		VideoLogError("vp_mm_player_get_subtitle_url is fail");
		return FALSE;
	}

	return TRUE;
}

bool vp_play_normal_view_get_video_duration(normal_view_handle pViewHandle, int *nDuration)
{
	if (!pViewHandle || !nDuration) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView *pNormalView = (NormalView *)pViewHandle;
	*nDuration = pNormalView->nDuration;

	return TRUE;
}

bool vp_play_normal_view_get_audio_track(normal_view_handle pViewHandle, int *nAudioTrack)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	*nAudioTrack = pNormalView->nDefaultAudioTrackIndex;

	return TRUE;
}

bool vp_play_normal_view_set_audio_track(normal_view_handle pViewHandle, int nAudioTrack)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->nDefaultAudioTrackIndex = nAudioTrack;

	return TRUE;
}

bool vp_play_normal_view_set_manual_pause(normal_view_handle pViewHandle, bool bManualPause)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->bManualPause = bManualPause;

	return TRUE;

}

bool vp_play_normal_view_get_manual_pause(normal_view_handle pViewHandle, bool *bManualPause)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	*bManualPause = pNormalView->bManualPause;

	return TRUE;
}

void vp_play_normal_check_detail_view(normal_view_handle pViewHandle) {

	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (pNormalView->pDetailHandle) {
		vp_detail_destroy(pNormalView->pDetailHandle);
		pNormalView->pDetailHandle = NULL;
	}

	return;
}

bool vp_play_normal_view_get_pause_status(normal_view_handle pViewHandle, bool *bPause)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}


	if (nState == VP_MM_PLAYER_STATE_PLAYING) {
		*bPause = FALSE;
	} else {
		*bPause = TRUE;
	}

	return TRUE;
}

bool vp_play_normal_view_get_sound_filter(normal_view_handle pViewHandle, video_sound_alive_t *nSoundAlive)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	*nSoundAlive = pNormalView->nSoundAlive;

	return TRUE;
}

bool vp_play_normal_view_set_sound_filter(normal_view_handle pViewHandle, video_sound_alive_t nSoundAlive)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->nSoundAlive = nSoundAlive;

	return TRUE;
}


bool vp_play_normal_view_set_rotate(normal_view_handle pViewHandle, video_play_rotate_t nRotate, bool bLayoutHide)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	PlayView	*pPlayView = pNormalView->pPlayView;

	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return FALSE;
	}

	if (nRotate == VIDEO_PLAY_ROTATE_NONE) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_0);
	} else if (nRotate == VIDEO_PLAY_ROTATE_90) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_90);
	} else if (nRotate == VIDEO_PLAY_ROTATE_180) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_180);
	} else if (nRotate == VIDEO_PLAY_ROTATE_270) {
		vp_mm_player_set_video_rotate(pNormalView->pPlayerHandle, VP_MM_PLAYER_ROTATE_270);
	}

	int nAngle = elm_win_rotation_get(pPlayView->pWin);
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

	if (nRotate != nCurRot) {
		if (nRotate == VIDEO_PLAY_ROTATE_NONE) {
			elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, 0);
		} else if (nRotate == VIDEO_PLAY_ROTATE_90) {
			elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, 90);
		} else if (nRotate == VIDEO_PLAY_ROTATE_180) {
			elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, 180);
		} else if (nRotate == VIDEO_PLAY_ROTATE_270) {
			elm_win_wm_rotation_preferred_rotation_set(pPlayView->pWin, 270);
		}
	}

	if (pNormalView->nRotate != nRotate) {
		if (pNormalView->pBookmarkHandle) {
			bool bIsReailze = FALSE;
			vp_play_bookmark_is_realize(pNormalView->pBookmarkHandle, &bIsReailze);

			if (bIsReailze) {
				_vp_play_normal_view_on_bookmark_mode(pNormalView, FALSE);
				vp_play_bookmark_unrealize(pNormalView->pBookmarkHandle);
			}
		}
	}

	if (pNormalView->pVolumeHandle) {
		bool bIsRealize = FALSE;

		if (!vp_play_volume_is_realize(pNormalView->pVolumeHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_volume_is_realize is fail");
		}

		if (bIsRealize) {
			if (!vp_play_volume_unrealize(pNormalView->pVolumeHandle)) {
				VideoLogWarning("vp_play_volume_unrealize is fail");
			}
		}
	}

	if (pNormalView->pVolumePopupHandle) {
		bool bIsRealize = FALSE;

		if (!vp_play_volume_popup_is_realize(pNormalView->pVolumePopupHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_volume_popup_is_realize is fail");
		}

		if (bIsRealize) {
			if (!vp_play_volume_popup_unrealize(pNormalView->pVolumePopupHandle)) {
				VideoLogWarning("vp_play_volume_popup_unrealize is fail");
			}
		}
	}

	if (pNormalView->pBrightnessHandle) {
		bool bIsRealize = FALSE;

		if (!vp_play_brightness_is_realize(pNormalView->pBrightnessHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_volume_is_realize is fail");
		}
		if (bIsRealize) {
			if (!vp_play_brightness_unrealize(pNormalView->pBrightnessHandle)) {
				VideoLogWarning("vp_play_brightness_unrealize is fail");
			}
		}
	}

	_vp_play_normal_view_set_rotate_lock_state(pNormalView);
	bool bLandscape = vp_play_util_get_landscape_check(nRotate);

	if (bLandscape) {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_LANDSCAPE_SHOW, "*");
#ifdef SUBTITLE_K_FEATURE
		elm_object_signal_emit(vp_play_subtitle_get_object(pNormalView->pSubtitle), VP_SUBTITLE_SIGNAL_MAIN_LANDSCAPE_SHOW, "*");
		evas_object_move(vp_play_subtitle_get_object(pNormalView->pSubtitle), VP_SUBTITLE_LANDSCAPE_POSX, VP_SUBTITLE_LANDSCAPE_POSY);
		pNormalView->nInitSubtitleX = VP_SUBTITLE_LANDSCAPE_POSX;
		pNormalView->nInitSubtitleY = VP_SUBTITLE_LANDSCAPE_POSY;
#endif
	} else {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_PORTRAIT_SHOW, "*");
#ifdef SUBTITLE_K_FEATURE
		elm_object_signal_emit(vp_play_subtitle_get_object(pNormalView->pSubtitle), VP_SUBTITLE_SIGNAL_MAIN_PORTRAIT_SHOW, "*");
		evas_object_move(vp_play_subtitle_get_object(pNormalView->pSubtitle), VP_SUBTITLE_PORTRAIT_POSX, VP_SUBTITLE_PORTRAIT_POSY);
		pNormalView->nInitSubtitleX = VP_SUBTITLE_PORTRAIT_POSX;
		pNormalView->nInitSubtitleY = VP_SUBTITLE_PORTRAIT_POSY;
#endif
	}

#ifndef SUBTITLE_K_FEATURE
	if (pNormalView->nDisplayMode != VP_MM_PLAYER_DISPLAY_FULL_SCREEN) {
		if (nRotate == VIDEO_PLAY_ROTATE_NONE || nRotate == VIDEO_PLAY_ROTATE_180) {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_PORTRAIT, "*");
		} else {
			elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_HIDE, "*");
		}
	} else {
		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SUBTITLE_HIDE, "*");
	}
#endif

	if (bLayoutHide) {
		_vp_play_normal_view_hide_layout(pNormalView, TRUE);
	}

	VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
	elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE, "*");
	pNormalView->nRotate = nRotate;

#ifdef FLICK_JUMP
	__vp_normal_flickjump_layout_position(pNormalView);
#endif

	if (pNormalView->pCtxPopup) {
		PlayView *pPlayView = pNormalView->pPlayView;

		if (!pPlayView) {
			VideoLogError("pPlayView is NULL");
			return FALSE;
		}

		int nW = 0;
		int nH = 0;

		Evas_Coord_Rectangle rect = {0,};
		elm_win_screen_size_get(pPlayView->pWin,  &rect.x, &rect.y, &rect.w, &rect.h);
		if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_NONE) {
			nW = rect.w / 2;
			nH = rect.h;
		} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_90) {
			nW = rect.h / 2;
			nH = rect.w;
		} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_180) {
			nW = rect.w / 2;
			nH = rect.h;
		} else if (pNormalView->nRotate == VIDEO_PLAY_ROTATE_270) {
			nW = rect.h / 2;
			nH = rect.w;
		}

		evas_object_move(pNormalView->pCtxPopup, nW, nH);

		evas_object_show(pNormalView->pCtxPopup);
	}

	return TRUE;
}

bool vp_play_normal_view_set_launching_mode(normal_view_handle pViewHandle, video_play_launching_type_t nLaunchingType)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->nLaunchingType = nLaunchingType;

	return TRUE;

}
bool vp_play_normal_view_update(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	VideoLogWarning(" == NORMAL VIEW UPDATE ==");

	if (pNormalView->pVolumeHandle) {
		bool bIsRealize = FALSE;
		if (!vp_play_volume_is_realize(pNormalView->pVolumeHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_volume_is_realize is fail");
		}

		if (bIsRealize) {
			/*update volume popup*/
			if (!vp_play_normal_view_volume_update(pNormalView)) {
				VideoLogWarning("vp_play_normal_view_volume_update is fail");
			}
		}
	}

	if (pNormalView->pVolumePopupHandle) {
		bool bIsRealize = FALSE;

		if (!vp_play_volume_popup_is_realize(pNormalView->pVolumePopupHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_volume_popup_is_realize is fail");
		}

		if (bIsRealize) {
			if (!vp_play_volume_popup_unrealize(pNormalView->pVolumePopupHandle)) {
				VideoLogWarning("vp_play_volume_popup_unrealize is fail");
			}
		}
	}

	if (pNormalView->pBrightnessHandle) {
		bool bIsRealize = FALSE;

		if (!vp_play_brightness_is_realize(pNormalView->pBrightnessHandle, &bIsRealize)) {
			VideoLogWarning("vp_play_volume_is_realize is fail");
		}
		if (bIsRealize) {
			if (!vp_play_brightness_unrealize(pNormalView->pBrightnessHandle)) {
				VideoLogWarning("vp_play_brightness_unrealize is fail");
			}
		}
	}

	bool bLock = FALSE;
	vp_play_normal_view_get_lock_screen(pNormalView, &bLock);
	if (bLock) {
		vp_play_normal_view_set_lock_screen(pNormalView, bLock);
	}

	_vp_play_normal_view_set_sound_path_state(pNormalView);
//	_vp_play_normal_view_set_rotate_lock_state(pNormalView);
//	_vp_play_normal_view_set_volume_lock_state(pNormalView);

	VideoLogWarning("share popup==%d,%d", pNormalView->bSharePanelFocusIn, pNormalView->bSharePanelPause);
	if (pNormalView->bSharePanelFocusIn && pNormalView->bSharePanelPause) {
		VideoLogWarning("pNormalView->bManualPause == FALSE");
		pNormalView->bSharePanelPause = FALSE;
		pNormalView->bSharepopup = FALSE;

		vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

		if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
			VideoLogWarning("vp_mm_player_get_state is fail");
		}
		if(!_vp_play_normal_view_check_during_call(pNormalView)) {
			if (nState != VP_MM_PLAYER_STATE_PLAYING) {
				if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
					VideoLogError("Unable to play");
				}
				_vp_play_normal_view_set_play_state(pNormalView);
			}
		}
	}

	return TRUE;
}

bool vp_play_normal_view_rotate_update(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	NormalView *pNormalView = (NormalView *)pViewHandle;
	_vp_play_normal_view_set_rotate_lock_state(pNormalView);
	if (pNormalView->bShowLayout) {
		_vp_play_normal_view_hide_layout(pNormalView, TRUE);
	}
	return TRUE;
}

bool vp_play_normal_view_set_share_panel_state(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("PlayView handle is NULL");
		return FALSE;
	}
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	pNormalView->bSharepopup = FALSE;
	pNormalView->bSharePanelFocusIn = TRUE;

	PlayView *pPlayView = pNormalView->pPlayView;
	if(pPlayView->bIsPlayBeforeShare) {
		vp_play_normal_view_resume_or_pause(pPlayView->pNormalView);
	}

	return TRUE;
}

bool vp_play_normal_view_on_more_menu(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	VideoLogInfo("bIsPopupShow=%d,bIsSubtitleShow=%d", pNormalView->bIsPopupShow, pNormalView->bIsSubtitleShow);

	if (pNormalView->pDetailHandle != NULL) {
		VideoLogError("Detail View exists");
		return NULL;
	}

	if (pNormalView->bIsPopupShow || pNormalView->bIsSubtitleShow) {
		if (pNormalView->pCtxPopup) {
			elm_ctxpopup_dismiss(pNormalView->pCtxPopup);
		}
		return TRUE;
	}

	if (pNormalView->pCtxPopup) {
		elm_ctxpopup_dismiss(pNormalView->pCtxPopup);

		if (!vp_play_volume_unrealize(pNormalView->pVolumeHandle)) {
			VideoLogWarning("vp_play_volume_unrealize is fail");
		}
	} else {
		__vp_normal_more_btn_clicked_cb((void *)pNormalView, pNormalView->pMoreBtn, NULL);
	}

	return TRUE;

}

bool vp_play_normal_view_volume_increase(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (pNormalView->pCtxPopup) {
		elm_ctxpopup_dismiss(pNormalView->pCtxPopup);
	}

	_vp_play_normal_view_on_volume_popup(pNormalView, FALSE);

	int nCurVal = 0;

	vp_play_volume_increase_value(pNormalView->pVolumeHandle);
	nCurVal = vp_play_volume_get_value(pNormalView->pVolumeHandle);

	_vp_play_normal_view_set_volume_state(pNormalView);

	pNormalView->nLastVolume = nCurVal;

	_vp_play_normal_view_create_layout_hide_timer(pNormalView);

	return TRUE;
}

bool vp_play_normal_view_volume_decrease(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (pNormalView->pCtxPopup) {
		elm_ctxpopup_dismiss(pNormalView->pCtxPopup);
	}

	_vp_play_normal_view_on_volume_popup(pNormalView, FALSE);

	int nCurVal = 0;

	vp_play_volume_decrease_value(pNormalView->pVolumeHandle);
	nCurVal = vp_play_volume_get_value(pNormalView->pVolumeHandle);

	_vp_play_normal_view_set_volume_state(pNormalView);

	pNormalView->nLastVolume = nCurVal;

	_vp_play_normal_view_create_layout_hide_timer(pNormalView);

	return TRUE;
}

bool vp_play_normal_view_volume_mute(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	_vp_play_normal_view_on_volume_popup(pNormalView, FALSE);

	int nMuteVal = vp_play_volume_get_mute_value(pNormalView->pVolumeHandle);
	if (nMuteVal > 0) {
		pNormalView->nLastVolume = nMuteVal;
		vp_play_volume_set_value(pNormalView->pVolumeHandle, pNormalView->nLastVolume);
	} else {
		vp_play_volume_set_mute_value(pNormalView->pVolumeHandle);
		pNormalView->nLastVolume = 0;
		vp_play_volume_set_value(pNormalView->pVolumeHandle, 0);
	}
	_vp_play_normal_view_set_volume_state(pNormalView);

	return TRUE;
}

bool vp_play_normal_view_volume_update(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (!pNormalView->pVolumeHandle) {
		return TRUE;
	}

	vp_play_volume_update_value(pNormalView->pVolumeHandle);
	_vp_play_normal_view_set_volume_state(pNormalView);

	return TRUE;
}


bool vp_play_normal_view_set_lock_screen(normal_view_handle pViewHandle, bool bLock)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	pNormalView->bLockScreen = bLock;
	VP_EVAS_TIMER_DEL(pNormalView->pLockScreenTimer);

	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	elm_object_part_content_unset(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_LOCKSCREEN);
	VP_EVAS_DEL(pNormalView->pLockBtn);
	PlayView *pPlayView = pNormalView->pPlayView;
	Evas_Object *pParent = NULL;
	if (!pNormalView->pSoundPathBtn) {
		pParent = pNormalView->pMainLayout;

	} else if (pNormalView->pFunctionLayout) {

		pParent = pNormalView->pFunctionLayout;

	} else {

		pParent = pNormalView->pMainLayout;
	}
	if (!pPlayView) {
		VideoLogError("pNormalView is NULL");
		return FALSE;
	}
	if (bLock) {
		VideoLogWarning("================ LOCK HOME KEY ==============");
		vp_play_util_key_grab(pPlayView->pWin, VP_HOME_KEY);

		_vp_play_normal_view_all_close_popup(pNormalView);

		VP_EVAS_TIMER_DEL(pNormalView->pHideTimer);
		VP_EVAS_TIMER_DEL(pNormalView->pHideFinishTimer);

		_vp_play_normal_view_hide_layout(pNormalView, TRUE);
		vp_play_volume_unset_mouse_callbacks(pNormalView->pVolumeHandle, pNormalView->pPlayView->pWin);

		pNormalView->pLockScreenTimer =
		    ecore_timer_add(VP_NORMAL_HIDE_SCREEN_LOCK_TIMER_INTERVAL,
		                    __vp_normal_lockscreen_timer_cb, (void *)pNormalView);

		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_LOCKSCREEN_SHOW, "*");

		pNormalView->pLockBtn = vp_button_create(
		                            pParent, "custom/flat_46_46/default", NULL,
		                            (Evas_Smart_Cb)__vp_normal_function_btn_clicked_cb,
		                            (Evas_Smart_Cb)__vp_normal_control_btn_press_cb,
		                            (Evas_Smart_Cb)__vp_normal_control_btn_unpress_cb,
		                            (void *)pNormalView);
		if (!pNormalView->pLockBtn) {
			VideoLogError("elm_button_add is fail");
			return FALSE;
		}


		Evas_Object *pIcon = NULL;

		pIcon = vp_button_create_icon(pNormalView->pLockBtn, edj_path, VP_PLAY_NORMAL_RES_LOCK);
		elm_object_part_content_set(pNormalView->pLockBtn, VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pLockBtn, edj_path, VP_PLAY_NORMAL_RES_LOCK_PRESS);
		elm_object_part_content_set(pNormalView->pLockBtn, VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

		pIcon = vp_button_create_icon(pNormalView->pLockBtn, edj_path, VP_PLAY_NORMAL_RES_LOCK);
		elm_object_part_content_set(pNormalView->pLockBtn, VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

		elm_object_part_content_set(pNormalView->pMainLayout, VP_PLAY_SWALLOW_NORMAL_LOCKSCREEN, pNormalView->pLockBtn);

		evas_object_show(pNormalView->pLockBtn);

		if (pNormalView->bManualPause == FALSE) {
			if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
				VideoLogWarning("Start Fail");
			}
			_vp_play_normal_view_set_play_state(pNormalView);
			_vp_play_normal_view_on_capture_mode(pNormalView);
		}
	} else {
		VideoLogWarning("================ UNLOCK HOME KEY ==============");
		vp_play_util_key_ungrab(pPlayView->pWin, VP_HOME_KEY);

		elm_object_signal_emit(pNormalView->pMainLayout, VP_NORMAL_SIGNAL_LOCKSCREEN_HIDE, "*");
		vp_play_volume_set_mouse_callbacks(pNormalView->pVolumeHandle, pNormalView->pPlayView->pWin);
		vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_UP_KEY);
		vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_DOWN_KEY);
		vp_play_util_key_grab(pPlayView->pWin, VP_VOLUME_MUTE);

		_vp_play_normal_view_show_layout(pNormalView);
	}
	return TRUE;
}

bool vp_play_normal_view_get_lock_screen(normal_view_handle pViewHandle, bool *bLock)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	*bLock = pNormalView->bLockScreen;

	return TRUE;
}

void vp_play_normal_view_hide_sound_popup(normal_view_handle pViewHandle)
{

	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (pNormalView->bIsPopupShow) {

		VP_EVAS_DEL(pNormalView->pPopup);
		pNormalView->bIsPopupShow = FALSE;
	}
}

void vp_play_normal_view_web_type_disconnect(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (!pNormalView->pPlayView) {
		VideoLogError("pNormalView->pPlayView is NULL");
		return;
	}

	PlayView *pPlayView = pNormalView->pPlayView;

	if (pPlayView->nStartPosition <= 0) {
		int nPosition = 0;
		if (!vp_mm_player_get_position(pNormalView->pPlayerHandle, &nPosition)) {
			VideoLogError("vp_mm_player_get_position is fail");
		}
		if (nPosition > 0) {
			pNormalView->nStartPosition = nPosition;
		}
	} else {
		pNormalView->nStartPosition = pPlayView->nStartPosition;
	}

	vp_mm_player_destroy(pNormalView->pPlayerHandle);
	pNormalView->pPlayerHandle = NULL;

	VideoLogInfo("pNormalView->nStartPosition : [%d]", pNormalView->nStartPosition);
}

void vp_play_normal_view_web_type_reconnect(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;

	if (!pNormalView->pPlayView) {
		VideoLogError("pNormalView->pPlayView is NULL");
		return;
	}

	if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB) {
		VideoLogInfo("Not WEB type");
		return;
	}

	if (pNormalView->pPlayerHandle) {
		vp_mm_player_destroy(pNormalView->pPlayerHandle);
		pNormalView->pPlayerHandle = NULL;
	}

	PlayView *pPlayView = pNormalView->pPlayView;
	if (pPlayView->nStartPosition > 0) {
		pNormalView->nStartPosition = pPlayView->nStartPosition;
	}

	VideoLogInfo("pNormalView->nStartPosition : [%d]", pNormalView->nStartPosition);
	_vp_play_normal_view_play_start(pNormalView, FALSE);
}

//For mini controller
bool vp_controller_play_normal_view_pause(normal_view_handle pViewHandle)
{
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	VideoLogWarning("Pause button");

	if (!vp_mm_player_pause(pNormalView->pPlayerHandle)) {
		VideoLogWarning("Pause Fail");
	}

	VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
	pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);
	pNormalView->bManualPause = TRUE;
	return TRUE;

}

bool vp_controller_play_normal_view_resume(normal_view_handle pViewHandle)
{
	VideoLogInfo("Resume button");
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}
	if(_vp_play_normal_view_check_during_call(pNormalView)) {
		return FALSE;
	}

	if (!vp_mm_player_play(pNormalView->pPlayerHandle)) {
		VideoLogWarning("Resume Fail");
	}
	VP_EVAS_IDLER_DEL(pNormalView->pControlIdler);
	pNormalView->pControlIdler = ecore_idler_add(__vp_normal_control_idler_cb, (void *)pNormalView);
	pNormalView->bManualPause = FALSE;
	return TRUE;
}

bool vp_controller_play_normal_view_next_play(normal_view_handle pViewHandle)
{
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	if (pNormalView->bHLSMode == FALSE) {
		if (pNormalView->nSpeedValue < 2) {
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
			_vp_play_normal_view_on_next_play(pNormalView, TRUE);
		}
	}
	return TRUE;
}

bool vp_controller_play_normal_view_prev_play(normal_view_handle pViewHandle)
{
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	if (pNormalView->bHLSMode == FALSE) {
		if (pNormalView->nSpeedValue < 2) {
			VP_EVAS_TIMER_DEL(pNormalView->pSpeedTimer);
			_vp_play_normal_view_on_prev_play(pNormalView, TRUE, FALSE);
		}
	}
	return TRUE;
}

char *vp_controller_normal_view_get_thumbnail(normal_view_handle pViewHandle)
{
	char *szThumbnail = NULL;
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL ");
		return NULL;
	}
	if (!pNormalView->szMediaURL) {
		VideoLogError("pNormalView->szMediaURL");
		return NULL;
	}
	vp_media_contents_get_video_thumbnail_path(pNormalView->szMediaURL, &szThumbnail);
	VideoLogInfo("thumbnail = %s:%s", pNormalView->szMediaURL, szThumbnail);
	return szThumbnail;
}

char *vp_controller_normal_view_get_title(normal_view_handle pViewHandle)
{
	char *pTitle = NULL;
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return NULL;
	}
	if (!pNormalView->szMediaURL) {
		VideoLogError("pNormalView->szMediaURL IS null");
		return NULL;
	}

	pTitle = vp_play_util_get_title_from_path(pNormalView->szMediaURL);
	VideoLogInfo("title = %s", pTitle);
	return pTitle;
}

int vp_controller_normal_view_get_duration(normal_view_handle pViewHandle)
{
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return 0;
	}
	if (!pNormalView->szMediaURL) {
		VideoLogError("pNormalView->szMediaURL IS null");
		return 0;
	}

	return pNormalView->nDuration;
}

bool vp_controller_normal_view_pop_view(normal_view_handle pViewHandle)
{
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}
	__vp_normal_naviframe_pop_cb(pNormalView, pNormalView->pNaviItem);
	return TRUE;
}

int vp_controller_normal_view_get_played_state(normal_view_handle pViewHandle)
{
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return nState;
	}
	if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
		VideoLogWarning("vp_mm_player_get_state is fail");
	}
	VideoLogError("state is %d", nState);

	return (int)nState;
}

bool vp_controller_normal_view_add_played_state_cb(normal_view_handle pViewHandle, vp_normal_view_state_cb fState_cb)
{
	NormalView	*pNormalView = (NormalView *)pViewHandle;
	if (!pNormalView) {
		VideoLogError("pViewHandle is NULL");
		return FALSE;
	}

	pNormalView->vp_mini_played_state_cb = fState_cb;
	return TRUE;
}

void vp_play_normal_view_db_change(normal_view_handle pViewHandle)
{
	if (!pViewHandle) {
		VideoLogError("pViewHandle is NULL");
		return;
	}

	NormalView	*pNormalView = (NormalView *)pViewHandle;
	PlayView *pPlayView = pNormalView->pPlayView;
	if (!pPlayView) {
		VideoLogError("pPlayView is NULL");
		return;
	}

	if (pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_WEB &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_PREVIEW &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MESSAGE &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_EMAIL &&
	        pNormalView->nLaunchingType != VIDEO_PLAY_TYPE_MULTI_PATH) {

		char *pVideoId = NULL;
		vp_media_contents_get_video_id(pNormalView->szMediaURL, &pVideoId);

		VideoSecureLogInfo("change == %d, %s, %s", pNormalView->nLaunchingType, pNormalView->szMediaURL, pVideoId);
		if (!vp_file_exists(pNormalView->szMediaURL) || !pVideoId) {
			VideoLogError("current file is deleted");
			VP_FREE(pVideoId);
			if (!pPlayView->pFunc) {
				VideoLogError("pPlayView pFunc is NULL");
				return;
			}

			if (!pPlayView->pFunc->vp_play_func_exit) {
				VideoLogError("pPlayView exit Func is NULL");
				return;
			}

			vp_mm_player_state_t nState = VP_MM_PLAYER_STATE_NONE;

			if (!vp_mm_player_get_state(pNormalView->pPlayerHandle, &nState)) {
				VideoLogWarning("vp_mm_player_get_state is fail");
			}
			if (nState == VP_MM_PLAYER_STATE_PAUSED) {
				vp_device_set_screen_off(FALSE);
				vp_device_display_lcd_unlock();

				pPlayView->pFunc->vp_play_func_exit(pPlayView);
			}
		}
		VP_FREE(pVideoId);
	}
}
