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

#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-type-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-speed.h"
#include "vp-play-util.h"
#include "vp-play-button.h"

/* check temp */
#include "vp-play-log.h"

#define VP_PLAY_SPEED_MAX_VALUE		1.5
#define VP_PLAY_SPEED_MIN_VALUE		0.5


typedef struct _PlaySpeedPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pLayout;
	Evas_Object *pBox;
	Evas_Object *pSlider;
	Evas_Object *pOkButton;
	Evas_Object *pCancelButton;
	Evas_Object *pAddButton;
	Evas_Object *pDelButton;

	double fSpeedVal;

	void *pUserData;

	PlaySpeedDoneCbFunc pDoneCb;
	PlaySpeedUpdateCbFunc pUpdateCb;

} PlaySpeedPopup;



static void _vp_play_speed_popup_destroy_handle(PlaySpeedPopup *
        pPlaySpeedPopup);
static void _vp_play_speed_update_text(PlaySpeedPopup *pPlaySpeedPopup);


/* callback functions */

static void __vp_play_speed_popup_ok_btn_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pUserData;

	if (pPlaySpeedPopup->pDoneCb) {
		pPlaySpeedPopup->pDoneCb(pPlaySpeedPopup->fSpeedVal, TRUE,
		                         (void *) pPlaySpeedPopup->pUserData);
	}
}

static void __vp_play_speed_popup_cancel_btn_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pUserData;

	if (pPlaySpeedPopup->pDoneCb) {
		pPlaySpeedPopup->pDoneCb(pPlaySpeedPopup->fSpeedVal, FALSE,
		                         (void *) pPlaySpeedPopup->pUserData);
	}
}


static void __vp_play_speed_popup_key_event_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pUserData;
	if (pPlaySpeedPopup->pDoneCb) {
		pPlaySpeedPopup->pDoneCb(pPlaySpeedPopup->fSpeedVal, FALSE,
		                         (void *) pPlaySpeedPopup->pUserData);
	}

}

static void __vp_play_speed_popup_mouse_event_cb(void *pUserData,
        Evas *pEvas,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pEventInfo == NULL) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Evas_Event_Mouse_Up *ev = pEventInfo;

	if (ev->button == 3) {
		PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pUserData;
		if (pPlaySpeedPopup->pDoneCb) {
			pPlaySpeedPopup->pDoneCb(pPlaySpeedPopup->fSpeedVal, FALSE,
			                         (void *) pPlaySpeedPopup->pUserData);
		}
	}
}

static void __vp_play_speed_popup_slider_change_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pUserData;

	double fValue = elm_slider_value_get(pPlaySpeedPopup->pSlider);
	VideoLogDebug("== %f ==", fValue);

	if (fValue != pPlaySpeedPopup->fSpeedVal) {
		if (pPlaySpeedPopup->pUpdateCb) {
			pPlaySpeedPopup->fSpeedVal = fValue;
			pPlaySpeedPopup->pUpdateCb(pPlaySpeedPopup->fSpeedVal,
			                           (void *) pPlaySpeedPopup->
			                           pUserData);
			_vp_play_speed_update_text(pPlaySpeedPopup);
		}
	}

	char szValue[6] = { 0, };
	snprintf(szValue, 6, "%0.1f X", (double) pPlaySpeedPopup->fSpeedVal);
	double realValue = atof(szValue);

	if (pPlaySpeedPopup->fSpeedVal != realValue) {
		elm_slider_value_set(pPlaySpeedPopup->pSlider,
		                     (double) realValue);
		pPlaySpeedPopup->fSpeedVal = realValue;
	}

}

