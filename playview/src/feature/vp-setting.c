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

#include "vp-setting.h"
#include "vp-device-language.h"

#include "vp-play-preference.h"
#include "vp-device.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SETTING_GENLIST_DATA_KEY		"vp.setting.genlist"

typedef struct _SettingPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;

	int nAppendItemCnt;
	Elm_Genlist_Item_Class *st_Setting_Itc;


	SettingInfo *pSettingInfo;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;

} SettingPopup;

static void _vp_setting_destroy_handle(SettingPopup *pSetting);
static void __vp_setting_genlist_realized(void *data, Evas_Object *obj,
		void *event_info);
static void __vp_setting_popup_rotate_cb(void *data, Evas_Object *obj,
		void *event_info);


/* callback functions */


static char *__vp_setting_genlist_text_get_cb(const void *pUserData,
		Evas_Object *pObj,
		const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *)pUserData;

	if (!strcmp(pPart, "elm.text.1")) {
		return strdup(szTxt);
	} else if (!strcmp(pPart, "elm.text.2")) {

		SettingPopup *pSetting =
			(SettingPopup *) evas_object_data_get(pObj,
					VP_SETTING_GENLIST_DATA_KEY);
		if (pSetting == NULL) {
			return NULL;
		}

		SettingInfo *pSettingInfo = pSetting->pSettingInfo;
		if (pSettingInfo == NULL) {
			return NULL;
		}

		if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_PLAY_SPEED)) {
			return g_strdup_printf("%0.1f x", pSettingInfo->fSpeed);
		} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_SOUND_ALIVE)) {
			if (pSettingInfo->nSoundAlive == VIDEO_SA_NONE) {
				return strdup(VP_PLAY_STRING_SOUND_ALIVE_NORMAL);
			} else if (pSettingInfo->nSoundAlive == VIDEO_SA_NORMAL) {
				return strdup(VP_PLAY_STRING_SOUND_ALIVE_NORMAL);
			} else if (pSettingInfo->nSoundAlive == VIDEO_SA_VOICE) {
				return strdup(VP_PLAY_STRING_SOUND_ALIVE_VOICE);
			} else if (pSettingInfo->nSoundAlive == VIDEO_SA_MOVIE) {
				return strdup(VP_PLAY_STRING_SOUND_ALIVE_MOVIE);
			} else if (pSettingInfo->nSoundAlive == VIDEO_SA_7_1_CH) {
				return strdup(VP_PLAY_STRING_SOUND_ALIVE_71CH);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_AUDIO_TRACK)) {
			return strdup(pSettingInfo->szAudioTrack);
		} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_REPEAT_SETTING)) {
			if (pSettingInfo->nRepeatMode == VIDEO_PLAY_REPEAT_OFF) {
				return strdup(VP_PLAY_STRING_REPEAT_OFF);
			} else if (pSettingInfo->nRepeatMode ==
					VIDEO_PLAY_REPEAT_ALL_STOP) {
				return strdup(VP_PLAY_STRING_REPEAT_ALL_OFF);
			} else if (pSettingInfo->nRepeatMode == VIDEO_PLAY_REPEAT_ONE) {
				return strdup(VP_PLAY_STRING_REPEAT_ONE);
			} else if (pSettingInfo->nRepeatMode == VIDEO_PLAY_REPEAT_ALL) {
				return strdup(VP_PLAY_STRING_REPEAT_ALL);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_CAPTURE)) {
			if (pSettingInfo->bCapture) {
				return strdup(VP_PLAY_STRING_COM_ON);
			} else {
				return strdup(VP_PLAY_STRING_COM_OFF);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_TAG_BUDDY)) {
			if (pSettingInfo->bTagBuddy) {
				return strdup(VP_PLAY_STRING_COM_ON);
			} else {
				return strdup(VP_PLAY_STRING_COM_OFF);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_EDIT_WEATHER_TAG)) {
			if (pSettingInfo->bEditWeatherTag) {
				return strdup(VP_PLAY_STRING_COM_ON);
			} else {
				return strdup(VP_PLAY_STRING_COM_OFF);
			}
		}
	}

	return NULL;
}

