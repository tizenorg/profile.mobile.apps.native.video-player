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

#include "vp-subtitle-sync.h"
#include "vp-play-util.h"
#include "vp-play-button.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_SYNC_MAX_VALUE		30.0
#define VP_SUBTITLE_SYNC_MIN_VALUE		-30.0

typedef struct _SubtitleSyncPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pLayout;
	Evas_Object *pBox;
	Evas_Object *pSlider;
	Evas_Object *pOkButton;
	Evas_Object *pCancelButton;
	Evas_Object *pAddButton;
	Evas_Object *pDelButton;

	float fSyncVal;

	void *pUserData;

	SubtitleSyncDoneCbFunc pDoneCb;
	SubtitleSyncUpdateCbFunc pUpdateCb;

} SubtitleSyncPopup;



static void _vp_subtitle_sync_popup_destroy_handle(SubtitleSyncPopup *
        pSubtitleSyncPopup);
static void _vp_subtitle_sync_update_text(SubtitleSyncPopup *
        pSubtitleSyncPopup);


/* callback functions */

static void __vp_subtitle_sync_popup_ok_btn_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pUserData;

	if (pSubtitleSyncPopup->pDoneCb) {
		pSubtitleSyncPopup->pDoneCb(pSubtitleSyncPopup->fSyncVal, TRUE,
		                            (void *) pSubtitleSyncPopup->
		                            pUserData);
	}
}

static void __vp_subtitle_sync_popup_cancel_btn_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pUserData;

	if (pSubtitleSyncPopup->pDoneCb) {
		pSubtitleSyncPopup->pDoneCb(pSubtitleSyncPopup->fSyncVal, FALSE,
		                            (void *) pSubtitleSyncPopup->
		                            pUserData);
	}
}


static void __vp_subtitle_sync_popup_key_event_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pUserData;

	if (pSubtitleSyncPopup->pDoneCb) {
		pSubtitleSyncPopup->pDoneCb(pSubtitleSyncPopup->fSyncVal, FALSE,
		                            (void *) pSubtitleSyncPopup->
		                            pUserData);
	}

}

static void __vp_subtitle_sync_popup_mouse_event_cb(void *pUserData,
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
		SubtitleSyncPopup *pSubtitleSyncPopup =
		    (SubtitleSyncPopup *) pUserData;
		if (pSubtitleSyncPopup->pDoneCb) {
			pSubtitleSyncPopup->pDoneCb(pSubtitleSyncPopup->fSyncVal,
			                            FALSE,
			                            (void *) pSubtitleSyncPopup->
			                            pUserData);
		}

	}
}

static void __vp_subtitle_sync_popup_slider_change_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pUserData;

	float fValue =
	    (float) elm_slider_value_get(pSubtitleSyncPopup->pSlider);

	if (fValue != pSubtitleSyncPopup->fSyncVal) {
		if (pSubtitleSyncPopup->pUpdateCb) {
			pSubtitleSyncPopup->pUpdateCb(pSubtitleSyncPopup->fSyncVal,
			                              (void *) pSubtitleSyncPopup->
			                              pUserData);
			pSubtitleSyncPopup->fSyncVal = fValue;
			_vp_subtitle_sync_update_text(pSubtitleSyncPopup);
		}
	}
}

