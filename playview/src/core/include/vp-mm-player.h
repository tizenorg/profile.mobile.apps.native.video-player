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

#pragma once

#include <stdbool.h>
#include <player.h>


typedef void *mm_player_handle;

typedef enum {
	VP_MM_PLAYER_FILTER_NONE = 0x00,
	VP_MM_PLAYER_FILTER_VOICE,
	VP_MM_PLAYER_FILTER_MOVIE,
	VP_MM_PLAYER_FILTER_VITUAL_71,
}vp_mm_player_sound_filter_t;

typedef enum {
	VP_MM_PLAYER_VIDEO_TYPE_X11 = 0x00,
	VP_MM_PLAYER_VIDEO_TYPE_EVAS
}vp_mm_player_video_sink_t;

typedef enum {
	VP_MM_PLAYER_ROTATE_0 = 0x00,
	VP_MM_PLAYER_ROTATE_90,
	VP_MM_PLAYER_ROTATE_180,
	VP_MM_PLAYER_ROTATE_270
}vp_mm_player_rotate_t;

typedef enum {
	VP_MM_PLAYER_REALIZE_ASYNC_CB = 0x00,
	VP_MM_PLAYER_SEEK_COMPLETE_CB,
	VP_MM_PLAYER_END_OF_STREAM_CB,
	VP_MM_PLAYER_INTERRUPT_CB,
	VP_MM_PLAYER_ERROR_CB,
	VP_MM_PLAYER_BUFFERING_CB,
	VP_MM_PLAYER_SUBTITLE_UPDATE_CB,
	VP_MM_PLAYER_PD_MESSAGE_CB,
	VP_MM_PLAYER_CAPTURE_VIDEO_CB,
	VP_MM_PLAYER_MISSED_PLUGIN_CB,
	VP_MM_PLAYER_IMAGE_BUFFER_CB,
}vp_mm_player_callback_t;


typedef enum {
	VP_MM_PLYAER_ERR_NONE = 0x00,
	VP_MM_PLAYER_ERR_OUT_OF_MEMORY,
	VP_MM_PLAYER_ERR_INVALID_PARAMETER,
	VP_MM_PLAYER_ERR_NO_SUCH_FILE,
	VP_MM_PLAYER_ERR_INVALID_OPERATION,
	VP_MM_PLAYER_ERR_FILE_NO_SPACE_ON_DEVICE,
	VP_MM_PLAYER_ERR_SEEK_FAILED,
	VP_MM_PLAYER_ERR_INVALID_STATE,
	VP_MM_PLAYER_ERR_NOT_SUPPORTED_FILE,
	VP_MM_PLAYER_ERR_INVALID_URI,
	VP_MM_PLAYER_ERR_SOUND_POLICY,
	VP_MM_PLAYER_ERR_CONNECTION_FAILED,
	VP_MM_PLAYER_ERR_VIDEO_CAPTUR_FAILED,
	VP_MM_PLAYER_ERR_DRM_EXPIRED,
	VP_MM_PLAYER_ERR_DRM_NO_LICENSE,
	VP_MM_PLAYER_ERR_DRM_FUTURE_USE
}vp_mm_player_error_t;

typedef enum {
	VP_MM_PLAYER_INTERRUPTED_COMPLETED = 0x00,
	VP_MM_PLAYER_INTERRUPTED_BY_MEDIA,
	VP_MM_PLAYER_INTERRUPTED_BY_CALL,
	VP_MM_PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG,
	VP_MM_PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT,
	VP_MM_PLAYER_INTERRUPTED_BY_ALARM,
	VP_MM_PLAYER_INTERRUPTED_BY_EMERGENCY,
	VP_MM_PLAYER_INTERRUPTED_RESUMABLE_MEDIA,
	VP_MM_PLAYER_INTERRUPTED_NOTIFICATION
}vp_mm_player_interrupt_t;

typedef enum {
	VP_MM_PLAYER_PD_STARTED = 0x00,
	VP_MM_PLAYER_PD_COMPLETED
}vp_mm_player_pd_message_t;

typedef enum {
	VP_MM_PLAYER_STATE_NONE = 0x00,
	VP_MM_PLAYER_STATE_IDLE,
	VP_MM_PLAYER_STATE_READY,
	VP_MM_PLAYER_STATE_PLAYING,
	VP_MM_PLAYER_STATE_PAUSED,
	VP_MM_PLAYER_STATE_STOP,	// define for AVRCP
}vp_mm_player_state_t;

