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

#include "vp-subtitle-alignment.h"
#include "vp-play-button.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_ALIGNMENT_GENLIST_DATA_KEY		"vp.subtitle.alignment.genlist"

typedef struct _SubtitleAlignmentPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_SubtitleAlignment_Itc;

	int nDefaultIndex;
	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;
	video_play_subtitle_alignment_t currentAlignment;
} SubtitleAlignmentPopup;

static void _vp_subtitle_alignment_destroy_handle(SubtitleAlignmentPopup *
        pSubtitleAlignment);

static void __vp_subtitle_alignment_genlist_item_selected_cb(void
        *pUserData,
        Evas_Object *
        pObject,
        void
        *pEventInfo);

static void __vp_subtitle_alignment_genlist_realized(void *data,
        Evas_Object *obj,
        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_alignment_popup_rotate_cb(void *data,
        Evas_Object *obj,
        void *event_info)
{
	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) data;
	if (!pSubtitleAlignment) {
		VideoLogError("pSubtitleAlignment IS null");
		return;
	}
	if (pSubtitleAlignment->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleAlignment->pParent,
	                            pSubtitleAlignment->pBox,
	                            pSubtitleAlignment->nListCount,
	                            VIDEO_POPUP_DEFAULT);

}

/* callback functions */
static char *__vp_subtitle_alignment_genlist_text_get_cb(const void
        *pUserData,
        Evas_Object *
        pObj,
        const char
        *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!strcmp(pPart, "elm.text.main.left")) {
		return strdup(szTxt);
	}

	return NULL;
}

static Evas_Object *__vp_subtitle_alignment_genlist_content_get_cb(const
        void
        *pUserData,
        Evas_Object
        * pObj,
        const
        char
        *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!strcmp(pPart, "elm.icon.right")) {
		Evas_Object *pRadioObj = NULL;

		SubtitleAlignmentPopup *pSubtitleAlignment =
		    (SubtitleAlignmentPopup *) evas_object_data_get(pObj,
		            VP_SUBTITLE_ALIGNMENT_GENLIST_DATA_KEY);
		if (pSubtitleAlignment == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}
		video_play_subtitle_alignment_t nAlignment =
		    VIDEO_SUBTITLE_ALIGNMENT_CENTER;
		if (!strcmp(szTxt, VP_PLAY_STRING_ALIGNMENT_LEFT)) {
			nAlignment = VIDEO_SUBTITLE_ALIGNMENT_LEFT;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_ALIGNMENT_CENTER)) {
			nAlignment = VIDEO_SUBTITLE_ALIGNMENT_CENTER;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_ALIGNMENT_RIGHT)) {
			nAlignment = VIDEO_SUBTITLE_ALIGNMENT_RIGHT;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nAlignment);
		elm_radio_group_add(pRadioObj, pSubtitleAlignment->pRadio);
		elm_radio_value_set(pSubtitleAlignment->pRadio,
		                    pSubtitleAlignment->currentAlignment);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_subtitle_alignment_genlist_item_selected_cb,
		                               pSubtitleAlignment);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_subtitle_alignment_genlist_item_selected_cb(void
        *pUserData,
        Evas_Object *
        pObject,
        void
        *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	if (pEventInfo == NULL) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;
	Elm_Object_Item *pSelectedItem =
	    elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	char *szTxt = (char *) elm_object_item_data_get(pItem);

	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	video_play_subtitle_alignment_t nAlignment =
	    VIDEO_SUBTITLE_ALIGNMENT_CENTER;
	if (!strcmp(szTxt, VP_PLAY_STRING_ALIGNMENT_LEFT)) {
		nAlignment = VIDEO_SUBTITLE_ALIGNMENT_LEFT;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_ALIGNMENT_CENTER)) {
		nAlignment = VIDEO_SUBTITLE_ALIGNMENT_CENTER;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_ALIGNMENT_RIGHT)) {
		nAlignment = VIDEO_SUBTITLE_ALIGNMENT_RIGHT;
	}

	pSubtitleAlignment->currentAlignment = nAlignment;
	if (pSubtitleAlignment->pCloseCb) {
		pSubtitleAlignment->pCloseCb(nAlignment, FALSE,
		                             (void *) pSubtitleAlignment->
		                             pUserData);
	}
}

static void __vp_subtitle_alignment_popup_key_event_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) pUserData;

	if (pSubtitleAlignment->pCloseCb) {
		pSubtitleAlignment->pCloseCb(-1, FALSE,
		                             (void *) pSubtitleAlignment->
		                             pUserData);
	}
}

