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
#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-volume.h"
#include "vp-play-button.h"

#include "vp-sound.h"
#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-mm-player.h"

#define VP_VOLUME_HIDE_LAYOUT_TIMER_INTERVAL	5.0

#define VP_VOLUME_PORTRAIT_POS_X	15
#define VP_VOLUME_PORTRAIT_POS_Y	60
#define VP_VOLUME_PORTRAIT_HEIGHT	241

#define VP_VOLUME_LANDSCAPE_POS_X	19
#define VP_VOLUME_LANDSCAPE_POS_Y	27
#define VP_VOLUME_LANDSCAPE_HEIGHT	170

#define VP_VOLUME_TOP_PADDING_H		87

#define VP_VOLUME_TEXT_MAX_LEN		4



typedef struct _VolumeWidget {
	Evas_Object *pParent;
	Evas_Object *pWin;
	Evas_Object *pLayout;
	Evas_Object *pFocusPoint;
	Ecore_Timer *pHideTimer;
	Ecore_Timer *pDeviceTimer;

	bool bLandscape;
	bool bIsRealize;
	bool bMouseDown;

	int nMaxVal;
	int nCurVal;
	int nTmpVal;
	int nMuteVal;

	void *pUserData;
	void *pPlayerHandle;

	NormalVolumeChangeCbFunc pChangeVolFunc;
} VolumeWidget;

static void _vp_play_volume_destory_handle(VolumeWidget *pVolumeWidget);
static double _vp_play_volume_get_mouse_pos_ratio(Evas_Object *pObj,
        bool bLandscape,
        int nCurY,
        Evas_Object *pWin);
static bool _vp_play_volume_set_value(VolumeWidget *pVolumeWidget,
                                      int nValue);
static void _vp_play_volume_create_timer(VolumeWidget *pVolumeWidget);

/* callback functions */
static void __vp_volume_focus_key_down_cb(void *pUserData, Evas *e,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pUserData;

	Evas_Event_Key_Down *ev = pEvent;

	if (!ev) {
		return;
	}
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		return;
	}

	VideoLogInfo("KEY: %s", ev->keyname);

	if (!strcmp(ev->keyname, "Up")) {
		vp_play_volume_increase_value(pVolumeWidget);
	} else if (!strcmp(ev->keyname, "Down")) {
		vp_play_volume_decrease_value(pVolumeWidget);
	}
}

static void __vp_volume_mouse_down_cb(void *pUserData, Evas *e,
                                      Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VolumeWidget *pVolumeWidget = (VolumeWidget *) pUserData;

	if (pVolumeWidget->bIsRealize == FALSE) {
		return;
	}

	Evas_Event_Mouse_Down *pMouseDownEvent =
	    (Evas_Event_Mouse_Down *) pEvent;

	double dRatio =
	    _vp_play_volume_get_mouse_pos_ratio(pVolumeWidget->pLayout,
	                                        pVolumeWidget->bLandscape,
	                                        pMouseDownEvent->canvas.y,
	                                        pVolumeWidget->pWin);

	int nCurVal =
	    pVolumeWidget->nMaxVal - (dRatio * pVolumeWidget->nMaxVal);

	_vp_play_volume_set_value(pVolumeWidget, nCurVal);

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);

	pVolumeWidget->bMouseDown = TRUE;
}

static void __vp_volume_mouse_up_cb(void *pUserData, Evas *e,
                                    Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VolumeWidget *pVolumeWidget = (VolumeWidget *) pUserData;

	Evas_Event_Mouse_Up *pMouseUpEvent = (Evas_Event_Mouse_Up *) pEvent;

	if (pVolumeWidget->bIsRealize == FALSE) {
		return;
	}

	double dRatio =
	    _vp_play_volume_get_mouse_pos_ratio(pVolumeWidget->pLayout,
	                                        pVolumeWidget->bLandscape,
	                                        pMouseUpEvent->canvas.y,
	                                        pVolumeWidget->pWin);

	int nCurVal =
	    pVolumeWidget->nMaxVal - (dRatio * pVolumeWidget->nMaxVal);

	_vp_play_volume_set_value(pVolumeWidget, nCurVal);
	_vp_play_volume_create_timer(pVolumeWidget);

	pVolumeWidget->bMouseDown = FALSE;
}

