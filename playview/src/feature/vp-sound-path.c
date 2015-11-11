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

#include "vp-sound.h"
#include "vp-sound-path.h"
#include <glib.h>


/* check temp */
#include "vp-play-log.h"

#define VP_SOUND_PATH_GENLIST_DATA_KEY		"vp.sound.path.genlist"

typedef struct _SoundPathPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_Sp_Itc;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;

	char *szBTName;

	int nItemCnt;
	video_sound_path_t soundPathType;
} SoundPathPopup;

static void __vp_sound_path_genlist_item_selected_cb(void *pUserData,
        Evas_Object *
        pObject,
        void *pEventInfo);

static void __vp_sound_genlist_realized(void *data, Evas_Object *obj,
                                        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_sound_path_popup_rotate_cb(void *data, Evas_Object *obj,
        void *event_info)
{
	SoundPathPopup *pSoundPath = (SoundPathPopup *) data;
	if (!pSoundPath) {
		VideoLogError("pSoundPath IS null");
		return;
	}
	if (pSoundPath->nItemCnt < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSoundPath->pParent, pSoundPath->pBox,
	                            pSoundPath->nItemCnt,
	                            VIDEO_POPUP_DEFAULT);

}

static void _vp_sound_path_destroy_handle(SoundPathPopup *pSoundPath)
{
	if (pSoundPath == NULL) {
		VideoLogError("pSoundPath is NULL");
		return;
	}
	evas_object_smart_callback_del(pSoundPath->pParent,
	                               "rotation,changed",
	                               __vp_sound_path_popup_rotate_cb);
	evas_object_smart_callback_del(pSoundPath->pGenList, "realized",
	                               __vp_sound_genlist_realized);

	VP_EVAS_DEL(pSoundPath->pRadio);
	VP_EVAS_DEL(pSoundPath->pGenList);
	VP_EVAS_DEL(pSoundPath->pButton);
	VP_EVAS_DEL(pSoundPath->pBox);

	if (pSoundPath->st_Sp_Itc) {
		elm_genlist_item_class_free(pSoundPath->st_Sp_Itc);
		pSoundPath->st_Sp_Itc = NULL;
	}


	VP_EVAS_DEL(pSoundPath->pPopup);

	VP_FREE(pSoundPath->szBTName);

	VP_FREE(pSoundPath);
}

/* callback functions */
static char *__vp_sound_path_genlist_text_get_cb(const void *pUserData,
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

static Evas_Object *__vp_sound_path_genlist_content_get_cb(const void
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

	if (!strcmp(pPart, "elm.icon.2")) {
		Evas_Object *pRadioObj = NULL;
		Evas_Object *pLayout = NULL;

		SoundPathPopup *pSoundPath =
		    (SoundPathPopup *) evas_object_data_get(pObj,
		            VP_SOUND_PATH_GENLIST_DATA_KEY);
		if (pSoundPath == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		video_sound_path_t nType = VIDEO_SOUND_PATH_NONE;

		if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_SPEAKER)) {
			nType = VIDEO_SOUND_PATH_SPEAKER;
		} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_EARJACK)) {
			nType = VIDEO_SOUND_PATH_EARJACK;
		} else if (!g_strcmp0(szTxt, pSoundPath->szBTName)) {
			nType = VIDEO_SOUND_PATH_BLUETOOTH;
		} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_HDMI)) {
			nType = VIDEO_SOUND_PATH_HDMI;
		} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_MIRRORING)) {
			nType = VIDEO_SOUND_PATH_MIRRORING;
		} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_USB_AUDIO)) {
			nType = VIDEO_SOUND_PATH_USB_AUDIO;
		}

		pLayout = elm_layout_add(pObj);
		pRadioObj = elm_radio_add(pLayout);

		elm_layout_theme_set(pLayout, "layout", "list/C/type.2",
		                     "default");
		elm_radio_group_add(pRadioObj, pSoundPath->pRadio);
		elm_radio_state_value_set(pRadioObj, nType);

		if (nType == pSoundPath->soundPathType) {
			VideoLogInfo("pSoundPath->soundPathType = %d",
			             pSoundPath->soundPathType);
			elm_radio_value_set(pSoundPath->pRadio, nType);
		}

		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_sound_path_genlist_item_selected_cb,
		                               (void *) pUserData);
		elm_object_part_content_set(pLayout, "elm.swallow.content",
		                            pRadioObj);
		evas_object_show(pRadioObj);
		evas_object_show(pLayout);

		return pLayout;
	}
	return NULL;
}