typedef enum {
	VP_MM_PLAYER_DISPLAY_LETTER_BOX = 0x00,
	VP_MM_PLAYER_DISPLAY_ORIGIN_OR_LETTER,
	VP_MM_PLAYER_DISPLAY_FULL_SCREEN,
	VP_MM_PLAYER_DISPLAY_CROPPED_FULL,
	VP_MM_PLAYER_DISPLAY_ORIGIN_SIZE,
}vp_mm_player_display_mode_t;

typedef enum {
	VP_MM_PLAYER_MISSED_PLUGIN_UNKNOWN = 0x00,
	VP_MM_PLAYER_MISSED_PLUGIN_AUDIO,
	VP_MM_PLAYER_MISSED_PLUGIN_VIDEO,
}vp_mm_player_missed_plugin_t;

typedef enum {
	VP_MM_PLAYER_SUBTITLE_HALIGN_LEFT = 0x00,
	VP_MM_PLAYER_SUBTITLE_HALIGN_CENTER,
	VP_MM_PLAYER_SUBTITLE_HALIGN_RIGHT
}vp_mm_player_subtitle_halign_t;

typedef void(*vp_mm_player_prepare_cb)(void *pUserData);
typedef void(*vp_mm_player_completed_cb )(void *pUserData);
typedef void(*vp_mm_player_seek_completed_cb )(void *pUserData);
typedef void(*vp_mm_player_interrupted_cb )(vp_mm_player_interrupt_t nCode, void *pUserData);
typedef void(*vp_mm_player_error_cb )(vp_mm_player_error_t nError, void *pUserData);
typedef void(*vp_mm_player_buffering_cb )(int nPercent, void *pUserData);
typedef void(*vp_mm_player_subtitle_updated_cb )(unsigned long nDuration, char *text, void *pUserData);
typedef void(*vp_mm_player_pd_message_cb )(vp_mm_player_pd_message_t nType, void *pUserData);
typedef void(*vp_mm_player_video_captured_cb )(unsigned char *pFrame, int nWidth, int nHeight, unsigned int nSize, void *pUserData);
typedef void(*vp_mm_player_missed_plugin_cb )(vp_mm_player_missed_plugin_t nMissedType, void *pUserData);
typedef void(*vp_mm_player_image_buffer_updated_cb)(char *pBuffer, int nSize, void *pUserData);


typedef struct _MMPlayerCbFunc {
	vp_mm_player_prepare_cb pPrepareCb;
	vp_mm_player_completed_cb pEndOfStreamCb;
	vp_mm_player_seek_completed_cb pSeekCompleteCb;
	vp_mm_player_interrupted_cb pInterruptedCb;
	vp_mm_player_error_cb pErrorCb;
	vp_mm_player_buffering_cb pBufferingCb;
	vp_mm_player_subtitle_updated_cb pSubtitleUpdateCb;
	vp_mm_player_pd_message_cb pPDMessageCb;
	vp_mm_player_video_captured_cb pCapturedCb;
	vp_mm_player_missed_plugin_cb pMissedPluginCb;
	vp_mm_player_image_buffer_updated_cb pImageBufferCb;
} MMPlayerCbFunc;


typedef struct _MMPlayerCbPipe {
	Ecore_Pipe *pPreparePipe;
	Ecore_Pipe *pEndOfStreamPipe;
	Ecore_Pipe *pSeekCompletePipe;
	Ecore_Pipe *pInterruptedPipe;
	Ecore_Pipe *pErrorPipe;
	Ecore_Pipe *pBufferingPipe;
	Ecore_Pipe *pSubtitleUpdatePipe;
	Ecore_Pipe *pPDMessagePipe;
	Ecore_Pipe *pCapturedPipe;
	Ecore_Pipe *pMissedPluginPipe;
	Ecore_Pipe *pImageBufferPipe;
} MMPlayerCbPipe;


typedef struct _MMPlayerCaptureParam {
	unsigned char *pFrame;
	int nWidth;
	int nHeight;
	unsigned int nSize;
	void *pParam;
} MMPlayerCaptureParam;

typedef struct _MMPlayerSubtitleParam {
	unsigned long nDuration;
	char *txt;
	void *pParam;
} MMPlayerSubtitleParam;

typedef struct _MMPlayerInterruptParam {
	vp_mm_player_interrupt_t nInterrupt;
	void *pParam;
} MMPlayerInterruptParam;