static void __vp_play_speed_btn_clicked_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pUserData;

	double fValue = elm_slider_value_get(pPlaySpeedPopup->pSlider);
	VideoLogDebug("== %f ==", fValue);

	if (pObj == pPlaySpeedPopup->pAddButton) {
		fValue += 0.1;
		if (fValue > (double) VP_PLAY_SPEED_MAX_VALUE) {
			fValue = (double) VP_PLAY_SPEED_MAX_VALUE;
		}
		elm_slider_value_set(pPlaySpeedPopup->pSlider, fValue);

		if (fValue != pPlaySpeedPopup->fSpeedVal) {
			if (pPlaySpeedPopup->pUpdateCb) {
				pPlaySpeedPopup->fSpeedVal = fValue;
				pPlaySpeedPopup->pUpdateCb(pPlaySpeedPopup->fSpeedVal,
				                           (void *) pPlaySpeedPopup->
				                           pUserData);
				_vp_play_speed_update_text(pPlaySpeedPopup);
			}
		}

	} else if (pObj == pPlaySpeedPopup->pDelButton) {
		fValue -= 0.1;
		if (fValue < (double) VP_PLAY_SPEED_MIN_VALUE) {
			fValue = (double) VP_PLAY_SPEED_MIN_VALUE;
		}

		elm_slider_value_set(pPlaySpeedPopup->pSlider, fValue);
		if (fValue != pPlaySpeedPopup->fSpeedVal) {
			if (pPlaySpeedPopup->pUpdateCb) {
				pPlaySpeedPopup->fSpeedVal = fValue;
				pPlaySpeedPopup->pUpdateCb(pPlaySpeedPopup->fSpeedVal,
				                           (void *) pPlaySpeedPopup->
				                           pUserData);
				_vp_play_speed_update_text(pPlaySpeedPopup);
			}
		}

	}
}



/* internal functions */
static void _vp_play_speed_popup_destroy_handle(PlaySpeedPopup *
        pPlaySpeedPopup)
{
	if (pPlaySpeedPopup == NULL) {
		VideoLogError("pPlaySpeedPopup is NULL");
		return;
	}

	VP_EVAS_DEL(pPlaySpeedPopup->pAddButton);
	VP_EVAS_DEL(pPlaySpeedPopup->pDelButton);

	VP_EVAS_DEL(pPlaySpeedPopup->pOkButton);
	VP_EVAS_DEL(pPlaySpeedPopup->pCancelButton);
	VP_EVAS_DEL(pPlaySpeedPopup->pSlider);
	VP_EVAS_DEL(pPlaySpeedPopup->pBox);
	VP_EVAS_DEL(pPlaySpeedPopup->pLayout);
	VP_EVAS_DEL(pPlaySpeedPopup->pPopup);

	VP_FREE(pPlaySpeedPopup);
}

static void _vp_play_speed_update_text(PlaySpeedPopup *pPlaySpeedPopup)
{
	if (pPlaySpeedPopup == NULL) {
		VideoLogError("pPlaySpeedPopup is NULL");
		return;
	}

	char szValue[6] = { 0, };

	snprintf(szValue, 6, "%0.1f X", (double) pPlaySpeedPopup->fSpeedVal);
	elm_object_part_text_set(pPlaySpeedPopup->pLayout, "elm.text.title",
	                         szValue);

}

static Evas_Object *_vp_play_speed_popup_create_ok_button(Evas_Object *
        pParent,
        void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;
	pObj = elm_button_add(pParent);
	elm_object_style_set(pObj, "popup_button/default");
	elm_object_domain_translatable_text_set(pObj, VP_SYS_STR_PREFIX,
	                                        VP_PLAY_STRING_COM_DONE);
	elm_object_part_content_set(pParent, "button2", pObj);
	evas_object_smart_callback_add(pObj, "clicked",
	                               __vp_play_speed_popup_ok_btn_cb,
	                               pUserData);
	evas_object_show(pObj);

	return pObj;
}


static Evas_Object *_vp_play_speed_popup_create_cancel_button(Evas_Object
        * pParent,
        void
        *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;
	pObj = elm_button_add(pParent);
	elm_object_style_set(pObj, "popup_button/default");
	elm_object_domain_translatable_text_set(pObj, VP_SYS_STR_PREFIX,
	                                        VP_PLAY_STRING_COM_CANCEL);
	elm_object_part_content_set(pParent, "button1", pObj);
	evas_object_smart_callback_add(pObj, "clicked",
	                               __vp_play_speed_popup_cancel_btn_cb,
	                               pUserData);
	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_play_speed_popup_create_slider(Evas_Object *
        pParent,
        void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}
	Evas_Object *pObj = NULL;

	pObj = elm_slider_add(pParent);
	elm_slider_indicator_show_set(pObj, EINA_FALSE);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, 0.5);

	elm_slider_min_max_set(pObj, VP_PLAY_SPEED_MIN_VALUE,
	                       VP_PLAY_SPEED_MAX_VALUE);
	evas_object_smart_callback_add(pObj, "changed",
	                               __vp_play_speed_popup_slider_change_cb,
	                               pUserData);

	return pObj;
}

