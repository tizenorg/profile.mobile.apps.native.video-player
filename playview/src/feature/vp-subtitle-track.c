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
#include "vp-play-util.h"
#include "vp-subtitle-track.h"
#include "vp-device-language.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_TRACK_GENLIST_DATA_KEY		"vp.subtitle.track.genlist"


typedef struct _SubtitleTrackItem {
	Elm_Object_Item *pItem;
	char *szName;
	int nIndex;
	void *pSubtitleTrackPopup;
#ifdef _SUBTITLE_MULTI_LANGUAGE
	bool bCheck;
#endif
} SubtitleTrackItem;


typedef struct _SubtitleTrackPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
#ifndef _SUBTITLE_MULTI_LANGUAGE
	Evas_Object *pRadio;
#endif
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_SubtitleTrack_Itc;

	int nCurrentIndex;
	int nListCount;

	void *pUserData;
#ifdef _SUBTITLE_MULTI_LANGUAGE
	SubtitleLanguageChangeCbFunc pAddLanguageCb;
	SubtitleLanguageChangeCbFunc pRemoveLanguageCb;
#endif
	PopupCloseCbFunc pCloseCb;

	GList *pItemList;
} SubtitleTrackPopup;



static void _vp_subtitle_track_destroy_handle(SubtitleTrackPopup *
        pSubtitleTrack);
#ifdef _SUBTITLE_MULTI_LANGUAGE
static void __vp_subtitle_track_popup_check_state_change_cb(void
        *pUserData,
        Evas_Object *
        pObject,
        void
        *pEventInfo);
#else
static void __vp_subtitle_track_genlist_item_selected_cb(void *pUserData,
        Evas_Object *
        pObject,
        void
        *pEventInfo);
#endif
static void __vp_subtitle_track_genlist_realized(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_track_popup_rotate_cb(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	SubtitleTrackPopup *pSubtitleTrack = (SubtitleTrackPopup *) data;
	if (!pSubtitleTrack) {
		VideoLogError("pSubtitleSize IS null");
		return;
	}
	if (pSubtitleTrack->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleTrack->pParent,
	                            pSubtitleTrack->pBox,
	                            pSubtitleTrack->nListCount,
	                            VIDEO_POPUP_DEFAULT);

}

/* callback functions */
static char *__vp_subtitle_track_genlist_text_get_cb(const void
        *pUserData,
        Evas_Object *pObj,
        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("Invalid user data");
		return NULL;
	}

	SubtitleTrackItem *pTrackItem = (SubtitleTrackItem *) pUserData;

	if (!strcmp(pPart, "elm.text")) {
		return strdup(pTrackItem->szName);
	}
	return NULL;
}

static Evas_Object *__vp_subtitle_track_genlist_content_get_cb(const void
                        *pUserData,
                        Evas_Object *pObj,
                        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("Invalid user data");
		return NULL;
	}

	SubtitleTrackItem *pTrackItem = (SubtitleTrackItem *) pUserData;

	if (!strcmp(pPart, "elm.icon")) {
		SubtitleTrackPopup *pSubtitleTrack =
		    (SubtitleTrackPopup *) pTrackItem->pSubtitleTrackPopup;
		if (pSubtitleTrack == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}
#ifdef _SUBTITLE_MULTI_LANGUAGE
		Evas_Object *pCheckBox = elm_check_add(pObj);
		evas_object_propagate_events_set(pCheckBox, EINA_FALSE);
		evas_object_smart_callback_add(pCheckBox, "changed",
		                               __vp_subtitle_track_popup_check_state_change_cb,
		                               pUserData);
		elm_check_state_set(pCheckBox, pTrackItem->bCheck);
		evas_object_show(pCheckBox);

		return pCheckBox;
#else
		Evas_Object *pRadioObj = NULL;
		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, pTrackItem->nIndex);
		elm_radio_group_add(pRadioObj, pSubtitleTrack->pRadio);
		elm_radio_value_set(pSubtitleTrack->pRadio,
		                    pSubtitleTrack->nCurrentIndex);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_subtitle_track_genlist_item_selected_cb,
		                               pTrackItem);
		evas_object_show(pRadioObj);

		return pRadioObj;
#endif
	}

	return NULL;
}