static void __vp_volume_mouse_move_cb(void *pUserData, Evas *e,
                                      Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pUserData;

	Evas_Event_Mouse_Move *pMouseMoveEvent =
	    (Evas_Event_Mouse_Move *) pEvent;

	if (pVolumeWidget->bIsRealize == FALSE
	        || pVolumeWidget->bMouseDown == FALSE) {
		return;
	}

	double dRatio =
	    _vp_play_volume_get_mouse_pos_ratio(pVolumeWidget->pLayout,
	                                        pVolumeWidget->bLandscape,
	                                        pMouseMoveEvent->cur.canvas.y,
	                                        pVolumeWidget->pWin);

	int nCurVal =
	    pVolumeWidget->nMaxVal - (dRatio * pVolumeWidget->nMaxVal);

	if (nCurVal != pVolumeWidget->nCurVal) {
		_vp_play_volume_set_value(pVolumeWidget, nCurVal);
	}

}

static Eina_Bool __vp_volume_hide_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pUserData;

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);

	if (!vp_play_volume_unrealize((volume_handle) pVolumeWidget)) {
		VideoLogWarning("vp_play_volume_unrealize is fail");
	}

	return EINA_FALSE;
}



/* internal functions */
static void _vp_play_volume_destory_handle(VolumeWidget *pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return;
	}

	pVolumeWidget->pChangeVolFunc = NULL;

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);
	VP_EVAS_TIMER_DEL(pVolumeWidget->pDeviceTimer);

	VP_EVAS_DEL(pVolumeWidget->pFocusPoint);
	VP_EVAS_DEL(pVolumeWidget->pLayout);
	VP_FREE(pVolumeWidget);
}

static double _vp_play_volume_get_mouse_pos_ratio(Evas_Object *pObj,
        bool bLandscape,
        int nCurY,
        Evas_Object *pWin)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return 0.0;
	}

	int nHeight = 0;
	int nCurrent = 0;
	double dRatio = 0.0;
	int w = 0;
	int h = 0;
	elm_win_screen_size_get(pWin, NULL, NULL, &w, &h);

	if ((elm_config_scale_get() - 2.6) == 0.0) {
		if (bLandscape) {
			nHeight = VP_VOLUME_LANDSCAPE_HEIGHT;
			nCurrent =
			    nCurY - VP_VOLUME_LANDSCAPE_POS_Y - VP_VOLUME_TOP_PADDING_H +
			    10;
		} else {
			nHeight = 350;
			nCurrent =
			    nCurY - VP_VOLUME_PORTRAIT_POS_Y - VP_VOLUME_TOP_PADDING_H -
			    310;
		}
	} else if ((elm_config_scale_get() - 1.8) == 0.0) {
		if (bLandscape) {
			nHeight = 125;
			nCurrent =
			    nCurY - VP_VOLUME_LANDSCAPE_POS_Y - VP_VOLUME_TOP_PADDING_H + 70;
		} else {
			nHeight = 255;
			nCurrent =
			    nCurY - VP_VOLUME_PORTRAIT_POS_Y - VP_VOLUME_TOP_PADDING_H - 42;
		}
	} else {
		if (bLandscape) {
			nHeight = 145;
			nCurrent =
			    nCurY - VP_VOLUME_LANDSCAPE_POS_Y - VP_VOLUME_TOP_PADDING_H + 60;
		} else {
			nHeight = 300;
			nCurrent =
			    nCurY - VP_VOLUME_PORTRAIT_POS_Y - VP_VOLUME_TOP_PADDING_H - 56;
		}
	}

	if (nCurrent < 0) {
		nCurrent = 0;
	} else if (nCurrent > nHeight) {
		nCurrent = nHeight;
	}
	if (nHeight > 0.0) {
		dRatio = (double) nCurrent / nHeight;
	}

	return dRatio;
}