static void __vp_sound_path_genlist_item_selected_cb(void *pUserData,
        Evas_Object *
        pObject,
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
	char *szTxt = (char *) elm_object_item_data_get(pItem);

	SoundPathPopup *pSoundPath = (SoundPathPopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	video_sound_path_t nType = VIDEO_SOUND_PATH_NONE;

	if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_SPEAKER)) {
		VideoLogWarning
		("SELECT SOUND PATH ITEM (VP_PLAY_STRING_SOUND_SPEAKER)");
		nType = VIDEO_SOUND_PATH_SPEAKER;
		vp_sound_set_active_route(VP_SOUND_DEVICE_SPEAKER);
	} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_EARJACK)) {
		VideoLogWarning
		("SELECT SOUND PATH ITEM (VP_PLAY_STRING_SOUND_EARJACK)");
		nType = VIDEO_SOUND_PATH_EARJACK;
		vp_sound_set_active_route(VP_SOUND_DEVICE_EARJACK);
	} else if (!g_strcmp0(szTxt, pSoundPath->szBTName)) {
		nType = VIDEO_SOUND_PATH_BLUETOOTH;
		VideoLogWarning
		("SELECT SOUND PATH ITEM (VIDEO_SOUND_PATH_BLUETOOTH)");
		vp_sound_set_active_route(VP_SOUND_DEVICE_BLUETOOTH);
	} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_HDMI)) {
		VideoLogWarning("SELECT SOUND PATH ITEM (VIDEO_SOUND_PATH_HDMI)");
		nType = VIDEO_SOUND_PATH_HDMI;
		vp_sound_set_active_route(VP_SOUND_DEVICE_HDMI);
	} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_MIRRORING)) {
		VideoLogWarning
		("SELECT SOUND PATH ITEM (VIDEO_SOUND_PATH_MIRRORING)");
		nType = VIDEO_SOUND_PATH_MIRRORING;
		vp_sound_set_active_route(VP_SOUND_DEVICE_MIRRORING);
	} else if (!g_strcmp0(szTxt, VP_PLAY_STRING_SOUND_USB_AUDIO)) {
		VideoLogWarning
		("SELECT SOUND PATH ITEM (VIDEO_SOUND_PATH_USB_AUDIO)");
		nType = VIDEO_SOUND_PATH_USB_AUDIO;
		vp_sound_set_active_route(VP_SOUND_DEVICE_USB_AUDIO);
	}

	pSoundPath->soundPathType = nType;
	if (pSoundPath->pCloseCb) {
		pSoundPath->pCloseCb((int) nType, FALSE,
		                     (void *) pSoundPath->pUserData);
	}
}

static void __vp_sound_path_popup_key_event_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SoundPathPopup *pSoundPath = (SoundPathPopup *) pUserData;

	if (pSoundPath->pCloseCb) {
		pSoundPath->pCloseCb(VIDEO_SOUND_PATH_NONE, FALSE,
		                     (void *) pSoundPath->pUserData);
	}
}

static void __vp_sound_path_popup_mouse_event_cb(void *pUserData,
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
		SoundPathPopup *pSoundPath = (SoundPathPopup *) pUserData;

		if (pSoundPath->pCloseCb) {
			pSoundPath->pCloseCb(VIDEO_SOUND_PATH_NONE, FALSE,
			                     (void *) pSoundPath->pUserData);
		}
	}
}

static Evas_Object *_vp_sound_path_create_genlist(Evas_Object *pParent)
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


