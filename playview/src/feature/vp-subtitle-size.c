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

#include "vp-subtitle-size.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_SIZE_GENLIST_DATA_KEY		"vp.subtitle.size.genlist"


typedef struct _SubtitleSizePopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_SubtitleSize_Itc;

	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;
	video_play_subtitle_size_t currentSubtitleSize;
} SubtitleSizePopup;



static void _vp_subtitle_size_destroy_handle(SubtitleSizePopup *
                        pSubtitleSize);
static void __vp_subtitle_size_genlist_item_selected_cb(void *pUserData,
                        Evas_Object *pObject,
                        void *pEventInfo);

static void __vp_subtitle_size_genlist_realized(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_size_popup_rotate_cb(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	SubtitleSizePopup *pSubtitleSize = (SubtitleSizePopup *) data;
	if (!pSubtitleSize) {
		VideoLogError("pSubtitleSize IS null");
		return;
	}
	if (pSubtitleSize->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleSize->pParent,
	                            pSubtitleSize->pBox,
	                            pSubtitleSize->nListCount,
	                            VIDEO_POPUP_DEFAULT);

}

/* callback functions */
static char *__vp_subtitle_size_genlist_text_get_cb(const void *pUserData,
                        Evas_Object *pObj,
                        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	if (!strcmp(pPart, "elm.text.main.left")) {

		char *szTxt = (char *) pUserData;
		char szTmp[VP_STRING_LEN_MAX] = { 0, };
		int nSize = 0;

		if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_SMALL)) {
			nSize = VP_SUBTITLE_SIZE_SMALL_VALUE;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_MEDIUM)) {
			nSize = VP_SUBTITLE_SIZE_MEDIUM_VALUE;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_LARGE)) {
			nSize = VP_SUBTITLE_SIZE_LARGE_VALUE;
		}

		snprintf(szTmp, VP_STRING_LEN_MAX - 1,
		         "<font_size=%d>%s</font_size>", nSize, szTxt);

		return strdup(szTmp);
	}

	return NULL;
}

static Evas_Object *__vp_subtitle_size_genlist_content_get_cb(const void
                        *pUserData,
                        Evas_Object *pObj,
                        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!strcmp(pPart, "elm.icon.right")) {
		Evas_Object *pRadioObj = NULL;

		SubtitleSizePopup *pSubtitleSize =
		    (SubtitleSizePopup *) evas_object_data_get(pObj,
		            VP_SUBTITLE_SIZE_GENLIST_DATA_KEY);
		if (pSubtitleSize == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}
		video_play_subtitle_size_t nSize = VIDEO_SUBTITLE_SIZE_MEDIUM;
		if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_SMALL)) {
			nSize = VIDEO_SUBTITLE_SIZE_SMALL;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_MEDIUM)) {
			nSize = VIDEO_SUBTITLE_SIZE_MEDIUM;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_LARGE)) {
			nSize = VIDEO_SUBTITLE_SIZE_LARGE;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nSize);
		elm_radio_group_add(pRadioObj, pSubtitleSize->pRadio);
		elm_radio_value_set(pSubtitleSize->pRadio,
		                    pSubtitleSize->currentSubtitleSize);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_subtitle_size_genlist_item_selected_cb,
		                               pSubtitleSize);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_subtitle_size_genlist_item_selected_cb(void *pUserData,
                        Evas_Object *pObject,
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

	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;
	Elm_Object_Item *pSelectedItem =
	    elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	char *szTxt = (char *) elm_object_item_data_get(pItem);

	SubtitleSizePopup *pSubtitleSize = (SubtitleSizePopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}
	video_play_subtitle_size_t nSize = VIDEO_SUBTITLE_SIZE_MEDIUM;
	if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_SMALL)) {
		nSize = VIDEO_SUBTITLE_SIZE_SMALL;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_MEDIUM)) {
		nSize = VIDEO_SUBTITLE_SIZE_MEDIUM;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE_LARGE)) {
		nSize = VIDEO_SUBTITLE_SIZE_LARGE;
	}

	pSubtitleSize->currentSubtitleSize = nSize;
	if (pSubtitleSize->pCloseCb) {
		pSubtitleSize->pCloseCb(nSize, FALSE,
		                        (void *) pSubtitleSize->pUserData);
	}
}

