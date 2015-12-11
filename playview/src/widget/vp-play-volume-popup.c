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


#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-volume-popup.h"

#include "vp-sound.h"
#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"


#define VP_VOLUME_POPUP_HIDE_LAYOUT_TIMER_INTERVAL	5.0


#define VP_VOLUME_POPUP_PORTRAIT_POS_X		12
#define VP_VOLUME_POPUP_PORTRAIT_POS_Y		275
#define VP_VOLUME_POPUP_PORTRAIT_HEIGHT		572

#define VP_VOLUME_POPUP_LANDSCAPE_POS_X		19
#define VP_VOLUME_POPUP_LANDSCAPE_POS_Y		67
#define VP_VOLUME_POPUP_LANDSCAPE_HEIGHT	572

#define VP_VOLUME_POPUP_TEXT_MAX_LEN		3



typedef struct _VolumePopupWidget {
	Evas_Object *pParent;
	Evas_Object *pLayout;
	Ecore_Timer *pHideTimer;
	Ecore_Timer *pDeviceTimer;

	bool bLandscape;
	bool bIsRealize;
	bool bMouseDown;

	int nMaxVal;
	int nCurVal;
} VolumePopupWidget;

static void _vp_play_volume_popup_destory_handle(VolumePopupWidget *
        pVolumeWidget);
static double _vp_play_volume_popup_get_mouse_pos_ratio(Evas_Object *
        pObj,
        bool bLandscape,
        int nCurY);
static void _vp_play_volume_popup_set_value(VolumePopupWidget *
        pVolumeWidget, int nValue);
static void _vp_play_volume_popup_create_timer(VolumePopupWidget *
        pVolumeWidget);

/* callback functions */
static void __vp_volume_popup_mouse_down_cb(void *pUserData, Evas *e,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VolumePopupWidget *pVolumeWidget = (VolumePopupWidget *) pUserData;

	if (pVolumeWidget->bIsRealize == FALSE) {
		return;
	}

	Evas_Event_Mouse_Down *pMouseDownEvent =
	    (Evas_Event_Mouse_Down *) pEvent;

	double dRatio =
	    _vp_play_volume_popup_get_mouse_pos_ratio(pVolumeWidget->pLayout,
	            pVolumeWidget->
	            bLandscape,
	            pMouseDownEvent->canvas.
	            y);

	int nCurVal =
	    pVolumeWidget->nMaxVal - (dRatio * pVolumeWidget->nMaxVal);

	_vp_play_volume_popup_set_value(pVolumeWidget, nCurVal);

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);

	pVolumeWidget->bMouseDown = TRUE;
}

static void __vp_volume_popup_mouse_up_cb(void *pUserData, Evas *e,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VolumePopupWidget *pVolumeWidget = (VolumePopupWidget *) pUserData;

	Evas_Event_Mouse_Up *pMouseUpEvent = (Evas_Event_Mouse_Up *) pEvent;

	if (pVolumeWidget->bIsRealize == FALSE) {
		return;
	}

	double dRatio =
	    _vp_play_volume_popup_get_mouse_pos_ratio(pVolumeWidget->pLayout,
	            pVolumeWidget->
	            bLandscape,
	            pMouseUpEvent->canvas.
	            y);

	int nCurVal =
	    pVolumeWidget->nMaxVal - (dRatio * pVolumeWidget->nMaxVal);

	_vp_play_volume_popup_set_value(pVolumeWidget, nCurVal);
	_vp_play_volume_popup_create_timer(pVolumeWidget);

	pVolumeWidget->bMouseDown = FALSE;
}

static void __vp_volume_popup_mouse_move_cb(void *pUserData, Evas *e,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	VolumePopupWidget *pVolumeWidget = (VolumePopupWidget *) pUserData;

	Evas_Event_Mouse_Move *pMouseMoveEvent =
	    (Evas_Event_Mouse_Move *) pEvent;

	if (pVolumeWidget->bIsRealize == FALSE
	        || pVolumeWidget->bMouseDown == FALSE) {
		return;
	}

	double dRatio =
	    _vp_play_volume_popup_get_mouse_pos_ratio(pVolumeWidget->pLayout,
	            pVolumeWidget->
	            bLandscape,
	            pMouseMoveEvent->cur.
	            canvas.y);

	int nCurVal =
	    pVolumeWidget->nMaxVal - (dRatio * pVolumeWidget->nMaxVal);

	if (nCurVal != pVolumeWidget->nCurVal) {
		_vp_play_volume_popup_set_value(pVolumeWidget, nCurVal);
	}

}

