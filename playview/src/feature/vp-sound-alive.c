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

#include "vp-sound-alive.h"
#include "vp-sound.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SOUND_ALIVE_GENLIST_DATA_KEY		"vp.sound.alive.genlist"

typedef struct _SoundAlivePopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_Sa_Itc;

	char *pMediUrl;
	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;
	video_sound_alive_t soundAliveType;
} SoundAlivePopup;

static void __vp_sound_alive_genlist_item_selected_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo);

static void __vp_sound_alive_genlist_realized(void *data,
		Evas_Object *obj,
		void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_sound_alive_popup_rotate_cb(void *data,
		Evas_Object *obj,
		void *event_info)
{
	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) data;
	if (!pSoundAlive) {
		VideoLogError("pSoundAlive IS null");
		return;
	}
	if (pSoundAlive->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSoundAlive->pParent, pSoundAlive->pBox,
				    pSoundAlive->nListCount,
				    VIDEO_POPUP_DEFAULT);

}

static void _vp_sound_alive_destroy_handle(SoundAlivePopup *pSoundAlive)
{
	if (pSoundAlive == NULL) {
		VideoLogError("pSoundAlive is NULL");
		return;
	}

	evas_object_smart_callback_del(pSoundAlive->pParent,
				       "rotation,changed",
				       __vp_sound_alive_popup_rotate_cb);
	evas_object_smart_callback_del(pSoundAlive->pGenList, "realized",
				       __vp_sound_alive_genlist_realized);

	VP_EVAS_DEL(pSoundAlive->pRadio);
	VP_EVAS_DEL(pSoundAlive->pGenList);
	VP_EVAS_DEL(pSoundAlive->pButton);
	VP_EVAS_DEL(pSoundAlive->pBox);

	if (pSoundAlive->st_Sa_Itc) {
		elm_genlist_item_class_free(pSoundAlive->st_Sa_Itc);
		pSoundAlive->st_Sa_Itc = NULL;
	}


	VP_EVAS_DEL(pSoundAlive->pPopup);

	VP_FREE(pSoundAlive->pMediUrl);

	VP_FREE(pSoundAlive);
}

/* callback functions */
static char *__vp_sound_alive_genlist_text_get_cb(const void *pUserData,
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

static Evas_Object *__vp_sound_alive_genlist_content_get_cb(const void
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

	if (!strcmp(pPart, "elm.icon.right")) {
		Evas_Object *pRadioObj = NULL;

		SoundAlivePopup *pSoundAlive =
			(SoundAlivePopup *) evas_object_data_get(pObj,
					VP_SOUND_ALIVE_GENLIST_DATA_KEY);
		if (pSoundAlive == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		video_sound_alive_t nType = VIDEO_SA_NONE;

		if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_NORMAL)) {
			nType = VIDEO_SA_NORMAL;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_VOICE)) {
			nType = VIDEO_SA_VOICE;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_MOVIE)) {
			nType = VIDEO_SA_MOVIE;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_71CH)) {
			nType = VIDEO_SA_7_1_CH;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nType);
		elm_radio_group_add(pRadioObj, pSoundAlive->pRadio);
		elm_radio_value_set(pSoundAlive->pRadio,
				    pSoundAlive->soundAliveType);
		evas_object_smart_callback_add(pRadioObj, "changed",
					       __vp_sound_alive_genlist_item_selected_cb,
					       pSoundAlive);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_sound_alive_genlist_item_selected_cb(void *pUserData,
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
	Elm_Object_Item *pSelectedItem =
		elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	char *szTxt = (char *) elm_object_item_data_get(pItem);
	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	video_sound_alive_t nType = VIDEO_SA_NONE;

	if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_NORMAL)) {
		VideoLogWarning
		("SELECT SOUND ALIVE ITEM (VP_PLAY_STRING_SOUND_ALIVE_NORMAL)");
		nType = VIDEO_SA_NORMAL;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_VOICE)) {
		VideoLogWarning
		("SELECT SOUND ALIVE ITEM (VP_PLAY_STRING_SOUND_ALIVE_VOICE)");
		nType = VIDEO_SA_VOICE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_MOVIE)) {
		VideoLogWarning
		("SELECT SOUND ALIVE ITEM (VP_PLAY_STRING_SOUND_ALIVE_MOVIE)");
		nType = VIDEO_SA_MOVIE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SOUND_ALIVE_71CH)) {
		VideoLogWarning
		("SELECT SOUND ALIVE ITEM (VP_PLAY_STRING_SOUND_ALIVE_71CH)");
		nType = VIDEO_SA_7_1_CH;
	}

	pSoundAlive->soundAliveType = nType;
	if (pSoundAlive->pCloseCb) {
		pSoundAlive->pCloseCb((int) nType, FALSE,
				      (void *) pSoundAlive->pUserData);
	}
}