static void __vp_subtitle_size_popup_key_event_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSizePopup *pSubtitleSize = (SubtitleSizePopup *) pUserData;

	if (pSubtitleSize->pCloseCb) {
		pSubtitleSize->pCloseCb(-1, FALSE,
		                        (void *) pSubtitleSize->pUserData);
	}
}

static void __vp_subtitle_size_popup_mouse_event_cb(void *pUserData,
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
		SubtitleSizePopup *pSubtitleSize =
		    (SubtitleSizePopup *) pUserData;

		if (pSubtitleSize->pCloseCb) {
			pSubtitleSize->pCloseCb(-1, FALSE,
			                        (void *) pSubtitleSize->pUserData);
		}
	}
}



/* internal functions */
static void _vp_subtitle_size_destroy_handle(SubtitleSizePopup *
        pSubtitleSize)
{
	if (pSubtitleSize == NULL) {
		VideoLogError("pSubtitleSize is NULL");
		return;
	}

	evas_object_smart_callback_del(pSubtitleSize->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_size_popup_rotate_cb);
	evas_object_smart_callback_del(pSubtitleSize->pGenList, "realized",
	                               __vp_subtitle_size_genlist_realized);

	VP_EVAS_DEL(pSubtitleSize->pRadio);
	VP_EVAS_DEL(pSubtitleSize->pGenList);
	VP_EVAS_DEL(pSubtitleSize->pButton);
	VP_EVAS_DEL(pSubtitleSize->pBox);

	if (pSubtitleSize->st_SubtitleSize_Itc) {
		elm_genlist_item_class_free(pSubtitleSize->st_SubtitleSize_Itc);
		pSubtitleSize->st_SubtitleSize_Itc = NULL;
	}

	VP_EVAS_DEL(pSubtitleSize->pPopup);

	VP_FREE(pSubtitleSize);
}


