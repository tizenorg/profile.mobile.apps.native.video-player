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

#include "vp-subtitle-bg-color.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_BG_COLOR_GENLIST_DATA_KEY		"vp.subtitle.bg.color.genlist"


typedef struct _SubtitleBGColorPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_SubtitleBGColor_Itc;

	int nDefaultIndex;
	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;
	video_play_subtitle_color_t currentBgColor;
} SubtitleBGColorPopup;



static void _vp_subtitle_bg_color_destroy_handle(SubtitleBGColorPopup *
        pSubtitleBGColor);
static void __vp_subtitle_bg_color_genlist_item_selected_cb(void
        *pUserData,
        Evas_Object *
        pObject,
        void
        *pEventInfo);


static void __vp_subtitle_bg_color_genlist_realized(void *data,
        Evas_Object *obj,
        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_bg_color_popup_rotate_cb(void *data,
        Evas_Object *obj,
        void *event_info)
{
	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) data;
	if (!pSubtitleBGColor) {
		VideoLogError("pSubtitleBGColor IS null");
		return;
	}
	if (pSubtitleBGColor->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleBGColor->pParent,
	                            pSubtitleBGColor->pBox,
	                            pSubtitleBGColor->nListCount,
	                            VIDEO_POPUP_DEFAULT);

}


/* callback functions */
static char *__vp_subtitle_bg_color_genlist_text_get_cb(const void
        *pUserData,
        Evas_Object *
        pObj,
        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!strcmp(pPart, "elm.text")) {
		return strdup(szTxt);
	}

	return NULL;
}

static Evas_Object *__vp_subtitle_bg_color_genlist_content_get_cb(const
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

	if (!strcmp(pPart, "elm.icon")) {
		Evas_Object *pRadioObj = NULL;

		SubtitleBGColorPopup *pSubtitleBGColor =
		    (SubtitleBGColorPopup *) evas_object_data_get(pObj,
		            VP_SUBTITLE_BG_COLOR_GENLIST_DATA_KEY);
		if (pSubtitleBGColor == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}
		video_play_subtitle_color_t nColor = VIDEO_SUBTITLE_COLOR_NONE;
		if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_NONE)) {
			nColor = VIDEO_SUBTITLE_COLOR_NONE;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_BLACK)) {
			nColor = VIDEO_SUBTITLE_COLOR_BLACK;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_WHITE)) {
			nColor = VIDEO_SUBTITLE_COLOR_WHITE;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nColor);
		elm_radio_group_add(pRadioObj, pSubtitleBGColor->pRadio);
		elm_radio_value_set(pSubtitleBGColor->pRadio,
		                    pSubtitleBGColor->currentBgColor);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_subtitle_bg_color_genlist_item_selected_cb,
		                               pSubtitleBGColor);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_subtitle_bg_color_genlist_item_selected_cb(void
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

	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	video_play_subtitle_color_t nColor = VIDEO_SUBTITLE_COLOR_NONE;
	if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_NONE)) {
		nColor = VIDEO_SUBTITLE_COLOR_NONE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_BLACK)) {
		nColor = VIDEO_SUBTITLE_COLOR_BLACK;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_WHITE)) {
		nColor = VIDEO_SUBTITLE_COLOR_WHITE;
	}

	pSubtitleBGColor->currentBgColor = nColor;
	if (pSubtitleBGColor->pCloseCb) {
		pSubtitleBGColor->pCloseCb(nColor, FALSE,
		                           (void *) pSubtitleBGColor->pUserData);
	}
}

static void __vp_subtitle_bg_color_popup_key_event_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) pUserData;

	if (pSubtitleBGColor->pCloseCb) {
		pSubtitleBGColor->pCloseCb(-1, FALSE,
		                           (void *) pSubtitleBGColor->pUserData);
	}
}

static void __vp_subtitle_bg_color_popup_mouse_event_cb(void *pUserData,
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
		SubtitleBGColorPopup *pSubtitleBGColor =
		    (SubtitleBGColorPopup *) pUserData;

		if (pSubtitleBGColor->pCloseCb) {
			pSubtitleBGColor->pCloseCb(-1, FALSE,
			                           (void *) pSubtitleBGColor->
			                           pUserData);
		}
	}
}



