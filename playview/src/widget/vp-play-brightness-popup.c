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

#include <pthread.h>
#include <app.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-brightness-popup.h"

#include "vp-device.h"
#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"


#define VP_BRIGHTNESS_HIDE_LAYOUT_TIMER_INTERVAL	5.0


#define VP_BRIGHTNESS_PORTRAIT_POS_X	128
#define VP_BRIGHTNESS_PORTRAIT_POS_Y	275
#define VP_BRIGHTNESS_PORTRAIT_HEIGHT	572

#define VP_BRIGHTNESS_LANDSCAPE_POS_X	135
#define VP_BRIGHTNESS_LANDSCAPE_POS_Y	67
#define VP_BRIGHTNESS_LANDSCAPE_HEIGHT	572

#define VP_BRIGHTNESS_TEXT_MAX_LEN	4

#define VP_BRIGHTNESS_BRIGHTNESS_UNIT	10

#define VP_BRIGHTNESS_ICON_NUM		12

typedef struct _BrightnessWidget {
	Evas_Object *pParent;
	Evas_Object *pLayout;
	Evas_Object *pIcon;

	Ecore_Timer *pHideTimer;
	Ecore_Timer *pDeviceTimer;

	bool bLandscape;
	bool bIsRealize;
	bool bMouseDown;

	int nMaxVal;
	int nMinVal;
	int nCurVal;

	int nBrightnessUnit;
} BrightnessWidget;

static void _vp_play_brightness_destory_handle(BrightnessWidget *
        pBrightnessWidget);
static double _vp_play_brightness_get_mouse_pos_ratio(Evas_Object *pObj,
        bool bLandscape,
        int nCurY);
static void _vp_play_brightness_set_value(BrightnessWidget *
        pBrightnessWidget, int nValue);
static void _vp_play_brightness_create_timer(BrightnessWidget *
        pBrightnessWidget);

/* callback functions */
static void __vp_brightness_mouse_down_cb(void *pUserData, Evas *e,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	BrightnessWidget *pBrightnessWidget = (BrightnessWidget *) pUserData;

	if (pBrightnessWidget->bIsRealize == FALSE) {
		return;
	}

	Evas_Event_Mouse_Down *pMouseDownEvent =
	    (Evas_Event_Mouse_Down *) pEvent;

	double dRatio =
	    _vp_play_brightness_get_mouse_pos_ratio(pBrightnessWidget->
	            pLayout,
	            pBrightnessWidget->
	            bLandscape,
	            pMouseDownEvent->canvas.
	            y);

	int nCurVal =
	    pBrightnessWidget->nMaxVal -
	    (dRatio * pBrightnessWidget->nMaxVal);

	_vp_play_brightness_set_value(pBrightnessWidget, nCurVal);

	VP_EVAS_TIMER_DEL(pBrightnessWidget->pHideTimer);

	pBrightnessWidget->bMouseDown = TRUE;
}

static void __vp_brightness_mouse_up_cb(void *pUserData, Evas *e,
                                        Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	BrightnessWidget *pBrightnessWidget = (BrightnessWidget *) pUserData;

	Evas_Event_Mouse_Up *pMouseUpEvent = (Evas_Event_Mouse_Up *) pEvent;

	if (pBrightnessWidget->bIsRealize == FALSE) {
		return;
	}

	double dRatio =
	    _vp_play_brightness_get_mouse_pos_ratio(pBrightnessWidget->
	            pLayout,
	            pBrightnessWidget->
	            bLandscape,
	            pMouseUpEvent->canvas.y);

	int nCurVal =
	    pBrightnessWidget->nMaxVal -
	    (dRatio * pBrightnessWidget->nMaxVal);

	_vp_play_brightness_set_value(pBrightnessWidget, nCurVal);
	_vp_play_brightness_create_timer(pBrightnessWidget);

	pBrightnessWidget->bMouseDown = FALSE;
}