#ifdef _SUBTITLE_MULTI_LANGUAGE
static bool __vp_subtitle_track_select_no_item(SubtitleTrackPopup *
        pSubtitleTrackPopup)
{
	if (pSubtitleTrackPopup == NULL) {
		VideoLogError("pSubtitleTrackPopup is NULL");
		return false;
	}

	if (pSubtitleTrackPopup->pItemList == NULL) {
		VideoLogError("pItemList is NULL");
		return false;
	}

	bool ret = true;
	int index = 0;
	for (; index < g_list_length(pSubtitleTrackPopup->pItemList); ++index) {
		SubtitleTrackItem *pItem = NULL;
		pItem =
		    (SubtitleTrackItem *) g_list_nth_data(pSubtitleTrackPopup->
		            pItemList, index);
		if (pItem && pItem->bCheck) {
			ret = false;
			break;
		}
	}

	return ret;
}

static void __vp_subtitle_track_popup_disable_button(SubtitleTrackPopup *
                        pSubtitleTrackPopup,
                        bool bDisabled)
{
	if (pSubtitleTrackPopup == NULL) {
		VideoLogError("pSubtitleTrackPopup is NULL");
		return;
	}

	if (pSubtitleTrackPopup->pPopup == NULL) {
		VideoLogError("pPopup is NULL");
		return;
	}

	Evas_Object *pRightButton =
	    elm_object_part_content_get(pSubtitleTrackPopup->pPopup,
	                                "button2");
	if (pRightButton
	        && bDisabled != elm_object_disabled_get(pRightButton)) {
		elm_object_disabled_set(pRightButton, bDisabled);
	}
}
#endif
static void __vp_subtitle_track_genlist_item_selected_cb(void *pUserData,
                        Evas_Object *pObject,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleTrackItem *pTrackItem = (SubtitleTrackItem *) pUserData;
	Elm_Object_Item *pSelectedItem =
	    elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	SubtitleTrackPopup *pSubtitleTrackPopup =
	    (SubtitleTrackPopup *) pTrackItem->pSubtitleTrackPopup;
	if (pSubtitleTrackPopup == NULL) {
		VideoLogError("SubtitleTrackPopup is NULL");
		return;
	}
#ifdef _SUBTITLE_MULTI_LANGUAGE
	Evas_Object *pCheckBox =
	    elm_object_item_part_content_get(pSelectedItem, "elm.icon");
	if (pCheckBox) {
		pTrackItem->bCheck = !pTrackItem->bCheck;
		elm_check_state_set(pCheckBox, pTrackItem->bCheck);
		VideoLogInfo("bCheck: %d, nIndex: %d", pTrackItem->bCheck,
		             pTrackItem->nIndex);
		if (__vp_subtitle_track_select_no_item(pSubtitleTrackPopup)) {
			vp_play_util_status_noti_show
			(VP_PLAY_STRING_SELECTED_LANGUAGE);
			__vp_subtitle_track_popup_disable_button(pSubtitleTrackPopup,
			        true);
		} else {
			__vp_subtitle_track_popup_disable_button(pSubtitleTrackPopup,
			        false);
		}
	}
#else
	if (pTrackItem->nIndex == pSubtitleTrackPopup->nCurrentIndex) {
		if (pSubtitleTrackPopup->pCloseCb) {
			VideoLogInfo("Selected lang track indexp[%d]",
			             pTrackItem->nIndex);
			pSubtitleTrackPopup->pCloseCb(pTrackItem->nIndex, FALSE,
			                              (void *) pSubtitleTrackPopup->
			                              pUserData);
		}
	} else {
		pSubtitleTrackPopup->nCurrentIndex = pTrackItem->nIndex;
		elm_radio_value_set(pSubtitleTrackPopup->pRadio,
		                    pSubtitleTrackPopup->nCurrentIndex);
	}
#endif
}

#ifdef _SUBTITLE_MULTI_LANGUAGE
static void __vp_subtitle_track_popup_check_state_change_cb(void
                        *pUserData,
                        Evas_Object *pObject,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleTrackItem *pTrackItem = (SubtitleTrackItem *) pUserData;
	SubtitleTrackPopup *pSubtitleTrackPopup =
	    (SubtitleTrackPopup *) pTrackItem->pSubtitleTrackPopup;
	if (pSubtitleTrackPopup == NULL) {
		VideoLogError("SubtitleTrackPopup is NULL");
		return;
	}
	pTrackItem->bCheck = elm_check_state_get(pObject);
	VideoLogInfo("bCheck: %d, nIndex: %d", pTrackItem->bCheck,
	             pTrackItem->nIndex);

	if (__vp_subtitle_track_select_no_item(pSubtitleTrackPopup)) {
		vp_play_util_status_noti_show(VP_PLAY_STRING_SELECTED_LANGUAGE);
		__vp_subtitle_track_popup_disable_button(pSubtitleTrackPopup,
		        true);
	} else {
		__vp_subtitle_track_popup_disable_button(pSubtitleTrackPopup,
		        false);
	}
}
#endif
static void __vp_subtitle_track_popup_key_event_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleTrackPopup *pSubtitleTrack = (SubtitleTrackPopup *) pUserData;

	if (pSubtitleTrack->pCloseCb) {
		pSubtitleTrack->pCloseCb(-1, FALSE,
		                         (void *) pSubtitleTrack->pUserData);
	}
}