/* internal functions */
static void _vp_subtitle_bg_color_destroy_handle(SubtitleBGColorPopup *
        pSubtitleBGColor)
{
	if (pSubtitleBGColor == NULL) {
		VideoLogError("pSubtitleBGColor is NULL");
		return;
	}

	evas_object_smart_callback_del(pSubtitleBGColor->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_bg_color_popup_rotate_cb);
	evas_object_smart_callback_del(pSubtitleBGColor->pGenList, "realized",
	                               __vp_subtitle_bg_color_genlist_realized);

	VP_EVAS_DEL(pSubtitleBGColor->pRadio);
	VP_EVAS_DEL(pSubtitleBGColor->pGenList);
	VP_EVAS_DEL(pSubtitleBGColor->pButton);
	VP_EVAS_DEL(pSubtitleBGColor->pBox);

	if (pSubtitleBGColor->st_SubtitleBGColor_Itc) {
		elm_genlist_item_class_free(pSubtitleBGColor->
		                            st_SubtitleBGColor_Itc);
		pSubtitleBGColor->st_SubtitleBGColor_Itc = NULL;
	}

	VP_EVAS_DEL(pSubtitleBGColor->pPopup);

	VP_FREE(pSubtitleBGColor);
}


static Evas_Object *_vp_subtitle_bg_color_create_genlist(Evas_Object *
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


static bool _vp_subtitle_bg_color_add_genlist_item(Evas_Object *pObj,
        void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) pUserData;

	if (pSubtitleBGColor->st_SubtitleBGColor_Itc) {
		elm_genlist_item_class_free(pSubtitleBGColor->
		                            st_SubtitleBGColor_Itc);
		pSubtitleBGColor->st_SubtitleBGColor_Itc = NULL;
	}

	pSubtitleBGColor->st_SubtitleBGColor_Itc =
	    elm_genlist_item_class_new();

	if (pSubtitleBGColor->st_SubtitleBGColor_Itc != NULL) {
		pSubtitleBGColor->st_SubtitleBGColor_Itc->version =
		    ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitleBGColor->st_SubtitleBGColor_Itc->item_style =
		    "1text.1icon.3/popup";
		pSubtitleBGColor->st_SubtitleBGColor_Itc->func.text_get =
		    (void *) __vp_subtitle_bg_color_genlist_text_get_cb;
		pSubtitleBGColor->st_SubtitleBGColor_Itc->func.content_get =
		    (void *) __vp_subtitle_bg_color_genlist_content_get_cb;
		pSubtitleBGColor->st_SubtitleBGColor_Itc->func.state_get = NULL;
		pSubtitleBGColor->st_SubtitleBGColor_Itc->func.del = NULL;
		pSubtitleBGColor->nListCount = 0;

		elm_genlist_item_append(pObj,
		                        pSubtitleBGColor->st_SubtitleBGColor_Itc,
		                        (void *) VP_PLAY_STRING_SUBTITLE_COLOR_NONE,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_bg_color_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleBGColor->nListCount++;
		elm_genlist_item_append(pObj,
		                        pSubtitleBGColor->st_SubtitleBGColor_Itc,
		                        (void *) VP_PLAY_STRING_SUBTITLE_COLOR_BLACK,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_bg_color_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleBGColor->nListCount++;
		elm_genlist_item_append(pObj,
		                        pSubtitleBGColor->st_SubtitleBGColor_Itc,
		                        (void *) VP_PLAY_STRING_SUBTITLE_COLOR_WHITE,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_bg_color_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleBGColor->nListCount++;
	}

	return TRUE;
}

/* external functions */
subtitle_bg_color_handle vp_subtitle_bg_color_create(Evas_Object *
        pParent,
        PopupCloseCbFunc
        pCloseCb,
        int nDefaultColor)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleBGColorPopup *pSubtitleBGColor = NULL;

	pSubtitleBGColor = calloc(1, sizeof(SubtitleBGColorPopup));

	if (pSubtitleBGColor == NULL) {
		VideoLogError("pSubtitleBGColor alloc fail");
		return NULL;
	}

	pSubtitleBGColor->pParent = pParent;
	pSubtitleBGColor->pCloseCb = pCloseCb;

	pSubtitleBGColor->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_BG_COLOR, NULL, 0.0, NULL,
	                    __vp_subtitle_bg_color_popup_key_event_cb,
	                    __vp_subtitle_bg_color_popup_mouse_event_cb,
	                    (void *) pSubtitleBGColor);
	if (pSubtitleBGColor->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_bg_color_destroy_handle(pSubtitleBGColor);
		return NULL;
	}

	pSubtitleBGColor->pGenList =
	    _vp_subtitle_bg_color_create_genlist(pSubtitleBGColor->pPopup);
	if (pSubtitleBGColor->pGenList == NULL) {
		VideoLogError("_vp_subtitle_bg_color_create_genlist fail");
		_vp_subtitle_bg_color_destroy_handle(pSubtitleBGColor);
		return NULL;
	}
	evas_object_data_set(pSubtitleBGColor->pGenList,
	                     VP_SUBTITLE_BG_COLOR_GENLIST_DATA_KEY,
	                     (void *) pSubtitleBGColor);
	evas_object_smart_callback_add(pSubtitleBGColor->pGenList, "realized",
	                               __vp_subtitle_bg_color_genlist_realized,
	                               NULL);

	pSubtitleBGColor->nDefaultIndex = nDefaultColor;
	pSubtitleBGColor->pRadio = elm_radio_add(pSubtitleBGColor->pGenList);

	if (!_vp_subtitle_bg_color_add_genlist_item
	        (pSubtitleBGColor->pGenList, (void *) pSubtitleBGColor)) {
		VideoLogError("_vp_subtitle_bg_color_add_genlist_item fail");
		return FALSE;
	}
	//elm_radio_value_set(pSubtitleBGColor->pRadio, nDefaultColor);
	pSubtitleBGColor->currentBgColor = nDefaultColor;
	evas_object_smart_callback_add(pSubtitleBGColor->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_bg_color_popup_rotate_cb,
	                               pSubtitleBGColor);

	pSubtitleBGColor->pBox = elm_box_add(pSubtitleBGColor->pPopup);
	vp_popup_set_popup_min_size(pSubtitleBGColor->pParent,
	                            pSubtitleBGColor->pBox,
	                            pSubtitleBGColor->nListCount,
	                            VIDEO_POPUP_DEFAULT);

	elm_box_pack_end(pSubtitleBGColor->pBox, pSubtitleBGColor->pGenList);
	elm_object_content_set(pSubtitleBGColor->pPopup,
	                       pSubtitleBGColor->pBox);

	return pSubtitleBGColor;
}

void vp_subtitle_bg_color_destroy(subtitle_bg_color_handle
                                  pSubtitleBGColorHandle)
{
	if (pSubtitleBGColorHandle == NULL) {
		VideoLogError("pSubtitleBGColorHandle is NULL");
		return;
	}

	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) pSubtitleBGColorHandle;

	_vp_subtitle_bg_color_destroy_handle(pSubtitleBGColor);

}