static void __vp_brightness_mouse_move_cb(void *pUserData, Evas *e,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	BrightnessWidget *pBrightnessWidget = (BrightnessWidget *) pUserData;

	Evas_Event_Mouse_Move *pMouseMoveEvent =
	    (Evas_Event_Mouse_Move *) pEvent;

	if (pBrightnessWidget->bIsRealize == FALSE
	        || pBrightnessWidget->bMouseDown == FALSE) {
		return;
	}

	double dRatio =
	    _vp_play_brightness_get_mouse_pos_ratio(pBrightnessWidget->
	            pLayout,
	            pBrightnessWidget->
	            bLandscape,
	            pMouseMoveEvent->cur.
	            canvas.y);

	int nCurVal =
	    pBrightnessWidget->nMaxVal -
	    (dRatio * pBrightnessWidget->nMaxVal);

	if (nCurVal != pBrightnessWidget->nCurVal) {
		_vp_play_brightness_set_value(pBrightnessWidget, nCurVal);
	}

}

static Eina_Bool __vp_brightness_hide_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	BrightnessWidget *pBrightnessWidget = (BrightnessWidget *) pUserData;

	VP_EVAS_TIMER_DEL(pBrightnessWidget->pHideTimer);

	if (!vp_play_brightness_unrealize
	        ((brightness_handle) pBrightnessWidget)) {
		VideoLogWarning("vp_play_brightness_unrealize is fail");
	}

	return EINA_FALSE;
}



/* internal functions */
static void _vp_play_brightness_destory_handle(BrightnessWidget *
        pBrightnessWidget)
{
	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidget is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pBrightnessWidget->pHideTimer);
	VP_EVAS_TIMER_DEL(pBrightnessWidget->pDeviceTimer);
	VP_EVAS_DEL(pBrightnessWidget->pLayout);

	VP_FREE(pBrightnessWidget);
}

static double _vp_play_brightness_get_mouse_pos_ratio(Evas_Object *pObj,
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
		nHeight = VP_BRIGHTNESS_LANDSCAPE_HEIGHT;
		nCurrent = nCurY - VP_BRIGHTNESS_LANDSCAPE_POS_Y;
	} else {
		nHeight = VP_BRIGHTNESS_PORTRAIT_HEIGHT;
		nCurrent = nCurY - VP_BRIGHTNESS_PORTRAIT_POS_Y;
	}

	if (nCurrent < 0) {
		nCurrent = 0;
	} else if (nCurrent > nHeight) {
		nCurrent = nHeight;
	}

	dRatio = (double) nCurrent / nHeight;

	return dRatio;
}


static void _vp_play_brightness_set_widget_position(BrightnessWidget *
        pBrightnessWidget)
{
	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidget is NULL");
		return;
	}

	bool bLandscape = pBrightnessWidget->bLandscape;
	int nWidth = 0;
	int nHeight = 0;

	elm_win_screen_size_get(pBrightnessWidget->pParent, NULL, NULL,
	                        &nWidth, &nHeight);

	if (bLandscape) {
		evas_object_move(pBrightnessWidget->pLayout,
		                 nHeight -
		                 VP_BRIGHTNESS_LANDSCAPE_POS_X * VP_SCALE,
		                 VP_BRIGHTNESS_LANDSCAPE_POS_Y * VP_SCALE);
	} else {
		evas_object_move(pBrightnessWidget->pLayout,
		                 nWidth - VP_BRIGHTNESS_PORTRAIT_POS_X * VP_SCALE,
		                 VP_BRIGHTNESS_PORTRAIT_POS_Y * VP_SCALE);
	}
}

