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

#include "vp-audio-track.h"

/* check temp */
#include "vp-play-log.h"

#define VP_AUDIO_TRACK_GENLIST_DATA_KEY		"vp.audio.track.genlist"

typedef struct _RegionCode {
	char *szCode;
	char *szName;
} RegionCode;


static RegionCode st_RegionCode[] = {
	{"az", "Azerbaijani"},
	{"ca", "Catalan"},
	{"cs", "Czech"},
	{"sr", "Serbian"},
	{"da", "Danish"},
	{"de", "German"},
	{"et", "Estonian"},
	{"en", "English"},
	{"es", "Spanish"},
	{"eu", "Basque"},
	{"fr", "French"},
	{"gl", "Galician"},
	{"hr", "Croatian"},
	{"is", "Icelandic"},
	{"it", "Italian"},
	{"lv", "Latvian"},
	{"lt", "Lithuanian"},
	{"hu", "Hungarian"},
	{"nl", "Dutch"},
	{"nb", "Norwegian"},
	{"uz", "Uzbek"},
	{"pl", "Polish"},
	{"pt", "Portuguese"},
	{"ru", "Russian"},
	{"ro", "Romanian"},
	{"sk", "Slovak"},
	{"sl", "Slovenian"},
	{"fi", "Finnish"},
	{"sv", "Swedish"},
	{"tr", "Turkish"},
	{"el", "Greek"},
	{"bg", "Bulgarian"},
	{"kk", "Kazakh"},
	{"uk", "Ukrainian"},
	{"hy", "Armenian"},
	{"ka", "Georgian"},
	{"ko", "Korean"},
	{"ja", "Japanese"},
};


typedef struct _AudioTrackItem {
	Elm_Object_Item *pItem;
	char *szName;
	int nIndex;
} AudioTrackItem;


typedef struct _AudioTrackPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_AudioTrack_Itc;

	int nDefaultIndex;
	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;

	GList *pItemList;
	int nAudoTrackIdx;
} AudioTrackPopup;



static void _vp_audio_track_destroy_handle(AudioTrackPopup *pAudioTrack);
static void __vp_audio_track_popup_rotate_cb(void *data,
        Evas_Object *obj,
        void *event_info);
static void __vp_audio_track_genlist_realized(void *data,
        Evas_Object *obj,
        void *event_info);
static void __vp_audio_track_genlist_item_selected_cb(void *pUserData,
        Evas_Object *
        pObject,
        void *pEventInfo);


/* callback functions */
static char *__vp_audio_track_genlist_text_get_cb(const void *pUserData,
        Evas_Object *pObj,
        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}
	AudioTrackItem *pItem = (AudioTrackItem *) pUserData;

	if (!strcmp(pPart, "elm.text")) {
		if (pItem->szName) {
			return strdup(pItem->szName);
		}
	}

	return NULL;
}

static Evas_Object *__vp_audio_track_genlist_content_get_cb(const void
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

	AudioTrackItem *pItem = (AudioTrackItem *) pUserData;

	if (!strcmp(pPart, "elm.icon")) {
		Evas_Object *pRadioObj = NULL;

		AudioTrackPopup *pAudioTrack =
		    (AudioTrackPopup *) evas_object_data_get(pObj,
		            VP_AUDIO_TRACK_GENLIST_DATA_KEY);
		if (pAudioTrack == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		if (pAudioTrack->pItemList == NULL) {
			VideoLogWarning("pItemList is NULL");
			return NULL;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, pItem->nIndex);
		elm_radio_group_add(pRadioObj, pAudioTrack->pRadio);
		elm_radio_value_set(pAudioTrack->pRadio,
		                    pAudioTrack->nAudoTrackIdx);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_audio_track_genlist_item_selected_cb,
		                               pAudioTrack);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_audio_track_genlist_item_selected_cb(void *pUserData,
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

	Elm_Object_Item *pElmItem = (Elm_Object_Item *) pEventInfo;
	Elm_Object_Item *pSelectedItem =
	    elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	AudioTrackItem *pItem =
	    (AudioTrackItem *) elm_object_item_data_get(pElmItem);

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pUserData;

	if (pItem == NULL) {
		VideoLogError("pItem is NULL");
		return;
	}

	pAudioTrack->nAudoTrackIdx = pItem->nIndex;
	if (pAudioTrack->pCloseCb) {
		pAudioTrack->pCloseCb(pItem->nIndex, FALSE,
		                      (void *) pAudioTrack->pUserData);
	}
}

static void __vp_audio_track_popup_key_event_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pUserData;

	if (pAudioTrack->pCloseCb) {
		pAudioTrack->pCloseCb(-1, FALSE, (void *) pAudioTrack->pUserData);
	}
}

static void __vp_audio_track_popup_mouse_event_cb(void *pUserData,
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
		AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pUserData;

		if (pAudioTrack->pCloseCb) {
			pAudioTrack->pCloseCb(-1, FALSE,
			                      (void *) pAudioTrack->pUserData);
		}
	}
}