#ifndef _SUBTITLE_MULTI_LANGUAGE
static void __vp_subtitle_track_popup_mouse_event_cb(void *pUserData,
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
		SubtitleTrackPopup *pSubtitleTrack =
		    (SubtitleTrackPopup *) pUserData;

		if (pSubtitleTrack->pCloseCb) {
			pSubtitleTrack->pCloseCb(-1, FALSE,
			                         (void *) pSubtitleTrack->pUserData);
		}
	}
}
#endif
/* internal functions */
static void _vp_subtitle_track_clear_item_list(SubtitleTrackPopup *
        pSubtitleTrack)
{
	if (pSubtitleTrack == NULL) {
		VideoLogError("pSubtitleTrack is NULL");
		return;
	}

	if (pSubtitleTrack->pItemList == NULL) {
		VideoLogError("pSubtitleTrack->pItemList is NULL");
		return;
	}


	int len = g_list_length(pSubtitleTrack->pItemList);
	int idx = 0;
	for (idx = 0; idx < len; idx++) {
		SubtitleTrackItem *pItem = NULL;

		pItem =
		    (SubtitleTrackItem *) g_list_nth_data(pSubtitleTrack->
		            pItemList, idx);
		if (pItem) {
			VP_EVAS_ITEM_DEL(pItem->pItem);
			VP_FREE(pItem->szName);

			memset(pItem, 0x00, sizeof(SubtitleTrackItem));
			VP_FREE(pItem);
		}
	}

	g_list_free(pSubtitleTrack->pItemList);
	pSubtitleTrack->pItemList = NULL;
}

static void _vp_subtitle_track_destroy_handle(SubtitleTrackPopup *
        pSubtitleTrack)
{
	if (pSubtitleTrack == NULL) {
		VideoLogError("pSubtitleTrack is NULL");
		return;
	}

	evas_object_smart_callback_del(pSubtitleTrack->pGenList, "realized",
	                               __vp_subtitle_track_genlist_realized);
	evas_object_smart_callback_del(pSubtitleTrack->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_track_popup_rotate_cb);

	_vp_subtitle_track_clear_item_list(pSubtitleTrack);
#ifndef _SUBTITLE_MULTI_LANGUAGE
	VP_EVAS_DEL(pSubtitleTrack->pRadio);
#endif
	VP_EVAS_DEL(pSubtitleTrack->pGenList);
	VP_EVAS_DEL(pSubtitleTrack->pButton);
	VP_EVAS_DEL(pSubtitleTrack->pBox);

	if (pSubtitleTrack->st_SubtitleTrack_Itc) {
		elm_genlist_item_class_free(pSubtitleTrack->st_SubtitleTrack_Itc);
		pSubtitleTrack->st_SubtitleTrack_Itc = NULL;
	}

	VP_EVAS_DEL(pSubtitleTrack->pPopup);

	VP_FREE(pSubtitleTrack);
}