static void __vp_subtitle_sync_btn_clicked_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pUserData;

	float fValue =
	    (float) elm_slider_value_get(pSubtitleSyncPopup->pSlider);
	VideoLogWarning("== %f ==", fValue);

	if (pObj == pSubtitleSyncPopup->pAddButton) {
		fValue += 0.2;
		if (fValue <= VP_SUBTITLE_SYNC_MAX_VALUE) {
			VideoLogWarning("== %f ==", fValue);
			elm_slider_value_set(pSubtitleSyncPopup->pSlider,
			                     (double) fValue);
			if (pSubtitleSyncPopup->pUpdateCb) {
				pSubtitleSyncPopup->pUpdateCb(pSubtitleSyncPopup->
				                              fSyncVal,
				                              (void *)
				                              pSubtitleSyncPopup->
				                              pUserData);
				pSubtitleSyncPopup->fSyncVal = fValue;
				_vp_subtitle_sync_update_text(pSubtitleSyncPopup);
			}
		}
	} else if (pObj == pSubtitleSyncPopup->pDelButton) {
		fValue -= 0.2;
		if (fValue >= VP_SUBTITLE_SYNC_MIN_VALUE) {
			VideoLogWarning("== %f ==", fValue);
			elm_slider_value_set(pSubtitleSyncPopup->pSlider,
			                     (double) fValue);
			if (pSubtitleSyncPopup->pUpdateCb) {
				pSubtitleSyncPopup->pUpdateCb(pSubtitleSyncPopup->
				                              fSyncVal,
				                              (void *)
				                              pSubtitleSyncPopup->
				                              pUserData);
				pSubtitleSyncPopup->fSyncVal = fValue;
				_vp_subtitle_sync_update_text(pSubtitleSyncPopup);
			}
		}
	}
}



/* internal functions */
static void _vp_subtitle_sync_popup_destroy_handle(SubtitleSyncPopup *
        pSubtitleSyncPopup)
{
	if (pSubtitleSyncPopup == NULL) {
		VideoLogError("pSubtitleSyncPopup is NULL");
		return;
	}

	VP_EVAS_DEL(pSubtitleSyncPopup->pAddButton);
	VP_EVAS_DEL(pSubtitleSyncPopup->pDelButton);

	VP_EVAS_DEL(pSubtitleSyncPopup->pOkButton);
	VP_EVAS_DEL(pSubtitleSyncPopup->pCancelButton);
	VP_EVAS_DEL(pSubtitleSyncPopup->pSlider);
	VP_EVAS_DEL(pSubtitleSyncPopup->pBox);
	VP_EVAS_DEL(pSubtitleSyncPopup->pLayout);
	VP_EVAS_DEL(pSubtitleSyncPopup->pPopup);

	VP_FREE(pSubtitleSyncPopup);
}

static void _vp_subtitle_sync_update_text(SubtitleSyncPopup *
        pSubtitleSyncPopup)
{
	if (pSubtitleSyncPopup == NULL) {
		VideoLogError("pSubtitleSyncPopup is NULL");
		return;
	}

	char szValue[6] = { 0, };

	snprintf(szValue, 6, "%0.1f", (double) pSubtitleSyncPopup->fSyncVal);
	elm_object_part_text_set(pSubtitleSyncPopup->pLayout,
	                         "elm.text.title", szValue);

}


static Evas_Object *_vp_subtitle_sync_popup_create_ok_button(Evas_Object *
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
	                                        VP_PLAY_STRING_COM_OK_IDS);
	elm_object_part_content_set(pParent, "button2", pObj);
	evas_object_smart_callback_add(pObj, "clicked",
	                               __vp_subtitle_sync_popup_ok_btn_cb,
	                               pUserData);
	evas_object_show(pObj);

	return pObj;
}


static Evas_Object
*_vp_subtitle_sync_popup_create_cancel_button(Evas_Object *pParent,
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
	                                        VP_PLAY_STRING_COM_CANCEL_IDS);
	elm_object_part_content_set(pParent, "button1", pObj);
	evas_object_smart_callback_add(pObj, "clicked",
	                               __vp_subtitle_sync_popup_cancel_btn_cb,
	                               pUserData);
	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_subtitle_sync_popup_create_slider(Evas_Object *
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
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_slider_min_max_set(pObj, VP_SUBTITLE_SYNC_MIN_VALUE,
	                       VP_SUBTITLE_SYNC_MAX_VALUE);
	evas_object_smart_callback_add(pObj, "changed",
	                               __vp_subtitle_sync_popup_slider_change_cb,
	                               pUserData);

	return pObj;
}