static void __vp_subtitle_alignment_popup_mouse_event_cb(void *pUserData,
        Evas *pEvas,
        Evas_Object *
        pObj,
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
		SubtitleAlignmentPopup *pSubtitleAlignment =
		    (SubtitleAlignmentPopup *) pUserData;

		if (pSubtitleAlignment->pCloseCb) {
			pSubtitleAlignment->pCloseCb(-1, FALSE,
			                             (void *) pSubtitleAlignment->
			                             pUserData);
		}
	}
}



/* internal functions */
static void _vp_subtitle_alignment_destroy_handle(SubtitleAlignmentPopup *
        pSubtitleAlignment)
{
	if (pSubtitleAlignment == NULL) {
		VideoLogError("pSubtitleAlignment is NULL");
		return;
	}

	evas_object_smart_callback_del(pSubtitleAlignment->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_alignment_popup_rotate_cb);
	evas_object_smart_callback_del(pSubtitleAlignment->pGenList,
	                               "realized",
	                               __vp_subtitle_alignment_genlist_realized);

	VP_EVAS_DEL(pSubtitleAlignment->pRadio);

	VP_EVAS_DEL(pSubtitleAlignment->pGenList);
	VP_EVAS_DEL(pSubtitleAlignment->pButton);
	VP_EVAS_DEL(pSubtitleAlignment->pBox);

	if (pSubtitleAlignment->st_SubtitleAlignment_Itc) {
		elm_genlist_item_class_free(pSubtitleAlignment->
		                            st_SubtitleAlignment_Itc);
		pSubtitleAlignment->st_SubtitleAlignment_Itc = NULL;
	}

	VP_EVAS_DEL(pSubtitleAlignment->pPopup);

	VP_FREE(pSubtitleAlignment);
}

static Evas_Object *_vp_subtitle_alignment_create_genlist(Evas_Object *
        pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_genlist_add(pParent);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(pObj);
	return pObj;
}