static Evas_Object *_vp_subtitle_size_create_genlist(Evas_Object *
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


static bool _vp_subtitle_size_add_genlist_item(Evas_Object *pObj,
        void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleSizePopup *pSubtitleSize = (SubtitleSizePopup *) pUserData;

	if (pSubtitleSize->st_SubtitleSize_Itc) {
		elm_genlist_item_class_free(pSubtitleSize->st_SubtitleSize_Itc);
		pSubtitleSize->st_SubtitleSize_Itc = NULL;
	}

	pSubtitleSize->st_SubtitleSize_Itc = elm_genlist_item_class_new();

	if (pSubtitleSize->st_SubtitleSize_Itc != NULL) {
		pSubtitleSize->st_SubtitleSize_Itc->version =
		    ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitleSize->st_SubtitleSize_Itc->item_style = "1line";
		pSubtitleSize->st_SubtitleSize_Itc->func.text_get =
		    (void *) __vp_subtitle_size_genlist_text_get_cb;
		pSubtitleSize->st_SubtitleSize_Itc->func.content_get =
		    (void *) __vp_subtitle_size_genlist_content_get_cb;
		pSubtitleSize->st_SubtitleSize_Itc->func.state_get = NULL;
		pSubtitleSize->st_SubtitleSize_Itc->func.del = NULL;
		pSubtitleSize->nListCount = 0;

		elm_genlist_item_append(pObj,
		                        pSubtitleSize->st_SubtitleSize_Itc,
		                        (void *) VP_PLAY_STRING_SUBTITLE_SIZE_LARGE,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_size_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleSize->nListCount++;
		elm_genlist_item_append(pObj, pSubtitleSize->st_SubtitleSize_Itc,
		                        (void *) VP_PLAY_STRING_SUBTITLE_SIZE_MEDIUM,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_size_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleSize->nListCount++;
		elm_genlist_item_append(pObj, pSubtitleSize->st_SubtitleSize_Itc,
		                        (void *) VP_PLAY_STRING_SUBTITLE_SIZE_SMALL,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_size_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleSize->nListCount++;
	}

	return TRUE;
}

/* external functions */
subtitle_size_handle vp_subtitle_size_create(Evas_Object *pParent,
        PopupCloseCbFunc pCloseCb,
        int nDefaultSize)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleSizePopup *pSubtitleSize = NULL;

	pSubtitleSize = calloc(1, sizeof(SubtitleSizePopup));

	if (pSubtitleSize == NULL) {
		VideoLogError("pSubtitleSize alloc fail");
		return NULL;
	}

	pSubtitleSize->pParent = pParent;
	pSubtitleSize->pCloseCb = pCloseCb;

	pSubtitleSize->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_SIZE, NULL, 0.0, NULL,
	                    __vp_subtitle_size_popup_key_event_cb,
	                    __vp_subtitle_size_popup_mouse_event_cb,
	                    (void *) pSubtitleSize);
	if (pSubtitleSize->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_size_destroy_handle(pSubtitleSize);
		return NULL;
	}

	pSubtitleSize->pGenList =
	    _vp_subtitle_size_create_genlist(pSubtitleSize->pPopup);
	if (pSubtitleSize->pGenList == NULL) {
		VideoLogError("_vp_subtitle_size_create_genlist fail");
		_vp_subtitle_size_destroy_handle(pSubtitleSize);
		return NULL;
	}
	evas_object_data_set(pSubtitleSize->pGenList,
	                     VP_SUBTITLE_SIZE_GENLIST_DATA_KEY,
	                     (void *) pSubtitleSize);
	evas_object_smart_callback_add(pSubtitleSize->pGenList, "realized",
	                               __vp_subtitle_size_genlist_realized,
	                               NULL);

	pSubtitleSize->pRadio = elm_radio_add(pSubtitleSize->pGenList);

	if (!_vp_subtitle_size_add_genlist_item
	        (pSubtitleSize->pGenList, (void *) pSubtitleSize)) {
		VideoLogError("_vp_subtitle_size_add_genlist_item fail");
		return FALSE;
	}
	//elm_radio_value_set(pSubtitleSize->pRadio, nDefaultSize);
	pSubtitleSize->currentSubtitleSize = nDefaultSize;
	evas_object_smart_callback_add(pSubtitleSize->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_size_popup_rotate_cb,
	                               pSubtitleSize);

	pSubtitleSize->pBox = elm_box_add(pSubtitleSize->pPopup);
	vp_popup_set_popup_min_size(pSubtitleSize->pParent,
	                            pSubtitleSize->pBox,
	                            pSubtitleSize->nListCount,
	                            VIDEO_POPUP_DEFAULT);
	elm_box_pack_end(pSubtitleSize->pBox, pSubtitleSize->pGenList);
	elm_object_content_set(pSubtitleSize->pPopup, pSubtitleSize->pBox);

	return pSubtitleSize;
}

void vp_subtitle_size_destroy(subtitle_size_handle pSubtitleSizeHandle)
{
	if (pSubtitleSizeHandle == NULL) {
		VideoLogError("pSubtitleSizeHandle is NULL");
		return;
	}

	SubtitleSizePopup *pSubtitleSize =
	    (SubtitleSizePopup *) pSubtitleSizeHandle;

	_vp_subtitle_size_destroy_handle(pSubtitleSize);

}

bool vp_subtitle_size_realize(subtitle_size_handle pSubtitleSizeHandle)
{
	if (pSubtitleSizeHandle == NULL) {
		VideoLogError("pSubtitleSizeHandle is NULL");
		return FALSE;
	}

	SubtitleSizePopup *pSubtitleSize =
	    (SubtitleSizePopup *) pSubtitleSizeHandle;

	evas_object_show(pSubtitleSize->pPopup);

	return TRUE;
}

bool vp_subtitle_size_unrealize(subtitle_size_handle pSubtitleSizeHandle)
{
	if (pSubtitleSizeHandle == NULL) {
		VideoLogError("pSubtitleSizeHandle is NULL");
		return FALSE;
	}

	SubtitleSizePopup *pSubtitleSize =
	    (SubtitleSizePopup *) pSubtitleSizeHandle;

	evas_object_hide(pSubtitleSize->pPopup);

	return TRUE;
}

bool vp_subtitle_size_set_user_data(subtitle_size_handle
                                    pSubtitleSizeHandle, void *pUserData)
{
	if (pSubtitleSizeHandle == NULL) {
		VideoLogError("pSubtitleSizeHandle is NULL");
		return FALSE;
	}

	SubtitleSizePopup *pSubtitleSize =
	    (SubtitleSizePopup *) pSubtitleSizeHandle;

	pSubtitleSize->pUserData = pUserData;

	return TRUE;
}
