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
#include <glib.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-zoom-guide.h"

#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"


typedef struct _ZoomGuideWidget {
	Evas_Object *pParent;
	Evas_Object *pLayout;
	Evas_Object *pItemLayout;

	Ecore_Timer *pHideTimer;

	bool bRealized;

	double fPosX;
	double fPosY;
	double fResultPosX;
	double fResultPosY;

	double fZoomValue;

	Evas_Coord_Rectangle rtOldRect;
} ZoomGuideWidget;



static void _vp_zoom_guide_destory_handle(ZoomGuideWidget *
                                        pZoomGuideWidget);
static void _vp_zoom_guide_update_item(ZoomGuideWidget *pZoomGuideWidget,
                                       bool bUpdate);


/* callback functions */
static Eina_Bool __vp_zoom_guide_hide_timer_cb(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return EINA_FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pUserData;

	elm_object_signal_emit(pZoomGuideWidget->pLayout,
	                       VP_NORMAL_SIGNAL_ZOOM_GUIDE_HIDE, "*");

	evas_object_hide(pZoomGuideWidget->pLayout);
	evas_object_hide(pZoomGuideWidget->pItemLayout);

	pZoomGuideWidget->pHideTimer = NULL;
	return EINA_FALSE;
}


static void __vp_zoom_guide_resize_cb(void *pUserData, Evas *pEvas,
                                      Evas_Object *pObj, void *pEvent)
{
	if (!pUserData) {
		VideoLogError("[ERR] No Exist pUserData.");
		return;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pUserData;
	Evas_Coord_Rectangle rtCurRect = { 0, };

	evas_object_geometry_get(pObj, &(rtCurRect.x), &(rtCurRect.y),
	                         &(rtCurRect.w), &(rtCurRect.h));

	if (pZoomGuideWidget->rtOldRect.x != rtCurRect.x ||
	        pZoomGuideWidget->rtOldRect.y != rtCurRect.y ||
	        pZoomGuideWidget->rtOldRect.w != rtCurRect.w ||
	        pZoomGuideWidget->rtOldRect.h != rtCurRect.h) {

		pZoomGuideWidget->rtOldRect.x = rtCurRect.x;
		pZoomGuideWidget->rtOldRect.y = rtCurRect.y;
		pZoomGuideWidget->rtOldRect.w = rtCurRect.w;
		pZoomGuideWidget->rtOldRect.h = rtCurRect.h;

		_vp_zoom_guide_update_item(pZoomGuideWidget, FALSE);
	}

}


/* internal functions */
static void _vp_zoom_guide_update_item(ZoomGuideWidget *pZoomGuideWidget,
                                       bool bUpdate)
{
	if (pZoomGuideWidget == NULL) {
		VideoLogError("pZoomGuideWidget is NULL");
		return;
	}
	Evas_Object *pParent = pZoomGuideWidget->pLayout;
	Evas_Object *pItem = pZoomGuideWidget->pItemLayout;

	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return;
	}

	if (pItem == NULL) {
		VideoLogError("pItem is NULL");
		return;
	}

	Evas_Coord_Rectangle nRect = { 0, };
	Evas_Coord_Rectangle nDstRect = { 0, };

	evas_object_geometry_get(pParent, &(nRect.x), &(nRect.y), &(nRect.w),
	                         &(nRect.h));

	nDstRect.w = nRect.w * pZoomGuideWidget->fZoomValue;
	nDstRect.h = nRect.h * pZoomGuideWidget->fZoomValue;

	nDstRect.x =
	    nRect.x + (nRect.w * pZoomGuideWidget->fPosX) - (nDstRect.w / 2);
	nDstRect.y =
	    nRect.y + (nRect.h * pZoomGuideWidget->fPosY) - (nDstRect.h / 2);

	if (nDstRect.x < nRect.x) {
		nDstRect.x = nRect.x;
	}

	if (nDstRect.y < nRect.y) {
		nDstRect.y = nRect.y;
	}

	if (nDstRect.w + nDstRect.x > nRect.w + nRect.x) {
		nDstRect.x = (nRect.w + nRect.x) - nDstRect.w;
	}

	if (nDstRect.h + nDstRect.y > nRect.h + nRect.y) {
		nDstRect.y = (nRect.h + nRect.y) - nDstRect.h;
	}

	pZoomGuideWidget->fResultPosX =
	    (double)((double)(nDstRect.x - nRect.x) / (double)(nRect.w));
	pZoomGuideWidget->fResultPosY =
	    (double)((double)(nDstRect.y - nRect.y) / (double)(nRect.h));

	evas_object_move(pItem, nDstRect.x, nDstRect.y);
	evas_object_resize(pItem, nDstRect.w, nDstRect.h);

	if (bUpdate) {
		elm_object_signal_emit(pZoomGuideWidget->pLayout,
		                       VP_NORMAL_SIGNAL_ZOOM_GUIDE_SHOW, "*");
		evas_object_show(pZoomGuideWidget->pLayout);
		evas_object_show(pZoomGuideWidget->pItemLayout);

		VP_EVAS_TIMER_DEL(pZoomGuideWidget->pHideTimer);
		pZoomGuideWidget->pHideTimer =
		    ecore_timer_add(1.5, __vp_zoom_guide_hide_timer_cb,
		                    (void *) pZoomGuideWidget);
	}

}