static Evas_Object *_vp_subtitle_track_create_genlist(Evas_Object *
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


static bool _vp_subtitle_track_add_genlist_item(Evas_Object *pObj,
                        void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleTrackPopup *pSubtitleTrack = (SubtitleTrackPopup *) pUserData;

	if (pSubtitleTrack->st_SubtitleTrack_Itc) {
		elm_genlist_item_class_free(pSubtitleTrack->st_SubtitleTrack_Itc);
		pSubtitleTrack->st_SubtitleTrack_Itc = NULL;
	}

	pSubtitleTrack->st_SubtitleTrack_Itc = elm_genlist_item_class_new();

	if (pSubtitleTrack->st_SubtitleTrack_Itc) {
		pSubtitleTrack->st_SubtitleTrack_Itc->version =
		    ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitleTrack->st_SubtitleTrack_Itc->item_style =
		    "1text.1icon.3/popup";
		pSubtitleTrack->st_SubtitleTrack_Itc->func.text_get =
		    (void *) __vp_subtitle_track_genlist_text_get_cb;
		pSubtitleTrack->st_SubtitleTrack_Itc->func.content_get =
		    (void *) __vp_subtitle_track_genlist_content_get_cb;
		pSubtitleTrack->st_SubtitleTrack_Itc->func.state_get = NULL;
		pSubtitleTrack->st_SubtitleTrack_Itc->func.del = NULL;
	}

	int len = g_list_length(pSubtitleTrack->pItemList);
	int idx = 0;
	pSubtitleTrack->nListCount = 0;

	for (idx = 0; idx < len; idx++) {
		SubtitleTrackItem *pItem = NULL;

		pItem =
		    (SubtitleTrackItem *) g_list_nth_data(pSubtitleTrack->
		            pItemList, idx);
		if (pItem) {
			pItem->pItem = elm_genlist_item_append(pObj,
			                                       pSubtitleTrack->
			                                       st_SubtitleTrack_Itc,
			                                       (void *) pItem, NULL,
			                                       ELM_GENLIST_ITEM_NONE,
			                                       __vp_subtitle_track_genlist_item_selected_cb,
			                                       pItem);
			pSubtitleTrack->nListCount++;
		}
	}

	return TRUE;
}

#ifdef _SUBTITLE_MULTI_LANGUAGE
static void __vp_subtitle_track_popup_left_button_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogInfo("");
	SubtitleTrackPopup *pSubtitleTrack = (SubtitleTrackPopup *) pUserData;

	if (pSubtitleTrack->pCloseCb) {
		pSubtitleTrack->pCloseCb(-1, FALSE,
		                         (void *) pSubtitleTrack->pUserData);
	}
}

static void __vp_subtitle_track_popup_right_button_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogInfo("");
	SubtitleTrackPopup *pSubtitleTrack = (SubtitleTrackPopup *) pUserData;

	if (pSubtitleTrack->pItemList == NULL) {
		VideoLogError("pSubtitleTrack->pItemList is NULL");
		return;
	}


	int len = g_list_length(pSubtitleTrack->pItemList);
	int idx = 0;
	for (idx = 0; idx < len; idx++) {
		SubtitleTrackItem *pItem = NULL;

		pItem =
		    (SubtitleTrackItem *) g_list_nth_data(pSubtitleTrack->
		            pItemList, idx);
		/*set subtitle's language */
		if (pItem) {
			VideoLogInfo("nIndex=%d,bCheck=%d", pItem->nIndex,
			             pItem->bCheck);
			if (pItem->bCheck && pSubtitleTrack->pAddLanguageCb) {
				pSubtitleTrack->pAddLanguageCb(pItem->nIndex,
				                               (void *) pSubtitleTrack->
				                               pUserData);
			} else if (!pItem->bCheck
			           && pSubtitleTrack->pRemoveLanguageCb) {
				pSubtitleTrack->pRemoveLanguageCb(pItem->nIndex,
				                                  (void *)
				                                  pSubtitleTrack->
				                                  pUserData);
			}
		}
	}

	if (pSubtitleTrack->pCloseCb) {
		pSubtitleTrack->pCloseCb(-1, FALSE,
		                         (void *) pSubtitleTrack->pUserData);
	}
}
#endif
/* external functions */
#ifdef _SUBTITLE_MULTI_LANGUAGE
subtitle_track_handle vp_subtitle_track_create(Evas_Object *pParent,
        PopupCloseCbFunc pCloseCb)
#else
subtitle_track_handle vp_subtitle_track_create(Evas_Object *pParent,
        PopupCloseCbFunc pCloseCb,
        int nDefaultIndex)
#endif
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleTrackPopup *pSubtitleTrack = NULL;

	pSubtitleTrack = calloc(1, sizeof(SubtitleTrackPopup));

	if (pSubtitleTrack == NULL) {
		VideoLogError("pSubtitleTrack alloc fail");
		return NULL;
	}

	pSubtitleTrack->pParent = pParent;
	pSubtitleTrack->pCloseCb = pCloseCb;
#ifdef _SUBTITLE_MULTI_LANGUAGE
	pSubtitleTrack->pPopup = vp_title_two_button_popup_create(pParent,
	                         VP_PLAY_STRING_SUBTITLE_LANGUAGE,
	                         NULL,
	                         VP_PLAY_STRING_COM_CANCEL,
	                         __vp_subtitle_track_popup_left_button_cb,
	                         VP_PLAY_STRING_COM_OK,
	                         __vp_subtitle_track_popup_right_button_cb,
	                         (void *)
	                         pSubtitleTrack);
