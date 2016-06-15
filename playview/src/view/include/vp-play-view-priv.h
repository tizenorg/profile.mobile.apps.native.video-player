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

#include <glib.h>
#include <stdbool.h>
#include <Elementary.h>
#include <net_connection.h>
#include <sound_manager.h>

#include "vp-play-view.h"
#include "vp-play-type-define.h"

typedef struct _PlayViewFunc {
	void (*vp_play_func_change_mode)	(void *pViewHandle, video_play_mode_t nMode, const char *szMediaURL, int nStartPos, bool bManualPause);
	void (*vp_play_func_exit) 		(void *pViewHandle);
}PlayViewFunc;

typedef struct _PlayViewCbFunc {
	vp_play_view_realize_cb		pRealizeCb;
	vp_play_view_destroy_cb		pDestroyCb;
	vp_play_view_event_key_down_cb 	pEventKeyDown;
	vp_play_view_event_key_up_cb	pEventKeyUp;
}PlayViewCbFunc;

typedef struct _PlayView {
	Evas_Object			*pParent;
	Evas_Object			*pWin;
	Ecore_X_Window			nXwinID;
	Ecore_X_Window			nCallerXwinID;

	bool				bAVRCP;
	bool				bWifi;
	bool				bIsPlayBeforeShare;

	bool				bStoreDownload;
	bool				bStopAfterRepeat;
	double				dStoreDL_FileSize;
	char				*pStoreOrderId;
	char				*pStoreServerId;
	char				*pStoreAppId;
	char				*pStoreMvId;

	char				*szCookie;
	char				*szProxy;

	Evas_Object			*pBackground;
	Evas_Object			*pRenderRect;
	Evas_Object			*pConformant;
	Evas_Object			*pMainLayout;
	Evas_Object			*pNaviframe;

	Ecore_Event_Handler		*pKeyEventDown;
	Ecore_Event_Handler		*pKeyEventUp;
	Ecore_Event_Handler		*pFocusIn;
	Ecore_Event_Handler		*pFocusOut;
	Ecore_Event_Handler		*pVisiblility;
	Ecore_Event_Handler		*pClientMessage;
	Ecore_Event_Handler		*pDeleteRequest;
	Ecore_Event_Handler		*pPropertyHandler;
	Ecore_Event_Handler		*pMouseInHandler;
	Ecore_Event_Handler		*pMouseOutHandler;
	Ecore_Event_Handler 		*pCallerWinEventHandler;
	Ecore_Event_Handler 		*pPalmHoldEventHandler;

	Ecore_Pipe			*pNotiPipe;

	Ecore_Timer			*pSbeamTimer;
	Ecore_Timer			*pVolumeTimer;
	Ecore_Idler			*pHideIdler; //used for hide win

	bool				bVolumeIncrease;
	bool				bViewChange;
	bool				bMouseOut;
	bool				bVisible;
	Ecore_Timer			*pMediaLongPressTimer;
	Ecore_Timer			*pMediaKeyTimer;
	int 				nMediaKeyPressCount;

	void				*pUserData;

	void				*pNormalView;
	void				*pMultiView;

	void				*pSmartPause;
	Ecore_Timer			*pSmartPauseStartTimer;
	Ecore_Timer			*pSmartPauseEndTimer;

	void				*pSensorHandle;
	void				*pSensorPopup;

	GList				*pMultiPathList;

	char				*szMediaURL;
	char				*szMediaTitle;
	char				*szDeviceID;
	char				*szAutoPlay;
	char				*szDMRID;

	int				nStartPosition;
	int				nDuration;
	bool				bStartPause;
	bool				bRealized;
	bool				bActivate;
	bool				bDestroy;

	int				nAudioIndex;
	char				*szSubtitleURL;
	bool				bManualAppPause;

	video_play_mode_t		nPlayMode;
	video_play_launching_type_t	nLaunchingType;
	video_play_rotate_t		nRotate;
	video_sound_alive_t		nSoundAlive;
	video_play_list_type_t		nListType;
	sound_stream_info_h             stream_info;

        bool                            reacquire_state;
	bool				bPopupMode;
	bool				bTrimExit;
	bool				bRelaunchCaller;

	PlayViewCbFunc			*pCallback;
	PlayViewFunc			*pFunc;

	void				*pHollICHandle;
	bool				bKeySpeed;
	double				fPlaySpeed;
	float				fSubtitleSyncValue;

	Ecore_Timer			*pHwKeyLongPressTimer;// for longpress HW key
	Ecore_Timer			*pDbUpdateTimer;	//db Update
	connection_h			connection;
} PlayView;