static void _vp_play_volume_set_widget_position(VolumeWidget *
        pVolumeWidget,
        Evas_Object *pWin)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return;
	}
	int w = 0;
	int h = 0;
	elm_win_screen_size_get(pWin, NULL, NULL, &w, &h);
	pVolumeWidget->pWin = pWin;
	bool bLandscape = pVolumeWidget->bLandscape;

	if ((elm_config_scale_get() - 2.6) == 0.0) {
		if (bLandscape) {
			elm_object_signal_emit(pVolumeWidget->pLayout,
			                       VP_NORMAL_SIGNAL_VOLUME_LANDSCAPE_MODE,
			                       "*");
			evas_object_move(pVolumeWidget->pLayout, h - 80, w / 10);
		} else {
			elm_object_signal_emit(pVolumeWidget->pLayout,
			                       VP_NORMAL_SIGNAL_VOLUME_PORTRAIT_MODE,
			                       "*");
			evas_object_move(pVolumeWidget->pLayout, w - w / 9, (5 * h) / 16);
		}
	} else if ((elm_config_scale_get() - 1.8) == 0.0) {
		if (bLandscape) {
			elm_object_signal_emit(pVolumeWidget->pLayout,
			                       VP_NORMAL_SIGNAL_VOLUME_LANDSCAPE_MODE,
			                       "*");
			evas_object_move(pVolumeWidget->pLayout, h - h / 16, w / 16);
		} else {
			elm_object_signal_emit(pVolumeWidget->pLayout,
			                       VP_NORMAL_SIGNAL_VOLUME_PORTRAIT_MODE,
			                       "*");
			evas_object_move(pVolumeWidget->pLayout, w - w / 10, h / 5);
		}
	} else {
		if (bLandscape) {
			elm_object_signal_emit(pVolumeWidget->pLayout,
			                       VP_NORMAL_SIGNAL_VOLUME_LANDSCAPE_MODE,
			                       "*");
			evas_object_move(pVolumeWidget->pLayout, h - h / 16, w / 16);
		} else {
			elm_object_signal_emit(pVolumeWidget->pLayout,
			                       VP_NORMAL_SIGNAL_VOLUME_PORTRAIT_MODE,
			                       "*");
			evas_object_move(pVolumeWidget->pLayout, w - w / 10, h / 5);
		}
	}
}


static void _vp_play_volume_update_value(VolumeWidget *pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return;
	}

	double nVolumeRatio = 0.0;
	char szPlayingTime[VP_VOLUME_TEXT_MAX_LEN] = { 0, };

	snprintf(szPlayingTime, VP_VOLUME_TEXT_MAX_LEN, "%d",
	         pVolumeWidget->nCurVal);

	nVolumeRatio =
	    ((double) pVolumeWidget->nCurVal /
	     (double) pVolumeWidget->nMaxVal);

	if (!edje_object_part_drag_value_set
	        (_EDJ(pVolumeWidget->pLayout), VP_PLAY_SWALLOW_VOLUME_DRAG_RECT,
	         0.0, nVolumeRatio)) {
		VideoLogWarning("Drag value set fail : %lf", nVolumeRatio);
	}

	elm_object_part_text_set(pVolumeWidget->pLayout,
	                         VP_PLAY_SWALLOW_VOLUME_VALUE_LABEL,
	                         szPlayingTime);
}

static Eina_Bool __vp_volume_device_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pUserData;

	int maxVolume = 0;
	if (!vp_sound_get_max_volume(&maxVolume)) {
		VideoLogError("vp_sound_get_max_volume failed");
	}

	if (pVolumeWidget->pDeviceTimer) {
		VP_EVAS_TIMER_DEL(pVolumeWidget->pDeviceTimer);
	}
	if (!vp_sound_set_volume(pVolumeWidget->nTmpVal)) {
		VideoLogError("vp_sound_set_volume failed");
	} else {
		pVolumeWidget->nCurVal = pVolumeWidget->nTmpVal;
		_vp_play_volume_update_value(pVolumeWidget);
		if (pVolumeWidget->pChangeVolFunc) {
			pVolumeWidget->pChangeVolFunc(pVolumeWidget->nCurVal,
			                              pVolumeWidget->pUserData);
		}

		VideoLogWarning
		("vp_sound_set_volume successed, need update volume bar! = %d",
		 pVolumeWidget->nCurVal);
	}

	return EINA_FALSE;
}