static void __vp_sound_alive_popup_key_event_cb(void *pUserData,
		Evas_Object *pObj,
		void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pUserData;

	if (pSoundAlive->pCloseCb) {
		pSoundAlive->pCloseCb(VIDEO_SA_NONE, FALSE,
				      (void *) pSoundAlive->pUserData);
	}
}

static void __vp_sound_alive_popup_mouse_event_cb(void *pUserData,
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
		SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pUserData;

		if (pSoundAlive->pCloseCb) {
			pSoundAlive->pCloseCb(VIDEO_SA_NONE, FALSE,
					      (void *) pSoundAlive->pUserData);
		}
	}
}

static Evas_Object *_vp_sound_alive_create_genlist(Evas_Object *pParent)
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


static bool _vp_sound_alive_add_genlist_item(Evas_Object *pObj,
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

	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pUserData;

	if (pSoundAlive->st_Sa_Itc) {
		elm_genlist_item_class_free(pSoundAlive->st_Sa_Itc);
		pSoundAlive->st_Sa_Itc = NULL;
	}

	pSoundAlive->st_Sa_Itc = elm_genlist_item_class_new();
	
	if (pSoundAlive->st_Sa_Itc != NULL) {
		pSoundAlive->st_Sa_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSoundAlive->st_Sa_Itc->item_style = "1line";
		pSoundAlive->st_Sa_Itc->func.text_get =
			(void *) __vp_sound_alive_genlist_text_get_cb;
		pSoundAlive->st_Sa_Itc->func.content_get =
			(void *) __vp_sound_alive_genlist_content_get_cb;
		pSoundAlive->st_Sa_Itc->func.state_get = NULL;
		pSoundAlive->st_Sa_Itc->func.del = NULL;
	}

	pSoundAlive->nListCount = 0;

	elm_genlist_item_append(pObj, pSoundAlive->st_Sa_Itc,
				(void *) VP_PLAY_STRING_SOUND_ALIVE_NORMAL,
				NULL, ELM_GENLIST_ITEM_NONE,
				__vp_sound_alive_genlist_item_selected_cb,
				pUserData);
	pSoundAlive->nListCount++;

	video_sound_device_type_t nSoundDevType = VP_SOUND_DEVICE_NONE;
	vp_sound_get_active_device(&nSoundDevType);
	if (nSoundDevType == VP_SOUND_DEVICE_SPEAKER) {
		elm_genlist_item_append(pObj, pSoundAlive->st_Sa_Itc,
					(void *) VP_PLAY_STRING_SOUND_ALIVE_VOICE,
					NULL, ELM_GENLIST_ITEM_NONE,
					__vp_sound_alive_genlist_item_selected_cb,
					pUserData);
		pSoundAlive->nListCount++;
	} else if (nSoundDevType == VP_SOUND_DEVICE_HDMI
			|| nSoundDevType == VP_SOUND_DEVICE_BLUETOOTH) {
	} else {
		elm_genlist_item_append(pObj, pSoundAlive->st_Sa_Itc,
					(void *) VP_PLAY_STRING_SOUND_ALIVE_VOICE,
					NULL, ELM_GENLIST_ITEM_NONE,
					__vp_sound_alive_genlist_item_selected_cb,
					pUserData);
		pSoundAlive->nListCount++;
		elm_genlist_item_append(pObj, pSoundAlive->st_Sa_Itc,
					(void *) VP_PLAY_STRING_SOUND_ALIVE_MOVIE,
					NULL, ELM_GENLIST_ITEM_NONE,
					__vp_sound_alive_genlist_item_selected_cb,
					pUserData);
		pSoundAlive->nListCount++;
		elm_genlist_item_append(pObj, pSoundAlive->st_Sa_Itc,
					(void *) VP_PLAY_STRING_SOUND_ALIVE_71CH,
					NULL, ELM_GENLIST_ITEM_NONE,
					__vp_sound_alive_genlist_item_selected_cb,
					pUserData);
		pSoundAlive->nListCount++;
	}

	return TRUE;
}