/* internal functions */
static void _vp_audio_track_clear_item_list(AudioTrackPopup *pAudioTrack)
{
	if (pAudioTrack == NULL) {
		VideoLogError("pAudioTrack is NULL");
		return;
	}

	if (pAudioTrack->pItemList == NULL) {
		VideoLogError("pAudioTrack->pItemList is NULL");
		return;
	}


	int len = g_list_length(pAudioTrack->pItemList);
	int idx = 0;
	for (idx = 0; idx < len; idx++) {
		AudioTrackItem *pItem = NULL;

		pItem =
		    (AudioTrackItem *) g_list_nth_data(pAudioTrack->pItemList,
		                                       idx);
		if (pItem) {
			VP_EVAS_ITEM_DEL(pItem->pItem);
			VP_FREE(pItem->szName);
			VP_FREE(pItem);
		}
	}

	g_list_free(pAudioTrack->pItemList);
	pAudioTrack->pItemList = NULL;
}

static void _vp_audio_track_destroy_handle(AudioTrackPopup *pAudioTrack)
{
	if (pAudioTrack == NULL) {
		VideoLogError("pAudioTrack is NULL");
		return;
	}

	evas_object_smart_callback_del(pAudioTrack->pGenList, "realized",
	                               __vp_audio_track_genlist_realized);
	evas_object_smart_callback_del(pAudioTrack->pParent,
	                               "rotation,changed",
	                               __vp_audio_track_popup_rotate_cb);

	_vp_audio_track_clear_item_list(pAudioTrack);


	VP_EVAS_DEL(pAudioTrack->pRadio);
	VP_EVAS_DEL(pAudioTrack->pGenList);
	VP_EVAS_DEL(pAudioTrack->pButton);
	VP_EVAS_DEL(pAudioTrack->pBox);

	if (pAudioTrack->st_AudioTrack_Itc) {
		elm_genlist_item_class_free(pAudioTrack->st_AudioTrack_Itc);
		pAudioTrack->st_AudioTrack_Itc = NULL;
	}


	VP_EVAS_DEL(pAudioTrack->pPopup);

	VP_FREE(pAudioTrack);
}

static Evas_Object *_vp_audio_track_create_genlist(Evas_Object *pParent)
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


static bool _vp_audio_track_add_genlist_item(Evas_Object *pObj,
        void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pUserData;

	if (pAudioTrack->st_AudioTrack_Itc) {
		elm_genlist_item_class_free(pAudioTrack->st_AudioTrack_Itc);
		pAudioTrack->st_AudioTrack_Itc = NULL;
	}

	pAudioTrack->st_AudioTrack_Itc = elm_genlist_item_class_new();
	if (pAudioTrack->st_AudioTrack_Itc == NULL) {
		VideoLogError("failed to create genlist class");
		return FALSE;
	}

	pAudioTrack->st_AudioTrack_Itc->version =
	    ELM_GENLIST_ITEM_CLASS_VERSION;
	pAudioTrack->st_AudioTrack_Itc->item_style = "1text.1icon.3/popup";
	pAudioTrack->st_AudioTrack_Itc->func.text_get =
	    (void *) __vp_audio_track_genlist_text_get_cb;
	pAudioTrack->st_AudioTrack_Itc->func.content_get =
	    (void *) __vp_audio_track_genlist_content_get_cb;
	pAudioTrack->st_AudioTrack_Itc->func.state_get = NULL;
	pAudioTrack->st_AudioTrack_Itc->func.del = NULL;
	int len = g_list_length(pAudioTrack->pItemList);
	int idx = 0;
	pAudioTrack->nListCount = 0;

	for (idx = 0; idx < len; idx++) {
		AudioTrackItem *pItem = NULL;

		pItem =
		    (AudioTrackItem *) g_list_nth_data(pAudioTrack->pItemList,
		                                       idx);
		if (pItem) {
			pItem->pItem =
			    elm_genlist_item_append(pObj,
			                            pAudioTrack->st_AudioTrack_Itc,
			                            (void *) pItem, NULL,
			                            ELM_GENLIST_ITEM_NONE,
			                            __vp_audio_track_genlist_item_selected_cb,
			                            pUserData);
			pAudioTrack->nListCount++;
		}
	}



	return TRUE;
}