static bool _vp_play_volume_set_value(VolumeWidget *pVolumeWidget,
                                      int nValue)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	pVolumeWidget->nTmpVal = pVolumeWidget->nCurVal;
	if (nValue > 0) {
		pVolumeWidget->nMuteVal = 0;
	}

	int nMaxValue = 0;
	if (vp_sound_get_max_volume(&nMaxValue) && nValue <= nMaxValue) {
		pVolumeWidget->nTmpVal = nValue;
	}

	if (pVolumeWidget->pDeviceTimer == NULL) {
		pVolumeWidget->pDeviceTimer = ecore_timer_add(0.1,
		                              __vp_volume_device_timer_cb,
		                              (void *)
		                              pVolumeWidget);
	}

	return TRUE;
}

static void _vp_play_volume_create_timer(VolumeWidget *pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pParent is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);

	pVolumeWidget->pHideTimer =
	    ecore_timer_add(VP_VOLUME_HIDE_LAYOUT_TIMER_INTERVAL,
	                    __vp_volume_hide_timer_cb,
	                    (void *) pVolumeWidget);
}

static Evas_Object *_vp_play_volume_create_layout(Evas_Object *pParent)
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
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VP_PLAY_VOLUME_EDJ_PATH);

	bRet =
	    elm_layout_file_set(pObj, edj_path,
	                        VP_PLAY_EDJ_GROUP_VOLUME);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
		return NULL;
	}

	evas_object_show(pObj);

	return pObj;
}


static bool _vp_play_volume_init_layout(VolumeWidget *pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pVolumeWidget->pParent;

	pVolumeWidget->pLayout = _vp_play_volume_create_layout(pParent);
	if (pVolumeWidget->pLayout == NULL) {
		VideoLogError("_vp_play_volume_create_layout is fail");
		return FALSE;
	}
	pVolumeWidget->pFocusPoint =
	    vp_button_create(pVolumeWidget->pLayout, "focus", NULL, NULL,
	                     NULL, NULL, (void *) pVolumeWidget);
	if (!pVolumeWidget->pFocusPoint) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	elm_object_focus_allow_set(pVolumeWidget->pFocusPoint, EINA_FALSE);
	elm_object_part_content_set(pVolumeWidget->pLayout,
	                            "pv.volume.point.focus",
	                            pVolumeWidget->pFocusPoint);

	return TRUE;
}

/* external functions */
volume_handle vp_play_volume_create(Evas_Object *pParent,
                                    void *pPlayerHandle,
                                    NormalVolumeChangeCbFunc pFunc)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	VolumeWidget *pVolumeWidget = NULL;

	pVolumeWidget = calloc(1, sizeof(VolumeWidget));

	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget alloc fail");
		return NULL;
	}

	pVolumeWidget->pChangeVolFunc = pFunc;
	pVolumeWidget->pParent = pParent;
	pVolumeWidget->bLandscape = FALSE;
	pVolumeWidget->pPlayerHandle = pPlayerHandle;

	if (!_vp_play_volume_init_layout(pVolumeWidget)) {
		VideoLogError("_vp_play_volume_init_layout is fail");
		_vp_play_volume_destory_handle(pVolumeWidget);
		return NULL;
	}

	if (!vp_sound_get_max_volume(&(pVolumeWidget->nMaxVal))) {
		VideoLogError("vp_sound_get_max_volume fail");
		_vp_play_volume_destory_handle(pVolumeWidget);
		return NULL;
	}

	if (!vp_sound_get_volume(&(pVolumeWidget->nCurVal))) {
		VideoLogError("vp_sound_get_volume fail");
		_vp_play_volume_destory_handle(pVolumeWidget);
		return NULL;
	}

	bool flag = vp_play_volume_unrealize(pVolumeWidget);
	if (!flag) {
		VideoLogError("vp_play_volume_unrealize return FALSE");
	}

	_vp_play_volume_update_value(pVolumeWidget);

	return (volume_handle) pVolumeWidget;
}

