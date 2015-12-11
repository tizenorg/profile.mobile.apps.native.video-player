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
#include <app.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"
#include "vp-play-button.h"

#include "vp-play-progressbar.h"

#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"


typedef struct _ProgressWidget {
	Evas_Object *pParent;
	Evas_Object *pLayout;

	Evas_Object *pFocusPoint;

	Evas_Object *pStartLabel;
	Evas_Object *pTotalLabel;

	video_progressbar_type_t nType;

	bool bEnableAdjustment;
	bool bRealized;

	int nDuration;
	int nCurrentPos;
	int nBufferingPos;
} ProgressWidget;



static void _vp_play_progressbar_destory_handle(ProgressWidget *
        pProgressWidget);



/* callback functions */
static void __vp_play_progressbar_key_focus_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pUserData;
	if (pProgressWidget->pLayout == NULL) {
		VideoLogError("pProgressWidget->pLayout is NULL");
		return;
	}


	elm_object_signal_emit(pProgressWidget->pLayout,
	                       VP_NORMAL_SIGNAL_PROGRESS_PRESS, "*");

	VideoLogInfo("progress focus in");
}

static void __vp_play_progressbar_key_unfocus_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pUserData;
	if (pProgressWidget->pLayout == NULL) {
		VideoLogError("pProgressWidget->pLayout is NULL");
		return;
	}

	elm_object_signal_emit(pProgressWidget->pLayout,
	                       VP_NORMAL_SIGNAL_PROGRESS_UNPRESS, "*");

	VideoLogInfo("progress focus out");
}

/* internal functions */

static void _vp_play_progressbar_value_update(ProgressWidget *
        pProgressWidget)
{
	if (pProgressWidget == NULL) {
		VideoLogError("pProgressWidget is NULL");
		return;
	}
	if (pProgressWidget->pLayout == NULL) {
		VideoLogError("pProgressWidget->pLayout is NULL");
		return;
	}

	double dPlayedRatio = 0.0;
	double dBufferingRatio = 0.0;

	char szPlayingTime[PROGRESSBAR_TIME_LENGTH] = { 0, };

	snprintf(szPlayingTime, PROGRESSBAR_TIME_LENGTH,
	         "%" PROGRESSBAR_TIME_FORMAT,
	         PROGRESSBAR_TIME_ARGS(pProgressWidget->nCurrentPos / 1000));

	dPlayedRatio =
	    ((double) pProgressWidget->nCurrentPos /
	     (double) pProgressWidget->nDuration);
	dBufferingRatio =
	    ((double) pProgressWidget->nBufferingPos / (double) 100);

	edje_object_part_drag_value_set(_EDJ(pProgressWidget->pLayout),
	                                VP_PLAY_SWALLOW_PROGRESS_DRAG_RECT,
	                                dPlayedRatio, 0.0);
	edje_object_part_drag_value_set(_EDJ(pProgressWidget->pLayout),
	                                "pv.progress.buffering",
	                                dBufferingRatio, 0.0);

	//elm_object_part_text_set(pProgressWidget->pStartLabel, "elm.text", szPlayingTime);
	elm_object_part_text_set(pProgressWidget->pLayout,
	                         VP_PLAY_SWALLOW_PROGRESS_PLAYING_LABEL,
	                         szPlayingTime);

}

static void _vp_play_progressbar_destory_handle(ProgressWidget *
        pProgressWidget)
{
	if (pProgressWidget == NULL) {
		VideoLogError("pProgressWidget is NULL");
		return;
	}

	vp_play_progressbar_unrealize((progressbar_handle) pProgressWidget);
	if (pProgressWidget->pFocusPoint) {
		evas_object_smart_callback_del(pProgressWidget->pFocusPoint,
		                               "focused",
		                               __vp_play_progressbar_key_focus_cb);
		evas_object_smart_callback_del(pProgressWidget->pFocusPoint,
		                               "unfocused",
		                               __vp_play_progressbar_key_unfocus_cb);
		VP_EVAS_DEL(pProgressWidget->pFocusPoint);
	}

	VP_EVAS_DEL(pProgressWidget->pStartLabel);
	VP_EVAS_DEL(pProgressWidget->pTotalLabel);

	VP_EVAS_DEL(pProgressWidget->pLayout);

	VP_FREE(pProgressWidget);
}