static void _vp_zoom_guide_destory_handle(ZoomGuideWidget *
        pZoomGuideWidget)
{
	if (pZoomGuideWidget == NULL) {
		VideoLogError("pZoomGuideWidget is NULL");
		return;
	}

	VP_EVAS_TIMER_DEL(pZoomGuideWidget->pHideTimer);

	VP_EVAS_DEL(pZoomGuideWidget->pLayout);
	VP_EVAS_DEL(pZoomGuideWidget->pItemLayout);

	VP_FREE(pZoomGuideWidget);
}

static Evas_Object *_vp_zoom_guide_create_layout(Evas_Object *pParent,
        void *pUserData)
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

	bRet =
	    elm_layout_file_set(pObj, VP_PLAY_ZOOM_GUIDE_EDJ_PATH,
	                        VP_PLAY_EDJ_GROUP_ZOOM_GUIDE);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
		return NULL;
	}
	evas_object_event_callback_add(pObj, EVAS_CALLBACK_RESIZE,
	                               __vp_zoom_guide_resize_cb, pUserData);

	evas_object_event_callback_add(pObj, EVAS_CALLBACK_MOVE,
	                               __vp_zoom_guide_resize_cb, pUserData);


	evas_object_show(pObj);

	return pObj;

}


static Evas_Object *_vp_zoom_guide_create_item_layout(Evas_Object *
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

	bRet =
	    elm_layout_file_set(pObj, VP_PLAY_ZOOM_GUIDE_EDJ_PATH,
	                        VP_PLAY_EDJ_GROUP_ZOOM_GUIDE_ITEM);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
		return NULL;
	}

	evas_object_show(pObj);

	return pObj;

}