static void __vp_setting_genlist_item_selected_cb(void *pUserData,
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
	char *szTxt = (char *)elm_object_item_data_get(pItem);

	SettingPopup *pSetting = (SettingPopup *) pUserData;

	Elm_Object_Item *item = (Elm_Object_Item *) pEventInfo;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}
	vp_setting_mode_t nMode = VP_SETTING_MODE_NONE;
	bool bPause = FALSE;

	if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_PLAY_SPEED)) {
		nMode = VP_SETTING_MODE_PLAY_SPEED;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_SOUND_ALIVE)) {
		nMode = VP_SETTING_MODE_SOUND_ALIVE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_AUDIO_TRACK)) {
		nMode = VP_SETTING_MODE_AUDIO_TRACK;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_REPEAT_SETTING)) {
		nMode = VP_SETTING_MODE_REPEAT_MODE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_POPUP_CAPTURE)) {
		nMode = VP_SETTING_MODE_CAPTURE;
	}

	if (pSetting->pCloseCb) {
		pSetting->pCloseCb((int)nMode, bPause, (void *)pSetting->pUserData);
	}
}

static void __vp_setting_popup_key_event_cb(void *pUserData,
		Evas_Object *pObj,
		void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	SettingPopup *pSetting = (SettingPopup *) pUserData;

	if (pSetting->pCloseCb) {
		pSetting->pCloseCb(VP_SETTING_MODE_NONE, FALSE,
				   (void *) pSetting->pUserData);
	}
}

static void __vp_setting_popup_mouse_event_cb(void *pUserData,
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
		SettingPopup *pSetting = (SettingPopup *) pUserData;

		if (pSetting->pCloseCb) {
			pSetting->pCloseCb(VP_SETTING_MODE_NONE, FALSE,
					   (void *) pSetting->pUserData);
		}
	}
}



/* internal functions */
static void _vp_setting_destroy_handle(SettingPopup *pSetting)
{
	if (pSetting == NULL) {
		VideoLogError("pSetting is NULL");
		return;
	}
	evas_object_smart_callback_del(pSetting->pGenList, "realized",
				       __vp_setting_genlist_realized);
	evas_object_smart_callback_del(pSetting->pParent, "rotation,changed",
				       __vp_setting_popup_rotate_cb);


	VP_EVAS_DEL(pSetting->pGenList);
	VP_EVAS_DEL(pSetting->pButton);
	VP_EVAS_DEL(pSetting->pBox);

	VP_EVAS_DEL(pSetting->pPopup);

	if (pSetting->st_Setting_Itc) {
		elm_genlist_item_class_free(pSetting->st_Setting_Itc);
		pSetting->st_Setting_Itc = NULL;
	}

	VP_FREE(pSetting->pSettingInfo->szAudioTrack);
	VP_FREE(pSetting->pSettingInfo);

	VP_FREE(pSetting);
}

static Evas_Object *_vp_setting_create_genlist(Evas_Object *pParent)
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