typedef struct _MMPlayerErrorParam {
	vp_mm_player_interrupt_t nError;
	void *pParam;
} MMPlayerErrorParam;

typedef struct _MMPlayerBufferingParam {
	int nPercent;
	void *pParam;
} MMPlayerBufferingParam;


typedef struct _MMPlayerPDParam {
	vp_mm_player_pd_message_t nMessage;
	void *pParam;
} MMPlayerPDParam;

typedef struct _MMPlayerMissedPluginParam {
	vp_mm_player_missed_plugin_t nMissedType;
	void *pParam;
} MMPlayerMissedPluginParam;


typedef struct _MMPlayerImageBufferParam {
	char *pBuffer;
	int nSize;
	void *pParam;
} MMPlayerImageBufferParam;

typedef struct _MMPlayer {
	player_h pPlayer;

	char *szURL;
	char *szSubtitleURL;
	char *szCookie;
	char *szProxy;

	bool bIsRealize;
	bool bDeactivateSubtitle;
	bool bIsMute;
	bool bIsScaling;
	bool bIsLooping;
	bool bIsClosedCaption;
	bool bIsCapturing;

	float fLeftVol;
	float fRightVol;

	vp_mm_player_sound_filter_t nSoundFilter;
	vp_mm_player_video_sink_t nVideoSink;
	vp_mm_player_rotate_t nRotate;
	vp_mm_player_state_t nState;
	vp_mm_player_display_mode_t nDisplayMode;
	sound_stream_info_h	stream_info;

	int nDuration;
	int nVideoWidth;
	int nVidehHeight;

	MMPlayerCbFunc *pCbFunc;
	MMPlayerCbPipe *pCbPipe;
	void *pParam;

	MMPlayerCaptureParam *pCaptureParam;
	MMPlayerSubtitleParam *pSubtitleParam;
	MMPlayerInterruptParam *pInterruptParam;
	MMPlayerErrorParam *pErrorParam;
	MMPlayerPDParam *pPDParam;
	MMPlayerBufferingParam *pBufferingParam;
	MMPlayerMissedPluginParam *pMissedParam;
	MMPlayerImageBufferParam *pImageBufferParam;

	int nZoomPosX;
	int nZoomPosY;
	bool reacquire_state;

	void *pVideoSink;
} MMPlayer;

mm_player_handle vp_mm_player_create();
void vp_mm_player_destroy(mm_player_handle pPlayerHandle);
bool vp_mm_player_realize(mm_player_handle pPlayerHandle, char *szPath);
bool vp_mm_player_realize_async(mm_player_handle pPlayerHandle, char *szPath);


bool vp_mm_player_unrealize(mm_player_handle pPlayerHandle);
bool vp_mm_player_is_realize(mm_player_handle pPlayerHandle);

bool vp_mm_player_set_callback(mm_player_handle pPlayerHandle, vp_mm_player_callback_t nCallbackType, void *pCallback);
bool vp_mm_player_set_user_param(mm_player_handle pPlayerHandle, void *pParam);
bool vp_mm_player_set_stream_info(mm_player_handle pPlayerHandle, void *stream_info);

bool vp_mm_player_play(mm_player_handle pPlayerHandle);
bool vp_mm_player_stop(mm_player_handle pPlayerHandle);
bool vp_mm_player_pause(mm_player_handle pPlayerHandle);
bool vp_mm_player_get_state(mm_player_handle pPlayerHandle, vp_mm_player_state_t *nState);

bool vp_mm_player_get_position(mm_player_handle pPlayerHandle, int *nPosition);
bool vp_mm_player_set_position(mm_player_handle pPlayerHandle, int nPosition);
bool vp_mm_player_set_position_by_key_frame(mm_player_handle pPlayerHandle, int nPosition);

bool vp_mm_player_get_buffering_position(mm_player_handle pPlayerHandle, int *nPosition);


bool vp_mm_player_set_scaling(mm_player_handle pPlayerHandle, bool bScale);
bool vp_mm_player_set_loop(mm_player_handle pPlayerHandle, bool bLoop);

bool vp_mm_player_set_mute(mm_player_handle pPlayerHandle, bool bMute);
bool vp_mm_player_get_mute(mm_player_handle pPlayerHandle, bool *bMute);
bool vp_mm_player_get_duration(mm_player_handle pPlayerHandle, int *nDuration);
bool vp_mm_player_get_codecinfo(mm_player_handle pPlayerHandle, char **audio_codec, char **video_codec);