static bool _vp_subtitle_sync_popup_create_button(Evas_Object *pParent,
        void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_RESROUCE_EDJ_PATH);
	free(path);
	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pUserData;

	pSubtitleSyncPopup->pDelButton =
	    vp_button_create(pParent, "playview/custom/flat_63_78/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_subtitle_sync_btn_clicked_cb,
	                     (Evas_Smart_Cb) NULL, (Evas_Smart_Cb) NULL,
	                     (void *) pSubtitleSyncPopup);



	pSubtitleSyncPopup->pAddButton =
	    vp_button_create(pParent, "playview/custom/flat_63_78/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_subtitle_sync_btn_clicked_cb,
	                     (Evas_Smart_Cb) NULL, (Evas_Smart_Cb) NULL,
	                     (void *) pSubtitleSyncPopup);

	Evas_Object *pIcon = NULL;

	pIcon =
	    vp_button_create_icon(pSubtitleSyncPopup->pDelButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_DEL);
	elm_object_part_content_set(pSubtitleSyncPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleSyncPopup->pDelButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_DEL_PRESS);
	elm_object_part_content_set(pSubtitleSyncPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleSyncPopup->pDelButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_DEL);
	elm_object_part_content_set(pSubtitleSyncPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleSyncPopup->pAddButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_ADD);
	elm_object_part_content_set(pSubtitleSyncPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleSyncPopup->pAddButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_ADD_PRESS);
	elm_object_part_content_set(pSubtitleSyncPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleSyncPopup->pAddButton,
	                          edj_path,
	                          VP_PLAY_RES_BUTTON_ADD);
	elm_object_part_content_set(pSubtitleSyncPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

	elm_object_part_content_set(pParent, "elm.swallow.icon.1",
	                            pSubtitleSyncPopup->pDelButton);
	elm_object_part_content_set(pParent, "elm.swallow.icon.2",
	                            pSubtitleSyncPopup->pAddButton);

	return TRUE;
}