static void _vp_play_brightness_update_icon(BrightnessWidget *
        pBrightnessWidget,
        double nValueRatio)
{
	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidget is NULL");
		return;
	}

	if (pBrightnessWidget->pIcon == NULL) {
		VideoLogError("pBrightnessWidget->pIcon is NULL");
		return;
	}
	char edj_path[1024] = {0};

	char *ppath = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", ppath , "edje", VP_PLAY_RESROUCE_EDJ_PATH);

	gchar *path = NULL;
	int req = 0;
	if (pBrightnessWidget->nCurVal == pBrightnessWidget->nMaxVal) {
		req = VP_BRIGHTNESS_ICON_NUM - 1;
	} else if (pBrightnessWidget->nCurVal > pBrightnessWidget->nMinVal) {
		req = (int)(nValueRatio * (VP_BRIGHTNESS_ICON_NUM - 2) + 1);
	}
	path = g_strdup_printf(VP_PLAY_BRIGHTNESS_POPUP_ICON_PATH, req);
	elm_image_file_set(pBrightnessWidget->pIcon,
	                   edj_path, path);
	g_free(path);
}

static void _vp_play_brightness_update_value(BrightnessWidget *
        pBrightnessWidget)
{
	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidget is NULL");
		return;
	}

	double nValueRatio = 0.0;
	char szPlayingTime[VP_BRIGHTNESS_TEXT_MAX_LEN] = { 0, };

	snprintf(szPlayingTime, VP_BRIGHTNESS_TEXT_MAX_LEN, "%d",
	         pBrightnessWidget->nCurVal);

	nValueRatio =
	    ((double) pBrightnessWidget->nCurVal /
	     (double)(pBrightnessWidget->nMaxVal -
	              pBrightnessWidget->nMinVal));

	if (!edje_object_part_drag_value_set
	        (_EDJ(pBrightnessWidget->pLayout),
	         VP_PLAY_SWALLOW_BRIGHTNESS_POPUP_DRAG_RECT, 0.0, nValueRatio)) {
		VideoLogWarning("Drag value set fail : %lf", nValueRatio);
	}

	elm_object_part_text_set(pBrightnessWidget->pLayout,
	                         VP_PLAY_SWALLOW_BRIGHTNESS_POPUP_VALUE_LABEL,
	                         szPlayingTime);
	/*update brightness icon */
	_vp_play_brightness_update_icon(pBrightnessWidget, nValueRatio);
}

static void _vp_play_brightness_create_timer(BrightnessWidget *
        pBrightnessWidget)
{
	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidgetis NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pBrightnessWidget->pHideTimer);

	pBrightnessWidget->pHideTimer =
	    ecore_timer_add(VP_BRIGHTNESS_HIDE_LAYOUT_TIMER_INTERVAL,
	                    __vp_brightness_hide_timer_cb,
	                    (void *) pBrightnessWidget);
}

static Eina_Bool __vp_brightness_popup_device_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	BrightnessWidget *pBrightnessWidget = (BrightnessWidget *) pUserData;

	if (pBrightnessWidget->pDeviceTimer) {
		VP_EVAS_TIMER_DEL(pBrightnessWidget->pDeviceTimer);
	}
	if (!vp_device_set_brightness(pBrightnessWidget->nCurVal)) {
		VideoLogError("vp_device_set_brightness");
	}

	return EINA_FALSE;
}

static void _vp_play_brightness_set_value(BrightnessWidget *
        pBrightnessWidget, int nValue)
{
	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidget is NULL");
		return;
	}

	pBrightnessWidget->nCurVal = nValue;

	_vp_play_brightness_update_value(pBrightnessWidget);

	evas_object_show(pBrightnessWidget->pLayout);

	if (pBrightnessWidget->pDeviceTimer == NULL) {
		pBrightnessWidget->pDeviceTimer = ecore_timer_add(0.1,
		                                  __vp_brightness_popup_device_timer_cb,
		                                  (void *)
		                                  pBrightnessWidget);
	}
}


static Evas_Object *_vp_play_brightness_create_layout(Evas_Object *
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
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VP_PLAY_BRIGHTNESS_POPUP_EDJ_PATH);

	bRet =
	    elm_layout_file_set(pObj, edj_path,
	                        VP_PLAY_EDJ_GROUP_BRIGHTNESS_POPUP);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
		return NULL;
	}

	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_play_brightness_create_icon(Evas_Object *pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pIcon = elm_image_add(pParent);
	evas_object_size_hint_weight_set(pIcon, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pIcon, EVAS_HINT_FILL,
	                                EVAS_HINT_FILL);
	elm_object_part_content_set(pParent,
	                            VP_PLAY_SWALLOW_BRIGHTNESS_POPUP_ICON,
	                            pIcon);
	evas_object_show(pIcon);

	return pIcon;
}