#else
	pSubtitleTrack->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_LANGUAGE, NULL, 0.0, NULL,
	                    __vp_subtitle_track_popup_key_event_cb,
	                    __vp_subtitle_track_popup_mouse_event_cb,
	                    (void *) pSubtitleTrack);
#endif
	if (pSubtitleTrack->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_track_destroy_handle(pSubtitleTrack);
		return NULL;
	}
#ifdef _SUBTITLE_MULTI_LANGUAGE
	eext_object_event_callback_add(pSubtitleTrack->pPopup,
	                               EEXT_CALLBACK_BACK,
	                               __vp_subtitle_track_popup_key_event_cb,
	                               (void *) pSubtitleTrack);
#endif

	pSubtitleTrack->pGenList =
	    _vp_subtitle_track_create_genlist(pSubtitleTrack->pPopup);
	if (pSubtitleTrack->pGenList == NULL) {
		VideoLogError("_vp_subtitle_track_create_genlist fail");
		_vp_subtitle_track_destroy_handle(pSubtitleTrack);
		return NULL;
	}

	evas_object_smart_callback_add(pSubtitleTrack->pGenList, "realized",
	                               __vp_subtitle_track_genlist_realized,
	                               NULL);
	evas_object_smart_callback_add(pSubtitleTrack->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_track_popup_rotate_cb,
	                               pSubtitleTrack);

	evas_object_data_set(pSubtitleTrack->pGenList,
	                     VP_SUBTITLE_TRACK_GENLIST_DATA_KEY,
	                     (void *) pSubtitleTrack);
#ifndef _SUBTITLE_MULTI_LANGUAGE
	pSubtitleTrack->nCurrentIndex = nDefaultIndex;
	pSubtitleTrack->pRadio = elm_radio_add(pSubtitleTrack->pGenList);
#endif
	pSubtitleTrack->pBox = elm_box_add(pSubtitleTrack->pPopup);


	return pSubtitleTrack;
}

void vp_subtitle_track_destroy(subtitle_track_handle pSubtitleTrackHandle)
{
	if (pSubtitleTrackHandle == NULL) {
		VideoLogError("pSubtitleTrackHandle is NULL");
		return;
	}

	SubtitleTrackPopup *pSubtitleTrack =
	    (SubtitleTrackPopup *) pSubtitleTrackHandle;

	_vp_subtitle_track_destroy_handle(pSubtitleTrack);

}

bool vp_subtitle_track_realize(subtitle_track_handle pSubtitleTrackHandle)
{
	if (pSubtitleTrackHandle == NULL) {
		VideoLogError("pSubtitleTrackHandle is NULL");
		return FALSE;
	}

	SubtitleTrackPopup *pSubtitleTrack =
	    (SubtitleTrackPopup *) pSubtitleTrackHandle;

	if (!_vp_subtitle_track_add_genlist_item
	        (pSubtitleTrack->pGenList, (void *) pSubtitleTrack)) {
		VideoLogError("_vp_subtitle_track_add_genlist_item fail");
		return FALSE;
	}
	if (pSubtitleTrack->nListCount == 1) {
		vp_popup_set_popup_min_size(pSubtitleTrack->pParent,
		                            pSubtitleTrack->pBox,
		                            pSubtitleTrack->nListCount,
		                            VIDEO_POPUP_DEFAULT);
		elm_scroller_policy_set(pSubtitleTrack->pGenList,
		                        ELM_SCROLLER_POLICY_OFF,
		                        ELM_SCROLLER_POLICY_OFF);
	} else {
		vp_popup_set_popup_min_size(pSubtitleTrack->pParent,
		                            pSubtitleTrack->pBox,
		                            pSubtitleTrack->nListCount,
		                            VIDEO_POPUP_DEFAULT);
	}

	//elm_radio_value_set(pSubtitleTrack->pRadio, pSubtitleTrack->nCurrentIndex);
	elm_box_pack_end(pSubtitleTrack->pBox, pSubtitleTrack->pGenList);
	elm_object_content_set(pSubtitleTrack->pPopup, pSubtitleTrack->pBox);

	evas_object_show(pSubtitleTrack->pPopup);

	return TRUE;
}