static bool _vp_subtitle_alignment_add_genlist_item(Evas_Object *pObj,
        void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) pUserData;

	if (pSubtitleAlignment->st_SubtitleAlignment_Itc) {
		elm_genlist_item_class_free(pSubtitleAlignment->
		                            st_SubtitleAlignment_Itc);
		pSubtitleAlignment->st_SubtitleAlignment_Itc = NULL;
	}

	pSubtitleAlignment->st_SubtitleAlignment_Itc =
	    elm_genlist_item_class_new();

	if (pSubtitleAlignment->st_SubtitleAlignment_Itc != NULL) {
		pSubtitleAlignment->st_SubtitleAlignment_Itc->version =
		    ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitleAlignment->st_SubtitleAlignment_Itc->item_style = "1line";
		pSubtitleAlignment->st_SubtitleAlignment_Itc->func.text_get =
		    (void *) __vp_subtitle_alignment_genlist_text_get_cb;
		pSubtitleAlignment->st_SubtitleAlignment_Itc->func.content_get =
		    (void *) __vp_subtitle_alignment_genlist_content_get_cb;
		pSubtitleAlignment->st_SubtitleAlignment_Itc->func.state_get = NULL;
		pSubtitleAlignment->st_SubtitleAlignment_Itc->func.del = NULL;
		pSubtitleAlignment->nListCount = 0;

		elm_genlist_item_append(pObj,
		                        pSubtitleAlignment->st_SubtitleAlignment_Itc,
		                        (void *) VP_PLAY_STRING_ALIGNMENT_LEFT, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_alignment_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleAlignment->nListCount++;
		elm_genlist_item_append(pObj,
		                        pSubtitleAlignment->st_SubtitleAlignment_Itc,
		                        (void *) VP_PLAY_STRING_ALIGNMENT_CENTER,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_alignment_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleAlignment->nListCount++;
		elm_genlist_item_append(pObj,
		                        pSubtitleAlignment->st_SubtitleAlignment_Itc,
		                        (void *) VP_PLAY_STRING_ALIGNMENT_RIGHT, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_alignment_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleAlignment->nListCount++;
	}

	return TRUE;
}

/* external functions */
subtitle_alignment_handle vp_subtitle_alignment_create(Evas_Object *
        pParent,
        PopupCloseCbFunc
        pCloseCb,
        int nDefaultAlign)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleAlignmentPopup *pSubtitleAlignment = NULL;

	pSubtitleAlignment = calloc(1, sizeof(SubtitleAlignmentPopup));

	if (pSubtitleAlignment == NULL) {
		VideoLogError("pSubtitleAlignment alloc fail");
		return NULL;
	}

	pSubtitleAlignment->pParent = pParent;
	pSubtitleAlignment->pCloseCb = pCloseCb;

	pSubtitleAlignment->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_ALIGNMENT, NULL, 0.0,
	                    NULL, __vp_subtitle_alignment_popup_key_event_cb,
	                    __vp_subtitle_alignment_popup_mouse_event_cb,
	                    (void *) pSubtitleAlignment);

	if (pSubtitleAlignment->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_alignment_destroy_handle(pSubtitleAlignment);
		return NULL;
	}
	pSubtitleAlignment->pGenList =
	    _vp_subtitle_alignment_create_genlist(pSubtitleAlignment->pPopup);
	if (pSubtitleAlignment->pGenList == NULL) {
		VideoLogError("_vp_subtitle_alignment_create_genlist fail");
		_vp_subtitle_alignment_destroy_handle(pSubtitleAlignment);
		return NULL;
	}
	evas_object_data_set(pSubtitleAlignment->pGenList,
	                     VP_SUBTITLE_ALIGNMENT_GENLIST_DATA_KEY,
	                     (void *) pSubtitleAlignment);
	evas_object_smart_callback_add(pSubtitleAlignment->pGenList,
	                               "realized",
	                               __vp_subtitle_alignment_genlist_realized,
	                               NULL);

	pSubtitleAlignment->nDefaultIndex = nDefaultAlign;
	pSubtitleAlignment->pRadio =
	    elm_radio_add(pSubtitleAlignment->pGenList);

	if (!_vp_subtitle_alignment_add_genlist_item
	        (pSubtitleAlignment->pGenList, (void *) pSubtitleAlignment)) {
		VideoLogError("_vp_subtitle_alignment_add_genlist_item fail");
		return FALSE;
	}
	//elm_radio_value_set(pSubtitleAlignment->pRadio, nDefaultAlign);
	pSubtitleAlignment->currentAlignment = nDefaultAlign;
	evas_object_smart_callback_add(pSubtitleAlignment->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_alignment_popup_rotate_cb,
	                               pSubtitleAlignment);

	pSubtitleAlignment->pBox = elm_box_add(pSubtitleAlignment->pPopup);
	vp_popup_set_popup_min_size(pSubtitleAlignment->pParent,
	                            pSubtitleAlignment->pBox,
	                            pSubtitleAlignment->nListCount,
	                            VIDEO_POPUP_DEFAULT);
	elm_box_pack_end(pSubtitleAlignment->pBox,
	                 pSubtitleAlignment->pGenList);
	elm_object_content_set(pSubtitleAlignment->pPopup,
	                       pSubtitleAlignment->pBox);

	return pSubtitleAlignment;
}

void vp_subtitle_alignment_destroy(subtitle_alignment_handle
                                   pSubtitleAlignmentHandle)
{
	if (pSubtitleAlignmentHandle == NULL) {
		VideoLogError("pSubtitleAlignmentHandle is NULL");
		return;
	}

	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) pSubtitleAlignmentHandle;

	_vp_subtitle_alignment_destroy_handle(pSubtitleAlignment);

}

bool vp_subtitle_alignment_realize(subtitle_alignment_handle
                                   pSubtitleAlignmentHandle)
{
	if (pSubtitleAlignmentHandle == NULL) {
		VideoLogError("pSubtitleAlignmentHandle is NULL");
		return FALSE;
	}

	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) pSubtitleAlignmentHandle;

	evas_object_show(pSubtitleAlignment->pPopup);

	return TRUE;
}

bool vp_subtitle_alignment_unrealize(subtitle_alignment_handle
                                     pSubtitleAlignmentHandle)
{
	if (pSubtitleAlignmentHandle == NULL) {
		VideoLogError("pSubtitleAlignmentHandle is NULL");
		return FALSE;
	}

	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) pSubtitleAlignmentHandle;

	evas_object_hide(pSubtitleAlignment->pPopup);

	return TRUE;
}

bool vp_subtitle_alignment_set_user_data(subtitle_alignment_handle
        pSubtitleAlignmentHandle,
        void *pUserData)
{
	if (pSubtitleAlignmentHandle == NULL) {
		VideoLogError("pSubtitleAlignmentHandle is NULL");
		return FALSE;
	}

	SubtitleAlignmentPopup *pSubtitleAlignment =
	    (SubtitleAlignmentPopup *) pSubtitleAlignmentHandle;

	pSubtitleAlignment->pUserData = pUserData;

	return TRUE;
}