static Eina_Bool __vp_volume_popup_hide_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	VolumePopupWidget *pVolumeWidget = (VolumePopupWidget *) pUserData;

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);

	if (!vp_play_volume_popup_unrealize
	        ((volume_popup_handle) pVolumeWidget)) {
		VideoLogWarning("vp_play_volume_popup_unrealize is fail");
	}

	return EINA_FALSE;
}



/* internal functions */
static void _vp_play_volume_popup_destory_handle(VolumePopupWidget *
        pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);
	VP_EVAS_TIMER_DEL(pVolumeWidget->pDeviceTimer);

	VP_EVAS_DEL(pVolumeWidget->pLayout);
	VP_FREE(pVolumeWidget);
}

static double _vp_play_volume_popup_get_mouse_pos_ratio(Evas_Object *
        pObj,
        bool bLandscape,
        int nCurY)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return 0.0;
	}

	int nHeight = 0;
	int nCurrent = 0;
	double dRatio = 0.0;

	evas_object_geometry_get(pObj, NULL, NULL, NULL, &nHeight);

	if (bLandscape) {
		nHeight = VP_VOLUME_POPUP_LANDSCAPE_HEIGHT;
		nCurrent = nCurY - VP_VOLUME_POPUP_LANDSCAPE_POS_Y;
	} else {
		nHeight = VP_VOLUME_POPUP_PORTRAIT_HEIGHT;
		nCurrent = nCurY - VP_VOLUME_POPUP_PORTRAIT_POS_Y;
	}

	if (nCurrent < 0) {
		nCurrent = 0;
	} else if (nCurrent > nHeight) {
		nCurrent = nHeight;
	}

	dRatio = (double) nCurrent / nHeight;

	return dRatio;
}


static void _vp_play_volume_popup_set_widget_position(VolumePopupWidget *
        pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return;
	}

	bool bLandscape = pVolumeWidget->bLandscape;

	if (bLandscape) {
		evas_object_move(pVolumeWidget->pLayout,
		                 VP_VOLUME_POPUP_LANDSCAPE_POS_X * VP_SCALE,
		                 VP_VOLUME_POPUP_LANDSCAPE_POS_Y * VP_SCALE);
	} else {
		evas_object_move(pVolumeWidget->pLayout,
		                 VP_VOLUME_POPUP_PORTRAIT_POS_X * VP_SCALE,
		                 VP_VOLUME_POPUP_PORTRAIT_POS_Y * VP_SCALE);
	}
}


static void _vp_play_volume_popup_update_value(VolumePopupWidget *
        pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return;
	}

	double nVolumeRatio = 0.0;
	char szPlayingTime[VP_VOLUME_POPUP_TEXT_MAX_LEN] = { 0, };

	snprintf(szPlayingTime, VP_VOLUME_POPUP_TEXT_MAX_LEN, "%d",
	         pVolumeWidget->nCurVal);

	nVolumeRatio =
	    ((double) pVolumeWidget->nCurVal /
	     (double) pVolumeWidget->nMaxVal);

	if (!edje_object_part_drag_value_set
	        (_EDJ(pVolumeWidget->pLayout),
	         VP_PLAY_SWALLOW_VOLUME_POPUP_DRAG_RECT, 0.0, nVolumeRatio)) {
		VideoLogWarning("Drag value set fail : %lf", nVolumeRatio);
	}

	elm_object_part_text_set(pVolumeWidget->pLayout,
	                         VP_PLAY_SWALLOW_VOLUME_POPUP_VALUE_LABEL,
	                         szPlayingTime);

	if (pVolumeWidget->nCurVal == 0) {
		edje_object_signal_emit(_EDJ(pVolumeWidget->pLayout), "set,mute",
		                        "volume.popup.icon");
	} else {
		edje_object_signal_emit(_EDJ(pVolumeWidget->pLayout),
		                        "set,volume", "volume.popup.icon");
	}
}

