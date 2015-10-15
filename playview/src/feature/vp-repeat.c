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
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-type-define.h"
#include "vp-play-macro-define.h"
#include "vp-play-preference.h"

#include "vp-repeat.h"

/* check temp */
#include "vp-play-log.h"

#define VP_REPEAT_GENLIST_DATA_KEY		"vp.repeat.genlist"

typedef struct _RepeatPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;

	Elm_Genlist_Item_Class *st_Repeat_Itc;
	int nListCount;
	char *pMediUrl;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;
	video_play_repeat_mode_t repeatMode;
} RepeatPopup;



static void _vp_repeat_destroy_handle(RepeatPopup *pRepeat);
static void __vp_repeat_genlist_item_selected_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo);


static void __vp_repeat_genlist_realized(void *data, Evas_Object *obj,
		void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_repeat_popup_rotate_cb(void *data, Evas_Object *obj,
					void *event_info)
{
	RepeatPopup *pRepeat = (RepeatPopup *) data;
	if (!pRepeat) {
		VideoLogError("pCapture IS null");
		return;
	}
	if (pRepeat->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pRepeat->pParent, pRepeat->pBox,
				    pRepeat->nListCount, VIDEO_POPUP_DEFAULT);

}

/* callback functions */
static char *__vp_repeat_genlist_text_get_cb(const void *pUserData,
		Evas_Object *pObj,
		const char *pPart)
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

static Evas_Object *__vp_repeat_genlist_content_get_cb(const void
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

		RepeatPopup *pRepeat =
			(RepeatPopup *) evas_object_data_get(pObj,
					VP_REPEAT_GENLIST_DATA_KEY);
		if (pRepeat == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		video_play_repeat_mode_t nType = VIDEO_PLAY_REPEAT_NONE;

		if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_OFF)) {
			nType = VIDEO_PLAY_REPEAT_OFF;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_ALL_OFF)) {
			nType = VIDEO_PLAY_REPEAT_ALL_STOP;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_ONE)) {
			nType = VIDEO_PLAY_REPEAT_ONE;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_ALL)) {
			nType = VIDEO_PLAY_REPEAT_ALL;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nType);
		elm_radio_group_add(pRadioObj, pRepeat->pRadio);
		elm_radio_value_set(pRepeat->pRadio, pRepeat->repeatMode);
		evas_object_smart_callback_add(pRadioObj, "changed",
					       __vp_repeat_genlist_item_selected_cb,
					       pRepeat);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_repeat_genlist_item_selected_cb(void *pUserData,
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

	RepeatPopup *pRepeat = (RepeatPopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	video_play_repeat_mode_t nType = VIDEO_PLAY_REPEAT_NONE;

	if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_OFF)) {
		nType = VIDEO_PLAY_REPEAT_OFF;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_ALL_OFF)) {
		nType = VIDEO_PLAY_REPEAT_ALL_STOP;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_ONE)) {
		nType = VIDEO_PLAY_REPEAT_ONE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_REPEAT_ALL)) {
		nType = VIDEO_PLAY_REPEAT_ALL;
	}

	if (nType != VIDEO_PLAY_REPEAT_NONE) {
		vp_play_preference_set_repeat_mode_key((int) nType);
	}

	pRepeat->repeatMode = nType;
	VideoLogInfo("repeatMode=%d", pRepeat->repeatMode);
	if (pRepeat->pCloseCb) {
		pRepeat->pCloseCb((int) nType, FALSE,
				  (void *) pRepeat->pUserData);
	}
}

static void __vp_repeat_popup_key_event_cb(void *pUserData,
		Evas_Object *pObj,
		void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	RepeatPopup *pRepeat = (RepeatPopup *) pUserData;

	if (pRepeat->pCloseCb) {
		pRepeat->pCloseCb(VIDEO_PLAY_REPEAT_NONE, FALSE,
				  (void *) pRepeat->pUserData);
	}
}