/* external functions */
sound_alive_handle vp_sound_alive_create(Evas_Object *pParent,
		char *pMediaUrl,
		PopupCloseCbFunc pCloseCb,
		video_sound_alive_t nType)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (pMediaUrl == NULL) {
		VideoLogError("pMediaUrl is NULL");
		return NULL;
	}

	SoundAlivePopup *pSoundAlive = NULL;

	pSoundAlive = calloc(1, sizeof(SoundAlivePopup));

	if (pSoundAlive == NULL) {
		VideoLogError("pSoundAlive alloc fail");
		return NULL;
	}

	pSoundAlive->pParent = pParent;
	pSoundAlive->pCloseCb = pCloseCb;

	VP_STRDUP(pSoundAlive->pMediUrl, pMediaUrl);

	pSoundAlive->pPopup =
		vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
				VP_PLAY_STRING_POPUP_SOUND_ALIVE, NULL, 0.0, NULL,
				__vp_sound_alive_popup_key_event_cb,
				__vp_sound_alive_popup_mouse_event_cb,
				(void *) pSoundAlive);
	if (pSoundAlive->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_sound_alive_destroy_handle(pSoundAlive);
		return NULL;
	}

	pSoundAlive->pGenList =
		_vp_sound_alive_create_genlist(pSoundAlive->pPopup);
	if (pSoundAlive->pGenList == NULL) {
		VideoLogError("_vp_sound_alive_create_genlist fail");
		_vp_sound_alive_destroy_handle(pSoundAlive);
		return NULL;
	}
	evas_object_data_set(pSoundAlive->pGenList,
			     VP_SOUND_ALIVE_GENLIST_DATA_KEY,
			     (void *) pSoundAlive);
	evas_object_smart_callback_add(pSoundAlive->pGenList, "realized",
				       __vp_sound_alive_genlist_realized,
				       NULL);

	pSoundAlive->pRadio = elm_radio_add(pSoundAlive->pGenList);

	if (!_vp_sound_alive_add_genlist_item
			(pSoundAlive->pGenList, (void *) pSoundAlive)) {
		VideoLogError("_vp_sound_alive_add_genlist_item fail");
		_vp_sound_alive_destroy_handle(pSoundAlive);
		return NULL;
	}

	VideoLogInfo("nType: %d", nType);
	pSoundAlive->soundAliveType = nType;
	evas_object_smart_callback_add(pSoundAlive->pParent,
				       "rotation,changed",
				       __vp_sound_alive_popup_rotate_cb,
				       pSoundAlive);

	pSoundAlive->pBox = elm_box_add(pSoundAlive->pPopup);

	vp_popup_set_popup_min_size(pSoundAlive->pParent, pSoundAlive->pBox,
				    pSoundAlive->nListCount,
				    VIDEO_POPUP_DEFAULT);

	elm_box_pack_end(pSoundAlive->pBox, pSoundAlive->pGenList);

	elm_object_content_set(pSoundAlive->pPopup, pSoundAlive->pBox);

	return pSoundAlive;
}

void vp_sound_alive_destroy(sound_alive_handle pSAHandle)
{
	if (pSAHandle == NULL) {
		VideoLogError("pSAHandle is NULL");
		return;
	}

	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pSAHandle;

	_vp_sound_alive_destroy_handle(pSoundAlive);

}

bool vp_sound_alive_realize(sound_alive_handle pSAHandle)
{
	if (pSAHandle == NULL) {
		VideoLogError("pSAHandle is NULL");
		return FALSE;
	}

	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pSAHandle;

	evas_object_show(pSoundAlive->pPopup);

	return TRUE;
}

bool vp_sound_alive_unrealize(sound_alive_handle pSAHandle)
{
	if (pSAHandle == NULL) {
		VideoLogError("pSAHandle is NULL");
		return FALSE;
	}

	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pSAHandle;

	evas_object_hide(pSoundAlive->pPopup);

	return TRUE;
}

bool vp_sound_alive_set_user_data(sound_alive_handle pSAHandle,
				  void *pUserData)
{
	if (pSAHandle == NULL) {
		VideoLogError("pSAHandle is NULL");
		return FALSE;
	}

	SoundAlivePopup *pSoundAlive = (SoundAlivePopup *) pSAHandle;

	pSoundAlive->pUserData = pUserData;

	return TRUE;
}