static bool _vp_setting_add_genlist_item(Evas_Object *pObj,
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

	SettingPopup *pSetting = (SettingPopup *) pUserData;
	bool bStreaming = FALSE;
//      bool bRepeat = FALSE;
	bool bDrm = FALSE;
	bool bExternalVideoMode = FALSE;
	bool bShowEditWeather = FALSE;

	if (pSetting->pSettingInfo) {
		bStreaming = pSetting->pSettingInfo->bStreaming;
//              bRepeat = pSetting->pSettingInfo->bRepeat;
		bDrm = pSetting->pSettingInfo->bDrm;
		bExternalVideoMode = pSetting->pSettingInfo->bExternalMode;
		bShowEditWeather = pSetting->pSettingInfo->bShowEditWeather;
	}

	if (pSetting->st_Setting_Itc) {
		elm_genlist_item_class_free(pSetting->st_Setting_Itc);
		pSetting->st_Setting_Itc = NULL;
	}

	pSetting->st_Setting_Itc = elm_genlist_item_class_new();

	if (pSetting->st_Setting_Itc != NULL) {
		pSetting->st_Setting_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSetting->st_Setting_Itc->item_style = "2text.2/popup";
		pSetting->st_Setting_Itc->func.text_get =
			(void *) __vp_setting_genlist_text_get_cb;
		pSetting->st_Setting_Itc->func.content_get = NULL;
		pSetting->st_Setting_Itc->func.state_get = NULL;
		pSetting->st_Setting_Itc->func.del = NULL;
	}

	if (bStreaming == FALSE) {
		elm_genlist_item_append(pObj, pSetting->st_Setting_Itc,
					(void *) VP_PLAY_STRING_POPUP_PLAY_SPEED,
					NULL, ELM_GENLIST_ITEM_NONE,
					__vp_setting_genlist_item_selected_cb,
					(void *) pSetting);
		pSetting->nAppendItemCnt++;
	}

	elm_genlist_item_append(pObj, pSetting->st_Setting_Itc,
				(void *) VP_PLAY_STRING_POPUP_SOUND_ALIVE,
				NULL, ELM_GENLIST_ITEM_NONE,
				__vp_setting_genlist_item_selected_cb,
				(void *) pSetting);
	pSetting->nAppendItemCnt++;


	if (bStreaming == FALSE) {
		elm_genlist_item_append(pObj, pSetting->st_Setting_Itc,
					(void *) VP_PLAY_STRING_POPUP_AUDIO_TRACK,
					NULL, ELM_GENLIST_ITEM_NONE,
					__vp_setting_genlist_item_selected_cb,
					(void *) pSetting);
		pSetting->nAppendItemCnt++;
		elm_genlist_item_append(pObj, pSetting->st_Setting_Itc,
					(void *)
					VP_PLAY_STRING_POPUP_REPEAT_SETTING, NULL,
					ELM_GENLIST_ITEM_NONE,
					__vp_setting_genlist_item_selected_cb,
					(void *) pSetting);
		pSetting->nAppendItemCnt++;

		if (bDrm == FALSE && bExternalVideoMode == FALSE) {
			elm_genlist_item_append(pObj, pSetting->st_Setting_Itc,
						(void *) VP_PLAY_STRING_POPUP_CAPTURE,
						NULL, ELM_GENLIST_ITEM_NONE,
						__vp_setting_genlist_item_selected_cb,
						(void *) pSetting);
			pSetting->nAppendItemCnt++;
		}
	}

	if (bStreaming == FALSE && bDrm == FALSE) {
		elm_genlist_item_append(pObj, pSetting->st_Setting_Itc,
					(void *) VP_PLAY_STRING_POPUP_TAG_BUDDY,
					NULL, ELM_GENLIST_ITEM_NONE,
					__vp_setting_genlist_item_selected_cb,
					(void *) pSetting);
		pSetting->nAppendItemCnt++;
		if (bShowEditWeather == TRUE) {
			elm_genlist_item_append(pObj, pSetting->st_Setting_Itc,
						(void *)
						VP_PLAY_STRING_POPUP_EDIT_WEATHER_TAG,
						NULL, ELM_GENLIST_ITEM_NONE,
						__vp_setting_genlist_item_selected_cb,
						(void *) pSetting);
			pSetting->nAppendItemCnt++;
		}
	}

	return TRUE;
}

static void __vp_setting_genlist_realized(void *data, Evas_Object *obj,
		void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_setting_popup_rotate_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	SettingPopup *pSetting = (SettingPopup *) data;
	if (!pSetting) {
		VideoLogError("pSetting IS null");
		return;
	}
	if (pSetting->nAppendItemCnt < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSetting->pParent, pSetting->pBox,
				    pSetting->nAppendItemCnt,
				    VIDEO_POPUP_2_TEXT);

}