static void __vp_repeat_popup_mouse_event_cb(void *pUserData,
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
		RepeatPopup *pRepeat = (RepeatPopup *) pUserData;

		if (pRepeat->pCloseCb) {
			pRepeat->pCloseCb(VIDEO_PLAY_REPEAT_NONE, FALSE,
					  (void *) pRepeat->pUserData);
		}
	}
}


/* internal functions */
static void _vp_repeat_destroy_handle(RepeatPopup *pRepeat)
{
	if (pRepeat == NULL) {
		VideoLogError("pRepeat is NULL");
		return;
	}
	evas_object_smart_callback_del(pRepeat->pGenList, "realized",
				       __vp_repeat_genlist_realized);
	evas_object_smart_callback_del(pRepeat->pParent, "rotation,changed",
				       __vp_repeat_popup_rotate_cb);


	VP_EVAS_DEL(pRepeat->pRadio);
	VP_EVAS_DEL(pRepeat->pGenList);
	VP_EVAS_DEL(pRepeat->pButton);
	VP_EVAS_DEL(pRepeat->pBox);

	VP_EVAS_DEL(pRepeat->pPopup);

	VP_FREE(pRepeat->pMediUrl);

	if (pRepeat->st_Repeat_Itc) {
		elm_genlist_item_class_free(pRepeat->st_Repeat_Itc);
		pRepeat->st_Repeat_Itc = NULL;
	}

	VP_FREE(pRepeat);
}

static Evas_Object *_vp_repeat_create_genlist(Evas_Object *pParent)
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


static bool _vp_repeat_add_genlist_item(Evas_Object *pObj,
					void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}

	RepeatPopup *pRepeat = (RepeatPopup *) pUserData;

	if (pRepeat->st_Repeat_Itc) {
		elm_genlist_item_class_free(pRepeat->st_Repeat_Itc);
		pRepeat->st_Repeat_Itc = NULL;
	}

	pRepeat->st_Repeat_Itc = elm_genlist_item_class_new();

	if (pRepeat->st_Repeat_Itc != NULL) {	
		pRepeat->st_Repeat_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pRepeat->st_Repeat_Itc->item_style = "1line";
		pRepeat->st_Repeat_Itc->func.text_get =
			(void *) __vp_repeat_genlist_text_get_cb;
		pRepeat->st_Repeat_Itc->func.content_get =
			(void *) __vp_repeat_genlist_content_get_cb;
		pRepeat->st_Repeat_Itc->func.state_get = NULL;
		pRepeat->st_Repeat_Itc->func.del = NULL;
		pRepeat->nListCount = 0;

		elm_genlist_item_append(pObj, pRepeat->st_Repeat_Itc,
				(void *) VP_PLAY_STRING_REPEAT_OFF, NULL,
				ELM_GENLIST_ITEM_NONE,
				__vp_repeat_genlist_item_selected_cb,
				pUserData);
		pRepeat->nListCount++;
		elm_genlist_item_append(pObj, pRepeat->st_Repeat_Itc,
				(void *) VP_PLAY_STRING_REPEAT_ALL_OFF, NULL,
				ELM_GENLIST_ITEM_NONE,
				__vp_repeat_genlist_item_selected_cb,
				pUserData);
		pRepeat->nListCount++;
		elm_genlist_item_append(pObj, pRepeat->st_Repeat_Itc,
				(void *) VP_PLAY_STRING_REPEAT_ONE, NULL,
				ELM_GENLIST_ITEM_NONE,
				__vp_repeat_genlist_item_selected_cb,
				pUserData);
		pRepeat->nListCount++;
		elm_genlist_item_append(pObj, pRepeat->st_Repeat_Itc,
				(void *) VP_PLAY_STRING_REPEAT_ALL, NULL,
				ELM_GENLIST_ITEM_NONE,
				__vp_repeat_genlist_item_selected_cb,
				pUserData);
		pRepeat->nListCount++;
	}

	return TRUE;
}