void vp_play_volume_destroy(volume_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	vp_play_volume_unset_mouse_callbacks(pVolumeWidget,
	                                     pVolumeWidget->pWin);
	_vp_play_volume_destory_handle(pVolumeWidget);
}

bool vp_play_volume_realize(volume_handle pVolumeHandle,
                            Evas_Object *pWin)
{
	VideoLogInfo("");
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	pVolumeWidget->bIsRealize = TRUE;

	_vp_play_volume_set_widget_position(pVolumeWidget, pWin);

	_vp_play_volume_set_value(pVolumeWidget, pVolumeWidget->nCurVal);

	evas_object_show(pVolumeWidget->pLayout);
	_vp_play_volume_create_timer(pVolumeWidget);

	return TRUE;
}

bool vp_play_volume_unrealize(volume_handle pVolumeHandle)
{
	VideoLogInfo("");
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	pVolumeWidget->bIsRealize = FALSE;

	elm_object_focus_allow_set(pVolumeWidget->pFocusPoint, EINA_FALSE);
	evas_object_hide(pVolumeWidget->pFocusPoint);
	evas_object_hide(pVolumeWidget->pLayout);
	return TRUE;
}

bool vp_play_volume_set_landscape_mode(volume_handle pVolumeHandle,
                                       bool bLandscape,
                                       Evas_Object *pWin)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	pVolumeWidget->bLandscape = bLandscape;

	_vp_play_volume_set_widget_position(pVolumeWidget, pWin);


	return TRUE;
}

bool vp_play_volume_set_user_data(volume_handle pVolumeHandle,
                                  void *pUserData)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	pVolumeWidget->pUserData = pUserData;

	return TRUE;
}

bool vp_play_volume_is_realize(volume_handle pVolumeHandle,
                               bool *bIsRealize)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	*bIsRealize = pVolumeWidget->bIsRealize;

	return TRUE;
}

bool vp_play_volume_increase_value(volume_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;
	int nVal = 0;

	if (pVolumeWidget->pDeviceTimer) {
		nVal = pVolumeWidget->nCurVal + 1;
	} else {
		if (!vp_sound_get_volume(&nVal)) {
			VideoLogError("vp_sound_get_volume fail");
			return FALSE;
		}

		nVal++;
	}

	_vp_play_volume_create_timer(pVolumeWidget);

	if (nVal > pVolumeWidget->nMaxVal) {
		pVolumeWidget->nCurVal = pVolumeWidget->nMaxVal;
		return TRUE;
	}

	_vp_play_volume_set_value(pVolumeWidget, nVal);

	return TRUE;

}

bool vp_play_volume_decrease_value(volume_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;
	int nVal = 0;

	if (pVolumeWidget->pDeviceTimer) {
		nVal = pVolumeWidget->nCurVal - 1;
	} else {
		if (!vp_sound_get_volume(&nVal)) {
			VideoLogError("vp_sound_get_volume fail");
			return FALSE;
		}

		nVal--;
	}

	_vp_play_volume_create_timer(pVolumeWidget);

	if (nVal < 0) {
		pVolumeWidget->nCurVal = 0;
		return TRUE;
	}

	_vp_play_volume_set_value(pVolumeWidget, nVal);

	return TRUE;

}

bool vp_play_volume_update_value(volume_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;
	int nVal = 0;

	if (pVolumeWidget->pDeviceTimer) {
		return TRUE;
	}

	if (!vp_sound_get_volume(&nVal)) {
		VideoLogError("vp_sound_get_volume fail");
		return FALSE;
	}

	if (pVolumeWidget->nCurVal != nVal) {
		pVolumeWidget->nCurVal = nVal;
		if (pVolumeWidget->bIsRealize) {
			_vp_play_volume_update_value(pVolumeWidget);
		}
	}

	return TRUE;
}


bool vp_play_volume_is_mute(volume_handle pVolumeHandle, bool *bIsMute)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	if (pVolumeWidget->nCurVal == 0) {
		*bIsMute = TRUE;
	} else {
		*bIsMute = FALSE;
	}

	return TRUE;

}