bool vp_subtitle_track_unrealize(subtitle_track_handle
                                 pSubtitleTrackHandle)
{
	if (pSubtitleTrackHandle == NULL) {
		VideoLogError("pSubtitleTrackHandle is NULL");
		return FALSE;
	}

	SubtitleTrackPopup *pSubtitleTrack =
	    (SubtitleTrackPopup *) pSubtitleTrackHandle;

	_vp_subtitle_track_clear_item_list(pSubtitleTrack);

	evas_object_hide(pSubtitleTrack->pPopup);

	return TRUE;
}

#ifdef _SUBTITLE_MULTI_LANGUAGE
bool vp_subtitle_track_add_Item(subtitle_track_handle
                                pSubtitleTrackHandle, char *szCode,
                                int nIndex, bool bCheck)
#else
bool vp_subtitle_track_add_Item(subtitle_track_handle
                                pSubtitleTrackHandle, char *szCode,
                                int nIndex)
#endif
{
	if (pSubtitleTrackHandle == NULL) {
		VideoLogError("pSubtitleTrackHandle is NULL");
		return FALSE;
	}

	SubtitleTrackPopup *pSubtitleTrack =
	    (SubtitleTrackPopup *) pSubtitleTrackHandle;

	SubtitleTrackItem *pItem = calloc(1, sizeof(SubtitleTrackItem));
	if (pItem == NULL) {
		VideoLogError("SubtitleTrackItem alloc fail");
		return FALSE;
	}

	pItem->pSubtitleTrackPopup = (void *) pSubtitleTrack;
	pItem->nIndex = nIndex;
#ifdef _SUBTITLE_MULTI_LANGUAGE
	pItem->bCheck = bCheck;
#endif
	char *pName = NULL;
	pName = vp_language_get_name(szCode);
	if (pName) {
		pItem->szName = pName;
	} else {
		char *pUnknown = NULL;
		//pUnknown = g_strdup_printf("%s(%s)", VP_PLAY_STRING_COM_UNKNOWN, szCode);
		pUnknown = g_strdup_printf("%s", VP_PLAY_STRING_COM_UNKNOWN);
		pItem->szName = pUnknown;
	}

	pSubtitleTrack->pItemList =
	    g_list_append(pSubtitleTrack->pItemList, pItem);

	return TRUE;
}

bool vp_subtitle_track_set_user_data(subtitle_track_handle
                                     pSubtitleTrackHandle,
                                     void *pUserData)
{
	if (pSubtitleTrackHandle == NULL) {
		VideoLogError("pSubtitleTrackHandle is NULL");
		return FALSE;
	}

	SubtitleTrackPopup *pSubtitleTrack =
	    (SubtitleTrackPopup *) pSubtitleTrackHandle;

	pSubtitleTrack->pUserData = pUserData;

	return TRUE;
}

#ifdef _SUBTITLE_MULTI_LANGUAGE
bool vp_subtitle_track_set_add_language_cb(subtitle_track_handle
        pSubtitleTrackHandle,
        SubtitleLanguageChangeCbFunc
        pCallback)
{
	if (pSubtitleTrackHandle == NULL) {
		VideoLogError("pSubtitleTrackHandle is NULL");
		return FALSE;
	}

	SubtitleTrackPopup *pSubtitleTrack =
	    (SubtitleTrackPopup *) pSubtitleTrackHandle;

	pSubtitleTrack->pAddLanguageCb = pCallback;

	return TRUE;
}

bool vp_subtitle_track_set_remove_language_cb(subtitle_track_handle
        pSubtitleTrackHandle,
        SubtitleLanguageChangeCbFunc
        pCallback)
{
	if (pSubtitleTrackHandle == NULL) {
		VideoLogError("pSubtitleTrackHandle is NULL");
		return FALSE;
	}

	SubtitleTrackPopup *pSubtitleTrack =
	    (SubtitleTrackPopup *) pSubtitleTrackHandle;

	pSubtitleTrack->pRemoveLanguageCb = pCallback;

	return TRUE;
}
#endif
bool vp_subtitle_track_get_name(char *szCode, char **szName)
{
	if (szCode == NULL) {
		VideoLogError("szCode is NULL");
		return FALSE;
	}
	char *pName = NULL;
	pName = vp_language_get_name(szCode);
	if (pName) {
		*szName = pName;
	} else {
		char *pUnknown = NULL;
		pUnknown = g_strdup_printf("%s", VP_PLAY_STRING_COM_UNKNOWN);
		*szName = pUnknown;
	}

	return TRUE;
}