/* external functions */
repeat_handle vp_repeat_create(Evas_Object *pParent, char *pMediaUrl,
			       PopupCloseCbFunc pCloseCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (pMediaUrl == NULL) {
		VideoLogError("pMediaUrl is NULL");
		return NULL;
	}

	RepeatPopup *pRepeat = NULL;

	pRepeat = calloc(1, sizeof(RepeatPopup));

	if (pRepeat == NULL) {
		VideoLogError("pRepeat alloc fail");
		return NULL;
	}

	pRepeat->pParent = pParent;
	pRepeat->pCloseCb = pCloseCb;

	VP_STRDUP(pRepeat->pMediUrl, pMediaUrl);

	pRepeat->pPopup =
		vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
				VP_PLAY_STRING_POPUP_REPEAT_SETTING, NULL, 0.0,
				NULL, __vp_repeat_popup_key_event_cb,
				__vp_repeat_popup_mouse_event_cb,
				(void *) pRepeat);

	if (pRepeat->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_repeat_destroy_handle(pRepeat);
		return NULL;
	}
	pRepeat->pGenList = _vp_repeat_create_genlist(pRepeat->pPopup);
	if (pRepeat->pGenList == NULL) {
		VideoLogError("_vp_repeat_create_genlist fail");
		_vp_repeat_destroy_handle(pRepeat);
		return NULL;
	}
	evas_object_data_set(pRepeat->pGenList, VP_REPEAT_GENLIST_DATA_KEY,
			     (void *) pRepeat);

	pRepeat->pRadio = elm_radio_add(pRepeat->pGenList);

	if (!_vp_repeat_add_genlist_item(pRepeat->pGenList, (void *) pRepeat)) {
		VideoLogError("_vp_repeat_add_genlist_item fail");
		_vp_repeat_destroy_handle(pRepeat);
		return NULL;
	}
	evas_object_smart_callback_add(pRepeat->pGenList, "realized",
				       __vp_repeat_genlist_realized, NULL);
	evas_object_smart_callback_add(pRepeat->pParent, "rotation,changed",
				       __vp_repeat_popup_rotate_cb, pRepeat);

	int nType = 0;

	vp_play_preference_get_repeat_mode_key(&nType);

	//elm_radio_value_set(pRepeat->pRadio, nType);
	pRepeat->repeatMode = nType;

	pRepeat->pBox = elm_box_add(pRepeat->pPopup);
	vp_popup_set_popup_min_size(pRepeat->pParent, pRepeat->pBox,
				    pRepeat->nListCount, VIDEO_POPUP_DEFAULT);

	elm_box_pack_end(pRepeat->pBox, pRepeat->pGenList);

	elm_object_content_set(pRepeat->pPopup, pRepeat->pBox);

	return pRepeat;
}

void vp_repeat_destroy(repeat_handle pRepeatHandle)
{
	if (pRepeatHandle == NULL) {
		VideoLogError("pRepeatHandle is NULL");
		return;
	}

	RepeatPopup *pRepeat = (RepeatPopup *) pRepeatHandle;

	_vp_repeat_destroy_handle(pRepeat);

}

bool vp_repeat_realize(repeat_handle pRepeatHandle)
{
	if (pRepeatHandle == NULL) {
		VideoLogError("pRepeatHandle is NULL");
		return FALSE;
	}

	RepeatPopup *pRepeat = (RepeatPopup *) pRepeatHandle;

	evas_object_show(pRepeat->pPopup);

	return TRUE;
}

bool vp_repeat_unrealize(repeat_handle pRepeatHandle)
{
	if (pRepeatHandle == NULL) {
		VideoLogError("pRepeatHandle is NULL");
		return FALSE;
	}

	RepeatPopup *pRepeat = (RepeatPopup *) pRepeatHandle;

	evas_object_hide(pRepeat->pPopup);

	return TRUE;
}

bool vp_repeat_set_user_data(repeat_handle pRepeatHandle, void *pUserData)
{
	if (pRepeatHandle == NULL) {
		VideoLogError("pRepeatHandle is NULL");
		return FALSE;
	}

	RepeatPopup *pRepeat = (RepeatPopup *) pRepeatHandle;

	pRepeat->pUserData = pUserData;

	return TRUE;
}