static bool _vp_play_brightness_init_layout(BrightnessWidget *
        pBrightnessWidget)
{
	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidget is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pBrightnessWidget->pParent;

	pBrightnessWidget->pLayout =
	    _vp_play_brightness_create_layout(pParent);
	if (pBrightnessWidget->pLayout == NULL) {
		VideoLogError("_vp_play_brightness_create_layout is fail");
		return FALSE;
	}

	pBrightnessWidget->pIcon =
	    _vp_play_brightness_create_icon(pBrightnessWidget->pLayout);
	if (pBrightnessWidget->pIcon == NULL) {
		VideoLogError("_vp_play_brightness_create_icon is fail");
		return FALSE;
	}

	evas_object_event_callback_add(pBrightnessWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_DOWN,
	                               __vp_brightness_mouse_down_cb,
	                               (void *) pBrightnessWidget);

	evas_object_event_callback_add(pBrightnessWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_UP,
	                               __vp_brightness_mouse_up_cb,
	                               (void *) pBrightnessWidget);

	evas_object_event_callback_add(pBrightnessWidget->pLayout,
	                               EVAS_CALLBACK_MOUSE_MOVE,
	                               __vp_brightness_mouse_move_cb,
	                               (void *) pBrightnessWidget);

	return TRUE;
}



/* external functions */
brightness_handle vp_play_brightness_create(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	BrightnessWidget *pBrightnessWidget = NULL;

	pBrightnessWidget = calloc(1, sizeof(BrightnessWidget));

	if (pBrightnessWidget == NULL) {
		VideoLogError("pBrightnessWidget alloc fail");
		return NULL;
	}

	pBrightnessWidget->pParent = pParent;
	pBrightnessWidget->bLandscape = FALSE;

	if (!_vp_play_brightness_init_layout(pBrightnessWidget)) {
		VideoLogError("_vp_play_brightness_init_layout is fail");
		_vp_play_brightness_destory_handle(pBrightnessWidget);
		return NULL;
	}

	if (!vp_device_get_max_brightness(&(pBrightnessWidget->nMaxVal))) {
		VideoLogError("vp_device_get_max_brightness fail");
		_vp_play_brightness_destory_handle(pBrightnessWidget);
		return NULL;
	}

	if (!vp_device_get_min_brightness(&(pBrightnessWidget->nMinVal))) {
		VideoLogError("vp_device_get_min_brightness fail");
		_vp_play_brightness_destory_handle(pBrightnessWidget);
		return NULL;
	}

	pBrightnessWidget->nBrightnessUnit =
	    pBrightnessWidget->nMaxVal / VP_BRIGHTNESS_BRIGHTNESS_UNIT;

	if (!vp_device_get_brightness(&(pBrightnessWidget->nCurVal))) {
		VideoLogError("vp_device_get_brightness fail");
		_vp_play_brightness_destory_handle(pBrightnessWidget);
		return NULL;
	}

	_vp_play_brightness_update_value(pBrightnessWidget);

	return (brightness_handle) pBrightnessWidget;
}

void vp_play_brightness_destroy(brightness_handle pBrightnessHandle)
{
	if (pBrightnessHandle == NULL) {
		VideoLogError("pBrightnessHandle is NULL");
		return;
	}

	BrightnessWidget *pBrightnessWidget =
	    (BrightnessWidget *) pBrightnessHandle;

	_vp_play_brightness_destory_handle(pBrightnessWidget);
}