static bool _vp_sound_path_add_genlist_item(Evas_Object *pObj,
        void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SoundPathPopup *pSoundPath = (SoundPathPopup *) pUserData;

	if (pSoundPath->st_Sp_Itc) {
		elm_genlist_item_class_free(pSoundPath->st_Sp_Itc);
		pSoundPath->st_Sp_Itc = NULL;
	}

	pSoundPath->st_Sp_Itc = elm_genlist_item_class_new();

	if (pSoundPath->st_Sp_Itc != NULL) {
		pSoundPath->st_Sp_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSoundPath->st_Sp_Itc->item_style = "1line";
		pSoundPath->st_Sp_Itc->func.text_get =
		    (void *) __vp_sound_path_genlist_text_get_cb;
		pSoundPath->st_Sp_Itc->func.content_get =
		    (void *) __vp_sound_path_genlist_content_get_cb;
		pSoundPath->st_Sp_Itc->func.state_get = NULL;
		pSoundPath->st_Sp_Itc->func.del = NULL;
	}

	if (vp_sound_device_is_enable(VP_SOUND_DEVICE_BLUETOOTH, pSoundPath)) {
		char *szBTName = NULL;
		if (vp_sound_get_a2dp_name(&szBTName)) {
			VP_STRDUP(pSoundPath->szBTName, szBTName);
			VP_FREE(szBTName);
			elm_genlist_item_append(pObj, pSoundPath->st_Sp_Itc,
			                        (void *) pSoundPath->szBTName, NULL,
			                        ELM_GENLIST_ITEM_NONE,
			                        __vp_sound_path_genlist_item_selected_cb,
			                        pUserData);
			pSoundPath->nItemCnt++;
		}
	}

	if (vp_sound_device_is_enable(VP_SOUND_DEVICE_HDMI, pSoundPath)) {
		elm_genlist_item_append(pObj, pSoundPath->st_Sp_Itc,
		                        (void *) VP_PLAY_STRING_SOUND_HDMI, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_sound_path_genlist_item_selected_cb,
		                        pUserData);
		pSoundPath->nItemCnt++;
	}

	if (vp_sound_device_is_enable(VP_SOUND_DEVICE_MIRRORING, pSoundPath)) {
		elm_genlist_item_append(pObj, pSoundPath->st_Sp_Itc,
		                        (void *) VP_PLAY_STRING_SOUND_MIRRORING,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_sound_path_genlist_item_selected_cb,
		                        pUserData);
		pSoundPath->nItemCnt++;
	}

	if (vp_sound_device_is_enable(VP_SOUND_DEVICE_USB_AUDIO, pSoundPath)) {
		elm_genlist_item_append(pObj, pSoundPath->st_Sp_Itc,
		                        (void *) VP_PLAY_STRING_SOUND_USB_AUDIO,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_sound_path_genlist_item_selected_cb,
		                        pUserData);
		pSoundPath->nItemCnt++;
	}

	if (vp_sound_device_is_enable(VP_SOUND_DEVICE_EARJACK, pSoundPath)) {
		elm_genlist_item_append(pObj, pSoundPath->st_Sp_Itc,
		                        (void *) VP_PLAY_STRING_SOUND_EARJACK,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_sound_path_genlist_item_selected_cb,
		                        pUserData);
		pSoundPath->nItemCnt++;
	}

	if (vp_sound_device_is_enable(VP_SOUND_DEVICE_EARJACK, pSoundPath)
	        && vp_sound_device_is_enable(VP_SOUND_DEVICE_BLUETOOTH,
	                                     pSoundPath)) {
		VideoLogError("Skip for sound policy");
	} else {
		elm_genlist_item_append(pObj, pSoundPath->st_Sp_Itc,
		                        (void *) VP_PLAY_STRING_SOUND_SPEAKER,
		                        NULL, ELM_GENLIST_ITEM_NONE,
		                        __vp_sound_path_genlist_item_selected_cb,
		                        pUserData);
		pSoundPath->nItemCnt++;
	}

	return TRUE;
}