bool vp_subtitle_bg_color_realize(subtitle_bg_color_handle
                                  pSubtitleBGColorHandle)
{
	if (pSubtitleBGColorHandle == NULL) {
		VideoLogError("pSubtitleBGColorHandle is NULL");
		return FALSE;
	}

	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) pSubtitleBGColorHandle;

	evas_object_show(pSubtitleBGColor->pPopup);

	return TRUE;
}

bool vp_subtitle_bg_color_unrealize(subtitle_bg_color_handle
                                    pSubtitleBGColorHandle)
{
	if (pSubtitleBGColorHandle == NULL) {
		VideoLogError("pSubtitleBGColorHandle is NULL");
		return FALSE;
	}

	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) pSubtitleBGColorHandle;

	evas_object_hide(pSubtitleBGColor->pPopup);

	return TRUE;
}

bool vp_subtitle_bg_color_set_user_data(subtitle_bg_color_handle
                                        pSubtitleBGColorHandle,
                                        void *pUserData)
{
	if (pSubtitleBGColorHandle == NULL) {
		VideoLogError("pSubtitleBGColorHandle is NULL");
		return FALSE;
	}

	SubtitleBGColorPopup *pSubtitleBGColor =
	    (SubtitleBGColorPopup *) pSubtitleBGColorHandle;

	pSubtitleBGColor->pUserData = pUserData;

	return TRUE;
}