int vp_play_volume_get_max_value(volume_handle pVolumeHandle)
{
	if (NULL == pVolumeHandle) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	return pVolumeWidget->nMaxVal;
}

int vp_play_volume_get_value(volume_handle pVolumeHandle)
{
	if (NULL == pVolumeHandle) {
		VideoLogError("pVolumeHandle is NULL");
		return 0;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	return pVolumeWidget->nCurVal;
}

bool vp_play_volume_set_value(volume_handle pVolumeHandle, int nVolumeVal)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	_vp_play_volume_create_timer(pVolumeWidget);

	pVolumeWidget->nCurVal = nVolumeVal;

	if (pVolumeWidget->nCurVal < 0) {
		pVolumeWidget->nCurVal = 0;
	}

	if (pVolumeWidget->nCurVal > pVolumeWidget->nMaxVal) {
		pVolumeWidget->nCurVal = pVolumeWidget->nMaxVal;
	}

	_vp_play_volume_set_value(pVolumeWidget, pVolumeWidget->nCurVal);

	return TRUE;
}

int vp_play_volume_get_mute_value(volume_handle pVolumeHandle)
{
	if (NULL == pVolumeHandle) {
		VideoLogError("pVolumeHandle is NULL");
		return 0;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	return pVolumeWidget->nMuteVal;
}

bool vp_play_volume_set_mute_value(volume_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	if (pVolumeWidget->nMuteVal == 0) {
		pVolumeWidget->nMuteVal = pVolumeWidget->nCurVal;
	}

	return TRUE;
}

bool vp_play_volume_set_focus_in(volume_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;

	evas_object_show(pVolumeWidget->pFocusPoint);
	elm_object_focus_allow_set(pVolumeWidget->pFocusPoint, EINA_TRUE);
	elm_object_focus_set(pVolumeWidget->pFocusPoint, EINA_TRUE);

	elm_object_focus_next_object_set(pVolumeWidget->pFocusPoint,
	                                 pVolumeWidget->pFocusPoint,
	                                 ELM_FOCUS_UP);
	elm_object_focus_next_object_set(pVolumeWidget->pFocusPoint,
	                                 pVolumeWidget->pFocusPoint,
	                                 ELM_FOCUS_DOWN);

	evas_object_event_callback_del(pVolumeWidget->pFocusPoint,
	                               EVAS_CALLBACK_KEY_DOWN,
	                               __vp_volume_focus_key_down_cb);
	evas_object_event_callback_add(pVolumeWidget->pFocusPoint,
	                               EVAS_CALLBACK_KEY_DOWN,
	                               __vp_volume_focus_key_down_cb,
	                               (void *) pVolumeWidget);
	return TRUE;
}

bool vp_play_volume_set_mouse_callbacks(volume_handle pVolumeHandle,
                                        Evas_Object *pWin)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;
	pVolumeWidget->pWin = pWin;

	evas_object_event_callback_add(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_DOWN,
	                               __vp_volume_mouse_down_cb,
	                               (void *) pVolumeWidget);

	evas_object_event_callback_add(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_UP,
	                               __vp_volume_mouse_up_cb,
	                               (void *) pVolumeWidget);

	evas_object_event_callback_add(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_MOVE,
	                               __vp_volume_mouse_move_cb,
	                               (void *) pVolumeWidget);
	return TRUE;
}

bool vp_play_volume_unset_mouse_callbacks(volume_handle pVolumeHandle,
        Evas_Object *pWin)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pVolumeHandle is NULL");
		return FALSE;
	}

	VolumeWidget *pVolumeWidget = (VolumeWidget *) pVolumeHandle;
	pVolumeWidget->pWin = pWin;

	evas_object_event_callback_del(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_DOWN,
	                               __vp_volume_mouse_down_cb);

	evas_object_event_callback_del(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_UP,
	                               __vp_volume_mouse_up_cb);

	evas_object_event_callback_del(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_MOVE,
	                               __vp_volume_mouse_move_cb);
	return TRUE;
}