/* external functions */
sound_path_handle vp_sound_path_create(Evas_Object *pParent,
                                       PopupCloseCbFunc pCloseCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}


	SoundPathPopup *pSoundPath = NULL;

	pSoundPath = calloc(1, sizeof(SoundPathPopup));

	if (pSoundPath == NULL) {
		VideoLogError("pSoundPath alloc fail");
		return NULL;
	}

	pSoundPath->pParent = pParent;
	pSoundPath->pCloseCb = pCloseCb;

	pSoundPath->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SOUND_PATH, NULL, 0.0, NULL,
	                    __vp_sound_path_popup_key_event_cb,
	                    __vp_sound_path_popup_mouse_event_cb,
	                    (void *) pSoundPath);
	if (pSoundPath->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_sound_path_destroy_handle(pSoundPath);
		return NULL;
	}

	pSoundPath->pGenList =
	    _vp_sound_path_create_genlist(pSoundPath->pPopup);
	if (pSoundPath->pGenList == NULL) {
		VideoLogError("_vp_sound_path_create_genlist fail");
		_vp_sound_path_destroy_handle(pSoundPath);
		return NULL;
	}
	evas_object_smart_callback_add(pSoundPath->pGenList, "realized",
	                               __vp_sound_genlist_realized, NULL);

	evas_object_data_set(pSoundPath->pGenList,
	                     VP_SOUND_PATH_GENLIST_DATA_KEY,
	                     (void *) pSoundPath);

	pSoundPath->pRadio = elm_radio_add(pSoundPath->pGenList);

	if (!_vp_sound_path_add_genlist_item
	        (pSoundPath->pGenList, (void *) pSoundPath)) {
		VideoLogError("_vp_sound_path_add_genlist_item fail");
		_vp_sound_path_destroy_handle(pSoundPath);
		return NULL;
	}

	video_sound_device_type_t nType = VP_SOUND_DEVICE_NONE;

	vp_sound_get_active_device(&nType);
	if (nType != VP_SOUND_DEVICE_NONE) {
		//elm_radio_value_set(pSoundPath->pRadio, nType);
		pSoundPath->soundPathType = nType;
	}
	evas_object_smart_callback_add(pSoundPath->pParent,
	                               "rotation,changed",
	                               __vp_sound_path_popup_rotate_cb,
	                               pSoundPath);

	pSoundPath->pBox = elm_box_add(pSoundPath->pPopup);

	vp_popup_set_popup_min_size(pSoundPath->pParent, pSoundPath->pBox,
	                            pSoundPath->nItemCnt,
	                            VIDEO_POPUP_DEFAULT);


	elm_box_pack_end(pSoundPath->pBox, pSoundPath->pGenList);

	elm_object_content_set(pSoundPath->pPopup, pSoundPath->pBox);

	return pSoundPath;
}

void vp_sound_path_destroy(sound_path_handle pSoundPathHandle)
{
	if (pSoundPathHandle == NULL) {
		VideoLogError("pSoundPathHandle is NULL");
		return;
	}

	SoundPathPopup *pSoundPath = (SoundPathPopup *) pSoundPathHandle;

	_vp_sound_path_destroy_handle(pSoundPath);

}

bool vp_sound_path_realize(sound_path_handle pSoundPathHandle)
{
	if (pSoundPathHandle == NULL) {
		VideoLogError("pSoundPathHandle is NULL");
		return FALSE;
	}

	SoundPathPopup *pSoundPath = (SoundPathPopup *) pSoundPathHandle;

	evas_object_show(pSoundPath->pPopup);

	return TRUE;
}

bool vp_sound_path_unrealize(sound_path_handle pSoundPathHandle)
{
	if (pSoundPathHandle == NULL) {
		VideoLogError("pSoundPathHandle is NULL");
		return FALSE;
	}

	SoundPathPopup *pSoundPath = (SoundPathPopup *) pSoundPathHandle;

	evas_object_hide(pSoundPath->pPopup);

	return TRUE;
}

bool vp_sound_path_set_user_data(sound_path_handle pSoundPathHandle,
                                 void *pUserData)
{
	if (pSoundPathHandle == NULL) {
		VideoLogError("pSoundPathHandle is NULL");
		return FALSE;
	}

	SoundPathPopup *pSoundPath = (SoundPathPopup *) pSoundPathHandle;

	pSoundPath->pUserData = pUserData;

	return TRUE;
}