bool vp_play_brightness_realize(brightness_handle pBrightnessHandle)
{
	if (pBrightnessHandle == NULL) {
		VideoLogError("pBrightnessHandle is NULL");
		return FALSE;
	}

	BrightnessWidget *pBrightnessWidget =
	    (BrightnessWidget *) pBrightnessHandle;

	pBrightnessWidget->bIsRealize = TRUE;

	_vp_play_brightness_set_widget_position(pBrightnessWidget);

	_vp_play_brightness_set_value(pBrightnessWidget,
	                              pBrightnessWidget->nCurVal);

	evas_object_show(pBrightnessWidget->pLayout);

	_vp_play_brightness_create_timer(pBrightnessWidget);

	return TRUE;
}

bool vp_play_brightness_unrealize(brightness_handle pBrightnessHandle)
{
	if (pBrightnessHandle == NULL) {
		VideoLogError("pBrightnessHandle is NULL");
		return FALSE;
	}

	BrightnessWidget *pBrightnessWidget =
	    (BrightnessWidget *) pBrightnessHandle;

	pBrightnessWidget->bIsRealize = FALSE;

	evas_object_hide(pBrightnessWidget->pLayout);

	return TRUE;
}

bool vp_play_brightness_set_landscape_mode(brightness_handle
        pBrightnessHandle,
        bool bLandscape)
{
	if (pBrightnessHandle == NULL) {
		VideoLogError("pBrightnessHandle is NULL");
		return FALSE;
	}

	BrightnessWidget *pBrightnessWidget =
	    (BrightnessWidget *) pBrightnessHandle;

	pBrightnessWidget->bLandscape = bLandscape;

	_vp_play_brightness_set_widget_position(pBrightnessWidget);


	return TRUE;
}

bool vp_play_brightness_is_realize(brightness_handle pBrightnessHandle,
                                   bool *bIsRealize)
{
	if (pBrightnessHandle == NULL) {
		VideoLogError("pBrightnessHandle is NULL");
		return FALSE;
	}

	BrightnessWidget *pBrightnessWidget =
	    (BrightnessWidget *) pBrightnessHandle;

	*bIsRealize = pBrightnessWidget->bIsRealize;

	return TRUE;
}

bool vp_play_brightness_set_value(brightness_handle pBrightnessHandle,
                                  int nCurVal)
{
	if (pBrightnessHandle == NULL) {
		VideoLogError("pBrightnessHandle is NULL");
		return FALSE;
	}

	BrightnessWidget *pBrightnessWidget =
	    (BrightnessWidget *) pBrightnessHandle;

	_vp_play_brightness_create_timer(pBrightnessWidget);

	pBrightnessWidget->nCurVal = nCurVal;

	if (pBrightnessWidget->nCurVal < pBrightnessWidget->nMinVal) {
		pBrightnessWidget->nCurVal = pBrightnessWidget->nMinVal;
		return TRUE;
	}

	if (pBrightnessWidget->nCurVal > pBrightnessWidget->nMaxVal) {
		pBrightnessWidget->nCurVal = pBrightnessWidget->nMaxVal;
		return TRUE;
	}

	_vp_play_brightness_set_value(pBrightnessWidget,
	                              pBrightnessWidget->nCurVal);

	return TRUE;
}

bool vp_play_brightness_get_value(int *nCurVal)
{

	int nVal = 0;

	if (!vp_device_get_brightness(&nVal)) {
		VideoLogError("vp_device_get_brightness fail");
		return FALSE;
	}

	*nCurVal = nVal;

	return TRUE;
}

bool vp_play_brightness_get_max_value(int *nMaxVal)
{
	int nVal = 0;

	if (!vp_device_get_max_brightness(&(nVal))) {
		VideoLogError("vp_device_get_brightness fail");
		return FALSE;
	}

	*nMaxVal = nVal;

	return TRUE;
}

bool vp_play_brightness_get_min_value(int *nMinVal)
{
	int nVal = 0;

	if (!vp_device_get_min_brightness(&(nVal))) {
		VideoLogError("vp_device_get_min_brightness fail");
		return FALSE;
	}

	*nMinVal = nVal;

	return TRUE;
}