/* external functions */
setting_popup_handle vp_setting_create(Evas_Object *pParent,
				       SettingInfo *pSettingInfo,
				       PopupCloseCbFunc pCloseCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (pSettingInfo == NULL) {
		VideoLogError("pSettingInfo is NULL");
		return NULL;
	}

	SettingPopup *pSetting = NULL;

	pSetting = calloc(1, sizeof(SettingPopup));

	if (pSetting == NULL) {
		VideoLogError("pSetting alloc fail");
		return NULL;
	}

	pSetting->pSettingInfo = calloc(1, sizeof(SettingInfo));
	if (pSetting->pSettingInfo == NULL) {
		VideoLogError("pSettingInfo alloc fail");
		_vp_setting_destroy_handle(pSetting);
		return NULL;
	}

	pSetting->pSettingInfo->fSpeed = pSettingInfo->fSpeed;
	pSetting->pSettingInfo->nSoundAlive = pSettingInfo->nSoundAlive;
	VP_STRDUP(pSetting->pSettingInfo->szAudioTrack,
		  pSettingInfo->szAudioTrack);
	pSetting->pSettingInfo->nRepeatMode = pSettingInfo->nRepeatMode;
	pSetting->pSettingInfo->bCapture = pSettingInfo->bCapture;
	pSetting->pSettingInfo->bTagBuddy = pSettingInfo->bTagBuddy;
	pSetting->pSettingInfo->bEditWeatherTag =
		pSettingInfo->bEditWeatherTag;
	pSetting->pSettingInfo->bStreaming = pSettingInfo->bStreaming;
	pSetting->pSettingInfo->bRepeat = pSettingInfo->bRepeat;
	pSetting->pSettingInfo->bSelectSubtitle =
		pSettingInfo->bSelectSubtitle;
	pSetting->pSettingInfo->bDrm = pSettingInfo->bDrm;
	pSetting->pSettingInfo->bExternalMode = pSettingInfo->bExternalMode;
	pSetting->pSettingInfo->bShowEditWeather =
		pSettingInfo->bShowEditWeather;

	pSetting->pParent = pParent;
	pSetting->pCloseCb = pCloseCb;

	pSetting->pPopup =
		vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
				VP_PLAY_STRING_SETTINGS, NULL, 0.0, NULL,
				__vp_setting_popup_key_event_cb,
				__vp_setting_popup_mouse_event_cb,
				(void *) pSetting);
	if (pSetting->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_setting_destroy_handle(pSetting);
		return NULL;
	}

	pSetting->pGenList = _vp_setting_create_genlist(pSetting->pPopup);
	if (pSetting->pGenList == NULL) {
		VideoLogError("_vp_setting_create_button fail");
		_vp_setting_destroy_handle(pSetting);
		return NULL;
	}
	evas_object_data_set(pSetting->pGenList, VP_SETTING_GENLIST_DATA_KEY,
			     (void *) pSetting);
	vp_language_list_create();

	if (!_vp_setting_add_genlist_item
			(pSetting->pGenList, (void *) pSetting)) {
		VideoLogError("_vp_setting_add_genlist_item fail");
		_vp_setting_destroy_handle(pSetting);
		return NULL;
	}
	evas_object_smart_callback_add(pSetting->pGenList, "realized",
				       __vp_setting_genlist_realized, NULL);
	evas_object_smart_callback_add(pSetting->pParent, "rotation,changed",
				       __vp_setting_popup_rotate_cb,
				       pSetting);

	pSetting->pBox = elm_box_add(pSetting->pPopup);
	vp_popup_set_popup_min_size(pSetting->pParent, pSetting->pBox,
				    pSetting->nAppendItemCnt,
				    VIDEO_POPUP_2_TEXT);

	elm_box_pack_end(pSetting->pBox, pSetting->pGenList);

	elm_object_content_set(pSetting->pPopup, pSetting->pBox);

	return pSetting;
}

void vp_setting_destroy(setting_popup_handle pSettingHandle)
{
	if (pSettingHandle == NULL) {
		VideoLogError("pSettingHandle is NULL");
		return;
	}

	SettingPopup *pSetting = (SettingPopup *) pSettingHandle;

	_vp_setting_destroy_handle(pSetting);

}

bool vp_setting_realize(setting_popup_handle pSettingHandle)
{
	if (pSettingHandle == NULL) {
		VideoLogError("pSettingHandle is NULL");
		return FALSE;
	}

	SettingPopup *pSetting = (SettingPopup *) pSettingHandle;

	evas_object_show(pSetting->pPopup);

	return TRUE;
}

bool vp_setting_unrealize(setting_popup_handle pSettingHandle)
{
	if (pSettingHandle == NULL) {
		VideoLogError("pSettingHandle is NULL");
		return FALSE;
	}

	SettingPopup *pSetting = (SettingPopup *) pSettingHandle;


	evas_object_hide(pSetting->pPopup);

	return TRUE;
}

bool vp_setting_set_user_data(setting_popup_handle pSettingHandle,
			      void *pUserData)
{
	if (pSettingHandle == NULL) {
		VideoLogError("pSettingHandle is NULL");
		return FALSE;
	}

	SettingPopup *pSetting = (SettingPopup *) pSettingHandle;

	pSetting->pUserData = pUserData;

	return TRUE;
}