bool vp_mm_player_set_video_sink(mm_player_handle pPlayerHandle, int nType, void *pSink);
bool vp_mm_player_set_video_rotate(mm_player_handle pPlayerHandle, int nRotate);
bool vp_mm_player_set_display_mode(mm_player_handle pPlayerHandle, vp_mm_player_display_mode_t nDisplayMode);

bool vp_mm_player_get_resolution(mm_player_handle pPlayerHandle, int *nWidth, int *nHeight);

bool vp_mm_player_set_subtitle_url(mm_player_handle pPlayerHandle, char *szSubtitlePath);
bool vp_mm_player_get_subtitle_url(mm_player_handle pPlayerHandle, char **szSubtitlePath);
bool vp_mm_player_set_subtitle_position(mm_player_handle pPlayerHandle, int nPosition);
bool vp_mm_player_set_subtitle_font(mm_player_handle pPlayerHandle, char *szFamilyName, char *szStyle, int nSize);
bool vp_mm_player_set_subtitle_font_color(mm_player_handle pPlayerHandle, unsigned int nForegroundColor, unsigned int nBGColor);
bool vp_mm_player_subtitle_ignore_markup_tags(mm_player_handle pPlayerHandle, bool bIgnoreMarkup);
bool vp_mm_player_subtitle_set_alignment_in_line(mm_player_handle pPlayerHandle, vp_mm_player_subtitle_halign_t type);
bool vp_mm_player_subtitle_set_alignment_horizontal(mm_player_handle pPlayerHandle, vp_mm_player_subtitle_halign_t type);
bool vp_mm_player_subtitle_apply_alignment_right_away(mm_player_handle pPlayerHandle);
bool vp_mm_player_set_deactivate_subtitle(mm_player_handle pPlayerHandle, bool bDeactivate);

bool vp_mm_player_set_volume(mm_player_handle pPlayerHandle, float fLeft, float fRight);
bool vp_mm_player_get_volume(mm_player_handle pPlayerHandle, float *fLeft, float *fRight);

bool vp_mm_player_set_sound_filter(mm_player_handle pPlayerHandle, vp_mm_player_sound_filter_t nFilter);
bool vp_mm_player_get_sound_filter(mm_player_handle pPlayerHandle, vp_mm_player_sound_filter_t *nFilter);

bool vp_mm_player_capture_start(mm_player_handle pPlayerHandle);
bool vp_mm_player_is_closed_caption_exist(mm_player_handle pPlayerHandle);

bool vp_mm_player_get_audio_track_count(mm_player_handle pPlayerHandle, int *nCount);
bool vp_mm_player_get_audio_track_language_code(mm_player_handle pPlayerHandle, int nIndex, char **szCode);
bool vp_mm_player_set_audio_track(mm_player_handle pPlayerHandle, int nIndex);

bool vp_mm_player_get_subtitle_track_count(mm_player_handle pPlayerHandle, int *nCount);
bool vp_mm_player_get_subtitle_track_language_code(mm_player_handle pPlayerHandle, int nIndex, char **szCode);
bool vp_mm_player_set_subtitle_track(mm_player_handle pPlayerHandle, int nIndex);
#ifdef _SUBTITLE_MULTI_LANGUAGE
bool vp_mm_player_add_subtitle_language(mm_player_handle pPlayerHandle, int nIndex);
bool vp_mm_player_remove_subtitle_language(mm_player_handle pPlayerHandle, int nIndex);
bool vp_mm_player_track_foreach_selected_subtitle_language(mm_player_handle pPlayerHandle, void *pCallback, void *pUserData);
#endif
bool vp_mm_player_set_cookie(mm_player_handle pPlayerHandle, const char *szCookie);
bool vp_mm_player_set_proxy(mm_player_handle pPlayerHandle, const char *szProxy);

bool vp_mm_player_set_hw_decoder(bool bEnable);
bool vp_mm_player_set_rate(mm_player_handle pPlayerHandle, float fRate);


bool vp_mm_player_set_zoom(mm_player_handle pPlayerHandle, float fZoom, int nPosX, int nPosY);
bool vp_mm_player_get_zoom_start_position(mm_player_handle pPlayerHandle, int *nPosX, int *nPosY);
bool vp_mm_player_set_visible(mm_player_handle pPlayerHandle, bool bVisible);
bool vp_mm_player_set_hub_download_mode(mm_player_handle pPlayerHandle, bool bDownload);