static Evas_Object *_vp_subtitle_sync_popup_create_layout(Evas_Object *
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
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VP_PLAY_SPEED_POPUP_EDJ);
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
subtitle_sync_popup vp_subtitle_sync_popup_create(Evas_Object *pParent,
        float fSyncVal,
        SubtitleSyncUpdateCbFunc
        pUpdateCb,
        SubtitleSyncDoneCbFunc
        pDoneCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup = NULL;

	pSubtitleSyncPopup = calloc(1, sizeof(SubtitleSyncPopup));

	if (pSubtitleSyncPopup == NULL) {
		VideoLogError("pSubtitleSyncPopup alloc fail");
		return NULL;
	}

	pSubtitleSyncPopup->pParent = pParent;
	pSubtitleSyncPopup->pUpdateCb = pUpdateCb;
	pSubtitleSyncPopup->pDoneCb = pDoneCb;
	pSubtitleSyncPopup->fSyncVal = fSyncVal;

	pSubtitleSyncPopup->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_EXPAND_WITH_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_SYNC, NULL, 0.0, NULL,
	                    __vp_subtitle_sync_popup_key_event_cb,
	                    __vp_subtitle_sync_popup_mouse_event_cb,
	                    (void *) pSubtitleSyncPopup);

	if (pSubtitleSyncPopup->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_sync_popup_destroy_handle(pSubtitleSyncPopup);
		return NULL;
	}

	pSubtitleSyncPopup->pCancelButton =
	    _vp_subtitle_sync_popup_create_cancel_button(pSubtitleSyncPopup->
	            pPopup,
	            (void *)
	            pSubtitleSyncPopup);
	if (pSubtitleSyncPopup->pCancelButton == NULL) {
		VideoLogError
		("_vp_subtitle_sync_popup_create_cancel_button fail");
		_vp_subtitle_sync_popup_destroy_handle(pSubtitleSyncPopup);
		return NULL;
	}

	pSubtitleSyncPopup->pOkButton =
	    _vp_subtitle_sync_popup_create_ok_button(pSubtitleSyncPopup->
	            pPopup,
	            (void *)
	            pSubtitleSyncPopup);
	if (pSubtitleSyncPopup->pOkButton == NULL) {
		VideoLogError("_vp_subtitle_sync_popup_create_ok_button fail");
		_vp_subtitle_sync_popup_destroy_handle(pSubtitleSyncPopup);
		return NULL;
	}

	pSubtitleSyncPopup->pLayout =
	    _vp_subtitle_sync_popup_create_layout(pSubtitleSyncPopup->pPopup);
	if (pSubtitleSyncPopup->pLayout == NULL) {
		VideoLogError("_vp_subtitle_sync_popup_create_layout fail");
		_vp_subtitle_sync_popup_destroy_handle(pSubtitleSyncPopup);
		return NULL;
	}

	pSubtitleSyncPopup->pSlider =
	    _vp_subtitle_sync_popup_create_slider(pSubtitleSyncPopup->pLayout,
	            pSubtitleSyncPopup);
	if (pSubtitleSyncPopup->pSlider == NULL) {
		VideoLogError("_vp_subtitle_sync_popup_create_slider fail");
		_vp_subtitle_sync_popup_destroy_handle(pSubtitleSyncPopup);
		return NULL;
	}

	_vp_subtitle_sync_popup_create_button(pSubtitleSyncPopup->pLayout,
	                                      pSubtitleSyncPopup);

	elm_slider_value_set(pSubtitleSyncPopup->pSlider,
	                     (double) pSubtitleSyncPopup->fSyncVal);
	_vp_subtitle_sync_update_text(pSubtitleSyncPopup);

	elm_object_part_content_set(pSubtitleSyncPopup->pLayout,
	                            "elm.swallow.content",
	                            pSubtitleSyncPopup->pSlider);
	elm_object_content_set(pSubtitleSyncPopup->pPopup,
	                       pSubtitleSyncPopup->pLayout);
	evas_object_show(pSubtitleSyncPopup->pLayout);

	elm_popup_orient_set(pSubtitleSyncPopup->pPopup,
	                     ELM_POPUP_ORIENT_CENTER);

	return pSubtitleSyncPopup;
}

void vp_subtitle_sync_popup_destroy(subtitle_sync_popup pSubtitleSync)
{
	if (pSubtitleSync == NULL) {
		VideoLogError("pSubtitleSync is NULL");
		return;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pSubtitleSync;

	_vp_subtitle_sync_popup_destroy_handle(pSubtitleSyncPopup);

}

bool vp_subtitle_sync_popup_realize(subtitle_sync_popup pSubtitleSync)
{
	if (pSubtitleSync == NULL) {
		VideoLogError("pSubtitleSync is NULL");
		return FALSE;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pSubtitleSync;

	evas_object_show(pSubtitleSyncPopup->pPopup);

	return TRUE;
}


bool vp_subtitle_sync_popup_unrealize(subtitle_sync_popup pSubtitleSync)
{
	if (pSubtitleSync == NULL) {
		VideoLogError("pSubtitleSync is NULL");
		return FALSE;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pSubtitleSync;

	evas_object_hide(pSubtitleSyncPopup->pPopup);

	return TRUE;
}



bool vp_subtitle_sync_popup_set_user_data(subtitle_sync_popup
        pSubtitleSync, void *pUserData)
{
	if (pSubtitleSync == NULL) {
		VideoLogError("pSubtitleSync is NULL");
		return FALSE;
	}

	SubtitleSyncPopup *pSubtitleSyncPopup =
	    (SubtitleSyncPopup *) pSubtitleSync;

	pSubtitleSyncPopup->pUserData = pUserData;

	return TRUE;
}
