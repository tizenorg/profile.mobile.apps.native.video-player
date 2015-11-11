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

#include "vp-subtitle-edge.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_EDGE_GENLIST_DATA_KEY		"vp.subtitle.edge.genlist"


typedef struct _SubtitleEdgePopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_SubtitleEdge_Itc;

	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;
	video_play_subtitle_edge_t currentSubtitleEdge;
} SubtitleEdgePopup;



static void _vp_subtitle_edge_destroy_handle(SubtitleEdgePopup *
        pSubtitleEdge);
static void __vp_subtitle_edge_genlist_item_selected_cb(void *pUserData,
                        Evas_Object *pObject,
                        void *pEventInfo);

static void __vp_subtitle_edge_genlist_realized(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_edge_popup_rotate_cb(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	SubtitleEdgePopup *pSubtitleEdge = (SubtitleEdgePopup *) data;
	if (!pSubtitleEdge) {
		VideoLogError("pSubtitleEdge IS null");
		return;
	}
	if (pSubtitleEdge->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleEdge->pParent,
	                            pSubtitleEdge->pBox,
	                            pSubtitleEdge->nListCount,
	                            VIDEO_POPUP_DEFAULT);

}

/* callback functions */
static char *__vp_subtitle_edge_genlist_text_get_cb(const void *pUserData,
                        Evas_Object *pObj,
                        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!g_strcmp0(pPart, "elm.text")) {
		return strdup(szTxt);
	}

	return NULL;
}

static Evas_Object *__vp_subtitle_edge_genlist_content_get_cb(const void
                        *pUserData,
                        Evas_Object *pObj,
                        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!g_strcmp0(pPart, "elm.icon")) {
		Evas_Object *pRadioObj = NULL;

		SubtitleEdgePopup *pSubtitleEdge =
		    (SubtitleEdgePopup *) evas_object_data_get(pObj,
		            VP_SUBTITLE_EDGE_GENLIST_DATA_KEY);
		if (pSubtitleEdge == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}
		video_play_subtitle_edge_t nEdge = VIDEO_SUBTITLE_EDGE_NONE;
		if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_RAISED)) {
			nEdge = VIDEO_SUBTITLE_EDGE_RAISED;
		} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_DEPRESSED)) {
			nEdge = VIDEO_SUBTITLE_EDGE_DEPRESSED;
		} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_UNIFORM)) {
			nEdge = VIDEO_SUBTITLE_EDGE_UNIFORM;
		} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_DROP_SHADOW)) {
			nEdge = VIDEO_SUBTITLE_EDGE_DROP_SHADOW;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nEdge);
		elm_radio_group_add(pRadioObj, pSubtitleEdge->pRadio);
		elm_radio_value_set(pSubtitleEdge->pRadio,
		                    pSubtitleEdge->currentSubtitleEdge);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_subtitle_edge_genlist_item_selected_cb,
		                               pSubtitleEdge);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_subtitle_edge_genlist_item_selected_cb(void *pUserData,
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

	SubtitleEdgePopup *pSubtitleEdge = (SubtitleEdgePopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}
	video_play_subtitle_edge_t nEdge = VIDEO_SUBTITLE_EDGE_NONE;
	if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_RAISED)) {
		nEdge = VIDEO_SUBTITLE_EDGE_RAISED;
	} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_DEPRESSED)) {
		nEdge = VIDEO_SUBTITLE_EDGE_DEPRESSED;
	} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_UNIFORM)) {
		nEdge = VIDEO_SUBTITLE_EDGE_UNIFORM;
	} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_EDGE_DROP_SHADOW)) {
		nEdge = VIDEO_SUBTITLE_EDGE_DROP_SHADOW;
	}

	pSubtitleEdge->currentSubtitleEdge = nEdge;
	if (pSubtitleEdge->pCloseCb) {
		pSubtitleEdge->pCloseCb(nEdge, FALSE,
		                        (void *) pSubtitleEdge->pUserData);
	}
}

static void __vp_subtitle_edge_popup_key_event_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleEdgePopup *pSubtitleEdge = (SubtitleEdgePopup *) pUserData;

	if (pSubtitleEdge->pCloseCb) {
		pSubtitleEdge->pCloseCb(-1, FALSE,
		                        (void *) pSubtitleEdge->pUserData);
	}
}