static bool _vp_zoom_guide_init_layout(ZoomGuideWidget *pZoomGuideWidget)
{
	if (pZoomGuideWidget == NULL) {
		VideoLogError("pZoomGuideWidget is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pZoomGuideWidget->pParent;

	pZoomGuideWidget->pLayout =
	    _vp_zoom_guide_create_layout(pParent, (void *) pZoomGuideWidget);
	if (pZoomGuideWidget->pLayout == NULL) {
		VideoLogError("_vp_zoom_guide_create_layout is fail");
		return FALSE;
	}

	pZoomGuideWidget->pItemLayout =
	    _vp_zoom_guide_create_item_layout(pZoomGuideWidget->pLayout);
	if (pZoomGuideWidget->pItemLayout == NULL) {
		VideoLogError("_vp_zoom_guide_create_item_layout is fail");
		return FALSE;
	}

	return TRUE;
}

/* external functions */
zoom_guide_handle vp_zoom_guide_create(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	ZoomGuideWidget *pZoomGuideWidget = NULL;

	pZoomGuideWidget = calloc(1, sizeof(ZoomGuideWidget));

	if (pZoomGuideWidget == NULL) {
		VideoLogError("pZoomGuideWidget alloc fail");
		return NULL;
	}

	pZoomGuideWidget->pParent = pParent;
	pZoomGuideWidget->bRealized = FALSE;
	pZoomGuideWidget->fZoomValue = 1.0;
	pZoomGuideWidget->fPosX = 0.0;
	pZoomGuideWidget->fPosY = 0.0;

	if (!_vp_zoom_guide_init_layout(pZoomGuideWidget)) {
		VideoLogError("_vp_zoom_guide_init_layout is fail");
		_vp_zoom_guide_destory_handle(pZoomGuideWidget);
		return NULL;
	}

	return (zoom_guide_handle) pZoomGuideWidget;
}

void vp_zoom_guide_destroy(zoom_guide_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	vp_zoom_guide_unrealize((zoom_guide_handle) pZoomGuideWidget);

	_vp_zoom_guide_destory_handle(pZoomGuideWidget);

	return;
}


bool vp_zoom_guide_realize(zoom_guide_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;
	pZoomGuideWidget->bRealized = TRUE;

	return TRUE;
}

bool vp_zoom_guide_unrealize(zoom_guide_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	pZoomGuideWidget->bRealized = FALSE;

	return TRUE;
}

Evas_Object *vp_zoom_guide_get_object(zoom_guide_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return NULL;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	return pZoomGuideWidget->pLayout;
}

bool vp_zoom_guide_set_zoom_value(zoom_guide_handle pWidgetHandle,
                                  double fZoomVal)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	pZoomGuideWidget->fZoomValue = 1.0 / fZoomVal;

	_vp_zoom_guide_update_item(pZoomGuideWidget, TRUE);

	return TRUE;
}

bool vp_zoom_guide_get_zoom_value(zoom_guide_handle pWidgetHandle,
                                  double *fZoomVal)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	*fZoomVal = pZoomGuideWidget->fZoomValue;

	return TRUE;
}

bool vp_zoom_guide_set_move_position(zoom_guide_handle pWidgetHandle,
                                     double fPosX, double fPosY)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	pZoomGuideWidget->fPosX = fPosX;
	pZoomGuideWidget->fPosY = fPosY;

	if (pZoomGuideWidget->fPosX > 1.0) {
		pZoomGuideWidget->fPosX = 1.0;
	}

	if (pZoomGuideWidget->fPosY > 1.0) {
		pZoomGuideWidget->fPosY = 1.0;
	}

	if (pZoomGuideWidget->fPosX < 0.0) {
		pZoomGuideWidget->fPosX = 0.0;
	}

	if (pZoomGuideWidget->fPosY < 0.0) {
		pZoomGuideWidget->fPosY = 0.0;
	}

	_vp_zoom_guide_update_item(pZoomGuideWidget, TRUE);

	return TRUE;
}


bool vp_zoom_guide_get_move_position(zoom_guide_handle pWidgetHandle,
                                     double *fPosX, double *fPosY)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	*fPosX = pZoomGuideWidget->fPosX;
	*fPosY = pZoomGuideWidget->fPosY;

	return TRUE;
}

bool vp_zoom_guide_get_real_position(zoom_guide_handle pWidgetHandle,
                                     double *fPosX, double *fPosY)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	*fPosX = pZoomGuideWidget->fResultPosX;
	*fPosY = pZoomGuideWidget->fResultPosY;

	return TRUE;
}


bool vp_zoom_guide_update(zoom_guide_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ZoomGuideWidget *pZoomGuideWidget = (ZoomGuideWidget *) pWidgetHandle;

	_vp_zoom_guide_update_item(pZoomGuideWidget, TRUE);

	return TRUE;
}