static bool _vp_play_speed_popup_create_button(Evas_Object *pParent,
        void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pUserData;

	pPlaySpeedPopup->pDelButton =
	    vp_button_create(pParent, "playview/custom/flat_63_78/default",
	                     NULL,
	                     (Evas_Smart_Cb) __vp_play_speed_btn_clicked_cb,
	                     (Evas_Smart_Cb) NULL, (Evas_Smart_Cb) NULL,
	                     (void *) pPlaySpeedPopup);



	pPlaySpeedPopup->pAddButton =
	    vp_button_create(pParent, "playview/custom/flat_63_78/default",
	                     NULL,
	                     (Evas_Smart_Cb) __vp_play_speed_btn_clicked_cb,
	                     (Evas_Smart_Cb) NULL, (Evas_Smart_Cb) NULL,
	                     (void *) pPlaySpeedPopup);

	Evas_Object *pIcon = NULL;

	pIcon =
	    vp_button_create_icon(pPlaySpeedPopup->pDelButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_DEL);
	elm_object_part_content_set(pPlaySpeedPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pPlaySpeedPopup->pDelButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_DEL_PRESS);
	elm_object_part_content_set(pPlaySpeedPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pPlaySpeedPopup->pDelButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_DEL);
	elm_object_part_content_set(pPlaySpeedPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pPlaySpeedPopup->pAddButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_ADD);
	elm_object_part_content_set(pPlaySpeedPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pPlaySpeedPopup->pAddButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_ADD_PRESS);
	elm_object_part_content_set(pPlaySpeedPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pPlaySpeedPopup->pAddButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_ADD);
	elm_object_part_content_set(pPlaySpeedPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

	elm_object_part_content_set(pParent, "elm.swallow.icon.1",
	                            pPlaySpeedPopup->pDelButton);
	elm_object_part_content_set(pParent, "elm.swallow.icon.2",
	                            pPlaySpeedPopup->pAddButton);

	return TRUE;
}

static Evas_Object *_vp_play_speed_popup_create_layout(Evas_Object *
        pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pLayout = elm_layout_add(pParent);
	if (pLayout == NULL) {
		VideoLogError("pLayout object is NULL");
		return NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VP_PLAY_SPEED_POPUP_EDJ);
	free(path);
	elm_layout_file_set(pLayout, edj_path,
	                    VP_PLAY_EDJ_GROUP_PLAY_SPEED_POPUP);

	evas_object_size_hint_weight_set(pLayout, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pLayout, EVAS_HINT_FILL,
	                                EVAS_HINT_FILL);

	evas_object_show(pLayout);

	return pLayout;

}



/* external functions */
play_speed_popup vp_play_speed_popup_create(Evas_Object *pParent,
        double fSpeedVal,
        PlaySpeedUpdateCbFunc
        pUpdateCb,
        PlaySpeedDoneCbFunc pDoneCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	PlaySpeedPopup *pPlaySpeedPopup = NULL;

	pPlaySpeedPopup = calloc(1, sizeof(PlaySpeedPopup));

	if (pPlaySpeedPopup == NULL) {
		VideoLogError("pPlaySpeedPopup alloc fail");
		return NULL;
	}

	pPlaySpeedPopup->pParent = pParent;
	pPlaySpeedPopup->pUpdateCb = pUpdateCb;
	pPlaySpeedPopup->pDoneCb = pDoneCb;
	pPlaySpeedPopup->fSpeedVal = fSpeedVal;

	pPlaySpeedPopup->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_EXPAND_WITH_CANCEL_BTN,
	                    VP_PLAY_STRING_POPUP_PLAY_SPEED, NULL, 0.0, NULL,
	                    __vp_play_speed_popup_key_event_cb,
	                    __vp_play_speed_popup_mouse_event_cb,
	                    (void *) pPlaySpeedPopup);

	if (pPlaySpeedPopup->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_play_speed_popup_destroy_handle(pPlaySpeedPopup);
		return NULL;
	}

	pPlaySpeedPopup->pCancelButton =
	    _vp_play_speed_popup_create_cancel_button(pPlaySpeedPopup->pPopup,
	            (void *)
	            pPlaySpeedPopup);
	if (pPlaySpeedPopup->pCancelButton == NULL) {
		VideoLogError("_vp_play_speed_popup_create_cancel_button fail");
		_vp_play_speed_popup_destroy_handle(pPlaySpeedPopup);
		return NULL;
	}

	pPlaySpeedPopup->pOkButton =
	    _vp_play_speed_popup_create_ok_button(pPlaySpeedPopup->pPopup,
	            (void *) pPlaySpeedPopup);
	if (pPlaySpeedPopup->pOkButton == NULL) {
		VideoLogError("_vp_play_speed_popup_create_ok_button fail");
		_vp_play_speed_popup_destroy_handle(pPlaySpeedPopup);
		return NULL;
	}

	pPlaySpeedPopup->pLayout =
	    _vp_play_speed_popup_create_layout(pPlaySpeedPopup->pPopup);
	if (pPlaySpeedPopup->pLayout == NULL) {
		VideoLogError("_vp_play_speed_popup_create_layout fail");
		_vp_play_speed_popup_destroy_handle(pPlaySpeedPopup);
		return NULL;
	}

	pPlaySpeedPopup->pSlider =
	    _vp_play_speed_popup_create_slider(pPlaySpeedPopup->pLayout,
	                                       pPlaySpeedPopup);
	if (pPlaySpeedPopup->pSlider == NULL) {
		VideoLogError("_vp_play_speed_popup_create_slider fail");
		_vp_play_speed_popup_destroy_handle(pPlaySpeedPopup);
		return NULL;
	}

	_vp_play_speed_popup_create_button(pPlaySpeedPopup->pLayout,
	                                   pPlaySpeedPopup);

	elm_slider_value_set(pPlaySpeedPopup->pSlider,
	                     (double) pPlaySpeedPopup->fSpeedVal);
	_vp_play_speed_update_text(pPlaySpeedPopup);

	elm_object_part_content_set(pPlaySpeedPopup->pLayout,
	                            "elm.swallow.content",
	                            pPlaySpeedPopup->pSlider);
	elm_object_content_set(pPlaySpeedPopup->pPopup,
	                       pPlaySpeedPopup->pLayout);
	evas_object_show(pPlaySpeedPopup->pLayout);

//      elm_popup_orient_set(pPlaySpeedPopup->pPopup, ELM_POPUP_ORIENT_BOTTOM);

	return pPlaySpeedPopup;
}