static void __vp_subtitle_edge_popup_mouse_event_cb(void *pUserData,
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
		SubtitleEdgePopup *pSubtitleEdge =
		    (SubtitleEdgePopup *) pUserData;

		if (pSubtitleEdge->pCloseCb) {
			pSubtitleEdge->pCloseCb(-1, FALSE,
			                        (void *) pSubtitleEdge->pUserData);
		}
	}
}



/* internal functions */
static void _vp_subtitle_edge_destroy_handle(SubtitleEdgePopup *
                pSubtitleEdge)
{
	if (pSubtitleEdge == NULL) {
		VideoLogError("pSubtitleEdge is NULL");
		return;
	}

	evas_object_smart_callback_del(pSubtitleEdge->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_edge_popup_rotate_cb);
	evas_object_smart_callback_del(pSubtitleEdge->pGenList, "realized",
	                               __vp_subtitle_edge_genlist_realized);

	VP_EVAS_DEL(pSubtitleEdge->pRadio);
	VP_EVAS_DEL(pSubtitleEdge->pGenList);
	VP_EVAS_DEL(pSubtitleEdge->pButton);
	VP_EVAS_DEL(pSubtitleEdge->pBox);

	if (pSubtitleEdge->st_SubtitleEdge_Itc) {
		elm_genlist_item_class_free(pSubtitleEdge->st_SubtitleEdge_Itc);
		pSubtitleEdge->st_SubtitleEdge_Itc = NULL;
	}

	VP_EVAS_DEL(pSubtitleEdge->pPopup);

	VP_FREE(pSubtitleEdge);
}