static void __vp_audio_track_genlist_realized(void *data,
        Evas_Object *obj,
        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_audio_track_popup_rotate_cb(void *data,
        Evas_Object *obj,
        void *event_info)
{
	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) data;
	if (!pAudioTrack) {
		VideoLogError("pAudioTrack IS null");
		return;
	}
	if (pAudioTrack->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pAudioTrack->pParent, pAudioTrack->pBox,
	                            pAudioTrack->nListCount,
	                            VIDEO_POPUP_DEFAULT);

}

/* external functions */
audio_track_handle vp_audio_track_create(Evas_Object *pParent,
        PopupCloseCbFunc pCloseCb,
        int nDefaultIndex)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	AudioTrackPopup *pAudioTrack = NULL;

	pAudioTrack = calloc(1, sizeof(AudioTrackPopup));

	if (pAudioTrack == NULL) {
		VideoLogError("pAudioTrack alloc fail");
		return NULL;
	}

	pAudioTrack->pParent = pParent;
	pAudioTrack->pCloseCb = pCloseCb;

	pAudioTrack->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_POPUP_AUDIO_TRACK, NULL, 0.0, NULL,
	                    __vp_audio_track_popup_key_event_cb,
	                    __vp_audio_track_popup_mouse_event_cb,
	                    (void *) pAudioTrack);
	if (pAudioTrack->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_audio_track_destroy_handle(pAudioTrack);
		return NULL;
	}

	pAudioTrack->pGenList =
	    _vp_audio_track_create_genlist(pAudioTrack->pPopup);
	if (pAudioTrack->pGenList == NULL) {
		VideoLogError("_vp_audio_track_create_genlist fail");
		_vp_audio_track_destroy_handle(pAudioTrack);
		return NULL;
	}
	evas_object_smart_callback_add(pAudioTrack->pGenList, "realized",
	                               __vp_audio_track_genlist_realized,
	                               NULL);
	evas_object_smart_callback_add(pAudioTrack->pParent,
	                               "rotation,changed",
	                               __vp_audio_track_popup_rotate_cb,
	                               pAudioTrack);

	evas_object_data_set(pAudioTrack->pGenList,
	                     VP_AUDIO_TRACK_GENLIST_DATA_KEY,
	                     (void *) pAudioTrack);
	pAudioTrack->nDefaultIndex = nDefaultIndex;

	pAudioTrack->pRadio = elm_radio_add(pAudioTrack->pGenList);
	pAudioTrack->pBox = elm_box_add(pAudioTrack->pPopup);


	return pAudioTrack;
}

void vp_audio_track_destroy(audio_track_handle pAudioTrackHandle)
{
	if (pAudioTrackHandle == NULL) {
		VideoLogError("pAudioTrackHandle is NULL");
		return;
	}

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pAudioTrackHandle;

	_vp_audio_track_destroy_handle(pAudioTrack);

}