static Evas_Object *_vp_play_progressbar_create_layout(Evas_Object *
        pParent,
        video_progressbar_type_t
        nType)
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
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VP_PLAY_PROGRESSBAR_EDJ_PATH);

	if (nType == VIDEO_PROGRESSBAR_TYPE_NORMAL) {
		bRet =
		    elm_layout_file_set(pObj, edj_path,
		                        VP_PLAY_EDJ_GROUP_PROGRESSBAR);
		if (bRet != EINA_TRUE) {
			VideoLogError("elm_layout_file_set fail");
			return NULL;
		}
	} else if (nType == VIDEO_PROGRESSBAR_TYPE_TRIM) {
	}

	evas_object_show(pObj);

	return pObj;

}

static void _vp_play_progressbar_layout_del_cb(void *pUserData, Evas *e,
        Evas_Object *pObject,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pProgressWidget is NULL");
		return;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pUserData;

	pProgressWidget->pLayout = NULL;
	pProgressWidget->pFocusPoint = NULL;

}

static bool _vp_play_progressbar_init_layout(ProgressWidget *
        pProgressWidget)
{
	if (pProgressWidget == NULL) {
		VideoLogError("pProgressWidget is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pProgressWidget->pParent;

	pProgressWidget->pLayout =
	    _vp_play_progressbar_create_layout(pParent,
	                                       pProgressWidget->nType);
	if (pProgressWidget->pLayout == NULL) {
		VideoLogError("_vp_play_progressbar_create_layout is fail");
		return FALSE;
	}

	evas_object_event_callback_add(pProgressWidget->pLayout,
	                               EVAS_CALLBACK_DEL,
	                               _vp_play_progressbar_layout_del_cb,
	                               (void *) pProgressWidget);


	pProgressWidget->pFocusPoint =
	    vp_button_create(pProgressWidget->pLayout, "focus", NULL, NULL,
	                     NULL, NULL, (void *) pProgressWidget);
	if (!pProgressWidget->pFocusPoint) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	elm_object_focus_next_object_set(pProgressWidget->pFocusPoint,
	                                 pProgressWidget->pFocusPoint,
	                                 ELM_FOCUS_RIGHT);
	elm_object_focus_next_object_set(pProgressWidget->pFocusPoint,
	                                 pProgressWidget->pFocusPoint,
	                                 ELM_FOCUS_LEFT);

	evas_object_smart_callback_add(pProgressWidget->pFocusPoint,
	                               "focused",
	                               __vp_play_progressbar_key_focus_cb,
	                               pProgressWidget);
	evas_object_smart_callback_add(pProgressWidget->pFocusPoint,
	                               "unfocused",
	                               __vp_play_progressbar_key_unfocus_cb,
	                               pProgressWidget);

	elm_object_part_content_set(pProgressWidget->pLayout,
	                            "pv.progress.point.focus",
	                            pProgressWidget->pFocusPoint);

	return TRUE;
}


progressbar_handle vp_play_progressbar_create(Evas_Object *pParent,
        video_progressbar_type_t
        nType)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	ProgressWidget *pProgressWidget = NULL;

	pProgressWidget = calloc(1, sizeof(ProgressWidget));

	if (pProgressWidget == NULL) {
		VideoLogError("pProgressbar alloc fail");
		return NULL;
	}

	pProgressWidget->pParent = pParent;
	pProgressWidget->nType = nType;
	pProgressWidget->bRealized = FALSE;

	if (!_vp_play_progressbar_init_layout(pProgressWidget)) {
		VideoLogError("_vp_play_progressbar_init_layout is fail");
		VP_FREE(pProgressWidget);
		return NULL;
	}



	return (progressbar_handle) pProgressWidget;
}

void vp_play_progressbar_destroy(progressbar_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return;
	}
	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;

	_vp_play_progressbar_destory_handle(pProgressWidget);

	return;
}