static Evas_Object *_vp_subtitle_edge_create_genlist(Evas_Object *
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


static bool _vp_subtitle_edge_add_genlist_item(Evas_Object *pObj,
                void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleEdgePopup *pSubtitleEdge = (SubtitleEdgePopup *) pUserData;

	if (pSubtitleEdge->st_SubtitleEdge_Itc) {
		elm_genlist_item_class_free(pSubtitleEdge->st_SubtitleEdge_Itc);
		pSubtitleEdge->st_SubtitleEdge_Itc = NULL;
	}

	pSubtitleEdge->st_SubtitleEdge_Itc = elm_genlist_item_class_new();

	if (pSubtitleEdge->st_SubtitleEdge_Itc != NULL) {
		pSubtitleEdge->st_SubtitleEdge_Itc->version =
		    ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitleEdge->st_SubtitleEdge_Itc->item_style =
		    "1text.1icon.3/popup";
		pSubtitleEdge->st_SubtitleEdge_Itc->func.text_get =
		    (void *) __vp_subtitle_edge_genlist_text_get_cb;
		pSubtitleEdge->st_SubtitleEdge_Itc->func.content_get =
		    (void *) __vp_subtitle_edge_genlist_content_get_cb;
		pSubtitleEdge->st_SubtitleEdge_Itc->func.state_get = NULL;
		pSubtitleEdge->st_SubtitleEdge_Itc->func.del = NULL;
		pSubtitleEdge->nListCount = 0;

		elm_genlist_item_append(pObj, pSubtitleEdge->st_SubtitleEdge_Itc,
		                        (void *) VP_PLAY_STRING_EDGE_NO_EDGE, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_edge_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleEdge->nListCount++;
		elm_genlist_item_append(pObj, pSubtitleEdge->st_SubtitleEdge_Itc,
		                        (void *) VP_PLAY_STRING_EDGE_RAISED, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_edge_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleEdge->nListCount++;
		elm_genlist_item_append(pObj, pSubtitleEdge->st_SubtitleEdge_Itc,
		                        (void *) VP_PLAY_STRING_EDGE_DEPRESSED, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_edge_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleEdge->nListCount++;
		elm_genlist_item_append(pObj, pSubtitleEdge->st_SubtitleEdge_Itc,
		                        (void *) VP_PLAY_STRING_EDGE_UNIFORM, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_edge_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleEdge->nListCount++;
		elm_genlist_item_append(pObj, pSubtitleEdge->st_SubtitleEdge_Itc,
		                        (void *) VP_PLAY_STRING_EDGE_DROP_SHADOW,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_edge_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleEdge->nListCount++;
	}

	return TRUE;
}

/* external functions */
subtitle_edge_handle vp_subtitle_edge_create(Evas_Object *pParent,
                        PopupCloseCbFunc pCloseCb,
                        int nDefaultSize)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleEdgePopup *pSubtitleEdge = NULL;

	pSubtitleEdge = calloc(1, sizeof(SubtitleEdgePopup));

	if (pSubtitleEdge == NULL) {
		VideoLogError("pSubtitleEdge alloc fail");
		return NULL;
	}

	pSubtitleEdge->pParent = pParent;
	pSubtitleEdge->pCloseCb = pCloseCb;

	pSubtitleEdge->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_EDGE, NULL, 0.0, NULL,
	                    __vp_subtitle_edge_popup_key_event_cb,
	                    __vp_subtitle_edge_popup_mouse_event_cb,
	                    (void *) pSubtitleEdge);
	if (pSubtitleEdge->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_edge_destroy_handle(pSubtitleEdge);
		return NULL;
	}

	pSubtitleEdge->pGenList =
	    _vp_subtitle_edge_create_genlist(pSubtitleEdge->pPopup);
	if (pSubtitleEdge->pGenList == NULL) {
		VideoLogError("_vp_subtitle_size_create_genlist fail");
		_vp_subtitle_edge_destroy_handle(pSubtitleEdge);
		return NULL;
	}
	evas_object_data_set(pSubtitleEdge->pGenList,
	                     VP_SUBTITLE_EDGE_GENLIST_DATA_KEY,
	                     (void *) pSubtitleEdge);
	evas_object_smart_callback_add(pSubtitleEdge->pGenList, "realized",
	                               __vp_subtitle_edge_genlist_realized,
	                               NULL);

	pSubtitleEdge->pRadio = elm_radio_add(pSubtitleEdge->pGenList);

	if (!_vp_subtitle_edge_add_genlist_item
	        (pSubtitleEdge->pGenList, (void *) pSubtitleEdge)) {
		VideoLogError("_vp_subtitle_size_add_genlist_item fail");
		return FALSE;
	}
	//elm_radio_value_set(pSubtitleEdge->pRadio, nDefaultSize);
	pSubtitleEdge->currentSubtitleEdge = nDefaultSize;
	evas_object_smart_callback_add(pSubtitleEdge->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_edge_popup_rotate_cb,
	                               pSubtitleEdge);

	pSubtitleEdge->pBox = elm_box_add(pSubtitleEdge->pPopup);
	vp_popup_set_popup_min_size(pSubtitleEdge->pParent,
	                            pSubtitleEdge->pBox,
	                            pSubtitleEdge->nListCount,
	                            VIDEO_POPUP_DEFAULT);
	elm_box_pack_end(pSubtitleEdge->pBox, pSubtitleEdge->pGenList);
	elm_object_content_set(pSubtitleEdge->pPopup, pSubtitleEdge->pBox);

	return pSubtitleEdge;
}

void vp_subtitle_edge_destroy(subtitle_edge_handle pSubtitleEdgeHandle)
{
	if (pSubtitleEdgeHandle == NULL) {
		VideoLogError("pSubtitleEdgeHandle is NULL");
		return;
	}

	SubtitleEdgePopup *pSubtitleEdge =
	    (SubtitleEdgePopup *) pSubtitleEdgeHandle;

	_vp_subtitle_edge_destroy_handle(pSubtitleEdge);

}

bool vp_subtitle_edge_realize(subtitle_edge_handle pSubtitleEdgeHandle)
{
	if (pSubtitleEdgeHandle == NULL) {
		VideoLogError("pSubtitleEdgeHandle is NULL");
		return FALSE;
	}

	SubtitleEdgePopup *pSubtitleEdge =
	    (SubtitleEdgePopup *) pSubtitleEdgeHandle;

	evas_object_show(pSubtitleEdge->pPopup);

	return TRUE;
}

bool vp_subtitle_edge_unrealize(subtitle_edge_handle pSubtitleEdgeHandle)
{
	if (pSubtitleEdgeHandle == NULL) {
		VideoLogError("pSubtitleEdgeHandle is NULL");
		return FALSE;
	}

	SubtitleEdgePopup *pSubtitleEdge =
	    (SubtitleEdgePopup *) pSubtitleEdgeHandle;

	evas_object_hide(pSubtitleEdge->pPopup);

	return TRUE;
}

bool vp_subtitle_edge_set_user_data(subtitle_edge_handle
                                    pSubtitleEdgeHandle, void *pUserData)
{
	if (pSubtitleEdgeHandle == NULL) {
		VideoLogError("pSubtitleEdgeHandle is NULL");
		return FALSE;
	}

	SubtitleEdgePopup *pSubtitleEdge =
	    (SubtitleEdgePopup *) pSubtitleEdgeHandle;

	pSubtitleEdge->pUserData = pUserData;

	return TRUE;
}