static Eina_Bool __vp_volume_popup_device_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	VolumePopupWidget *pVolumeWidget = (VolumePopupWidget *) pUserData;

	if (pVolumeWidget->pDeviceTimer) {
		VP_EVAS_TIMER_DEL(pVolumeWidget->pDeviceTimer);
	}
	if (!vp_sound_set_volume(pVolumeWidget->nCurVal)) {
		VideoLogError("vp_sound_set_volume");
		return EINA_FALSE;
	}

	return EINA_FALSE;
}

static void _vp_play_volume_popup_set_value(VolumePopupWidget *
        pVolumeWidget, int nValue)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pParent is NULL");
		return;
	}

	pVolumeWidget->nCurVal = nValue;

	_vp_play_volume_popup_update_value(pVolumeWidget);

	if (pVolumeWidget->pDeviceTimer == NULL) {
		pVolumeWidget->pDeviceTimer = ecore_timer_add(0.1,
		                              __vp_volume_popup_device_timer_cb,
		                              (void *)
		                              pVolumeWidget);
	}
}

static void _vp_play_volume_popup_create_timer(VolumePopupWidget *
        pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pParent is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pVolumeWidget->pHideTimer);

	pVolumeWidget->pHideTimer =
	    ecore_timer_add(VP_VOLUME_POPUP_HIDE_LAYOUT_TIMER_INTERVAL,
	                    __vp_volume_popup_hide_timer_cb,
	                    (void *) pVolumeWidget);
}

static Evas_Object *_vp_play_volume_popup_create_layout(Evas_Object *
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
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VP_PLAY_VOLUME_POPUP_EDJ_PATH);
	free(path);
	bRet =
	    elm_layout_file_set(pObj, edj_path,
	                        VP_PLAY_EDJ_GROUP_VOLUME_POPUP);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
		return NULL;
	}

	evas_object_show(pObj);

	return pObj;
}


static bool _vp_play_volume_popup_init_layout(VolumePopupWidget *
        pVolumeWidget)
{
	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pVolumeWidget->pParent;

	pVolumeWidget->pLayout = _vp_play_volume_popup_create_layout(pParent);
	if (pVolumeWidget->pLayout == NULL) {
		VideoLogError("_vp_play_volume_create_layout is fail");
		return FALSE;
	}
	evas_object_event_callback_add(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_DOWN,
	                               __vp_volume_popup_mouse_down_cb,
	                               (void *) pVolumeWidget);

	evas_object_event_callback_add(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_UP,
	                               __vp_volume_popup_mouse_up_cb,
	                               (void *) pVolumeWidget);

	evas_object_event_callback_add(pVolumeWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_MOVE,
	                               __vp_volume_popup_mouse_move_cb,
	                               (void *) pVolumeWidget);

	return TRUE;
}



/* external functions */
volume_popup_handle vp_play_volume_popup_create(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	VolumePopupWidget *pVolumeWidget = NULL;

	pVolumeWidget = calloc(1, sizeof(VolumePopupWidget));

	if (pVolumeWidget == NULL) {
		VideoLogError("pVolumeWidget alloc fail");
		return NULL;
	}

	pVolumeWidget->pParent = pParent;
	pVolumeWidget->bLandscape = FALSE;

	if (!_vp_play_volume_popup_init_layout(pVolumeWidget)) {
		VideoLogError("_vp_play_volume_init_layout is fail");
		_vp_play_volume_popup_destory_handle(pVolumeWidget);
		return NULL;
	}

	if (!vp_sound_get_max_volume(&(pVolumeWidget->nMaxVal))) {
		VideoLogError("vp_sound_get_max_volume fail");
		_vp_play_volume_popup_destory_handle(pVolumeWidget);
		return NULL;
	}

	if (!vp_sound_get_volume(&(pVolumeWidget->nCurVal))) {
		VideoLogError("vp_sound_get_volume fail");
		_vp_play_volume_popup_destory_handle(pVolumeWidget);
		return NULL;
	}

	_vp_play_volume_popup_update_value(pVolumeWidget);

	return (volume_popup_handle) pVolumeWidget;
}