bool vp_play_progressbar_realize(progressbar_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;
	pProgressWidget->bRealized = TRUE;

	return TRUE;
}

bool vp_play_progressbar_unrealize(progressbar_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;
	pProgressWidget->bRealized = FALSE;

	return TRUE;
}

Evas_Object *vp_play_progressbar_get_object(progressbar_handle
        pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return NULL;
	}
	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;

	return pProgressWidget->pLayout;
}

Evas_Object *vp_play_progressbar_get_focus_object(progressbar_handle
        pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return NULL;
	}
	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;

	return pProgressWidget->pFocusPoint;
}

bool vp_play_progressbar_set_duration(progressbar_handle pWidgetHandle,
                                      int nDuration)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;
	if (pProgressWidget->pLayout == NULL) {
		VideoLogError("pProgressWidget->pLayout is NULL");
		return FALSE;
	}

	char szDurationTime[PROGRESSBAR_TIME_LENGTH] = { 0, };

	pProgressWidget->nDuration = nDuration;

	snprintf(szDurationTime, PROGRESSBAR_TIME_LENGTH,
	         "%" PROGRESSBAR_TIME_FORMAT,
	         PROGRESSBAR_TIME_ARGS(pProgressWidget->nDuration / 1000));

	//elm_object_part_text_set(pProgressWidget->pTotalLabel, "elm.text", szDurationTime);
	elm_object_part_text_set(pProgressWidget->pLayout,
	                         VP_PLAY_SWALLOW_PROGRESS_DURATION_LABEL,
	                         szDurationTime);

	return TRUE;
}

bool vp_play_progressbar_set_position(progressbar_handle pWidgetHandle,
                                      int nPosition)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;

	pProgressWidget->nCurrentPos = nPosition;

	_vp_play_progressbar_value_update(pProgressWidget);

	return TRUE;
}

bool vp_play_progressbar_set_buffering_position(progressbar_handle
        pWidgetHandle,
        int nBufferinPosition)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;

	pProgressWidget->nBufferingPos = nBufferinPosition;

	_vp_play_progressbar_value_update(pProgressWidget);

	return TRUE;
}

bool vp_play_progressbar_set_landscape_mode(progressbar_handle
        pWidgetHandle,
        bool bLandscape)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;
	if (pProgressWidget->pLayout == NULL) {
		VideoLogError("pProgressWidget->pLayout is NULL");
		return FALSE;
	}

	if (bLandscape) {
		elm_object_signal_emit(pProgressWidget->pLayout,
		                       VP_NORMAL_SIGNAL_PROGRESS_LANDSCAPE_MODE,
		                       "*");
	} else {
		elm_object_signal_emit(pProgressWidget->pLayout,
		                       VP_NORMAL_SIGNAL_PROGRESS_PORTRAIT_MODE,
		                       "*");
	}

	return TRUE;

}

bool vp_play_progressbar_set_opacity(progressbar_handle pWidgetHandle,
                                     bool bOpacity)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	ProgressWidget *pProgressWidget = (ProgressWidget *) pWidgetHandle;

	if (pProgressWidget->pLayout == NULL) {
		VideoLogError("pProgressWidget->pLayout is NULL");
		return FALSE;
	}

	if (bOpacity) {
		elm_object_signal_emit(pProgressWidget->pLayout,
		                       VP_NORMAL_SIGNAL_PROGRESS_OPACITY_60, "*");
	} else {
		elm_object_signal_emit(pProgressWidget->pLayout,
		                       VP_NORMAL_SIGNAL_PROGRESS_OPACITY_DEFAULT,
		                       "*");
	}

	return TRUE;

}