void vp_play_speed_popup_destroy(play_speed_popup pPlaySpeedHandle)
{
	if (pPlaySpeedHandle == NULL) {
		VideoLogError("pPlaySpeedHandle is NULL");
		return;
	}

	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pPlaySpeedHandle;

	_vp_play_speed_popup_destroy_handle(pPlaySpeedPopup);

}

bool vp_play_speed_popup_realize(play_speed_popup pPlaySpeedHandle)
{
	if (pPlaySpeedHandle == NULL) {
		VideoLogError("pPlaySpeedHandle is NULL");
		return FALSE;
	}

	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pPlaySpeedHandle;

	evas_object_show(pPlaySpeedPopup->pPopup);

	return TRUE;
}


bool vp_play_speed_popup_unrealize(play_speed_popup pPlaySpeedHandle)
{
	if (pPlaySpeedHandle == NULL) {
		VideoLogError("pPlaySpeedHandle is NULL");
		return FALSE;
	}

	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pPlaySpeedHandle;

	evas_object_hide(pPlaySpeedPopup->pPopup);

	return TRUE;
}



bool vp_play_speed_popup_set_user_data(play_speed_popup pPlaySpeedHandle,
                                       void *pUserData)
{
	if (pPlaySpeedHandle == NULL) {
		VideoLogError("pPlaySpeedHandle is NULL");
		return FALSE;
	}

	PlaySpeedPopup *pPlaySpeedPopup = (PlaySpeedPopup *) pPlaySpeedHandle;

	pPlaySpeedPopup->pUserData = pUserData;

	return TRUE;
}