bool vp_audio_track_realize(audio_track_handle pAudioTrackHandle)
{
	if (pAudioTrackHandle == NULL) {
		VideoLogError("pAudioTrackHandle is NULL");
		return FALSE;
	}

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pAudioTrackHandle;

	int len = g_list_length(pAudioTrack->pItemList);
	pAudioTrack->nListCount = len;
	if (len == 1) {
		vp_popup_set_popup_min_size(pAudioTrack->pParent,
		                            pAudioTrack->pBox,
		                            pAudioTrack->nListCount,
		                            VIDEO_POPUP_DEFAULT);
		elm_scroller_policy_set(pAudioTrack->pGenList,
		                        ELM_SCROLLER_POLICY_OFF,
		                        ELM_SCROLLER_POLICY_OFF);
	} else {
		vp_popup_set_popup_min_size(pAudioTrack->pParent,
		                            pAudioTrack->pBox,
		                            pAudioTrack->nListCount,
		                            VIDEO_POPUP_DEFAULT);
	}

	elm_box_pack_end(pAudioTrack->pBox, pAudioTrack->pGenList);
	elm_object_content_set(pAudioTrack->pPopup, pAudioTrack->pBox);

	if (!_vp_audio_track_add_genlist_item
	        (pAudioTrack->pGenList, (void *) pAudioTrack)) {
		VideoLogError("_vp_audio_track_add_genlist_item fail");
		return FALSE;
	}
	//elm_radio_value_set(pAudioTrack->pRadio, pAudioTrack->nDefaultIndex);
	pAudioTrack->nAudoTrackIdx = pAudioTrack->nDefaultIndex;

	evas_object_show(pAudioTrack->pPopup);

	return TRUE;
}

bool vp_audio_track_unrealize(audio_track_handle pAudioTrackHandle)
{
	if (pAudioTrackHandle == NULL) {
		VideoLogError("pAudioTrackHandle is NULL");
		return FALSE;
	}

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pAudioTrackHandle;

	_vp_audio_track_clear_item_list(pAudioTrack);

	evas_object_hide(pAudioTrack->pPopup);

	return TRUE;
}

bool vp_audio_track_add_Item(audio_track_handle pAudioTrackHandle,
                             char *szCode, int nIndex)
{
	if (pAudioTrackHandle == NULL) {
		VideoLogError("pAudioTrackHandle is NULL");
		return FALSE;
	}

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pAudioTrackHandle;

	AudioTrackItem *pItem = calloc(1, sizeof(AudioTrackItem));
	if (pItem == NULL) {
		VideoLogError("AudioTrackItem alloc fail");
		return FALSE;
	}

	pItem->nIndex = nIndex;

	int nCount = sizeof(st_RegionCode) / sizeof(RegionCode);

	int nSelectIdex = -1;
	int nIdx = 0;
	for (nIdx = 0; nIdx < nCount; nIdx++) {
		if (!strcmp(st_RegionCode[nIdx].szCode, szCode)) {
			nSelectIdex = nIdx;
			break;
		}
	}

	if (nSelectIdex == -1) {
		char *szDefaultTrack =
		    g_strdup_printf("%s %d", VP_PLAY_STRING_AUDIO_TRACK, 0);

		if (!strcmp(szDefaultTrack, szCode)) {
			VP_STRDUP(pItem->szName, szCode);
		} else {
			VP_STRDUP(pItem->szName, VP_PLAY_STRING_COM_UNKNOWN);
		}
		VP_FREE(szDefaultTrack);
	} else {
		VP_STRDUP(pItem->szName, st_RegionCode[nSelectIdex].szName);
	}

	pAudioTrack->pItemList = g_list_append(pAudioTrack->pItemList, pItem);

	return TRUE;
}

bool vp_audio_track_set_user_data(audio_track_handle pAudioTrackHandle,
                                  void *pUserData)
{
	if (pAudioTrackHandle == NULL) {
		VideoLogError("pAudioTrackHandle is NULL");
		return FALSE;
	}

	AudioTrackPopup *pAudioTrack = (AudioTrackPopup *) pAudioTrackHandle;

	pAudioTrack->pUserData = pUserData;

	return TRUE;
}

bool vp_audio_track_get_name(char *szCode, char **szName)
{
	if (szCode == NULL) {
		VideoLogError("szCode is NULL");
		return FALSE;
	}
	int nCount = sizeof(st_RegionCode) / sizeof(RegionCode);

	int nIdx = 0;
	for (nIdx = 0; nIdx < nCount; nIdx++) {
		if (!strcmp(st_RegionCode[nIdx].szCode, szCode)) {
			VP_STRDUP(*szName, st_RegionCode[nIdx].szName);
			return TRUE;
		}
	}

	if (*szName == NULL) {
		VP_STRDUP(*szName, VP_PLAY_STRING_COM_UNKNOWN);
	}

	return TRUE;
}