void vp_play_volume_popup_destroy(volume_popup_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pParent is NULL");
		return;
	}

	VolumePopupWidget *pVolumeWidget =
	    (VolumePopupWidget *) pVolumeHandle;

	_vp_play_volume_popup_destory_handle(pVolumeWidget);
}

bool vp_play_volume_popup_realize(volume_popup_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	VolumePopupWidget *pVolumeWidget =
	    (VolumePopupWidget *) pVolumeHandle;

	pVolumeWidget->bIsRealize = TRUE;

	_vp_play_volume_popup_set_widget_position(pVolumeWidget);

	_vp_play_volume_popup_set_value(pVolumeWidget,
	                                pVolumeWidget->nCurVal);

	evas_object_show(pVolumeWidget->pLayout);
	_vp_play_volume_popup_create_timer(pVolumeWidget);

	return TRUE;
}

bool vp_play_volume_popup_unrealize(volume_popup_handle pVolumeHandle)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	VolumePopupWidget *pVolumeWidget =
	    (VolumePopupWidget *) pVolumeHandle;

	pVolumeWidget->bIsRealize = FALSE;

	evas_object_hide(pVolumeWidget->pLayout);

	return TRUE;
}

bool vp_play_volume_popup_set_landscape_mode(volume_popup_handle
        pVolumeHandle,
        bool bLandscape)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	VolumePopupWidget *pVolumeWidget =
	    (VolumePopupWidget *) pVolumeHandle;

	pVolumeWidget->bLandscape = bLandscape;

	_vp_play_volume_popup_set_widget_position(pVolumeWidget);


	return TRUE;
}

bool vp_play_volume_popup_is_realize(volume_popup_handle pVolumeHandle,
                                     bool *bIsRealize)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	VolumePopupWidget *pVolumeWidget =
	    (VolumePopupWidget *) pVolumeHandle;

	*bIsRealize = pVolumeWidget->bIsRealize;

	return TRUE;
}


bool vp_play_volume_popup_set_value(volume_popup_handle pVolumeHandle,
                                    int nCurVal)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	VolumePopupWidget *pVolumeWidget =
	    (VolumePopupWidget *) pVolumeHandle;

	_vp_play_volume_popup_create_timer(pVolumeWidget);

	pVolumeWidget->nCurVal = nCurVal;

	if (pVolumeWidget->nCurVal < 0) {
		pVolumeWidget->nCurVal = 0;
		return TRUE;
	}

	if (pVolumeWidget->nCurVal > pVolumeWidget->nMaxVal) {
		pVolumeWidget->nCurVal = pVolumeWidget->nMaxVal;
		return TRUE;
	}

	_vp_play_volume_popup_set_value(pVolumeWidget,
	                                pVolumeWidget->nCurVal);

	return TRUE;
}

bool vp_play_volume_popup_get_value(int *nCurVal)
{

	int nVal = 0;

	if (!vp_sound_get_volume(&nVal)) {
		VideoLogError("vp_sound_get_volume fail");
		return FALSE;
	}

	*nCurVal = nVal;

	return TRUE;
}

bool vp_play_volume_popup_get_max_value(int *nMaxVal)
{
	int nVal = 0;

	if (!vp_sound_get_max_volume(&nVal)) {
		VideoLogError("vp_sound_get_max_volume fail");
		return FALSE;
	}

	*nMaxVal = nVal;

	return TRUE;
}


bool vp_play_volume_popup_is_mute(volume_popup_handle pVolumeHandle,
                                  bool *bIsMute)
{
	if (pVolumeHandle == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	VolumePopupWidget *pVolumeWidget =
	    (VolumePopupWidget *) pVolumeHandle;

	if (pVolumeWidget->nCurVal == 0) {
		*bIsMute = TRUE;
	} else {
		*bIsMute = FALSE;
	}

	return TRUE;

}
